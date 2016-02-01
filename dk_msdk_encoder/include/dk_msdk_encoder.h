#ifndef _DK_MSDK_ENCODER_H_
#define _DK_MSDK_ENCODER_H_

#include <dk_video_base.h>

class msdk_encoder;
class EXP_CLASS dk_msdk_encoder : public dk_video_encoder
{
public:
	typedef enum _RC_MODE
	{
		RC_MODE_CONSTQP,
		RC_MODE_VBR,
		RC_MODE_CBR,
		RC_MODE_AVBR,
		RC_MODE_LOOK_AHEAD
	} RC_MODE;

	typedef enum _MSDK_API_FEATURE
	{
		MSDK_FEATURE_NONE,
		MSDK_FEATURE_MVC,
		MSDK_FEATURE_JPEG_DECODE,
		MSDK_FEATURE_LOW_LATENCY,
		MSDK_FEATURE_MVC_VIEWOUTPUT,
		MSDK_FEATURE_JPEG_ENCODE,
		MSDK_FEATURE_LOOK_AHEAD,
		MSDK_FEATURE_PLUGIN_API
	} MSDK_API_FEATURE;


	typedef struct EXP_CLASS _configuration_t
	{
		MEMORY_TYPE		mem_type;
		void *			d3d_device;
		SUBMEDIA_TYPE	cs;
		int				width;
		int				height;
		int				bitrate;
		int				peak_bitrate;
		int				vbv_max_bitrate;
		int				vbv_size;
		RC_MODE			rc_mode;
		USAGE			usage;
		int				keyframe_interval;
		SUBMEDIA_TYPE	codec;
		int				fps;
		int				preset;
		int				numb;
		int				slice_per_frame;
		int				enable_4k;
		bool			mvc;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;

	dk_msdk_encoder(void);
	~dk_msdk_encoder(void);

	dk_msdk_encoder::ENCODER_STATE state(void);

	dk_msdk_encoder::ERR_CODE initialize_encoder(void * config);
	dk_msdk_encoder::ERR_CODE release_encoder(void);

	dk_msdk_encoder::ERR_CODE encode(dk_msdk_encoder::dk_video_entity_t * input, dk_msdk_encoder::dk_video_entity_t * bitstream);
	dk_msdk_encoder::ERR_CODE encode(dk_msdk_encoder::dk_video_entity_t * input);
	dk_msdk_encoder::ERR_CODE get_queued_data(dk_msdk_encoder::dk_video_entity_t * input);

	dk_msdk_encoder::ERR_CODE encode_async(dk_msdk_encoder::dk_video_entity_t * input);
	dk_msdk_encoder::ERR_CODE check_encoding_finish(void);


private:
	media_sdk_encoder * _core;

};
















#endif