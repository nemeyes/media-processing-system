#include "byte_stream_shared_memory_source.h"
#include "GroupsockHelper.hh"
#include "buffers/session_queue_manager.h"
#include "log/logger.h"

byte_stream_shared_memory_source* byte_stream_shared_memory_source::createNew(UsageEnvironment& env, char const* stream_name,
	unsigned preferredFrameSize, unsigned playTimePerFrame)
{
	std::string session_id = session_queue_manager::instance().create_stream_session(stream_name);
	if (session_id.length()<1)
	{
		char log[MAX_PATH] = { 0 };
		_snprintf(log, sizeof(log), "shared memory related to token name '%s' is not available", stream_name);
		logger::instance().make_system_fatal_log(log);
		return NULL;
	}
	else
	{
		byte_stream_shared_memory_source* new_source = new byte_stream_shared_memory_source(env, stream_name, preferredFrameSize, playTimePerFrame);
		strcpy(new_source->_session_id, session_id.c_str());
		return new_source;
	}
}

void byte_stream_shared_memory_source::seekToByteAbsolute(u_int64_t byteNumber, u_int64_t numBytesToStream)
{
	fNumBytesToStream = numBytesToStream;
	fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void byte_stream_shared_memory_source::seekToByteRelative(int64_t offset, u_int64_t numBytesToStream)
{
	fNumBytesToStream = numBytesToStream;
	fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void byte_stream_shared_memory_source::seekToEnd()
{

}

byte_stream_shared_memory_source::byte_stream_shared_memory_source(UsageEnvironment& env, char const* stream_name,
	unsigned preferredFrameSize, unsigned playTimePerFrame)
	: framed_shared_memory_source(env, stream_name)
	, fPreferredFrameSize(preferredFrameSize)
	, fPlayTimePerFrame(playTimePerFrame)
	, fLastPlayTime(0)
	, fHaveStartedReading(False)
	, fLimitNumBytesToStream(False)
	, fNumBytesToStream(0)
	, _recv_sps(True)
	, _recv_pps(True)
	, _recv_idr(True)
	, _sps_size(0)
	, _pps_size(0)
	, _recv_time(0)
{
	//memset( _real_frame, 0x00, FRAMESIZE );
	// Test whether the file is seekable
	//fFidIsSeekable = FileIsSeekable(fFid);

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

byte_stream_shared_memory_source::~byte_stream_shared_memory_source()
{
	session_queue_manager::instance().delete_stream_session(_stream_name, _session_id);
}

void byte_stream_shared_memory_source::doGetNextFrame()
{
	if (fLimitNumBytesToStream && fNumBytesToStream == 0)
	{
		handleClosure();
		return;
	}
	read_from_shared_memory();
}

void byte_stream_shared_memory_source::doStopGettingFrames()
{
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void byte_stream_shared_memory_source::read_from_shared_memory(void)
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
				char log[MAX_PATH] = { 0 };
				_snprintf(log, sizeof(log), "%d bytes is truncated", fFrameSize - fMaxSize);
				logger::instance().make_system_info_log(log);
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
