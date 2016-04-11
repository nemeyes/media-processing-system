#include "dk_celt_encoder.h"
#include "celt_encoder.h"

dk_celt_encoder::_configuration_t::_configuration_t(void)
	: codingrate(48000)
	, framesize(20)
	, complexity(10)
{
}

dk_celt_encoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
	codingrate = clone.codingrate;
	framesize = clone.framesize;
	complexity = clone.complexity;
}

dk_celt_encoder::_configuration_t dk_celt_encoder::_configuration_t::operator=(const _configuration_t & clone)
{
	codingrate = clone.codingrate;
	framesize = clone.framesize;
	complexity = clone.complexity;
	return (*this);
}


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

dk_celt_encoder::err_code dk_celt_encoder::initialize_encoder(void * config)
{
	return _core->initialize_encoder(static_cast<dk_celt_encoder::configuration_t*>(config));
}

dk_celt_encoder::err_code dk_celt_encoder::encode(dk_celt_encoder::dk_audio_entity_t * pcm, dk_celt_encoder::dk_audio_entity_t * encoded)
{
	return _core->encode(pcm, encoded);
}

dk_celt_encoder::err_code dk_celt_encoder::encode(dk_celt_encoder::dk_audio_entity_t * pcm)
{
	return _core->encode(pcm);
}

dk_celt_encoder::err_code dk_celt_encoder::get_queued_data(dk_celt_encoder::dk_audio_entity_t * encoded)
{
	return _core->get_queued_data(encoded);
}

dk_celt_encoder::err_code dk_celt_encoder::release_encoder(void)
{
	return _core->release_encoder();
}