#include "dk_aac_decoder.h"
#include "faad2_decoder.h"

debuggerking::aac_decoder::_configuration_t::_configuration_t(void)
	: object_type(aac_decoder::aac_object_type_ssr)
	, input_format(aac_decoder::format_type_raw)
	, output_format(aac_decoder::format_type_16bit)
	, mix_down(false)
{
}

debuggerking::aac_decoder::_configuration_t::_configuration_t(const aac_decoder::_configuration_t & clone)
{
	object_type = clone.object_type;
	input_format = clone.input_format;
	output_format = clone.output_format;
}

debuggerking::aac_decoder::_configuration_t debuggerking::aac_decoder::_configuration_t::operator=(const aac_decoder::_configuration_t & clone)
{
	object_type = clone.object_type;
	input_format = clone.input_format;
	output_format = clone.output_format;
	return (*this);
}

debuggerking::aac_decoder::aac_decoder(void)
{
	_core = new faad2_decoder(this);
}

debuggerking::aac_decoder::~aac_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

int32_t debuggerking::aac_decoder::initialize_decoder(void * config)
{
	int32_t status = aac_decoder::initialize(static_cast<audio_base::configuration_t*>(config));
	if (status != aac_decoder::err_code_t::success)
		return status;
	return _core->initialize_decoder(static_cast<aac_decoder::configuration_t*>(config));
}

int32_t debuggerking::aac_decoder::release_decoder(void)
{
	int32_t status = _core->release_decoder();
	if (status != aac_decoder::err_code_t::success)
		return status;
	return audio_base::release();
}

int32_t debuggerking::aac_decoder::decode(aac_decoder::entity_t * encoded, aac_decoder::entity_t * pcm)
{
	return _core->decode(encoded, pcm);
}

int32_t debuggerking::aac_decoder::decode(audio_decoder::entity_t * encoded)
{
	return _core->decode(encoded);
}

int32_t debuggerking::aac_decoder::get_queued_data(audio_decoder::entity_t * pcm)
{
	return _core->get_queued_data(pcm);
}

void debuggerking::aac_decoder::after_decoding_callback(uint8_t * decoded, size_t size)
{

}