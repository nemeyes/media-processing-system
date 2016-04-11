#include "dk_aac_encoder.h"
#include "faac_encoder.h"

dk_aac_encoder::_configuration_t::_configuration_t(void)
	: mpeg_version(dk_aac_encoder::version_type_mpeg4)
	, object_type(dk_aac_encoder::aac_object_type_low)
	, allow_midside(0)
	, use_lfe(0)
	, use_tns(0)
	, bitdepth(32)
	, framesize(0)
	, ob(0)
	, bandwidth(0)
	, quantization_quality(100)
	, shortctl(block_type_normal)
	, input_format(dk_aac_encoder::format_type_16bit)
	, output_format(dk_aac_encoder::format_type_raw)
{
}

dk_aac_encoder::_configuration_t::_configuration_t(const _configuration_t & clone)
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

dk_aac_encoder::_configuration_t dk_aac_encoder::_configuration_t::operator = (const _configuration_t & clone)
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

dk_aac_encoder::dk_aac_encoder(void)
{
	_core = new faac_encoder(this);
}

dk_aac_encoder::~dk_aac_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_aac_encoder::err_code dk_aac_encoder::initialize_encoder(void * config)
{
	return _core->initialize_encoder(static_cast<dk_aac_encoder::configuration_t*>(config));
}

dk_aac_encoder::err_code dk_aac_encoder::release_encoder(void)
{
	return _core->release_encoder();
}

dk_aac_encoder::err_code dk_aac_encoder::encode(dk_aac_encoder::dk_audio_entity_t * pcm, dk_aac_encoder::dk_audio_entity_t * encoded)
{
	return _core->encode(pcm, encoded);
}

dk_aac_encoder::err_code dk_aac_encoder::encode(dk_aac_encoder::dk_audio_entity_t * pcm)
{
	return _core->encode(pcm);
}

dk_aac_encoder::err_code dk_aac_encoder::get_queued_data(dk_aac_encoder::dk_audio_entity_t * encoded)
{
	return _core->get_queued_data(encoded);
}

uint8_t * dk_aac_encoder::extradata(void)
{
	return _core->extradata();
}

size_t dk_aac_encoder::extradata_size(void)
{
	return _core->extradata_size();
}


