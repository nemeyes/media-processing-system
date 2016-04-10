#include "dk_ff_mp3_decoder.h"
#include "mp3_decoder.h"

dk_ff_mp3_decoder::dk_ff_mp3_decoder(void)
{
	_core = new mp3_decoder(this);
}

dk_ff_mp3_decoder::~dk_ff_mp3_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_ff_mp3_decoder::err_code dk_ff_mp3_decoder::initialize_decoder(void * config)
{
	return _core->initialize_decoder(static_cast<dk_ff_mp3_decoder::configuration_t*>(config));
}

dk_ff_mp3_decoder::err_code dk_ff_mp3_decoder::decode(dk_ff_mp3_decoder::dk_audio_entity_t * encoded, dk_ff_mp3_decoder::dk_audio_entity_t * pcm)
{
	return _core->decode(encoded, pcm);
}

dk_ff_mp3_decoder::err_code dk_ff_mp3_decoder::release_decoder(void)
{
	return _core->release_decoder();
}