#ifndef _DK_X264_ENCODER_H_
#define _DK_X264_ENCODER_H_

#include <dk_video_base.h>

class x264_encoder;
class EXP_CLASS dk_x264_encoder : public dk_video_encoder
{
public:
	typedef enum _PRESET_TYPE
	{
		PRESET_TYPE_ULTRA_FAST,
		PRESET_TYPE_SUPER_FAST,
		PRESET_TYPE_VERY_FAST,
		PRESET_TYPE_FASTER,
		PRESET_TYPE_FAST,
		PRESET_TYPE_MEDIUM,
		PRESET_TYPE_SLOW,
		PRESET_TYPE_SLOWER,
		PRESET_TYPE_VERY_SLOW,
		PRESET_TYPE_PLACEBO
	} PRESET_TYPE;

	typedef enum _TUNE_TYPE
	{
		TUNE_TYPE_FILM,
		TUNE_TYPE_ANIMATION,
		TUNE_TYPE_GRAIN,
		TUNE_TYPE_STILLIMAGE,
		TUNE_TYPE_PSNR,
		TUNE_TYPE_SSIM,
		TUNE_TYPE_FASTDECODE,
		TUNE_TYPE_ZEROLATENCY
	} TUNE_TYPE;

	typedef enum _CODEC_PROFILE_TYPE
	{
		CODEC_PROFILE_TYPE_BASELINE,
		CODEC_PROFILE_TYPE_MAIN,
		CODEC_PROFILE_TYPE_HIGH
	} CODEC_PROFILE_TYPE;

	typedef enum _RC_MODE
	{
		RC_MODE_CQP,
		RC_MODE_CRF,
		RC_MODE_ABR
	} RC_MODE;

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


	typedef struct EXP_CLASS _configuration_t
	{
		SUBMEDIA_TYPE cs;
		int width;
		int height;
		int max_width;
		int max_height;
		int bitrate;
		int tune;
		int keyframe_interval;
		int profile;
		int fps;
		int preset;
		int vbv_max_bitrate;
		int vbv_size;
		int invalidate_ref_frames_enable_flag;
		int intra_refresh_enable_flag;
		int intra_refresh_period;
		int intra_refresh_duration;
		int numb;
		int bitstream_buffer_size;
		_configuration_t(void)
			: cs(SUBMEDIA_TYPE_YV12)
			, width(1280)
			, height(1024)
			, max_width(1920)
			, max_height(1080)
			, bitrate(4000000)
			, fps(30)
			, vbv_max_bitrate(4000000)
			, vbv_size(4000000)
			, tune(TUNE_TYPE_FILM)
			, keyframe_interval(2)
			, profile(CODEC_PROFILE_TYPE_HIGH)
			//, qp(28)
			, preset(PRESET_TYPE_ULTRA_FAST)
			, invalidate_ref_frames_enable_flag(0)
			, intra_refresh_enable_flag(0)
			, intra_refresh_period(0)
			, intra_refresh_duration(0)
			, numb(0)
			, bitstream_buffer_size(0)
		{
		}

		_configuration_t(const _configuration_t & clone)
		{
			cs = clone.cs;
			width = clone.width;
			height = clone.height;
			max_width = clone.max_width;
			max_height = clone.max_height;
			bitrate = clone.bitrate;
			tune = clone.tune;
			keyframe_interval = clone.keyframe_interval;
			profile = clone.profile;
			fps = clone.fps;
			//qp = clone.qp;
			preset = clone.preset;
			vbv_max_bitrate = clone.vbv_max_bitrate;
			vbv_size = clone.vbv_size;
			invalidate_ref_frames_enable_flag = clone.invalidate_ref_frames_enable_flag;
			intra_refresh_enable_flag = clone.intra_refresh_enable_flag;
			intra_refresh_period = clone.intra_refresh_period;
			intra_refresh_duration = clone.intra_refresh_duration;
			numb = clone.numb;
			bitstream_buffer_size = clone.bitstream_buffer_size;
		}

		_configuration_t operator=(const _configuration_t & clone)
		{
			cs = clone.cs;
			width = clone.width;
			height = clone.height;
			max_width = clone.max_width;
			max_height = clone.max_height;
			bitrate = clone.bitrate;
			tune = clone.tune;
			keyframe_interval = clone.keyframe_interval;
			profile = clone.profile;
			fps = clone.fps;
			//qp = clone.qp;
			preset = clone.preset;
			vbv_max_bitrate = clone.vbv_max_bitrate;
			vbv_size = clone.vbv_size;
			invalidate_ref_frames_enable_flag = clone.invalidate_ref_frames_enable_flag;
			intra_refresh_enable_flag = clone.intra_refresh_enable_flag;
			intra_refresh_period = clone.intra_refresh_period;
			intra_refresh_duration = clone.intra_refresh_duration;
			numb = clone.numb;
			bitstream_buffer_size = clone.bitstream_buffer_size;
			return (*this);
		}
	} configuration_t;

	dk_x264_encoder(void);
	~dk_x264_encoder(void);

	dk_x264_encoder::ENCODER_STATE state(void);

	dk_x264_encoder::ERR_CODE initialize_encoder(void * config);
	dk_x264_encoder::ERR_CODE release_encoder(void);

	dk_x264_encoder::ERR_CODE encode(dk_video_encoder::dk_video_entity_t * input, dk_video_encoder::dk_video_entity_t * bitstream);
	dk_x264_encoder::ERR_CODE encode(dk_video_encoder::dk_video_entity_t * input);
	dk_x264_encoder::ERR_CODE get_queued_data(dk_video_encoder::dk_video_entity_t * input);

	dk_x264_encoder::ERR_CODE encode_async(dk_video_encoder::dk_video_entity_t * input);
	dk_x264_encoder::ERR_CODE check_encoding_finish(void);


	void on_acquire_bitstream(uint8_t * bistream, size_t size) {};

private:
	x264_encoder * _core;
};










#endif