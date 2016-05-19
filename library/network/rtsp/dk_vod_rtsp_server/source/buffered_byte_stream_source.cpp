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


		//0x00 0x00 0x00 0x01 0x06 0x05 0x08 0xbc 0x97 0xb8 0x4d 0x96 0x9f 0x48 0xb9 0xbc 0xe4 0x7c 0x1c 0x1a 0x39 0x2f 0x37  00 00 00 00 00 00 00 00
		//char sei[31] = { 0x00, 0x00, 0x00, 0x01, 0x06, 0x05, 0x08, 0xbc, 0x97, 0xb8, 0x4d, 0x96, 0x9f, 0x48, 0xb9, 0xbc, 0xe4, 0x7c, 0x1c, 0x1a, 0x39, 0x2f, 0x37, 00, 00, 00, 00, 00, 00, 00, 00 };
		if (data_size > 0)
		{
			fFrameSize = data_size;// +sizeof(sei);
			if (fFrameSize > fMaxSize)
			{
				fNumTruncatedBytes = fFrameSize - fMaxSize;
				fFrameSize = fMaxSize;
			}
			memcpy(fTo, fRealFrame, fFrameSize);
			//memcpy(fTo + data_size, sei, sizeof(sei));
			gettimeofday(&fPresentationTime, NULL);

#if 1
			if (interval == 0)
			{
				FramedSource::afterGetting(this);//nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);// 
				//nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);
			}
			else
			{
				int64_t timestamp_microsec = interval * 1000;
				if (_reader->get_scale() != 1.f)
					timestamp_microsec = (int64_t)((float)timestamp_microsec/_reader->get_scale());
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
