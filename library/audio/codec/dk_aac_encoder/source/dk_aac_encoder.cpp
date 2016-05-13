#include "dk_aac_encoder.h"
#include "faac_encoder.h"

debuggerking::aac_encoder::_configuration_t::_configuration_t(void)
	: mpeg_version(debuggerking::aac_encoder::version_type_mpeg4)
	, object_type(debuggerking::aac_encoder::aac_object_type_low)
	, allow_midside(0)
	, use_lfe(0)
	, use_tns(0)
	, bitdepth(32)
	, framesize(0)
	, ob(0)
	, bandwidth(0)
	, quantization_quality(100)
	, shortctl(block_type_normal)
	, input_format(debuggerking::aac_encoder::format_type_16bit)
	, output_format(debuggerking::aac_encoder::format_type_raw)
{
}

debuggerking::aac_encoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
	mpeg_version = clone.mpeg_version;
	object_type = clone.object_type;
	allow_midside = clone.allow_midside;
	use_lfe = clone.use_lfe;
	use_tns = clone.use_tns;
	bitdepth = clone.bitdepth;
	framesize = clone.framesize;
	ob = clone.ob;
	bandwidth = clone.bandwidth;
	quantization_quality = clone.quantization_quality;
	shortctl = clone.shortctl;
	input_format = clone.input_format;
	output_format = clone.output_format;
}

debuggerking::aac_encoder::_configuration_t debuggerking::aac_encoder::_configuration_t::operator = (const _configuration_t & clone)
{
	mpeg_version = clone.mpeg_version;
	object_type = clone.object_type;
	allow_midside = clone.allow_midside;
	use_lfe = clone.use_lfe;
	use_tns = clone.use_tns;
	bitdepth = clone.bitdepth;
	framesize = clone.framesize;
	ob = clone.ob;
	bandwidth = clone.bandwidth;
	quantization_quality = clone.quantization_quality;
	shortctl = clone.shortctl;
	input_format = clone.input_format;
	output_format = clone.output_format;
	return (*this);
}

debuggerking::aac_encoder::aac_encoder(void)
{
	_core = new faac_encoder(this);
}

debuggerking::aac_encoder::~aac_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

int32_t debuggerking::aac_encoder::initialize_encoder(void * config)
{
	int32_t status = audio_base::initialize(static_cast<audio_base::configuration_t*>(config));
	if (status != aac_encoder::err_code_t::success)
		return status;
	return _core->initialize_encoder(static_cast<aac_encoder::configuration_t*>(config));
}

int32_t debuggerking::aac_encoder::release_encoder(void)
{
	int32_t status = _core->release_encoder();
	if (status != aac_encoder::err_code_t::success)
		return status;
	return audio_base::release();
}

int32_t debuggerking::aac_encoder::encode(aac_encoder::entity_t * pcm, aac_encoder::entity_t * encoded)
{
	return _core->encode(pcm, encoded);
}

int32_t debuggerking::aac_encoder::encode(aac_encoder::entity_t * pcm)
{
	return _core->encode(pcm);
}

int32_t debuggerking::aac_encoder::get_queued_data(aac_encoder::entity_t * encoded)
{
	return _core->get_queued_data(encoded);
}

void debuggerking::aac_encoder::after_encoding_callback(uint8_t * bistream, size_t size)
{

}
