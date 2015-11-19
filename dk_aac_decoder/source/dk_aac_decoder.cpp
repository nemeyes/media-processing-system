#include "dk_aac_decoder.h"
#include "faad2_decoder.h"

dk_aac_decoder::_configuration_t::_configuration_t(void)
	: object_type(dk_aac_decoder::AAC_OBJECT_TYPE_SSR)
	, samplerate(0)
	, channels(2)
	, bitdepth(16)
	, bitrate(0)
	, input_format(dk_aac_decoder::FORMAT_TYPE_RAW)
	, output_format(dk_aac_decoder::FORMAT_TYPE_16BIT)
	, extradata_size(0)
	, mix_down(false)
{
	memset(extradata, 0x00, sizeof(extradata));
}

dk_aac_decoder::_configuration_t::_configuration_t(const _configuration_t & clone)
{
	object_type = clone.object_type;
	samplerate = clone.samplerate;
	channels = clone.channels;
	bitdepth = clone.bitdepth;
	input_format = clone.input_format;
	output_format = clone.output_format;
	extradata_size = clone.extradata_size;
	memcpy(extradata, clone.extradata, extradata_size);
}

dk_aac_decoder::_configuration_t dk_aac_decoder::_configuration_t::operator=(const _configuration_t & clone)
{
	object_type = clone.object_type;
	samplerate = clone.samplerate;
	channels = clone.channels;
	bitdepth = clone.bitdepth;
	input_format = clone.input_format;
	output_format = clone.output_format;
	extradata_size = clone.extradata_size;
	memcpy(extradata, clone.extradata, extradata_size);
	return (*this);
}


dk_aac_decoder::dk_aac_decoder(void)
{
	_core = new faad2_decoder();
}

dk_aac_decoder::~dk_aac_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_aac_decoder::ERR_CODE dk_aac_decoder::initialize_decoder(void * config)
{
	return _core->initialize_decoder(static_cast<dk_aac_decoder::configuration_t*>(config));
}

dk_aac_decoder::ERR_CODE dk_aac_decoder::release_decoder(void)
{
	return _core->release_decoder();
}

dk_aac_decoder::ERR_CODE dk_aac_decoder::decode(dk_audio_entity_t * encoded, dk_audio_entity_t * pcm)
{
	return _core->decode(encoded, pcm);
}


/*dk_aac_decoder::ERR_CODE dk_aac_decoder::initialize(dk_aac_decoder::configuration_t config, unsigned char * extra_data, int extra_data_size, int & samplerate, int & channels)
{
	return _core->initialize(config, extra_data, extra_data_size, samplerate, channels);
}

dk_aac_decoder::ERR_CODE dk_aac_decoder::release(void)
{
	return _core->release();
}

dk_aac_decoder::ERR_CODE dk_aac_decoder::decode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int & osize)
{
	return _core->decode(input, isize, output, osize);
}*/