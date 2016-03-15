#ifndef _BUFFERED_BYTE_STREAM_SOURCE_H_
#define _BUFFERED_BYTE_STREAM_SOURCE_H_

#ifndef _BUFFERED_FRAMED_SOURCE_H_
#include "buffered_framed_source.h"
#endif

#include "media_file_reader.h"

#define SESSION_VIDEO_FRAME_SIZE 1000000

class buffered_byte_stream_source : public buffered_framed_source
{
public:
	static buffered_byte_stream_source * createNew(UsageEnvironment & env, char const * stream_name, std::shared_ptr<media_file_reader> reader, unsigned preferredFrameSize = 0, unsigned playTimePerFrame = 0);
	// "preferredFrameSize" == 0 means 'no preference'
	// "playTimePerFrame" is in microseconds

	//static byte_stream_shared_memory_source* createNew( UsageEnvironment& env, HANDLE sm, unsigned preferredFrameSize = 0, unsigned playTimePerFrame = 0 );
	// an alternative version of "createNew()" that's used if you already have
	// an open file.

	//u_int64_t		fileSize() const { return fFileSize; }
	// 0 means zero-length, unbounded, or unknown

	void seekToByteAbsolute(u_int64_t byteNumber, u_int64_t numBytesToStream = 0);
	// if "numBytesToStream" is >0, then we limit the stream to that number of bytes, before treating it as EOF
	void seekToByteRelative(int64_t offset, u_int64_t numBytesToStream = 0);
	void seekToEnd(); // to force EOF handling on the next read

	//unsigned char *	get_sps() { return _sps; }
	//unsigned char *	get_pps() { return _pps; }
	//int get_sps_size() { return _sps_size; }
	//int get_pps_size() { return _pps_size; }

protected:
	buffered_byte_stream_source(UsageEnvironment & env, char const * stream_name, std::shared_ptr<media_file_reader> reader, unsigned preferredFrameSize, unsigned playTimePerFrame);
	virtual ~buffered_byte_stream_source(void);

	//void			doReadFromFile();
	void read_from_buffer(void);



private:
	// redefined virtual functions:
	virtual void doGetNextFrame();
	virtual void doStopGettingFrames();

protected:
	//u_int64_t		fFileSize;

private:
	unsigned fPreferredFrameSize;
	unsigned fPlayTimePerFrame;
	//Boolean			fFidIsSeekable;
	unsigned fLastPlayTime;
	Boolean fHaveStartedReading;
	Boolean fLimitNumBytesToStream;
	u_int64_t fNumBytesToStream; //used if "fLimitNumBytesToStream" is True

	unsigned char fRealFrame[SESSION_VIDEO_FRAME_SIZE];
};

#endif
