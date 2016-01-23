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
		void * d3d_device;
		int cs;
		int width;
		int height;
		int bitrate;
		int peak_bitrate;
		int vbv_max_bitrate;
		int vbv_size;
		int rc_mode;
		int usage;
		int keyframe_interval;
		int profile;
		int fps;
		int preset;
		int numb;
		int slice_per_frame;
		int enable_4k;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;

	dk_vce_encoder(void);
	~dk_vce_encoder(void);

	dk_vce_encoder::ENCODER_STATE state(void);

	dk_vce_encoder::ERR_CODE initialize_encoder(void * config);
	dk_vce_encoder::ERR_CODE release_encoder(void);

	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * input, dk_vce_encoder::dk_video_entity_t * bitstream);
	dk_vce_encoder::ERR_CODE encode(dk_vce_encoder::dk_video_entity_t * input);
	dk_vce_encoder::ERR_CODE get_queued_data(dk_vce_encoder::dk_video_entity_t * input);

	dk_vce_encoder::ERR_CODE encode_async(dk_vce_encoder::dk_video_entity_t * input);
	dk_vce_encoder::ERR_CODE check_encoding_flnish(void);
private:
	vce_encoder * _core;


};








#endif