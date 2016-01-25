#include "h264_video_shared_memory_sms.h"
#include "byte_stream_shared_memory_source.h"
#include "H264VideoRTPSink.hh"
#if defined(USE_H264_VIDEO_STREAM_FRAMER)
#include "H264VideoStreamFramer.hh"
#else
#include "H264VideoStreamDiscreteFramer.hh"
#endif

h264_video_shared_memory_sms* h264_video_shared_memory_sms::createNew(UsageEnvironment& env, char const* stream_name, Boolean reuseFirstSource)
{
	return new h264_video_shared_memory_sms(env, stream_name, reuseFirstSource);
}

h264_video_shared_memory_sms::h264_video_shared_memory_sms(UsageEnvironment& env, char const* stream_name, Boolean reuseFirstSource)
	: shared_memory_sms(env, stream_name, reuseFirstSource)
	, fAuxSDPLine(NULL)
	, fDoneFlag(0)
	, fDummyRTPSink(NULL)
	, _sps_size(0)
	, _pps_size(0)
{

}

h264_video_shared_memory_sms::~h264_video_shared_memory_sms(void)
{
	delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void* clientData)
{
	h264_video_shared_memory_sms* sms = (h264_video_shared_memory_sms*)clientData;
	sms->afterPlayingDummy1();
}

void h264_video_shared_memory_sms::afterPlayingDummy1(void)
{
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData)
{
	h264_video_shared_memory_sms* sms = (h264_video_shared_memory_sms*)clientData;
	sms->checkForAuxSDPLine1();
}

void h264_video_shared_memory_sms::checkForAuxSDPLine1(void)
{
	char const* dasl;

	if (fAuxSDPLine != NULL)
	{
		// Signal the event loop that we're done:
		setDoneFlag();
	}
	else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL)
	{
		fAuxSDPLine = strDup(dasl);
		fDummyRTPSink = NULL;
		// Signal the event loop that we're done:
		setDoneFlag();
	}
	else if (!fDoneFlag)
	{
		// try again after a brief delay:
		int uSecsToDelay = 100000; // 100 ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
	}
}

char const* h264_video_shared_memory_sms::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
	if (fAuxSDPLine != NULL)
		return fAuxSDPLine; // it's already been set up (for a previous client)

	if (fDummyRTPSink == NULL)
	{
		// we're not already setting it up for another, concurrent stream
		// Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
		// until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
		// and we need to start reading data from our file until this changes.
		fDummyRTPSink = rtpSink;

		// Start reading the file:
		fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

		// Check whether the sink's 'auxSDPLine()' is ready:
		checkForAuxSDPLine(this);
	}

	envir().taskScheduler().doEventLoop(&fDoneFlag);
	return fAuxSDPLine;
}

FramedSource* h264_video_shared_memory_sms::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
	//estBitrate = 6000;//500; // kbps, estimate
#if defined(USE_HIGH_BITRATE)
	//estBitrate = (6000000 + 500)/1000;
	//estBitrate	= 60000;
	estBitrate = (4000000 + 500) / 1000;
#else
	estBitrate = (4000000 + 500) / 1000;
#endif

	// Create the video source:
	byte_stream_shared_memory_source* shared_memory_source = byte_stream_shared_memory_source::createNew(envir(), _stream_name);
	if (shared_memory_source == NULL)
		return NULL;

	if (shared_memory_source->get_sps_size()>0)
	{
		_sps_size = shared_memory_source->get_sps_size();
		memcpy(_sps, shared_memory_source->get_sps(), _sps_size);
	}

	if (shared_memory_source->get_pps_size()>0)
	{
		_pps_size = shared_memory_source->get_pps_size();
		memcpy(_pps, shared_memory_source->get_pps(), _pps_size);
	}

	return H264VideoStreamDiscreteFramer::createNew(envir(), shared_memory_source);
}

RTPSink* h264_video_shared_memory_sms::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	if (_sps_size>0 && _pps_size>0)
	{
		unsigned int profile_level_id = 0;
		profile_level_id = _sps[1] << 16 | _sps[2] << 8 | _sps[3];
		return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, _sps, _sps_size, _pps, _pps_size, profile_level_id);
	}
	else
		return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
