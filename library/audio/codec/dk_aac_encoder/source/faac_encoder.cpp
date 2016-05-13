#include "faac_encoder.h"


debuggerking::faac_encoder::faac_encoder(aac_encoder * front)
	: _front(front)
	, _faac_encoder(nullptr)
	, _buffer(0)
	, _buffer_index(0)
	, _buffer4queue(nullptr)
{
	_buffer = static_cast<int*>(malloc(10 * 2 * 48000 * sizeof(int)));
}

debuggerking::faac_encoder::~faac_encoder(void)
{
	if (_buffer)
	{
		free(_buffer);
		_buffer = 0;
	}
	_buffer_index = 0;
}

int32_t debuggerking::faac_encoder::initialize_encoder(aac_encoder::configuration_t * config)
{
	_faac_encoder = faacEncOpen(config->samplerate, config->channels, &config->framesize, &config->ob);
	if (!_faac_encoder)
		return aac_encoder::err_code_t::fail;

	// set encoder configuration
	faacEncConfiguration * _faac_encoder_config = faacEncGetCurrentConfiguration(_faac_encoder);
	switch (config->mpeg_version)
	{
	case aac_encoder::version_type_mpeg2:
		_faac_encoder_config->mpegVersion = MPEG2;
		break;
	case aac_encoder::version_type_mpeg4:
		_faac_encoder_config->mpegVersion = MPEG4;
		break;
	default:
		_faac_encoder_config->mpegVersion = MPEG4;
		break;
	}
	switch (config->object_type)
	{
	case aac_encoder::aac_object_type_main:
		_faac_encoder_config->aacObjectType = MAIN;
		break;
	case aac_encoder::aac_object_type_low:
		_faac_encoder_config->aacObjectType = LOW;
		break;
	case aac_encoder::aac_object_type_ssr:
		_faac_encoder_config->aacObjectType = SSR;
		break;
	case aac_encoder::aac_object_type_ltp:
		_faac_encoder_config->aacObjectType = LTP;
		break;
	}

	_faac_encoder_config->allowMidside = config->allow_midside;
	_faac_encoder_config->useTns = config->use_tns;

	if (config->channels > 6)
		_faac_encoder_config->useLfe = 1;
	else
		_faac_encoder_config->useLfe = config->use_lfe;

	_faac_encoder_config->quantqual = config->quantization_quality;

	switch (config->shortctl)
	{
	case aac_encoder::block_type_normal:
		_faac_encoder_config->shortctl = SHORTCTL_NORMAL;
		break;
	case aac_encoder::block_type_noshort:
		_faac_encoder_config->shortctl = SHORTCTL_NOSHORT;
		break;
	case aac_encoder::block_type_nolong:
		_faac_encoder_config->shortctl = SHORTCTL_NOLONG;
		break;
	default:
		_faac_encoder_config->shortctl = SHORTCTL_NORMAL;
		break;
	}

	switch (config->input_format)
	{
	case aac_encoder::format_type_16bit:
		_faac_encoder_config->inputFormat = FAAC_INPUT_16BIT;
		break;
	case aac_encoder::format_type_24bit:
		_faac_encoder_config->inputFormat = FAAC_INPUT_24BIT;
		break;
	case aac_encoder::format_type_32bit:
		_faac_encoder_config->inputFormat = FAAC_INPUT_32BIT;
		break;
	case aac_encoder::format_type_float:
		_faac_encoder_config->inputFormat = FAAC_INPUT_FLOAT;
		break;
	default:
		_faac_encoder_config->inputFormat = FAAC_INPUT_16BIT;
		break;
	}

	switch (config->output_format)
	{
	case aac_encoder::format_type_raw:
		_faac_encoder_config->outputFormat = 0;
		break;
	case aac_encoder::format_type_adts:
		_faac_encoder_config->outputFormat = 1;
		break;
	case aac_encoder::format_type_latm:
		_faac_encoder_config->outputFormat = 2;
		break;
	default:
		_faac_encoder_config->outputFormat = 0;
		break;
	}

	_faac_encoder_config->bitRate = config->bitrate;

	if (!faacEncSetConfiguration(_faac_encoder, _faac_encoder_config))
	{
		faacEncClose(_faac_encoder);
		_faac_encoder = 0;
		return aac_encoder::err_code_t::fail;
	}

	uint8_t * extradata = nullptr;
	unsigned long extradata_size = 0;
	faacEncGetDecoderSpecificInfo(_faac_encoder, &extradata, &extradata_size);
	_front->set_extradata(extradata, extradata_size);

	_buffer4queue = (uint8_t*)malloc(config->ob);

	_config = *config;
	_buffer_index = 0;
	return aac_encoder::err_code_t::success;
}

int32_t debuggerking::faac_encoder::release_encoder(void)
{
	if (_buffer4queue)
	{
		free(_buffer4queue);
		_buffer4queue = nullptr;
	}

	if (_faac_encoder)
	{
		faacEncClose(_faac_encoder);
		_faac_encoder = 0;
	}
	_buffer_index = 0;
	return aac_encoder::err_code_t::success;
}

int32_t debuggerking::faac_encoder::encode(aac_encoder::entity_t * pcm, aac_encoder::entity_t * encoded)
{
#if 0	
	int32_t bytes_written = faacEncEncode(_faac_encoder, (int32_t*)pcm->data, pcm->data_size, (uint8_t*)encoded->data, (unsigned int)encoded->data_capacity);
	if (bytes_written > 0)
	{
		encoded->data_size = bytes_written;
		return dk_aac_encoder::ERR_CODE_SUCCESS;
	}
	else
	{
		encoded->data_size = 0;
		return dk_aac_encoder::ERR_CODE_FAILED;
	}

#else
	encoded->data_size = 0;
	if (!_faac_encoder || _config.bitdepth!=16)
		return aac_encoder::err_code_t::fail;

	memcpy((int16_t*)_buffer + _buffer_index, pcm->data, pcm->data_size);
	_buffer_index += (pcm->data_size >> 1);

	unsigned long cur_buffer_index = 0;
	int16_t * cur_buffer = (int16_t*)_buffer;
	while (cur_buffer_index + _config.framesize <= _buffer_index)
	{
		int bytes_written = faacEncEncode(_faac_encoder, (int*)cur_buffer, _config.framesize, (uint8_t*)encoded->data + encoded->data_size, _config.ob);
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
	return aac_encoder::err_code_t::success;
}

int32_t debuggerking::faac_encoder::encode(aac_encoder::entity_t * pcm)
{
	if (!_faac_encoder || _config.bitdepth!=16)
		return aac_encoder::err_code_t::fail;

	memcpy(((uint16_t*)_buffer) + _buffer_index, pcm->data, pcm->data_size);
	_buffer_index += (pcm->data_size >> 1);

	unsigned long cur_buffer_index = 0;
	short * cur_buffer = (int16_t*)_buffer;
	//size_t buffer4queue_pos = 0;
	while (cur_buffer_index + _config.framesize <= _buffer_index)
	{
		int bytes_written = faacEncEncode(_faac_encoder, (int*)cur_buffer, _config.framesize, _buffer4queue, _config.ob);
		if (bytes_written > 0)
		{
			if (_front)
				_front->push(_buffer4queue, bytes_written, 0);
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
	return aac_encoder::err_code_t::success;
}

int32_t debuggerking::faac_encoder::get_queued_data(aac_encoder::entity_t * encoded)
{
	if (_front)
	{
		return _front->pop((uint8_t*)encoded->data, encoded->data_size, encoded->timestamp);
	}
	else
	{
		return aac_encoder::err_code_t::fail;
	}
}