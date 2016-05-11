#ifndef _DK_NVENC_ENCODER_H_
#define _DK_NVENC_ENCODER_H_

#include <dk_video_base.h>

namespace debuggerking
{
	class nvenc_core;
	class EXP_CLASS nvenc_encoder : public video_encoder
	{
		friend class nvenc_core;
	public:
		typedef struct EXP_CLASS _nvenc_submedia_type_t : public nvenc_encoder::video_submedia_type_t
		{
			static const int32_t h264_sp = boundary + 1; //stereo profile
			static const int32_t h264_stsp = boundary + 2; //svc_temporal_scalability profile
			static const int32_t h264_php = boundary + 3; //progressive high profile
			static const int32_t h264_chp = boundary + 4; //contrained high profile
		} nvenc_submedia_type_t;

		typedef enum _preset_type
		{
			preset_default,
			preset_high_performance,
			preset_high_quality,
			preset_bluelay_disk,
			preset_low_latency_default,
			preset_low_latency_high_quality,
			preset_low_latency_high_performance,
			preset_lossless_default,
			preset_lossless_high_performance
		} preset_type;

		typedef enum _rate_control_mode
		{
			rate_control_constant_qp = 0x00,
			rate_control_vbr = 0x01,
			rate_control_cbr = 0x02,
			rate_control_vbr_min_qp = 0x04,
			rate_control_two_pass_quality = 0x08,
			rate_control_two_pass_framesize_cap = 0x10,
			rate_control_two_pass_vbr = 0x20
		} rate_control_mode;

		typedef enum _frame_field_mode
		{
			frame_field_mode_frame = 0x01,  /**< frame mode */
			frame_field_mode_field = 0x02,  /**< field mode */
			frame_field_mode_mbaff = 0x03   /**< mb adaptive frame/field */
		} frame_field_mode;

		typedef enum _motion_vector_precision
		{
			motion_vector_precision_default = 0x0,       /**<Driver selects QuarterPel motion vector precision by default*/
			motion_vector_precision_full_pel = 0x01,    /**< fullpel  motion vector precision */
			motion_vector_precision_half_pel = 0x02,    /**< halfpel motion vector precision */
			motion_vector_precision_quarter_pel = 0x03     /**< quarterpel motion vector precision */
		} motion_vector_precision;

		typedef enum _encode_level
		{
			encode_level_autoselect = 0,
			encode_level_h264_1 = 10,
			encode_level_h264_1b = 9,
			encode_level_h264_11 = 11,
			encode_level_h264_12 = 12,
			encode_level_h264_13 = 13,
			encode_level_h264_2 = 20,
			encode_level_h264_21 = 21,
			encode_level_h264_22 = 22,
			encode_level_h264_3 = 30,
			encode_level_h264_31 = 31,
			encode_level_h264_32 = 32,
			encode_level_h264_4 = 40,
			encode_level_h264_41 = 41,
			encode_level_h264_42 = 42,
			encode_level_h264_5 = 50,
			encode_level_h264_51 = 51,
			encode_level_h264_52 = 52,
			encode_level_hevc_1 = 30,
			encode_level_hevc_2 = 60,
			encode_level_hevc_21 = 63,
			encode_level_hevc_3 = 90,
			encode_level_hevc_31 = 93,
			encode_level_hevc_4 = 120,
			encode_level_hevc_41 = 123,
			encode_level_hevc_5 = 150,
			encode_level_hevc_51 = 153,
			encode_level_hevc_52 = 156,
			encode_level_hevc_6 = 180,
			encode_level_hevc_61 = 183,
			encode_level_hevc_62 = 186,
			encode_tier_hevc_main = 0,
			encode_tier_hevc_high = 1
		} encode_level;


		typedef struct EXP_CLASS _configuration_t : public video_encoder::configuration_t
		{
			int32_t device_id;
			int32_t max_height;
			int32_t max_width;
			int32_t preset;
			int32_t rc_mode;
			int32_t frame_field_mode;
			int32_t motioin_vector_precision;
			int32_t encode_level;
			int32_t vbv_max_bitrate;
			int32_t vbv_size;
			uint32_t qp;
			float i_quant_factor;
			float b_quant_factor;
			float i_quant_offset;
			float b_quant_offset;
			int32_t invalidate_reference_frames_enable;
			int32_t intra_refresh_enable;
			int32_t intra_refresh_period;
			int32_t intra_refresh_duration;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t & operator=(const _configuration_t & clone);
		} configuration_t;

		nvenc_encoder(void);
		virtual ~nvenc_encoder(void);

		nvenc_encoder::encoder_state state(void);

		int32_t initialize_encoder(void * config);
		int32_t release_encoder(void);

		int32_t encode(nvenc_encoder::entity_t * input, nvenc_encoder::entity_t * bitstream);
		int32_t encode(nvenc_encoder::entity_t * input);
		int32_t get_queued_data(nvenc_encoder::entity_t * bitstream);
		virtual void after_encoding_callback(uint8_t * bistream, size_t size);

	private:
		nvenc_core * _core;

	};
 };





















#endif