#include "faac_encoder.h"


faac_encoder::faac_encoder(dk_aac_encoder * front)
	: _front(front)
	, _faac_encoder(nullptr)
	, _extradata(nullptr)
	, _extradata_size(0)
	, _buffer(0)
	, _buffer_index(0)
	, _buffer4queue(nullptr)
{
	_buffer = static_cast<int*>(malloc(10 * 2 * 48000 * sizeof(int)));
}

faac_encoder::~faac_encoder(void)
{
	if (_buffer)
	{
		free(_buffer);
		_buffer = 0;
	}
	_buffer_index = 0;
}

dk_aac_encoder::ERR_CODE faac_encoder::initialize_encoder(dk_aac_encoder::configuration_t * config)
{
	_faac_encoder = faacEncOpen(config->samplerate, config->channels, &config->framesize, &config->ob);
	if (!_faac_encoder)
		return dk_aac_encoder::ERR_CODE_FAIL;

	// set encoder configuration
	faacEncConfiguration * _faac_encoder_config = faacEncGetCurrentConfiguration(_faac_encoder);
	switch (config->mpeg_version)
	{
	case dk_aac_encoder::VERSION_TYPE_MPEG2:
		_faac_encoder_config->mpegVersion = MPEG2;
		break;
	case dk_aac_encoder::VERSION_TYPE_MPEG4:
		_faac_encoder_config->mpegVersion = MPEG4;
		break;
	default:
		_faac_encoder_config->mpegVersion = MPEG4;
		break;
	}
	switch (config->object_type)
	{
	case dk_aac_encoder::AAC_OBJECT_TYPE_MAIN:
		_faac_encoder_config->aacObjectType = MAIN;
		break;
	case dk_aac_encoder::AAC_OBJECT_TYPE_LOW:
		_faac_encoder_config->aacObjectType = LOW;
		break;
	case dk_aac_encoder::AAC_OBJECT_TYPE_SSR:
		_faac_encoder_config->aacObjectType = SSR;
		break;
	case dk_aac_encoder::AAC_OBJECT_TYPE_LTP:
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
	case dk_aac_encoder::BLOCK_TYPE_NORMAL:
		_faac_encoder_config->shortctl = SHORTCTL_NORMAL;
		break;
	case dk_aac_encoder::BLOCK_TYPE_NOSHORT:
		_faac_encoder_config->shortctl = SHORTCTL_NOSHORT;
		break;
	case dk_aac_encoder::BLOCK_TYPE_NOLONG:
		_faac_encoder_config->shortctl = SHORTCTL_NOLONG;
		break;
	default:
		_faac_encoder_config->shortctl = SHORTCTL_NORMAL;
		break;
	}

	switch (config->input_format)
	{
	case dk_aac_encoder::FORMAT_TYPE_16BIT:
		_faac_encoder_config->inputFormat = FAAC_INPUT_16BIT;
		break;
	case dk_aac_encoder::FORMAT_TYPE_24BIT:
		_faac_encoder_config->inputFormat = FAAC_INPUT_24BIT;
		break;
	case dk_aac_encoder::FORMAT_TYPE_32BIT:
		_faac_encoder_config->inputFormat = FAAC_INPUT_32BIT;
		break;
	case dk_aac_encoder::FORMAT_TYPE_FLOAT:
		_faac_encoder_config->inputFormat = FAAC_INPUT_FLOAT;
		break;
	default:
		_faac_encoder_config->inputFormat = FAAC_INPUT_16BIT;
		break;
	}

	switch (config->output_format)
	{
	case dk_aac_encoder::FORMAT_TYPE_RAW:
		_faac_encoder_config->outputFormat = 0;
		break;
	case dk_aac_encoder::FORMAT_TYPE_ADTS:
		_faac_encoder_config->outputFormat = 1;
		break;
	case dk_aac_encoder::FORMAT_TYPE_LATM:
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
		return dk_aac_encoder::ERR_CODE_FAIL;
	}

	_extradata = nullptr;
	_extradata_size = 0;
	faacEncGetDecoderSpecificInfo(_faac_encoder, &_extradata, &_extradata_size);

	_buffer4queue = (uint8_t*)malloc(config->ob);

	_config = *config;
	_buffer_index = 0;
	return dk_aac_encoder::ERR_CODE_SUCCESS;
}

dk_aac_encoder::ERR_CODE faac_encoder::release_encoder(void)
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
	return dk_aac_encoder::ERR_CODE_SUCCESS;
}

dk_aac_encoder::ERR_CODE faac_encoder::encode(dk_aac_encoder::dk_audio_entity_t * pcm, dk_aac_encoder::dk_audio_entity_t * encoded)
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
		return dk_aac_encoder::ERR_CODE_FAIL;

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
	return dk_aac_encoder::ERR_CODE_SUCCESS;
}

dk_aac_encoder::ERR_CODE faac_encoder::encode(dk_aac_encoder::dk_audio_entity_t * pcm)
{
	if (!_faac_encoder || _config.bitdepth!=16)
		return dk_aac_encoder::ERR_CODE_FAIL;

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
	return dk_aac_encoder::ERR_CODE_SUCCESS;
}

dk_aac_encoder::ERR_CODE faac_encoder::get_queued_data(dk_aac_encoder::dk_audio_entity_t * encoded)
{
	if (_front)
	{
		return _front->pop((uint8_t*)encoded->data, encoded->data_size, encoded->pts);
	}
	else
	{
		return dk_aac_encoder::ERR_CODE_FAIL;
	}
}

uint8_t * faac_encoder::extradata(void)
{
	return _extradata;
}

size_t faac_encoder::extradata_size(void)
{
	return (size_t)_extradata_size;
}


/*dk_aac_encoder::ERR_CODE faac_encoder::initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, uint8_t * extra_data, size_t & extra_data_size)
{
	_config = config;

	_faac_encoder = faacEncOpen(_config.samplerate, _config.channels, &_config.input_samples, &_config.max_output_bytes);
	if (!_faac_encoder)
		return dk_aac_encoder::ERR_CODE_FAILED;

	// set encoder configuration
	faacEncConfiguration * _faac_encoder_config = faacEncGetCurrentConfiguration(_faac_encoder);
	switch (_config.mpeg_version)
	{
	case dk_aac_encoder::VERSION_TYPE_MPEG2 :
		_faac_encoder_config->mpegVersion = MPEG2;
		break;
	case dk_aac_encoder::VERSION_TYPE_MPEG4 :
		_faac_encoder_config->mpegVersion = MPEG4;
		break;
	default :
		_faac_encoder_config->mpegVersion = MPEG4;
		break;
	}
	switch (_config.object_type)
	{
	case dk_aac_encoder::AAC_OBJECT_TYPE_MAIN :
		_faac_encoder_config->aacObjectType = MAIN;
		break;
	case dk_aac_encoder::AAC_OBJECT_TYPE_LOW :
		_faac_encoder_config->aacObjectType = LOW;
		break;
	case dk_aac_encoder::AAC_OBJECT_TYPE_SSR :
		_faac_encoder_config->aacObjectType = SSR;
		break;
	case dk_aac_encoder::AAC_OBJECT_TYPE_LTP :
		_faac_encoder_config->aacObjectType = LTP;
		break;
	}
	
	_faac_encoder_config->allowMidside = _config.allow_midside;
	_faac_encoder_config->useTns = _config.use_tns;

	if (_config.channels > 6)
		_faac_encoder_config->useLfe = 1;
	else
		_faac_encoder_config->useLfe = _config.use_lfe;

	_faac_encoder_config->quantqual = _config.quantization_quality;
	
	switch (_config.shortctl)
	{
	case dk_aac_encoder::BLOCK_TYPE_NORMAL :
		_faac_encoder_config->shortctl = SHORTCTL_NORMAL;
		break;
	case dk_aac_encoder::BLOCK_TYPE_NOSHORT:
		_faac_encoder_config->shortctl = SHORTCTL_NOSHORT;
		break;
	case dk_aac_encoder::BLOCK_TYPE_NOLONG:
		_faac_encoder_config->shortctl = SHORTCTL_NOLONG;
		break;
	default:
		_faac_encoder_config->shortctl = SHORTCTL_NORMAL;
		break;
	}

	switch (_config.input_format)
	{
	case dk_aac_encoder::FORMAT_TYPE_16BIT :
		_faac_encoder_config->inputFormat = FAAC_INPUT_16BIT;
		break;
	case dk_aac_encoder::FORMAT_TYPE_24BIT:
		_faac_encoder_config->inputFormat = FAAC_INPUT_24BIT;
		break;
	case dk_aac_encoder::FORMAT_TYPE_32BIT:
		_faac_encoder_config->inputFormat = FAAC_INPUT_32BIT;
		break;
	case dk_aac_encoder::FORMAT_TYPE_FLOAT:
		_faac_encoder_config->inputFormat = FAAC_INPUT_FLOAT;
		break;
	default :
		_faac_encoder_config->inputFormat = FAAC_INPUT_16BIT;
		break;
	}

	switch (_config.output_format)
	{
	case dk_aac_encoder::FORMAT_TYPE_RAW :
		_faac_encoder_config->outputFormat = 0;
		break;
	case dk_aac_encoder::FORMAT_TYPE_ADTS:
		_faac_encoder_config->outputFormat = 1;
		break;
	case dk_aac_encoder::FORMAT_TYPE_LATM :
		_faac_encoder_config->outputFormat = 2;
		break;
	default:
		_faac_encoder_config->outputFormat = 0;
		break;
	}

	if (!faacEncSetConfiguration(_faac_encoder, _faac_encoder_config))
	{
		faacEncClose(_faac_encoder);
		_faac_encoder = 0;
		return dk_aac_encoder::ERR_CODE_FAILED;
	}

	unsigned char * tmp_extra_data = 0;
	unsigned long tmp_extra_data_size = 0;
	faacEncGetDecoderSpecificInfo(_faac_encoder, &tmp_extra_data, &tmp_extra_data_size);
	if (extra_data && tmp_extra_data && tmp_extra_data_size > 0)
	{
		extra_data_size = tmp_extra_data_size;
		memcpy(extra_data, tmp_extra_data, extra_data_size);
		//if (tmp_extra_data)
		//	free(tmp_extra_data);
	}

	input_samples = _config.input_samples;
	max_output_bytes = _config.max_output_bytes;

	_buffer_index = 0;
	return dk_aac_encoder::ERR_CODE_SUCCESS;
}

dk_aac_encoder::ERR_CODE faac_encoder::release(void)
{
	if (_faac_encoder)
	{
		faacEncClose(_faac_encoder);
		_faac_encoder = 0;
	}
	_buffer_index = 0;
	return dk_aac_encoder::ERR_CODE_SUCCESS;
}

dk_aac_encoder::ERR_CODE faac_encoder::encode(int32_t * input, size_t isize, uint8_t * output, size_t osize, size_t & bytes_written)
{
	bytes_written = faacEncEncode(_faac_encoder, input, isize, output, (unsigned int)osize);
	if (bytes_written > 0)
	{
		osize = bytes_written;
		return dk_aac_encoder::ERR_CODE_SUCCESS;
	}
	else
	{
		osize = 0;
		return dk_aac_encoder::ERR_CODE_FAILED;
	}
}

dk_aac_encoder::ERR_CODE faac_encoder::encode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize)
{
	osize = 0;
	int bytes_written = 0;


	if (!_faac_encoder || ((_config.bitpersamples / 8) > 4) || ((_config.bitpersamples / 8) < 1))
		return dk_aac_encoder::ERR_CODE_FAILED;

	unsigned int index = 0;
	switch (_config.bitpersamples / 8) 
	{
		//case 1:
		//{
		//	for (index = 0; index < isize; index++)
		//		_pcm_buffer[index] = (input[index] - 128) * 65536;
		//	break;
		//}
		case 2:
		{
			//int outsize = 32 * 1024;

			memcpy(((short*)_buffer) + _buffer_index, input, isize);
			_buffer_index += (isize / sizeof(short));

			unsigned long cur_buffer_index = 0;
			short * cur_buffer = (short *)_buffer;
			while (cur_buffer_index + _config.input_samples <= _buffer_index)
			{
				bytes_written = faacEncEncode(_faac_encoder, (int*)cur_buffer, _config.input_samples, output + osize, _config.max_output_bytes);
				if (bytes_written > 0)
					osize += bytes_written;
				cur_buffer_index += _config.input_samples;
				cur_buffer += _config.input_samples;
			}

			if (cur_buffer_index > 0)
			{
				int left = _buffer_index - cur_buffer_index;
				if (left > 0)
					memmove(_buffer, ((short*)_buffer) + cur_buffer_index, left*sizeof(short));
				_buffer_index = left;
			}
			break;
		}
		//case 3:
		//{
		//	isize = isize / sizeof(int);
		//	for (index = 0; index < isize; index++)
		//	{
		//		int s = input[3 * index] | (input[3 * index + 1] << 8) | (input[3 * index + 2] << 16);

		//		// fix sign
		//		if (s & 0x800000)
		//			s |= 0xff000000;

		//		_pcm_buffer[index] = s;
		//	}
		//	break;
		//}
		//case 4:
		//{
		//	memcpy(_pcm_buffer, input, isize);
		//	break;
		//}
	}

	return dk_aac_encoder::ERR_CODE_SUCCESS;
}*/