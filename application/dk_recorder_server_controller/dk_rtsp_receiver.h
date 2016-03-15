#pragma once
#include <windows.h>
#include <cstdint>
#include "dk_bit_vector.h"
#include <dk_live_rtsp_client.h>
#include <dk_ff_video_decoder.h>
#include <dk_aac_decoder.h>
#include <dk_ff_mp3_decoder.h>
#include <dk_directdraw_renderer.h>
#include <dk_mmwave_renderer.h>
#include "dk_mpeg2ts_saver.h"

class dk_rtsp_receiver : public dk_live_rtsp_client
{
public:
	dk_rtsp_receiver(void);
	~dk_rtsp_receiver(void);

	void start_preview(const char * url, const char * username, const char * password, int transport_option, int recv_option, HWND hwnd);
	void stop_preview(void);

	void start_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option);
	void stop_recording(void);

	void on_begin_video(dk_live_rtsp_client::vsubmedia_type smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp);
	void on_recv_video(dk_live_rtsp_client::vsubmedia_type smt, const uint8_t * data, size_t data_size, long long timestamp);

	void on_begin_audio(dk_live_rtsp_client::asubmedia_type smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp);
	void on_recv_audio(dk_live_rtsp_client::asubmedia_type smt, const uint8_t * data, size_t data_size, long long timestamp);

private:
	// A general bit copy operation:
	void parse_vui(CBitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale, unsigned& fixed_frame_rate_flag, int* sar_width, int* sar_height);
	int parse_pps(uint8_t* data, int sizeOfSPS);
	int parse_sps(uint8_t* data, int sizeOfSPS, int *width, int *height, int* sar_width, int* sar_height);
	int parse_mpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	int parse_jpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	void make_adts_header(uint8_t* data, int size, char audioObjectType, char samplingFreqIndex, char channelConfig);

private:
	bool _is_preview_enabled;
	bool _is_recording_enabled;

	HWND _hwnd;

	dk_video_decoder * _video_decoder;
	void * _video_decoder_config;
	dk_video_renderer * _video_renderer;
	void * _video_renderer_config;

	dk_audio_decoder * _audio_decoder;
	void * _audio_decoder_config;
	dk_audio_renderer * _audio_renderer;
	void * _audio_renderer_config;

	uint8_t * _video_buffer;
	uint8_t * _audio_buffer;

	dk_mpeg2ts_saver * _mpeg2ts_saver;
};

