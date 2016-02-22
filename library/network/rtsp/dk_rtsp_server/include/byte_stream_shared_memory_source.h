#ifndef _BYTE_STREAM_SHARED_MEMORY_SOURCE_H_
#define _BYTE_STREAM_SHARED_MEMORY_SOURCE_H_

#ifndef _FRAMED_SHARED_MEMORY_SOURCE_H_
#include "framed_shared_memory_source.h"
#endif

#if defined(USE_HIGH_BITRATE)
#define SESSION_VIDEO_FRAME_SIZE		1000000
#define SESSION_VIDEO_FRAME_QUEUE_SIZE	240  
#else
#define SESSION_VIDEO_FRAME_SIZE		500000
#define SESSION_VIDEO_FRAME_QUEUE_SIZE	60  
#endif

class byte_stream_shared_memory_source : public framed_shared_memory_source
{
public:
	static byte_stream_shared_memory_source* createNew(UsageEnvironment& env, char const* stream_name, unsigned preferredFrameSize = 0, unsigned playTimePerFrame = 0);
	// "preferredFrameSize" == 0 means 'no preference'
	// "playTimePerFrame" is in microseconds

	//static byte_stream_shared_memory_source* createNew( UsageEnvironment& env, HANDLE sm, unsigned preferredFrameSize = 0, unsigned playTimePerFrame = 0 );
	// an alternative version of "createNew()" that's used if you already have
	// an open file.

	//u_int64_t		fileSize() const { return fFileSize; }
	// 0 means zero-length, unbounded, or unknown

	void				seekToByteAbsolute(u_int64_t byteNumber, u_int64_t numBytesToStream = 0);
	// if "numBytesToStream" is >0, then we limit the stream to that number of bytes, before treating it as EOF
	void				seekToByteRelative(int64_t offset, u_int64_t numBytesToStream = 0);
	void				seekToEnd(); // to force EOF handling on the next read

	unsigned char*	get_sps() { return _sps; }
	unsigned char*	get_pps() { return _pps; }
	int				get_sps_size() { return _sps_size; }
	int				get_pps_size() { return _pps_size; }



protected:
	byte_stream_shared_memory_source(UsageEnvironment& env, char const* stream_name, unsigned preferredFrameSize, unsigned playTimePerFrame);
	virtual ~byte_stream_shared_memory_source(void);

	//void			doReadFromFile();
	void				read_from_shared_memory(void);



private:
	// redefined virtual functions:
	virtual void		doGetNextFrame();
	virtual void		doStopGettingFrames();

protected:
	//u_int64_t		fFileSize;

private:
	unsigned		fPreferredFrameSize;
	unsigned		fPlayTimePerFrame;
	//Boolean			fFidIsSeekable;
	unsigned		fLastPlayTime;
	Boolean			fHaveStartedReading;
	Boolean			fLimitNumBytesToStream;
	u_int64_t		fNumBytesToStream; //used if "fLimitNumBytesToStream" is True

	unsigned char	fRealFrame[SESSION_VIDEO_FRAME_SIZE];

	//	unsigned char	*_redundancy;
	//	unsigned int	_redundancy_size;
	char			_session_id[MAX_PATH];
	Boolean			_recv_sps;
	Boolean			_recv_pps;
	Boolean			_recv_idr;
	unsigned char	_sps[100];
	unsigned char	_pps[100];
	int				_sps_size;
	int				_pps_size;

	DWORD			_recv_time;

};

#endif
