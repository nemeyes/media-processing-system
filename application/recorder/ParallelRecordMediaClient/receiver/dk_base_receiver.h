#ifndef _DK_BASE_RECEIVER_H_
#define _DK_BASE_RECEIVER_H_

#include <dk_bit_vector.h>
#include <dk_video_base.h>
#include <dk_audio_base.h>

#define VIDEO_BUFFER_SIZE 1920 * 1080 * 4
#define AUDIO_BUFFER_SIZE 48000 * 2 * 8 //48000hz * 16bitdetph * 8 channels ex) for 2channel 192000

namespace debuggerking
{
	class base_receiver
	{
	public:
		base_receiver(void);
		virtual ~base_receiver(void);

		void parse_vui(CBitVector & bv, unsigned & num_units_in_tick, unsigned & time_scale, unsigned & fixed_frame_rate_flag, int * sar_width, int * sar_height);
		int32_t parse_pps(uint8_t * pps, int32_t pps_size);
		int32_t parse_sps(uint8_t * data, int32_t sizeOfSPS, int32_t * width, int32_t * height, int32_t * sar_width, int32_t * sar_height);
		int32_t parse_mpeg(uint8_t * data, int32_t size, int32_t * width, int32_t * height, int32_t * sar_width, int32_t * sar_height);
		int32_t parse_jpeg(uint8_t * data, int32_t size, int32_t * width, int32_t * height, int32_t * sar_width, int32_t * sar_height);
		void make_adts_header(uint8_t * data, int32_t size, char audioObjectType, char samplingFreqIndex, char channelConfig);

	protected:
		HWND _hwnd;

		video_decoder * _video_decoder;
		void * _video_decoder_config;
		video_renderer * _video_renderer;
		void * _video_renderer_config;

		audio_decoder * _audio_decoder;
		void * _audio_decoder_config;
		audio_renderer * _audio_renderer;
		void * _audio_renderer_config;

		uint8_t * _video_buffer;
		uint8_t * _audio_buffer;
	};
};
#endif