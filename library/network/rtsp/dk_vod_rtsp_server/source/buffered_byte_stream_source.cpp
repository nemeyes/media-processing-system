#include "buffered_byte_stream_source.h"
#include "GroupsockHelper.hh"

buffered_byte_stream_source * buffered_byte_stream_source::createNew(UsageEnvironment & env, char const * stream_name, std::shared_ptr<debuggerking::media_source_reader> reader, unsigned preferredFrameSize, unsigned playTimePerFrame)
{
	buffered_byte_stream_source * new_source = new buffered_byte_stream_source(env, stream_name, reader, preferredFrameSize, playTimePerFrame);
	return new_source;
}

void buffered_byte_stream_source::seekToByteAbsolute(u_int64_t byteNumber, u_int64_t numBytesToStream)
{
	fNumBytesToStream = numBytesToStream;
	fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void buffered_byte_stream_source::seekToByteRelative(int64_t offset, u_int64_t numBytesToStream)
{
	fNumBytesToStream = numBytesToStream;
	fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void buffered_byte_stream_source::seekToEnd()
{
}

buffered_byte_stream_source::buffered_byte_stream_source(UsageEnvironment & env, char const * stream_name, std::shared_ptr<debuggerking::media_source_reader> reader, unsigned preferredFrameSize, unsigned playTimePerFrame)
	: buffered_framed_source(env, stream_name, reader)
	, fPreferredFrameSize(preferredFrameSize)
	, fPlayTimePerFrame(playTimePerFrame)
	, fLastPlayTime(0)
	, fHaveStartedReading(False)
	, fLimitNumBytesToStream(False)
	, fNumBytesToStream(0)
{
}

buffered_byte_stream_source::~buffered_byte_stream_source()
{
}

void buffered_byte_stream_source::doGetNextFrame()
{
	if (fLimitNumBytesToStream && fNumBytesToStream == 0)
	{
		handleClosure();
		return;
	}
	read_from_buffer();
}

void buffered_byte_stream_source::doStopGettingFrames()
{
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void buffered_byte_stream_source::read_from_buffer(void)
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
	if (_reader)
	{
		fFrameSize = 0;
		size_t data_size = 0;

		long long interval = 0;
		long long timestamp = 0;
		memset(fRealFrame, 0x00, sizeof(fRealFrame));
		_reader->read(debuggerking::media_source_reader::media_type_t::video, fRealFrame, sizeof(fRealFrame), data_size, interval, timestamp);

		if (data_size > 0)
		{
			fFrameSize = data_size;
			if (fFrameSize > fMaxSize)
			{
				fNumTruncatedBytes = fFrameSize - fMaxSize;
				fFrameSize = fMaxSize;
			}
			memcpy(fTo, fRealFrame, fFrameSize);
			gettimeofday(&fPresentationTime, NULL);

#if 1
			if (interval == 0)
			{
				FramedSource::afterGetting(this);//nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);// 
			}
			else
			{
				int64_t timestamp_microsec = interval * 1000;
				if (_reader->get_scale() != 1.f)
					timestamp_microsec /= _reader->get_scale();
				nextTask() = envir().taskScheduler().scheduleDelayedTask(timestamp_microsec, (TaskFunc*)FramedSource::afterGetting, this);
			}
#else
			FramedSource::afterGetting(this);
#endif
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
