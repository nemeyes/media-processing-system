#include "dk_aac_encoder.h"
#include "faac_encoder.h"

dk_aac_encoder::_configuration_t::_configuration_t(void)
	: mpeg_version(dk_aac_encoder::VERSION_TYPE_MPEG4)
	, object_type(dk_aac_encoder::AAC_OBJECT_TYPE_LOW)
	, allow_midside(0)
	, use_lfe(0)
	, use_tns(0)
	, bitrate(128000)
	, samplerate(0)
	, channels(2)
	, bitdepth(32)
	, framesize(0)
	, ob(0)
	, bandwidth(0)
	, quantization_quality(100)
	, shortctl(BLOCK_TYPE_NORMAL)
	, input_format(dk_aac_encoder::FORMAT_TYPE_16BIT)
	, output_format(dk_aac_encoder::FORMAT_TYPE_RAW)
{
}

dk_aac_encoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
	mpeg_version = clone.mpeg_version;
	object_type = clone.object_type;
	allow_midside = clone.allow_midside;
	use_lfe = clone.use_lfe;
	use_tns = clone.use_tns;
	bitrate = clone.bitrate;
	samplerate = clone.samplerate;
	channels = clone.channels;
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
	bitrate = clone.bitrate;
	samplerate = clone.samplerate;
	channels = clone.channels;
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

dk_aac_encoder::ERR_CODE dk_aac_encoder::initialize_encoder(void * config)
{
	return _core->initialize_encoder(static_cast<dk_aac_encoder::configuration_t*>(config));
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::release_encoder(void)
{
	return _core->release_encoder();
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::encode(dk_aac_encoder::dk_audio_entity_t * pcm, dk_aac_encoder::dk_audio_entity_t * encoded)
{
	return _core->encode(pcm, encoded);
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::encode(dk_aac_encoder::dk_audio_entity_t * pcm)
{
	return _core->encode(pcm);
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::get_queued_data(dk_aac_encoder::dk_audio_entity_t * encoded)
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

/*dk_aac_encoder::ERR_CODE dk_aac_encoder::initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, uint8_t * extra_data, size_t & extra_data_size)
{
	return _core->initialize(config, input_samples, max_output_bytes, extra_data, extra_data_size);
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::release(void)
{
	return _core->release();
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::encode(int32_t * input, size_t isize, uint8_t * output, size_t osize, size_t & bytes_written)
{
	return _core->encode(input, isize, output, osize, bytes_written);
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::encode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize)
{
	return _core->encode(input, isize, output, osize);
}*/



