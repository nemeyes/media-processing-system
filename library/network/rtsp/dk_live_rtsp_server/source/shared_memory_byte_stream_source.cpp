#include "shared_memory_byte_stream_source.h"
#include "GroupsockHelper.hh"
//#include "buffers/session_queue_manager.h"
//#include "log/logger.h"

shared_memory_byte_stream_source * shared_memory_byte_stream_source::createNew(UsageEnvironment & env, char const * stream_name, unsigned preferredFrameSize, unsigned playTimePerFrame)
{
	shared_memory_byte_stream_source * new_source = new shared_memory_byte_stream_source(env, stream_name, preferredFrameSize, playTimePerFrame);
	return new_source;
}

void shared_memory_byte_stream_source::seekToByteAbsolute(u_int64_t byteNumber, u_int64_t numBytesToStream)
{
	fNumBytesToStream = numBytesToStream;
	fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void shared_memory_byte_stream_source::seekToByteRelative(int64_t offset, u_int64_t numBytesToStream)
{
	fNumBytesToStream = numBytesToStream;
	fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void shared_memory_byte_stream_source::seekToEnd()
{

}

shared_memory_byte_stream_source::shared_memory_byte_stream_source(UsageEnvironment & env, char const * stream_name, unsigned preferredFrameSize, unsigned playTimePerFrame)
	: shared_memory_framed_source(env, stream_name)
	, fPreferredFrameSize(preferredFrameSize)
	, fPlayTimePerFrame(playTimePerFrame)
	, fLastPlayTime(0)
	, fHaveStartedReading(False)
	, fLimitNumBytesToStream(False)
	, fNumBytesToStream(0)
	, _sm_video_receiver(nullptr)
{
	_sm_video_receiver = new shared_memory_video_receiver();
	_sm_video_receiver->connect(stream_name);


	while (session_queue_manager::instance().get_sps_size(stream_name)<1 || session_queue_manager::instance().get_pps_size(stream_name)<1)
		Sleep(10);

	if (session_queue_manager::instance().get_sps_size(stream_name)>0)
	{
		_sps_size = session_queue_manager::instance().get_sps_size(stream_name);
		memcpy(_sps, session_queue_manager::instance().get_sps(stream_name), _sps_size);
	}
	if (session_queue_manager::instance().get_pps_size(stream_name)>0)
	{
		_pps_size = session_queue_manager::instance().get_pps_size(stream_name);
		memcpy(_pps, session_queue_manager::instance().get_pps(stream_name), _pps_size);
	}
}

shared_memory_byte_stream_source::~shared_memory_byte_stream_source()
{
	if (_sm_video_receiver)
	{
		_sm_video_receiver->disconnect();
		delete _sm_video_receiver;
		_sm_video_receiver = nullptr;
	}
}

void shared_memory_byte_stream_source::doGetNextFrame()
{
	if (fLimitNumBytesToStream && fNumBytesToStream == 0)
	{
		handleClosure();
		return;
	}
	read_from_shared_memory();
}

void shared_memory_byte_stream_source::doStopGettingFrames()
{
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void shared_memory_byte_stream_source::read_from_shared_memory(void)
{
	// Try to read as many bytes as will fit in the buffer provided (or "fPreferredFrameSize" if less)
	/*
	if( fLimitNumBytesToStream && fNumBytesToStream < (u_int64_t)fMaxSize )
	{
	fMaxSize = (unsigned)fNumBytesToStream;
	}
	if( fPreferredFrameSize>0 && fPreferredFrameSize<fMaxSize )
	{
	fMaxSize = fPreferredFrameSize;
	}
	*/
	fFrameSize = 0;
	int size = 0;
	if (!session_queue_manager::instance().pop(_stream_name, _session_id, fRealFrame, &size))
	{
		char log[MAX_PATH] = { 0 };
		_snprintf(log, sizeof(log), "the source stops being readable");
		logger::instance().make_system_error_log(log);
		handleClosure(this);
	}
	else
	{

		if (size>0)
		{
			fFrameSize = size;
			if (fFrameSize>fMaxSize)
			{
				fNumTruncatedBytes = fFrameSize - fMaxSize;
				fFrameSize = fMaxSize;
				//char log[MAX_PATH] = { 0 };
				//_snprintf(log, sizeof(log), "%d bytes is truncated", fFrameSize - fMaxSize);
				//logger::instance().make_system_info_log(log);
			}
			memcpy(fTo, fRealFrame, fFrameSize);
			gettimeofday(&fPresentationTime, NULL);
			FramedSource::afterGetting(this);
			//nextTask() = envir().taskScheduler().scheduleDelayedTask( 10000,(TaskFunc*)FramedSource::afterGetting, this );
		}
		else
		{
			fFrameSize = 0;
			fTo = 0;
			gettimeofday(&fPresentationTime, NULL);
			nextTask() = envir().taskScheduler().scheduleDelayedTask(10000, (TaskFunc*)FramedSource::afterGetting, this);
		}
	}
}
