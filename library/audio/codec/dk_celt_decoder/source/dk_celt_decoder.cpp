#include "dk_celt_decoder.h"
#include "celt_decoder.h"

dk_celt_decoder::_configuration_t::_configuration_t(void)
	: framesize(20)
{
}

dk_celt_decoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
	framesize = clone.framesize;
}

dk_celt_decoder::_configuration_t dk_celt_decoder::_configuration_t::operator = (const _configuration_t & clone)
{
	framesize = clone.framesize;
	return (*this);
}

dk_celt_decoder::dk_celt_decoder(void)
{
	_core = new celt_decoder(this);
}

dk_celt_decoder::~dk_celt_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_celt_decoder::err_code dk_celt_decoder::initialize_decoder(void * config)
{
	return _core->initialize_decoder(static_cast<dk_celt_decoder::configuration_t*>(config));
}

dk_celt_decoder::err_code dk_celt_decoder::decode(dk_celt_decoder::dk_audio_entity_t * encoded, dk_celt_decoder::dk_audio_entity_t * pcm)
{
	return _core->decode(encoded, pcm);
}

dk_celt_decoder::err_code dk_celt_decoder::release_decoder(void)
{
	return _core->release_decoder();
}