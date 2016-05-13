#include "dk_celt_decoder.h"
#include "celt_decoder.h"

debuggerking::celt_decoder::_configuration_t::_configuration_t(void)
	: framesize(20)
{
}

debuggerking::celt_decoder::_configuration_t::_configuration_t(const celt_decoder::_configuration_t & clone)
{
	framesize = clone.framesize;
}

debuggerking::celt_decoder::_configuration_t debuggerking::celt_decoder::_configuration_t::operator = (const celt_decoder::_configuration_t & clone)
{
	framesize = clone.framesize;
	return (*this);
}

debuggerking::celt_decoder::celt_decoder(void)
{
	_core = new celt_core(this);
}

debuggerking::celt_decoder::~celt_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t debuggerking::celt_decoder::initialize_decoder(void * config)
{
	int32_t status = celt_decoder::initialize(static_cast<audio_base::configuration_t*>(config));
	if (status != celt_decoder::err_code_t::success)
		return status;
	return _core->initialize_decoder(static_cast<celt_decoder::configuration_t*>(config));
}


int32_t debuggerking::celt_decoder::release_decoder(void)
{
	int32_t status = _core->release_decoder();
	if (status != celt_decoder::err_code_t::success)
		return status;
	return audio_base::release();
}

int32_t debuggerking::celt_decoder::decode(celt_decoder::entity_t * encoded, celt_decoder::entity_t * pcm)
{
	return _core->decode(encoded, pcm);
}

int32_t debuggerking::celt_decoder::decode(celt_decoder::entity_t * encoded)
{
	return _core->decode(encoded);
}

int32_t debuggerking::celt_decoder::get_queued_data(celt_decoder::entity_t * pcm)
{
	return _core->get_queued_data(pcm);
}

void debuggerking::celt_decoder::after_decoding_callback(uint8_t * decoded, size_t size)
{

}

