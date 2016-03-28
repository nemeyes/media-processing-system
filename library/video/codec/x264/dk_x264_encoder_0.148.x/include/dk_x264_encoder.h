#ifndef _DK_X264_ENCODER_H_
#define _DK_X264_ENCODER_H_

#include <dk_video_base.h>

class x264_encoder;
class EXP_CLASS dk_x264_encoder : public dk_video_encoder
{
public:
	typedef enum _preset_type
	{
		preset_type_ultra_fast,
		preset_type_super_fast,
		preset_type_very_fast,
		preset_type_faster,
		preset_type_fast,
		preset_type_medium,
		preset_type_slow,
		preset_type_slower,
		preset_type_very_slow,
		preset_type_placebo
	} preset_type;

	typedef enum _tune_type
	{
		tune_type_film,
		tune_type_animation,
		tune_type_grain,
		tune_type_stillimage,
		tune_type_psnr,
		tune_type_ssim,
		tune_type_fastdecode,
		tune_type_zerolatency
	} tune_type;

	typedef enum _rc_mode
	{
		rc_mode_cqp,
		rc_mode_crf,
		rc_mode_abr
	} rc_mode;

	typedef struct EXP_CLASS _x264_submedia_type_t : public dk_x264_encoder::_submedia_type_t
	{
		static const int32_t submedia_type_h264_h10p = 21; //stereo profile
		static const int32_t submedia_type_h264_h422p = 22; //svc_temporal_scalability profile
		static const int32_t submedia_type_h264_h444p = 23; //progressive high profile
	} x264_submedia_type_t;

	/*typedef enum _PIC_TYPE
	{
		PIC_TYPE_P = 0x0,     //Forward predicted
		PIC_TYPE_B = 0x01,    //Bi-directionally predicted picture
		PIC_TYPE_I = 0x02,    //Intra predicted picture
		PIC_TYPE_IDR = 0x03,    //IDR picture
		PIC_TYPE_BI = 0x04,    //Bi-directionally predicted with only Intra MBs
		PIC_TYPE_SKIPPED = 0x05,   //Picture is skipped
		PIC_TYPE_INTRA_REFRESH = 0x06,    //First picture in intra refresh cycle
		PIC_TYPE_UNKNOWN = 0xFF     //Picture type unknown
	} PIC_TYPE;*/


	typedef struct EXP_CLASS _configuration_t : public dk_video_encoder::configuration_t
	{
		int32_t max_width;
		int32_t max_height;
		int32_t tune;
		int32_t preset;
		int32_t vbv_max_bitrate;
		int32_t vbv_size;
		int32_t invalidate_ref_frames_enable_flag;
		int32_t intra_refresh_enable_flag;
		int32_t intra_refresh_period;
		int32_t intra_refresh_duration;
		int32_t numb;
		_configuration_t(void)
			: max_width(1920)
			, max_height(1080)
			, vbv_max_bitrate(4000000)
			, vbv_size(4000000)
			, tune(dk_x264_encoder::tune_type_zerolatency)
			, preset(dk_x264_encoder::preset_type_ultra_fast)
			, numb(0)
		{
		}

		_configuration_t(const _configuration_t & clone)
		{
			max_width = clone.max_width;
			max_height = clone.max_height;
			vbv_max_bitrate = clone.vbv_max_bitrate;
			vbv_size = clone.vbv_size;
			tune = clone.tune;
			preset = clone.preset;
			numb = clone.numb;
			/*invalidate_ref_frames_enable_flag = clone.invalidate_ref_frames_enable_flag;
			intra_refresh_enable_flag = clone.intra_refresh_enable_flag;
			intra_refresh_period = clone.intra_refresh_period;
			intra_refresh_duration = clone.intra_refresh_duration;*/
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			max_width = clone.max_width;
			max_height = clone.max_height;
			vbv_max_bitrate = clone.vbv_max_bitrate;
			vbv_size = clone.vbv_size;
			tune = clone.tune;
			preset = clone.preset;
			numb = clone.numb;
			return (*this);
		}
	} configuration_t;

	dk_x264_encoder(void);
	~dk_x264_encoder(void);

	dk_x264_encoder::encoder_state state(void);

	dk_x264_encoder::err_code initialize_encoder(void * config);
	dk_x264_encoder::err_code release_encoder(void);

	dk_x264_encoder::err_code encode(dk_video_encoder::dk_video_entity_t * input, dk_video_encoder::dk_video_entity_t * bitstream);
	dk_x264_encoder::err_code encode(dk_video_encoder::dk_video_entity_t * input);
	dk_x264_encoder::err_code get_queued_data(dk_video_encoder::dk_video_entity_t * input);

	dk_x264_encoder::err_code encode_async(dk_video_encoder::dk_video_entity_t * input);
	dk_x264_encoder::err_code check_encoding_finish(void);


	void on_acquire_bitstream(uint8_t * bistream, size_t size) {};

private:
	x264_encoder * _core;
};










#endif