#include "dk_rtmp_client.h"
#include "rtmp_client.h"

debuggerking::rtmp_client::rtmp_client(bool split_thread)
	: _split_thread(split_thread)
{
	_core = new rtmp_core(this);
}

debuggerking::rtmp_client::~rtmp_client(void)
{
	if (_core)
		delete _core;

	_core = nullptr;
}

uint8_t * debuggerking::rtmp_client::get_sps(size_t & sps_size)
{
	return _core->get_sps(sps_size);
}

uint8_t * debuggerking::rtmp_client::get_pps(size_t & pps_size)
{
	return _core->get_pps(pps_size);
}

void debuggerking::rtmp_client::set_sps(uint8_t * sps, size_t sps_size)
{
	return _core->set_sps(sps, sps_size);
}

void debuggerking::rtmp_client::set_pps(uint8_t * pps, size_t pps_size)
{
	return _core->set_pps(pps, pps_size);
}

void debuggerking::rtmp_client::clear_sps(void)
{
	return _core->clear_sps();
}

void debuggerking::rtmp_client::clear_pps(void)
{
	return _core->clear_pps();
}

debuggerking::rtmp_client::rtmp_state debuggerking::rtmp_client::state(void)
{
	return _core->state();
}

int32_t debuggerking::rtmp_client::subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat)
{
	return _core->subscribe_begin(url, username, password, recv_option, repeat);
}

int32_t debuggerking::rtmp_client::subscribe_end(void)
{
	return _core->subscribe_end();
}

void debuggerking::rtmp_client::on_begin_video(int32_t smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp)
{

}

void debuggerking::rtmp_client::on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp)
{

}

void debuggerking::rtmp_client::on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp)
{

}

void debuggerking::rtmp_client::on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp)
{

}

int32_t debuggerking::rtmp_client::publish_begin(int32_t vsmt, int32_t asmt, const char * url, const char * username, const char * password)
{
	return _core->publish_begin(vsmt, asmt, url, username, password);
}

int32_t debuggerking::rtmp_client::publish_end(void)
{
	return _core->publish_end();
}

int32_t debuggerking::rtmp_client::publish_video(uint8_t * bitstream, size_t nb, long long timestamp)
{
	return _core->publish_video(bitstream, nb, timestamp);
}

int32_t debuggerking::rtmp_client::publish_audio(uint8_t * bitstream, size_t nb, bool configstr, long long timestamp)
{
	return _core->publish_audio(bitstream, nb, configstr, timestamp);
}