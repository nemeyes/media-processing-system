#include "buffered_h264_sms.h"
#include "buffered_byte_stream_source.h"
#include "H264VideoRTPSink.hh"
#if defined(USE_H264_VIDEO_STREAM_FRAMER)
#include "H264VideoStreamFramer.hh"
#else
#include "H264VideoStreamDiscreteFramer.hh"
#endif

buffered_h264_sms* buffered_h264_sms::createNew(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource, std::shared_ptr<media_source_reader> reader)
{
	return new buffered_h264_sms(env, stream_name, reuseFirstSource, reader);
}

buffered_h264_sms::buffered_h264_sms(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource, std::shared_ptr<media_source_reader> reader)
	: buffered_sms(env, stream_name, reuseFirstSource, reader)
	, fAuxSDPLine(NULL)
	, fDoneFlag(0)
	, fDummyRTPSink(NULL)
{

}

buffered_h264_sms::~buffered_h264_sms(void)
{
	delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void * client_data)
{
	buffered_h264_sms * sms = static_cast<buffered_h264_sms*>(client_data);
	sms->afterPlayingDummy1();
}

void buffered_h264_sms::afterPlayingDummy1(void)
{
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
}

static void checkForAuxSDPLine(void * client_data)
{
	buffered_h264_sms * sms = static_cast<buffered_h264_sms*>(client_data);
	sms->checkForAuxSDPLine1();
}

void buffered_h264_sms::checkForAuxSDPLine1(void)
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

char const* buffered_h264_sms::getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource)
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

FramedSource* buffered_h264_sms::createNewStreamSource(unsigned /*clientSessionId*/, unsigned & estBitrate)
{
	estBitrate = 8*1024;//500; // kbps, estimate

	// Create the video source:
	buffered_byte_stream_source * buffered_source = buffered_byte_stream_source::createNew(envir(), _stream_name, _reader);
	if (buffered_source == NULL)
		return NULL;

	return H264VideoStreamDiscreteFramer::createNew(envir(), buffered_source);
}

RTPSink* buffered_h264_sms::createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * inputSource)
{
	size_t sps_size = 0;
	size_t pps_size = 0;
	const uint8_t * sps = _reader->get_sps(sps_size);
	const uint8_t * pps = _reader->get_pps(pps_size);

	if (sps && pps && (sps_size > 0) && (pps_size > 0))
	{
		//unsigned int profile_level_id = 0;
		//profile_level_id = _sps[1] << 16 | _sps[2] << 8 | _sps[3];
		return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, sps, sps_size, pps, pps_size/*, profile_level_id*/);
	}
	else
	{
		return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	}
}
