
#include "aac_buffer_sink.h"
#include <H264VideoRTPSource.hh>

aac_buffer_sink::aac_buffer_sink(debuggerking::rtsp_client * front, UsageEnvironment & env, unsigned buffer_size, int32_t channels, int32_t samplerate, int32_t bitrate, char * configstr, size_t configstr_size)
	: buffer_sink(front, debuggerking::rtsp_client::media_type_t::audio, debuggerking::rtsp_client::audio_submedia_type_t::aac, env, buffer_size)
	, _channels(channels)
	, _samplerate(samplerate)
	, _bitrate(bitrate)
	, _configstr_size(configstr_size)
{
	memcpy(_configstr, configstr, configstr_size);
}

aac_buffer_sink::~aac_buffer_sink(void)
{
}

void aac_buffer_sink::after_getting_frame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time)
{
	buffer_sink::after_getting_frame(frame_size, truncated_bytes, presentation_time);
}
