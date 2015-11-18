#include "dk_celt_decoder.h"
#include "celt_decoder.h"

dk_celt_decoder::dk_celt_decoder(void)
{
	_core = new celt_decoder();
}

dk_celt_decoder::~dk_celt_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_celt_decoder::ERR_CODE dk_celt_decoder::initialize_decoder(dk_celt_decoder::configuration_t * config)
{
	return _core->initialize_decoder(config);
}

dk_celt_decoder::ERR_CODE dk_celt_decoder::decode(dk_audio_entity_t * encoded, dk_audio_entity_t * pcm)
{
	return _core->decode(encoded, pcm);
}

/*dk_celt_decoder::ERR_CODE dk_celt_decoder::decode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize)
{
	return _core->decode(input, isize, output, osize);
}*/

dk_celt_decoder::ERR_CODE dk_celt_decoder::release_decoder(void)
{
	return _core->release_decoder();
}