#pragma once
#include <windows.h>
#include <cstdint>
#include "dk_bit_vector.h"
#include <dk_rtsp_client.h>
#include "dk_mpeg2ts_recorder.h"

class dk_rtsp_recorder : public dk_rtsp_client
{
public:
	dk_rtsp_recorder(void);
	virtual ~dk_rtsp_recorder(void);

	void start_recording(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, const char * id);
	void stop_recording(void);

	void on_begin_video(dk_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp);
	void on_recv_video(dk_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long timestamp);

	void on_begin_audio(dk_rtsp_client::AUDIO_SUBMEDIA_TYPE_T smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp);
	void on_recv_audio(dk_rtsp_client::AUDIO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long timestamp);

private:
	// A general bit copy operation:
	void parse_vui(CBitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale, unsigned& fixed_frame_rate_flag, int* sar_width, int* sar_height);
	int parse_pps(uint8_t* data, int sizeOfSPS);
	int parse_sps(uint8_t* data, int sizeOfSPS, int *width, int *height, int* sar_width, int* sar_height);
	int parse_mpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	int parse_jpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	void make_adts_header(uint8_t* data, int size, char audioObjectType, char samplingFreqIndex, char channelConfig);

private:
	char _id[500];

	dk_ff_mpeg2ts_muxer::configuration_t _config;
	dk_mpeg2ts_recorder * _mpeg2ts_recorder;

	long long _chunk_size_bytes;
};

