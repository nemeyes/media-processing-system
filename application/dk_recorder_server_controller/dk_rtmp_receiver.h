#pragma once
#include <windows.h>
#include <cstdint>
#include <dk_rtmp_client.h>
#include <dk_ff_video_decoder.h>
#include <dk_directdraw_renderer.h>
#include <dk_ff_mp3_decoder.h>
#include <dk_aac_decoder.h>
#include <dk_mmwave_renderer.h>
#include "dk_mpeg2ts_saver.h"
#include "dk_bit_vector.h"

class dk_rtmp_receiver : public dk_rtmp_client
{
public:
	dk_rtmp_receiver(void);
	~dk_rtmp_receiver(void);

	void start_preview(const char * url, const char * username, const char * password, int transport_option, int recv_option, HWND handle);
	void stop_preview(void);

	void start_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option);
	void stop_recording(void);

	void on_begin_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_recv_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_begin_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, uint8_t * configstr, size_t configstr_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_recv_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long presentation_time);

private:
	void parse_vui(CBitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale, unsigned& fixed_frame_rate_flag, int* sar_width, int* sar_height);
	int parse_pps(uint8_t* data, int sizeOfSPS);
	int parse_sps(uint8_t* data, int sizeOfSPS, int *width, int *height, int* sar_width, int* sar_height);
	int parse_mpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	int parse_jpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	void make_adts_header(uint8_t* data, int size, char audioObjectType, char samplingFreqIndex, char channelConfig);

private:
	bool _is_preview_enabled;
	bool _is_recording_enabled;
	HWND _normal_hwnd;

	dk_ff_video_decoder * _video_decoder;
	dk_ff_video_decoder::configuration_t _video_decoder_config;
	dk_directdraw_renderer * _video_renderer;
	dk_directdraw_renderer::configuration_t _video_renderer_config;


	dk_audio_decoder * _audio_decoder;
	void * _audio_decoder_config;
	dk_mmwave_renderer * _audio_renderer;
	dk_mmwave_renderer::configuration_t _audio_renderer_config;

	dk_mpeg2ts_saver * _mpeg2ts_saver;

	uint8_t * _video_buffer;
	uint8_t * _audio_buffer;
	int64_t _frame_count;
};

