#include "dk_ac3_encoder.h"
#include "aften_encoder.h"

dk_ac3_encoder::_configuration_t::_configuration_t(void)
{
}

dk_ac3_encoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
}

dk_ac3_encoder::_configuration_t dk_ac3_encoder::_configuration_t::operator = (const _configuration_t & clone)
{
	return (*this);
}

dk_ac3_encoder::dk_ac3_encoder(void)
{
	_core = new aften_encoder(this);
}

dk_ac3_encoder::~dk_ac3_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_ac3_encoder::ERR_CODE dk_ac3_encoder::initialize_encoder(void * config)
{
	return _core->initialize_encoder(static_cast<dk_ac3_encoder::configuration_t*>(config));
}

dk_ac3_encoder::ERR_CODE dk_ac3_encoder::release_encoder(void)
{
	return _core->release_encoder();
}

dk_ac3_encoder::ERR_CODE dk_ac3_encoder::encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded)
{
	return _core->encode(pcm, encoded);
}

dk_ac3_encoder::ERR_CODE dk_ac3_encoder::encode(dk_audio_entity_t * pcm)
{
	return _core->encode(pcm);
}

dk_ac3_encoder::ERR_CODE dk_ac3_encoder::get_queued_data(dk_audio_entity_t * encoded)
{
	return _core->get_queued_data(encoded);
}
