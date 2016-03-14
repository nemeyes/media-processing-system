
#include "h264_buffer_sink.h"

h264_buffer_sink::h264_buffer_sink(dk_live_rtsp_client * front, UsageEnvironment & env, const char * spspps, unsigned buffer_size)
	: h2645_buffer_sink(front, dk_live_rtsp_client::vsubmedia_type_h264, env, buffer_size, spspps, 0, 0)
{

}

h264_buffer_sink::~h264_buffer_sink(void)
{

}

h264_buffer_sink* h264_buffer_sink::createNew(dk_live_rtsp_client * front, UsageEnvironment & env, const char * spspps, unsigned buffer_size)
{
	return new h264_buffer_sink(front, env, spspps, buffer_size);
}
