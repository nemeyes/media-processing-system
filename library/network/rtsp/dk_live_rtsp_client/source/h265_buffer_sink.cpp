
#include "h265_buffer_sink.h"

h265_buffer_sink::h265_buffer_sink(debuggerking::live_rtsp_client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
	: h2645_buffer_sink(front, debuggerking::live_rtsp_client::video_submedia_type_t::hevc, env, vps, vps_size, sps, sps_size, pps, pps_size, buffer_size)
{

}

h265_buffer_sink::~h265_buffer_sink(void)
{

}

h265_buffer_sink * h265_buffer_sink::createNew(debuggerking::live_rtsp_client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
{
	return new h265_buffer_sink(front, env, vps, vps_size, sps, sps_size, pps, pps_size, buffer_size);
}
