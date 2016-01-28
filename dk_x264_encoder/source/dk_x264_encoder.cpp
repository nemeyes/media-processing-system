#include "dk_x264_encoder.h"
#include "x264_encoder.h"

dk_x264_encoder::dk_x264_encoder(void)
#if defined(WITH_CALLBACK_THREAD)
	: dk_video_encoder(false)
#else
	: dk_video_encoder(true)
#endif
{
	_core = new x264_encoder(this);
}

dk_x264_encoder::~dk_x264_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_x264_encoder::ENCODER_STATE dk_x264_encoder::state(void)
{
	return _core->state();
}

dk_x264_encoder::ERR_CODE dk_x264_encoder::initialize_encoder(void * config)
{
	return _core->initialize_encoder(static_cast<dk_x264_encoder::configuration_t*>(config));
}

dk_x264_encoder::ERR_CODE dk_x264_encoder::release_encoder(void)
{
	return _core->release_encoder();
}

dk_x264_encoder::ERR_CODE dk_x264_encoder::encode(dk_video_encoder::dk_video_entity_t * input, dk_video_encoder::dk_video_entity_t * bitstream)
{
	return _core->encode(input, bitstream);
}

dk_x264_encoder::ERR_CODE dk_x264_encoder::encode(dk_video_encoder::dk_video_entity_t * input)
{
	return _core->encode(input);
}

dk_x264_encoder::ERR_CODE dk_x264_encoder::get_queued_data(dk_video_encoder::dk_video_entity_t * input)
{
	return _core->get_queued_data(input);
}

dk_video_encoder::ERR_CODE dk_x264_encoder::encode_async(dk_video_encoder::dk_video_entity_t * input)
{
	return _core->encode_async(input);
}

dk_video_encoder::ERR_CODE dk_x264_encoder::check_encoding_finish(void)
{
	return _core->check_encoding_finish();
}