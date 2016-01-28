#include "dk_vce_encoder.h"
#include "vce_encoder.h"

dk_vce_encoder::_configuration_t::_configuration_t(void)
	: mem_type(dk_vce_encoder::MEMORY_TYPE_HOST)
	, cs(SUBMEDIA_TYPE_YV12)
	, width(1280)
	, height(1024)
	, bitrate(4000000)
	, peak_bitrate(4000000)
	, vbv_max_bitrate(4000000)
	, vbv_size(4000000)
	, rc_mode(dk_vce_encoder::RC_MODE_CBR)
	, usage(dk_vce_encoder::USAGE_TRANSCONDING)
	, keyframe_interval(2)
	, profile(dk_vce_encoder::CODEC_PROFILE_TYPE_HIGH)
	, fps(60)
	, preset(dk_vce_encoder::PRESET_TYPE_QUALITY)
	, numb(0)
	, enable_4k(0)
{
}

dk_vce_encoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
	mem_type = clone.mem_type;
	cs = clone.cs;
	width = clone.width;
	height = clone.height;
	bitrate = clone.bitrate;
	peak_bitrate = clone.peak_bitrate;
	vbv_max_bitrate = clone.vbv_max_bitrate;
	vbv_size = clone.vbv_size;
	rc_mode = clone.rc_mode;
	usage = clone.usage;
	keyframe_interval = clone.keyframe_interval;
	profile = clone.profile;
	fps = clone.fps;
	preset = clone.preset;
	numb = clone.numb;
	enable_4k = clone.enable_4k;
}

dk_vce_encoder::_configuration_t dk_vce_encoder::_configuration_t::operator=(const dk_vce_encoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	cs = clone.cs;
	width = clone.width;
	height = clone.height;
	bitrate = clone.bitrate;
	peak_bitrate = clone.peak_bitrate;
	vbv_max_bitrate = clone.vbv_max_bitrate;
	vbv_size = clone.vbv_size;
	rc_mode = clone.rc_mode;
	usage = clone.usage;
	keyframe_interval = clone.keyframe_interval;
	profile = clone.profile;
	fps = clone.fps;
	preset = clone.preset;
	numb = clone.numb;
	enable_4k = clone.enable_4k;
	return (*this);
}

dk_vce_encoder::dk_vce_encoder(void)
#if defined(WITH_AMF_CALLBACK_THREAD)
	: dk_video_encoder(false)
#else
	: dk_video_encoder(true)
#endif
{
	_core = new vce_encoder(this);
}

dk_vce_encoder::~dk_vce_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_vce_encoder::ENCODER_STATE dk_vce_encoder::state(void)
{
	return _core->state();
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::initialize_encoder(void * config)
{
	return _core->initialize_encoder(static_cast<dk_vce_encoder::configuration_t*>(config));
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::release_encoder(void)
{
	return _core->release_encoder();
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::encode(dk_vce_encoder::dk_video_entity_t * input, dk_vce_encoder::dk_video_entity_t * bitstream)
{
	return _core->encode(input, bitstream);
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::encode(dk_vce_encoder::dk_video_entity_t * input)
{
	return _core->encode(input);
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::get_queued_data(dk_vce_encoder::dk_video_entity_t * bitstream)
{
	return _core->get_queued_data(bitstream);
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::encode_async(dk_video_encoder::dk_video_entity_t * input)
{
	return _core->encode_async(input);
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::check_encoding_finish(void)
{
	return _core->check_encoding_finish();
}