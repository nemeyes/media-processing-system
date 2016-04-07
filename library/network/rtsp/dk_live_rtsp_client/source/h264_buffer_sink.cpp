
#include "h264_buffer_sink.h"

h264_buffer_sink::h264_buffer_sink(dk_live_rtsp_client * front, UsageEnvironment & env, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
	: h2645_buffer_sink(front, dk_live_rtsp_client::vsubmedia_type_h264, env, 0, 0, sps, sps_size, pps, pps_size, buffer_size)
{

}

h264_buffer_sink::~h264_buffer_sink(void)
{

}

h264_buffer_sink* h264_buffer_sink::createNew(dk_live_rtsp_client * front, UsageEnvironment & env, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
{
	return new h264_buffer_sink(front, env, sps, sps_size, pps, pps_size, buffer_size);
}
