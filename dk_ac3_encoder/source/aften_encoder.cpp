#include "aften_encoder.h"


aften_encoder::aften_encoder(dk_ac3_encoder * front)
	: _front(front)
	//, _extradata(nullptr)
	//, _extradata_size(0)
	, _buffer(0)
	, _buffer_index(0)
	, _buffer4queue(nullptr)
{
	_buffer = static_cast<int*>(malloc(10 * 2 * 48000 * sizeof(int)));
}

aften_encoder::~aften_encoder(void)
{
	if (_buffer)
	{
		free(_buffer);
		_buffer = 0;
	}
	_buffer_index = 0;
}

dk_ac3_encoder::ERR_CODE aften_encoder::initialize_encoder(dk_ac3_encoder::configuration_t * config)
{
	aften_set_defaults(&_aften_context);


	_aften_encoder = faacEncOpen(config->samplerate, config->channels, &config->framesize, &config->ob);
	if (!_aften_encoder)
		return dk_ac3_encoder::ERR_CODE_FAILED;

	// set encoder configuration
	faacEncConfiguration * _aften_encoder_config = faacEncGetCurrentConfiguration(_aften_encoder);
	switch (config->mpeg_version)
	{
	case dk_ac3_encoder::VERSION_TYPE_MPEG2:
		_aften_encoder_config->mpegVersion = MPEG2;
		break;
	case dk_ac3_encoder::VERSION_TYPE_MPEG4:
		_aften_encoder_config->mpegVersion = MPEG4;
		break;
	default:
		_aften_encoder_config->mpegVersion = MPEG4;
		break;
	}
	switch (config->object_type)
	{
	case dk_ac3_encoder::AAC_OBJECT_TYPE_MAIN:
		_aften_encoder_config->aacObjectType = MAIN;
		break;
	case dk_ac3_encoder::AAC_OBJECT_TYPE_LOW:
		_aften_encoder_config->aacObjectType = LOW;
		break;
	case dk_ac3_encoder::AAC_OBJECT_TYPE_SSR:
		_aften_encoder_config->aacObjectType = SSR;
		break;
	case dk_ac3_encoder::AAC_OBJECT_TYPE_LTP:
		_aften_encoder_config->aacObjectType = LTP;
		break;
	}

	_aften_encoder_config->allowMidside = config->allow_midside;
	_aften_encoder_config->useTns = config->use_tns;

	if (config->channels > 6)
		_aften_encoder_config->useLfe = 1;
	else
		_aften_encoder_config->useLfe = config->use_lfe;

	_aften_encoder_config->quantqual = config->quantization_quality;

	switch (config->shortctl)
	{
	case dk_ac3_encoder::BLOCK_TYPE_NORMAL:
		_aften_encoder_config->shortctl = SHORTCTL_NORMAL;
		break;
	case dk_ac3_encoder::BLOCK_TYPE_NOSHORT:
		_aften_encoder_config->shortctl = SHORTCTL_NOSHORT;
		break;
	case dk_ac3_encoder::BLOCK_TYPE_NOLONG:
		_aften_encoder_config->shortctl = SHORTCTL_NOLONG;
		break;
	default:
		_aften_encoder_config->shortctl = SHORTCTL_NORMAL;
		break;
	}

	switch (config->input_format)
	{
	case dk_ac3_encoder::FORMAT_TYPE_16BIT:
		_aften_encoder_config->inputFormat = FAAC_INPUT_16BIT;
		break;
	case dk_ac3_encoder::FORMAT_TYPE_24BIT:
		_aften_encoder_config->inputFormat = FAAC_INPUT_24BIT;
		break;
	case dk_ac3_encoder::FORMAT_TYPE_32BIT:
		_aften_encoder_config->inputFormat = FAAC_INPUT_32BIT;
		break;
	case dk_ac3_encoder::FORMAT_TYPE_FLOAT:
		_aften_encoder_config->inputFormat = FAAC_INPUT_FLOAT;
		break;
	default:
		_aften_encoder_config->inputFormat = FAAC_INPUT_16BIT;
		break;
	}

	switch (config->output_format)
	{
	case dk_ac3_encoder::FORMAT_TYPE_RAW:
		_aften_encoder_config->outputFormat = 0;
		break;
	case dk_ac3_encoder::FORMAT_TYPE_ADTS:
		_aften_encoder_config->outputFormat = 1;
		break;
	case dk_ac3_encoder::FORMAT_TYPE_LATM:
		_aften_encoder_config->outputFormat = 2;
		break;
	default:
		_aften_encoder_config->outputFormat = 0;
		break;
	}

	_aften_encoder_config->bitRate = config->bitrate;

	if (!faacEncSetConfiguration(_aften_encoder, _aften_encoder_config))
	{
		faacEncClose(_aften_encoder);
		_aften_encoder = 0;
		return dk_ac3_encoder::ERR_CODE_FAILED;
	}

	_extradata = nullptr;
	_extradata_size = 0;
	faacEncGetDecoderSpecificInfo(_aften_encoder, &_extradata, &_extradata_size);

	_buffer4queue = (uint8_t*)malloc(config->ob);

	_config = *config;
	_buffer_index = 0;
	return dk_ac3_encoder::ERR_CODE_SUCCESS;
}

dk_ac3_encoder::ERR_CODE aften_encoder::release_encoder(void)
{
	if (_buffer4queue)
	{
		free(_buffer4queue);
		_buffer4queue = nullptr;
	}

	if (_aften_encoder)
	{
		faacEncClose(_aften_encoder);
		_aften_encoder = 0;
	}
	_buffer_index = 0;
	return dk_ac3_encoder::ERR_CODE_SUCCESS;
}

dk_ac3_encoder::ERR_CODE aften_encoder::encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded)
{
#if 0	
	int32_t bytes_written = faacEncEncode(_aften_encoder, (int32_t*)pcm->data, pcm->data_size, (uint8_t*)encoded->data, (unsigned int)encoded->data_capacity);
	if (bytes_written > 0)
	{
		encoded->data_size = bytes_written;
		return dk_ac3_encoder::ERR_CODE_SUCCESS;
	}
	else
	{
		encoded->data_size = 0;
		return dk_ac3_encoder::ERR_CODE_FAILED;
	}

#else
	encoded->data_size = 0;
	if (!_aften_encoder || _config.bitdepth != 16)
		return dk_ac3_encoder::ERR_CODE_FAILED;

	memcpy((int16_t*)_buffer + _buffer_index, pcm->data, pcm->data_size);
	_buffer_index += (pcm->data_size >> 1);

	unsigned long cur_buffer_index = 0;
	int16_t * cur_buffer = (int16_t*)_buffer;
	while (cur_buffer_index + _config.framesize <= _buffer_index)
	{
		int bytes_written = faacEncEncode(_aften_encoder, (int*)cur_buffer, _config.framesize, (uint8_t*)encoded->data + encoded->data_size, _config.ob);
		if (bytes_written > 0)
			encoded->data_size += bytes_written;
		cur_buffer_index += _config.framesize;
		cur_buffer += _config.framesize;
	}

	if (cur_buffer_index > 0)
	{
		int left = _buffer_index - cur_buffer_index;
		if (left > 0)
			memmove(_buffer, ((short*)_buffer) + cur_buffer_index, left*sizeof(short));
		_buffer_index = left;
	}
#endif
	return dk_ac3_encoder::ERR_CODE_SUCCESS;
}

dk_ac3_encoder::ERR_CODE aften_encoder::encode(dk_audio_entity_t * pcm)
{
	if (!_aften_encoder || _config.bitdepth != 16)
		return dk_ac3_encoder::ERR_CODE_FAILED;

	memcpy(((uint16_t*)_buffer) + _buffer_index, pcm->data, pcm->data_size);
	_buffer_index += (pcm->data_size >> 1);

	unsigned long cur_buffer_index = 0;
	short * cur_buffer = (int16_t*)_buffer;
	//size_t buffer4queue_pos = 0;
	while (cur_buffer_index + _config.framesize <= _buffer_index)
	{
		int bytes_written = faacEncEncode(_aften_encoder, (int*)cur_buffer, _config.framesize, _buffer4queue, _config.ob);
		if (bytes_written > 0)
		{
			if (_front)
				_front->push(_buffer4queue, bytes_written);
		}
		cur_buffer_index += _config.framesize;
		cur_buffer += _config.framesize;
	}

	if (cur_buffer_index > 0)
	{
		int left = _buffer_index - cur_buffer_index;
		if (left > 0)
			memmove(_buffer, ((int16_t*)_buffer) + cur_buffer_index, left*sizeof(int16_t));
		_buffer_index = left;
	}
	return dk_ac3_encoder::ERR_CODE_SUCCESS;
}

dk_ac3_encoder::ERR_CODE aften_encoder::get_queued_data(dk_audio_entity_t * encoded)
{
	if (_front)
	{
		return _front->pop((uint8_t*)encoded->data, encoded->data_size);
	}
	else
	{
		return dk_ac3_encoder::ERR_CODE_FAILED;
	}
}

uint8_t * aften_encoder::extradata(void)
{
	return _extradata;
}

size_t aften_encoder::extradata_size(void)
{
	return (size_t)_extradata_size;
}
