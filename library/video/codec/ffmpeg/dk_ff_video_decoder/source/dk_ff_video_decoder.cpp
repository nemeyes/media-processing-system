#include "dk_ff_video_decoder.h"
#include "ffmpeg_decoder.h"

dk_ff_video_decoder::dk_ff_video_decoder(void)
{
	_core = new ffmpeg_decoder(this);
}

dk_ff_video_decoder::~dk_ff_video_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_ff_video_decoder::err_code dk_ff_video_decoder::initialize_decoder(void * config)
{
	dk_ff_video_decoder::err_code status = dk_ff_video_decoder::err_code_fail;
	status = _core->initialize_decoder(static_cast<dk_ff_video_decoder::configuration_t*>(config));
	return status;
}

dk_ff_video_decoder::err_code dk_ff_video_decoder::release_decoder(void)
{
	return _core->release_decoder();
}

dk_ff_video_decoder::err_code dk_ff_video_decoder::decode(dk_ff_video_decoder::dk_video_entity_t * encoded, dk_ff_video_decoder::dk_video_entity_t * decoded)
{
	return _core->decode(encoded, decoded);
}

dk_ff_video_decoder::err_code dk_ff_video_decoder::decode(dk_ff_video_decoder::dk_video_entity_t * encoded)
{
	return _core->decode(encoded);
}

dk_ff_video_decoder::err_code dk_ff_video_decoder::get_queued_data(dk_ff_video_decoder::dk_video_entity_t * decoded)
{
	return _core->get_queued_data(decoded);
}