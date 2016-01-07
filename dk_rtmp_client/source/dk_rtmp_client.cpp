#include "dk_rtmp_client.h"
#include "rtmp_client.h"

dk_rtmp_client::dk_rtmp_client(bool split_thread)
	: _split_thread(split_thread)
{
	_core = new rtmp_client(this);
}

dk_rtmp_client::~dk_rtmp_client(void)
{
	if (_core)
		delete _core;

	_core = nullptr;
}

uint8_t * dk_rtmp_client::get_sps(size_t & sps_size)
{
	return _core->get_sps(sps_size);
}

uint8_t * dk_rtmp_client::get_pps(size_t & pps_size)
{
	return _core->get_pps(pps_size);
}

void dk_rtmp_client::set_sps(uint8_t * sps, size_t sps_size)
{
	return _core->set_sps(sps, sps_size);
}

void dk_rtmp_client::set_pps(uint8_t * pps, size_t pps_size)
{
	return _core->set_pps(pps, pps_size);
}

void dk_rtmp_client::clear_sps(void)
{
	return _core->clear_sps();
}

void dk_rtmp_client::clear_pps(void)
{
	return _core->clear_pps();
}

dk_rtmp_client::STATE_T dk_rtmp_client::state(void)
{
	return _core->state();
}

dk_rtmp_client::ERR_CODE dk_rtmp_client::subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat)
{
	return _core->subscribe_begin(url, username, password, recv_option, repeat);
}

dk_rtmp_client::ERR_CODE dk_rtmp_client::subscribe_end(void)
{
	return _core->subscribe_end();
}

dk_rtmp_client::ERR_CODE dk_rtmp_client::publish_begin(VIDEO_SUBMEDIA_TYPE_T vsmt, AUDIO_SUBMEDIA_TYPE_T asmt, const char * url, const char * username, const char * password)
{
	return _core->publish_begin(vsmt, asmt, url, username, password);
}

dk_rtmp_client::ERR_CODE dk_rtmp_client::publish_video(uint8_t * bitstream, size_t nb)
{
	return _core->publish_video(bitstream, nb);
}

dk_rtmp_client::ERR_CODE dk_rtmp_client::publish_audio(uint8_t * bitstream, size_t nb)
{
	return _core->publish_audio(bitstream, nb);
}

dk_rtmp_client::ERR_CODE dk_rtmp_client::publish_end(void)
{
	return _core->publish_end();
}