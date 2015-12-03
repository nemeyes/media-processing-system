#include "dk_rtmp_client.h"
#include "rtmp_client.h"

dk_rtmp_client::dk_rtmp_client(void)
{
	_core = new rtmp_client(this);
}

dk_rtmp_client::~dk_rtmp_client(void)
{
	if (_core)
		delete _core;

	_core = nullptr;
}

dk_rtmp_client::ERROR_CODE dk_rtmp_client::play(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat)
{
	return _core->play(url, username, password, recv_option, repeat);
}

dk_rtmp_client::ERROR_CODE dk_rtmp_client::stop(void)
{
	return _core->stop();
}

uint8_t * dk_rtmp_client::get_sps(size_t & sps_size)
{
	return _core->get_sps(sps_size);
}

uint8_t * dk_rtmp_client::get_pps(size_t & pps_size)
{
	return _core->get_pps(pps_size);
}