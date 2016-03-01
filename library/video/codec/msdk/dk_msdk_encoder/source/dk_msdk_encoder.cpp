#include "dk_msdk_encoder.h"

dk_msdk_encoder::_configuration_t::_configuration_t(void)
	: mem_type(dk_msdk_encoder::MEMORY_TYPE_HOST)
	, cs(dk_msdk_encoder::SUBMEDIA_TYPE_YV12)
	, width(1280)
	, height(1024)
	, bitrate(4000000)
	, peak_bitrate(4000000)
	, vbv_max_bitrate(4000000)
	, vbv_size(4000000)
	, rc_mode(dk_msdk_encoder::RC_MODE_CBR)
	, usage(dk_msdk_encoder::USAGE_TRANSCONDING)
	, keyframe_interval(2)
	, codec(dk_msdk_encoder::SUBMEDIA_TYPE_H264_HP)
	, fps(60)
	, preset(dk_msdk_encoder::PRESET_TYPE_QUALITY)
	, numb(0)
	, enable_4k(0)
	, quality(0)
{
}

dk_msdk_encoder::_configuration_t::_configuration_t(const _configuration_t & clone)
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
	codec = clone.codec;
	fps = clone.fps;
	preset = clone.preset;
	numb = clone.numb;
	enable_4k = clone.enable_4k;
	quality = clone.quality;
}

dk_msdk_encoder::_configuration_t dk_msdk_encoder::_configuration_t::operator=(const dk_msdk_encoder::_configuration_t & clone)
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
	codec = clone.codec;
	fps = clone.fps;
	preset = clone.preset;
	numb = clone.numb;
	enable_4k = clone.enable_4k;
	quality = clone.quality;
	return (*this);
}


dk_msdk_encoder::dk_msdk_encoder(void)
{

}

dk_msdk_encoder::~dk_msdk_encoder(void)
{

}

dk_msdk_encoder::ENCODER_STATE dk_msdk_encoder::state(void)
{

}

dk_msdk_encoder::ERR_CODE dk_msdk_encoder::initialize_encoder(void * config)
{

}

dk_msdk_encoder::ERR_CODE dk_msdk_encoder::release_encoder(void)
{

}

dk_msdk_encoder::ERR_CODE dk_msdk_encoder::encode(dk_msdk_encoder::dk_video_entity_t * input, dk_msdk_encoder::dk_video_entity_t * bitstream)
{

}

dk_msdk_encoder::ERR_CODE dk_msdk_encoder::encode(dk_msdk_encoder::dk_video_entity_t * input)
{

}

dk_msdk_encoder::ERR_CODE dk_msdk_encoder::get_queued_data(dk_msdk_encoder::dk_video_entity_t * input)
{

}

dk_msdk_encoder::ERR_CODE dk_msdk_encoder::encode_async(dk_msdk_encoder::dk_video_entity_t * input)
{

}

dk_msdk_encoder::ERR_CODE dk_msdk_encoder::check_encoding_finish(void)
{

}