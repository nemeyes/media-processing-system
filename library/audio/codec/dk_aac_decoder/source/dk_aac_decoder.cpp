#include "dk_aac_decoder.h"
#include "faad2_decoder.h"

dk_aac_decoder::_configuration_t::_configuration_t(void)
	: object_type(dk_aac_decoder::aac_object_type_ssr)
	, input_format(dk_aac_decoder::format_type_raw)
	, output_format(dk_aac_decoder::format_type_16bit)
	, mix_down(false)
{
}

dk_aac_decoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
	object_type = clone.object_type;
	input_format = clone.input_format;
	output_format = clone.output_format;
}

dk_aac_decoder::_configuration_t dk_aac_decoder::_configuration_t::operator=(const _configuration_t & clone)
{
	object_type = clone.object_type;
	input_format = clone.input_format;
	output_format = clone.output_format;
	return (*this);
}

dk_aac_decoder::dk_aac_decoder(void)
{
	_core = new faad2_decoder(this);
}

dk_aac_decoder::~dk_aac_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_aac_decoder::err_code dk_aac_decoder::initialize_decoder(void * config)
{
	return _core->initialize_decoder(static_cast<dk_aac_decoder::configuration_t*>(config));
}

dk_aac_decoder::err_code dk_aac_decoder::release_decoder(void)
{
	return _core->release_decoder();
}

dk_aac_decoder::err_code dk_aac_decoder::decode(dk_aac_decoder::dk_audio_entity_t * encoded, dk_aac_decoder::dk_audio_entity_t * pcm)
{
	return _core->decode(encoded, pcm);
}