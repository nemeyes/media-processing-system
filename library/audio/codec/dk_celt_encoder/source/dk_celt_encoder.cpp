#include "dk_celt_encoder.h"
#include "celt_encoder.h"

debuggerking::celt_encoder::_configuration_t::_configuration_t(void)
	: codingrate(48000)
	, framesize(20)
	, complexity(10)
{
}

debuggerking::celt_encoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
	codingrate = clone.codingrate;
	framesize = clone.framesize;
	complexity = clone.complexity;
}

debuggerking::celt_encoder::_configuration_t debuggerking::celt_encoder::_configuration_t::operator=(const _configuration_t & clone)
{
	codingrate = clone.codingrate;
	framesize = clone.framesize;
	complexity = clone.complexity;
	return (*this);
}


debuggerking::celt_encoder::celt_encoder(void)
{
	_core = new celt_core(this);
}

debuggerking::celt_encoder::~celt_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t debuggerking::celt_encoder::initialize_encoder(void * config)
{
	int32_t status = audio_base::initialize(static_cast<audio_base::configuration_t*>(config));
	if (status != celt_encoder::err_code_t::success)
		return status;
	return _core->initialize_encoder(static_cast<celt_encoder::configuration_t*>(config));
}

int32_t debuggerking::celt_encoder::release_encoder(void)
{
	int32_t status = _core->release_encoder();
	if (status != celt_encoder::err_code_t::success)
		return status;
	return audio_base::release();
}

int32_t debuggerking::celt_encoder::encode(celt_encoder::entity_t * pcm, celt_encoder::entity_t * encoded)
{
	return _core->encode(pcm, encoded);
}

int32_t debuggerking::celt_encoder::encode(celt_encoder::entity_t * pcm)
{
	return _core->encode(pcm);
}

int32_t debuggerking::celt_encoder::get_queued_data(celt_encoder::entity_t * encoded)
{
	return _core->get_queued_data(encoded);
}

void debuggerking::celt_encoder::after_encoding_callback(uint8_t * bistream, size_t size)
{

}