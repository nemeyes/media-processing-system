#pragma once
//#include <windows.h>
#include <cstdint>
#include <dk_rtmp_client.h>
#include "dk_bit_vector.h"

class dk_rtmp_subscriber : public dk_rtmp_client
{
public:
	dk_rtmp_subscriber(void);
	~dk_rtmp_subscriber(void);

	dk_rtmp_subscriber::ERR_CODE play(void);
	dk_rtmp_subscriber::ERR_CODE stop(void);

	dk_rtmp_subscriber::ERR_CODE set_url(wchar_t * url);
	dk_rtmp_subscriber::ERR_CODE set_username(wchar_t * username);
	dk_rtmp_subscriber::ERR_CODE set_password(wchar_t * password);
	dk_rtmp_subscriber::ERR_CODE set_recv_option(uint16_t option);
	dk_rtmp_subscriber::ERR_CODE set_recv_timeout(uint64_t timeout);
	dk_rtmp_subscriber::ERR_CODE set_connection_timeout(uint64_t timeout);
	dk_rtmp_subscriber::ERR_CODE set_repeat(bool repeat);



	dk_rtmp_subscriber::ERR_CODE get_url(wchar_t ** url);
	dk_rtmp_subscriber::ERR_CODE get_username(wchar_t ** username);
	dk_rtmp_subscriber::ERR_CODE get_password(wchar_t ** password);
	dk_rtmp_subscriber::ERR_CODE get_recv_option(uint16_t & option);
	dk_rtmp_subscriber::ERR_CODE get_recv_timeout(uint64_t & timeout);
	dk_rtmp_subscriber::ERR_CODE get_connection_timeout(uint64_t & timeout);
	dk_rtmp_subscriber::ERR_CODE get_repeat(bool & repeat);

	//dk_rtmp_subscriber::ERR_CODE set_video_width(int32_t width);
	//dk_rtmp_subscriber::ERR_CODE set_video_height(int32_t height);
	//dk_rtmp_subscriber::ERR_CODE get_video_width(int32_t & width);
	//dk_rtmp_subscriber::ERR_CODE get_video_height(int32_t & height);

	/*dk_rtmp_subscriber::ERR_CODE set_configstr(uint8_t * configstr, size_t size);
	dk_rtmp_subscriber::ERR_CODE set_audio_samplerate(int32_t samplerate);
	dk_rtmp_subscriber::ERR_CODE set_audio_bitdepth(int32_t bitdepth);
	dk_rtmp_subscriber::ERR_CODE set_audio_channels(int32_t channels);

	dk_rtmp_subscriber::ERR_CODE get_configstr(uint8_t * configstr, size_t & size);
	dk_rtmp_subscriber::ERR_CODE get_audio_samplerate(int32_t & samplerate);
	dk_rtmp_subscriber::ERR_CODE get_audio_bitdepth(int32_t & bitdepth);
	dk_rtmp_subscriber::ERR_CODE get_audio_channels(int32_t & channels);*/




	void on_begin_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_recv_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_begin_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_recv_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long presentation_time);

private:
	void parse_vui(dk_bit_vector & bv, unsigned & num_units_in_tick, unsigned & time_scale, unsigned & fixed_frame_rate_flag, int * sar_width, int * sar_height);
	int parse_pps(uint8_t * data, int sizeOfSPS);
	int parse_sps(uint8_t * data, int sizeOfSPS, int * width, int * height, int * sar_width, int * sar_height);
	int parse_mpeg(uint8_t * data, int size, int * width, int * height, int * sar_width, int * sar_height);
	int parse_jpeg(uint8_t * data, int size, int * width, int * height, int * sar_width, int * sar_height);

private:
	wchar_t _url[500];
	wchar_t _username[200];
	wchar_t _password[200];
	RECV_OPTION_T _recv_option;
	uint64_t _recv_timeout;
	uint64_t _conn_timeout;
	bool _repeat;

#if defined(WITH_DEBUG_VIDEO_ES)
	HANDLE _video_file;
#endif
};