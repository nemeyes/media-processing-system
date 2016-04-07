
#include "h265_buffer_sink.h"

h265_buffer_sink::h265_buffer_sink(dk_live_rtsp_client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
	: h2645_buffer_sink(front, dk_live_rtsp_client::vsubmedia_type_hevc, env, vps, vps_size, sps, sps_size, pps, pps_size, buffer_size)
{

}

h265_buffer_sink::~h265_buffer_sink(void)
{

}

h265_buffer_sink * h265_buffer_sink::createNew(dk_live_rtsp_client * front, UsageEnvironment & env, const char * vps, unsigned vps_size, const char * sps, unsigned sps_size, const char * pps, unsigned pps_size, unsigned buffer_size)
{
	return new h265_buffer_sink(front, env, vps, vps_size, sps, sps_size, pps, pps_size, buffer_size);
}
