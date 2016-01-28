#pragma once
#include <windows.h>
#include <cstdint>
#include <dk_rtsp_client.h>
#include <dk_ff_video_decoder.h>
#include <dk_directdraw_renderer.h>
#include "dk_mpeg2ts_saver.h"
#include "dk_bit_vector.h"

class dk_rtsp_receiver : public dk_rtsp_client
{
public:
	dk_rtsp_receiver(void);
	~dk_rtsp_receiver(void);

	void start_preview(const char * url, const char * username, const char * password, int transport_option, int recv_option, HWND handle);
	void stop_preview(void);

	void start_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option);
	void stop_recording(void);

	void on_begin_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, struct timeval presentation_time);
	void on_recv_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, struct timeval presentation_time);

private:
	// A general bit copy operation:
	void parse_vui(CBitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale, unsigned& fixed_frame_rate_flag, int* sar_width, int* sar_height);
	int parse_pps(uint8_t* data, int sizeOfSPS);
	int parse_sps(uint8_t* data, int sizeOfSPS, int *width, int *height, int* sar_width, int* sar_height);
	int parse_mpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	int parse_jpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
	void make_adts_header(uint8_t* data, int size, char audioObjectType, char samplingFreqIndex, char channelConfig);

	/*void shift_bits(unsigned char* toBasePtr, unsigned toBitOffset, unsigned char const* fromBasePtr, unsigned fromBitOffset, unsigned numBits);
	uint32_t __inline Log2Bin(uint32_t value);
	void remove_emulation_bytes(uint8_t *dst, uint8_t *src, uint32_t max_size, uint32_t num_bytes_in_nal_unit, uint32_t *copy_size);*/

private:
	bool _is_preview_enabled;
	bool _is_recording_enabled;

	dk_ff_video_decoder * _decoder;
	dk_ff_video_decoder::configuration_t _decoder_config;	

	//dk_ff_mpeg2ts_muxer * _mpeg2ts_muxer;
	dk_mpeg2ts_saver * _mpeg2ts_saver;

	HWND _normal_hwnd;
	dk_directdraw_renderer * _renderer;
	dk_directdraw_renderer::configuration_t _renderer_config;


	uint8_t * _buffer;

	int64_t _frame_count;
	//dk_image_creator * _image_creator;

};

