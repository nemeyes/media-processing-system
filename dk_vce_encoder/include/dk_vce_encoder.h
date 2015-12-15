#ifndef _DK_VCE_ENCODER_H_
#define _DK_VCE_ENCODER_H_

#include <dk_video_base.h>

class vce_encoder;
class EXP_CLASS dk_vce_encoder : public dk_video_encoder
{
public:
	typedef enum _COLOR_SPACE
	{
		COLOR_SPACE_I420,
		COLOR_SPACE_YV12,
		COLOR_SPACE_NV12,
		COLOR_SPACE_RGB24,
		COLOR_SPACE_RGB32
	} COLOR_SPACE;

	typedef enum _CODEC_PROFILE_TYPE
	{
		CODEC_PROFILE_TYPE_BASELINE,
		CODEC_PROFILE_TYPE_MAIN,
		CODEC_PROFILE_TYPE_HIGH
	} CODEC_PROFILE_TYPE;

	typedef enum _RC_MODE
	{
		RC_MODE_CONSTQP,
		RC_MODE_VBR,
		RC_MODE_CBR,
		RC_MODE_PEAK_CONSTRAINED_VBR,
		RC_MODE_LATENCY_CONSTRAINED_VBR
	} RC_MODE;

	typedef enum _PRESET_TYPE
	{
		PRESET_TYPE_BALANCED,
		PRESET_TYPE_SPEED,
		PRESET_TYPE_QUALITY
	} PRESET_TYPE;

	typedef enum _PIC_STRUCT
	{
		PIC_STRUCT_FRAME,                 /**< Progressive frame */
		PIC_STRUCT_TOP_FIELD,                 /**< Field encoding top field first */
		PIC_STRUCT_BOTTOM_FIELD                  /**< Field encoding bottom field first */
	} PIC_STRUCT;

	typedef enum _USAGE
	{
		USAGE_TRANSCONDING = 0,
		USAGE_ULTRA_LOW_LATENCY,
		USAGE_LOW_LATENCY,
		USAGE_WEBCAM
	} USAGE;

	typedef struct EXP_CLASS _configuration_t
	{
		MEMORY_TYPE mem_type;
		int cs;
		int width;
		int height;
		int bitrate;
		int rc_mode;
		int usage;
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
			: cs(COLOR_SPACE_YV12)
			, width(1280)
			, height(1024)
			, bitrate(4000000)
			, rc_mode(RC_MODE_CBR)
			, usage(USAGE_TRANSCONDING)
			, keyframe_interval(2)
			, profile(CODEC_PROFILE_TYPE_HIGH)
			, fps(60)
			, preset(PRESET_TYPE_QUALITY)
			, vbv_max_bitrate(4000000)
			, vbv_size(4000000)
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
			bitrate = clone.bitrate;
			rc_mode = clone.rc_mode;
			usage = clone.usage;
			keyframe_interval = clone.keyframe_interval;
			profile = clone.profile;
			fps = clone.fps;
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
			bitrate = clone.bitrate;
			rc_mode = clone.rc_mode;
			usage = clone.usage;
			keyframe_interval = clone.keyframe_interval;
			profile = clone.profile;
			fps = clone.fps;
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

	dk_vce_encoder(void);
	~dk_vce_encoder(void);

	dk_vce_encoder::ERR_CODE initialize(void * config);
	dk_vce_encoder::ERR_CODE release(void);

	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * rawstream, dk_vce_encoder::dk_video_entity_t * bitstream);
	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * rawstream);
	dk_vce_encoder::ERR_CODE get_queued_data(dk_vce_encoder::dk_video_entity_t * bitstream);

	//dk_vce_encoder::ERR_CODE encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, dk_vce_encoder::PIC_TYPE & pic_type, bool flush = false);

private:
	vce_encoder * _core;


};








#endif