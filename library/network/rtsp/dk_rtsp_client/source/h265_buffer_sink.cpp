
#include "h265_buffer_sink.h"

h265_buffer_sink::h265_buffer_sink(dk_rtsp_client * front, UsageEnvironment & env, const char * vps, const char * sps, const char * pps, unsigned buffer_size)
	: h2645_buffer_sink(front, dk_rtsp_client::SUBMEDIA_TYPE_H265, env, buffer_size, vps, sps, pps)
{

}

h265_buffer_sink::~h265_buffer_sink(void)
{

}

h265_buffer_sink * h265_buffer_sink::createNew(dk_rtsp_client * front, UsageEnvironment & env, const char *vps, const char *sps, const char *pps, unsigned buffer_size)
{
	return new h265_buffer_sink(front, env, vps, sps, pps, buffer_size);
}
