#include "dk_msdk_encoder.h"

#include <mfxmvc.h>
#include <mfxvideo.h>
#include <mfxvp8.h>
#include <mfxvideo++.h>
#include <mfxplugin.h>
#include <mfxplugin++.h>

class msdk_encoder
{
public:
	msdk_encoder(dk_msdk_encoder * front);
	~msdk_encoder(void);

	dk_msdk_encoder::ENCODER_STATE state(void);

	dk_msdk_encoder::ERR_CODE initialize_encoder(dk_msdk_encoder::configuration_t * config);
	dk_msdk_encoder::ERR_CODE release_encoder(void);

	dk_msdk_encoder::ERR_CODE encode(dk_msdk_encoder::dk_video_entity_t * input, dk_msdk_encoder::dk_video_entity_t * bitstream);
	dk_msdk_encoder::ERR_CODE encode(dk_msdk_encoder::dk_video_entity_t * input);
	dk_msdk_encoder::ERR_CODE get_queued_data(dk_msdk_encoder::dk_video_entity_t * bitstream);

	dk_msdk_encoder::ERR_CODE encode_async(dk_video_encoder::dk_video_entity_t * input);
	dk_msdk_encoder::ERR_CODE check_encoding_finish(void);

private:
	bool check_version(mfxVersion * version, dk_msdk_encoder::MSDK_API_FEATURE feature);


private:
	dk_msdk_encoder::ENCODER_STATE _state;
	dk_msdk_encoder::configuration_t * _config;
	dk_msdk_encoder * _front;

	MFXVideoSession _session;
	MFXVideoENCODE * _encoder;

	std::shared_ptr<MFXVideoUSER> _user_module;
	std::shared_ptr<MFXPlugin> _plugin;
};