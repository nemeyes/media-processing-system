#ifndef _DK_NVENC_ENCODER_H_
#define _DK_NVENC_ENCODER_H_

#include <dk_video_base.h>

class nvenc_encoder;
class EXP_CLASS dk_nvenc_encoder : public dk_video_encoder
{
public:
	typedef enum _codec_type
	{
		codec_h264,
		codec_hevc
	} codec_type;

	typedef enum _codec_profile_type
	{
		codec_profile_auto,
		codec_h264_profile_baseline,
		codec_h264_profile_main,
		codec_h264_profile_high,
		codec_h264_profile_high_444,
		codec_h264_profile_stereo,
		codec_h264_profile_svc_temporal_scalability,
		codec_h264_profile_progressive_high,
		codec_h264_profile_constrained_high,
		codec_hevc_profile_main
	} codec_profile_type;

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

	/*typedef enum _picture_struct_type
	{
		picture_struct_frame = 0x01,
		picture_struct_field_top_bottom = 0x02,
		picture_struct_field_bottom_top = 0x03
	} picture_struct_type;*/

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


	typedef struct EXP_CLASS _configuration_t : public dk_video_encoder::configuration_t
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

	dk_nvenc_encoder(void);
	virtual ~dk_nvenc_encoder(void);

	dk_nvenc_encoder::encoder_state state(void);

	dk_nvenc_encoder::err_code initialize_encoder(void * config);
	dk_nvenc_encoder::err_code release_encoder(void);

	dk_nvenc_encoder::err_code encode(dk_nvenc_encoder::dk_video_entity_t * input, dk_nvenc_encoder::dk_video_entity_t * bitstream);
	dk_nvenc_encoder::err_code encode(dk_nvenc_encoder::dk_video_entity_t * input);
	dk_nvenc_encoder::err_code get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream);

	dk_nvenc_encoder::err_code encode_async(dk_nvenc_encoder::dk_video_entity_t * input);
	dk_nvenc_encoder::err_code check_encoding_flnish(void);

private:
	nvenc_encoder * _core;

};























#endif