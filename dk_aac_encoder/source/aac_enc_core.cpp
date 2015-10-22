#include "aac_enc_core.h"


aac_enc_core::aac_enc_core(void)
	: _faac_encoder(0)
	, _pcm_buffer(0)
	, _pcm_buffer_index(0)
{
	_pcm_buffer = static_cast<int*>(malloc(10 * 2 * 48000 * sizeof(int)));
}

aac_enc_core::~aac_enc_core(void)
{
	if (_pcm_buffer)
	{
		free(_pcm_buffer);
		_pcm_buffer = 0;
	}
	_pcm_buffer_index = 0;
}

dk_aac_encoder::ERR_CODE aac_enc_core::initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, unsigned char * extra_data, unsigned long & extra_data_size)
{
	_config = config;

	_faac_encoder = faacEncOpen(_config.sample_rate, _config.channels, &_config.input_samples, &_config.max_output_bytes);
	if (!_faac_encoder)
		return dk_aac_encoder::ERR_CODE_FAILED;

	/* set encoder configuration */
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
	}

	input_samples = _config.input_samples;
	max_output_bytes = _config.max_output_bytes;

	_pcm_buffer_index = 0;
	return dk_aac_encoder::ERR_CODE_SUCCESS;
}

dk_aac_encoder::ERR_CODE aac_enc_core::release(void)
{
	if (_faac_encoder)
	{
		faacEncClose(_faac_encoder);
		_faac_encoder = 0;
	}
	_pcm_buffer_index = 0;
	return dk_aac_encoder::ERR_CODE_SUCCESS;
}

dk_aac_encoder::ERR_CODE aac_enc_core::encode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int &osize)
{
	osize = 0;
	int bytes_written = 0;

	if (!_faac_encoder || ((_config.bitpersamples / 8) > 4) || ((_config.bitpersamples / 8) < 1))
		return dk_aac_encoder::ERR_CODE_FAILED;

	unsigned int index = 0;
	switch (_config.bitpersamples / 8) 
	{
		/* TODO
		case 1:
		{
			for (index = 0; index < isize; index++)
				_pcm_buffer[index] = (input[index] - 128) * 65536;
			break;
		}
		*/
		case 2:
		{
			memcpy(((short*)_pcm_buffer) + _pcm_buffer_index, input, isize);
			_pcm_buffer_index += (isize / sizeof(short));

			unsigned long current_pcm_buffer_index = 0;
			short * current_pcm_buffer = (short *)_pcm_buffer;
			while (current_pcm_buffer_index + _config.input_samples <= _pcm_buffer_index)
			{
				bytes_written = faacEncEncode(_faac_encoder, (int*)current_pcm_buffer, _config.input_samples, output + osize, _config.max_output_bytes);
				if (bytes_written > 0)
					osize += bytes_written;
				current_pcm_buffer_index += _config.input_samples;
				current_pcm_buffer += _config.input_samples;
			}

			if (current_pcm_buffer_index > 0)
			{
				int left = _pcm_buffer_index - current_pcm_buffer_index;
				if (left > 0)
					memmove(_pcm_buffer, ((short*)_pcm_buffer) + current_pcm_buffer_index, left*sizeof(short));
				_pcm_buffer_index = left;
			}
			break;
		}
		/* TODO
		case 3:
		{
			isize = isize / sizeof(int);
			for (index = 0; index < isize; index++)
			{
				int s = input[3 * index] | (input[3 * index + 1] << 8) | (input[3 * index + 2] << 16);

				// fix sign
				if (s & 0x800000)
					s |= 0xff000000;

				_pcm_buffer[index] = s;
			}
			break;
		}
		case 4:
		{
		//isize = isize / sizeof(int);
			memcpy(_pcm_buffer, input, isize);
			break;
		}*/
	}

	return dk_aac_encoder::ERR_CODE_SUCCESS;
}