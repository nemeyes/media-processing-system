
#include "h265_buffer_sink.h"

h265_buffer_sink::h265_buffer_sink(dk_live_rtsp_client * front, UsageEnvironment & env, const char * vps, const char * sps, const char * pps, unsigned buffer_size)
	: h2645_buffer_sink(front, dk_live_rtsp_client::SUBMEDIA_TYPE_HEVC, env, buffer_size, vps, sps, pps)
{

}

h265_buffer_sink::~h265_buffer_sink(void)
{

}

h265_buffer_sink * h265_buffer_sink::createNew(dk_live_rtsp_client * front, UsageEnvironment & env, const char *vps, const char *sps, const char *pps, unsigned buffer_size)
{
	return new h265_buffer_sink(front, env, vps, sps, pps, buffer_size);
}
