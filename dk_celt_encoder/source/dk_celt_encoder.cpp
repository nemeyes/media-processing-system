#include "dk_celt_encoder.h"
#include "celt_encoder.h"

dk_celt_encoder::dk_celt_encoder(void)
{
	_core = new celt_encoder();
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
	return _core->initialize(config);
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded)
{
	return _core->encode((int16_t*)pcm->data, pcm->data_size, (uint8_t*)encoded->data, encoded->data_size);
}
//
//dk_celt_encoder::ERR_CODE dk_celt_encoder::encode(int16_t * input, size_t isize, uint8_t * output, size_t & osize)
//{
//	
//}

dk_celt_encoder::ERR_CODE dk_celt_encoder::release_encoder(void)
{
	return _core->release();
}