#ifndef _MEDIA_EDGE_FILE_ENCODER_H_
#define _MEDIA_EDGE_FILE_ENCODER_H_

#if defined(WITH_MEDIA_FOUNDATION)
#include <dk_mf_player_framework.h>

class media_edge_file_encoder
{
public:
	media_edge_file_encoder(void);
	virtual ~media_edge_file_encoder(void);

	void play(const wchar_t * filepath, HWND hwnd);
	void stop(void);
private:
	dk_mf_player_framework _player;
};

#else

#include <dk_file_demuxer.h>
#include <dk_ff_video_decoder.h>
#include <dk_directdraw_renderer.h>
#include <dk_aac_decoder.h>
#include <dk_ff_mp3_decoder.h>
#include <dk_mmwave_renderer.h>
#include "bitvector.h"

class media_edge_file_encoder : public dk_file_demuxer
{
public:
	media_edge_file_encoder(void);
	virtual ~media_edge_file_encoder(void);

	void play(const char * filepath, HWND hwnd);
	void stop(void);

	void on_begin_video(dk_file_demuxer::vsubmedia_type smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_recv_video(dk_file_demuxer::vsubmedia_type smt, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_begin_audio(dk_file_demuxer::asubmedia_type smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long presentation_time);
	void on_recv_audio(dk_file_demuxer::asubmedia_type smt, const uint8_t * data, size_t data_size, long long presentation_time);

private:
	void parse_vui(CBitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale, unsigned& fixed_frame_rate_flag, int* sar_width, int* sar_height);
	int parse_pps(uint8_t* data, int sizeOfSPS);
	int parse_sps(uint8_t* data, int sizeOfSPS, int *width, int *height, int* sar_width, int* sar_height);
	int parse_mpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	int parse_jpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	void make_adts_header(uint8_t* data, int size, char audioObjectType, char samplingFreqIndex, char channelConfig);

private:
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
};



#endif






#endif