#include "buffered_byte_stream_source.h"
#include "GroupsockHelper.hh"

buffered_byte_stream_source * buffered_byte_stream_source::createNew(UsageEnvironment & env, char const * stream_name, std::shared_ptr<media_file_reader> reader, unsigned preferredFrameSize, unsigned playTimePerFrame)
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

buffered_byte_stream_source::buffered_byte_stream_source(UsageEnvironment & env, char const * stream_name, std::shared_ptr<media_file_reader> reader, unsigned preferredFrameSize, unsigned playTimePerFrame)
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
		long long timestamp = 0;
		_reader->read(media_file_reader::media_type_video, fRealFrame, sizeof(fRealFrame), data_size, timestamp);

		if (data_size > 0)
		{
			fFrameSize = data_size;
			if (fFrameSize>fMaxSize)
			{
				fNumTruncatedBytes = fFrameSize - fMaxSize;
				fFrameSize = fMaxSize;
			}
			memcpy(fTo, fRealFrame, fFrameSize);
			gettimeofday(&fPresentationTime, NULL);
			FramedSource::afterGetting(this);
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
