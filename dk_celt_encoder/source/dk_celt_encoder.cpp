#include "dk_celt_encoder.h"
#include "celt_encoder.h"

dk_celt_encoder::dk_celt_encoder(void)
{
	_core = new celt_encoder(this);
}

dk_celt_encoder::~dk_celt_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::initialize_encoder(dk_celt_encoder::configuration_t * config)
{
	return _core->initialize_encoder(config);
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded)
{
	//return _core->encode((int16_t*)pcm->data, pcm->data_size, (uint8_t*)encoded->data, encoded->data_size);
	return _core->encode(pcm, encoded);
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::encode(dk_audio_entity_t * pcm)
{
	return _core->encode(pcm);
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::get_queued_data(dk_audio_entity_t * encoded)
{
	return _core->get_queued_data(encoded);
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::release_encoder(void)
{
	return _core->release_encoder();
}