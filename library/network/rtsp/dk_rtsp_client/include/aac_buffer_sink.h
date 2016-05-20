#ifndef _AAC_BUFFER_SINK_H_
#define _AAC_BUFFER_SINK_H_

#include "buffer_sink.h"
#include "dk_rtsp_client.h"

class aac_buffer_sink : public buffer_sink
{
public:
	aac_buffer_sink(debuggerking::rtsp_client * front, UsageEnvironment & env, unsigned buffer_size, int32_t channels, int32_t samplerate, int32_t bitrate, char * configstr, size_t configstr_size);
	virtual ~aac_buffer_sink(void);

protected:
	virtual void after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time);

protected:
	int32_t _channels;
	int32_t _samplerate;
	int32_t _bitrate;
	char _configstr[100];
	size_t _configstr_size;
};



















#endif