#include "faad2_decoder.h"

const int MAXCHANNELS = 6;
const int chmap[MAXCHANNELS][MAXCHANNELS + 1] = {
	// first column tell us if we need to remap
	{ 0, },					// mono
	{ 0, },					// l, r
	{ 1, 1, 2, 0, },			// c ,l, r -> l, r, c
	{ 1, 1, 2, 0, 3, },		// c, l, r, bc -> l, r, c, bc
	{ 1, 1, 2, 0, 3, 4, },		// c, l, r, bl, br -> l, r, c, bl, br
	{ 1, 1, 2, 0, 5, 3, 4 }	// c, l, r, bl, br, lfe -> l, r, c, lfe, bl, br
};

faad2_decoder::faad2_decoder(void)
	: _aac_decoder(0)
	, _aac_config(0)
	, _buffer(0)
	, _buffer_size(0)
	, _calc_frames(0)
	, _bytes_consumed(0)
	, _decoded_frames(0)
{

}

faad2_decoder::~faad2_decoder(void)
{

}

dk_aac_decoder::ERR_CODE faad2_decoder::initialize_decoder(dk_aac_decoder::configuration_t * config)
{
	_config = *config;

	_aac_decoder = faacDecOpen();
	if (!_aac_decoder)
		return dk_aac_decoder::ERR_CODE_FAIL;
	_aac_config = faacDecGetCurrentConfiguration(_aac_decoder);
	if (!_aac_config)
		return dk_aac_decoder::ERR_CODE_FAIL;

	switch (_config.output_format)
	{
	case dk_aac_decoder::FORMAT_TYPE_16BIT:
		_aac_config->outputFormat = FAAD_FMT_16BIT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_24BIT:
		_aac_config->outputFormat = FAAD_FMT_24BIT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_32BIT:
		_aac_config->outputFormat = FAAD_FMT_32BIT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_FLOAT:
		_aac_config->outputFormat = FAAD_FMT_16BIT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_FIXED:
		_aac_config->outputFormat = FAAD_FMT_FLOAT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_DOUBLE:
		_aac_config->outputFormat = FAAD_FMT_FIXED;
		break;
	default:
		_aac_config->outputFormat = FAAD_FMT_DOUBLE;
		break;
	}

	//_aac_config->downMatrix = _config.mix_down;
	faacDecSetConfiguration(_aac_decoder, _aac_config);

	if (faacDecInit2(_aac_decoder, _config.extradata, _config.extradata_size, &_config.samplerate, &_channels) < 0)
		return dk_aac_decoder::ERR_CODE_FAIL;

	if (_config.mix_down)
	{
		_config.channels = 2; //TODO : check with mono
	}

	mp4AudioSpecificConfig info;
	AudioSpecificConfig(_config.extradata, _config.extradata_size, &info);

	_calc_frames = 0;
	_bytes_consumed = 0;
	_decoded_frames = 0;

	// Buffer Size for decoded PCM: 1s of 192kHz 32-bit with 8 channels
	// 192000 (Samples) * 4 (Bytes per Sample) * 8 (channels)
	//#define LAV_AUDIO_BUFFER_SIZE 6144000
	_buffer_size = 192000 * 4 * 8;
	_buffer = static_cast<unsigned char*>(malloc(_buffer_size));

	return dk_aac_decoder::ERR_CODE_SUCCESS;
}

dk_aac_decoder::ERR_CODE faad2_decoder::release_decoder(void)
{
	if (_aac_decoder)
	{
		faacDecClose(_aac_decoder);
		_aac_decoder = 0;
	}

	if (_buffer)
	{
		free(_buffer);
		_buffer = 0;
	}
	return dk_aac_decoder::ERR_CODE_SUCCESS;
}

dk_aac_decoder::ERR_CODE faad2_decoder::decode(dk_audio_entity_t * encoded, dk_audio_entity_t * pcm)
{
	if (!_aac_decoder)
		return dk_aac_decoder::ERR_CODE_FAIL;

	int16_t * out_samples = (int16_t*)faacDecDecode(_aac_decoder, &_aac_frame_info, (uint8_t*)encoded->data, encoded->data_size);
	if (_aac_frame_info.error)
	{
		pcm->data_size = 0;
		return dk_aac_decoder::ERR_CODE_FAIL;
	}

	//_calc_frames++;
	//_decoded_frames++;
	//_bytes_consumed += encoded->data_size;
	//if (_calc_frames == 43)
	//{
	//	_config.bitrate = (int)((_bytes_consumed << 3) / (_decoded_frames / 43.07));
	//	_calc_frames = 0;
	//}

	if (!_aac_frame_info.error && out_samples)
	{
		int channel_index = _aac_frame_info.channels - 1;
		if (chmap[channel_index][0])
		{
			// dshow remapping
			int16_t * dst_buffer = (int16_t*)pcm->data;
			for (unsigned int i = 0; i < _aac_frame_info.samples; i += _aac_frame_info.channels, out_samples += _aac_frame_info.channels)
			{
				for (unsigned int j = 1; j <= _aac_frame_info.channels; j++)
				{
					*dst_buffer++ = out_samples[chmap[channel_index][j]];
				}
			}
		}
		else
		{
			pcm->data_size = _aac_frame_info.samples << 1;//float(_aac_frame_info.samples << 1) * float(_config.channels/_aac_frame_info.channels);
			if (pcm->data_size>0)
				memcpy(pcm->data, out_samples, pcm->data_size);
			return dk_aac_decoder::ERR_CODE_SUCCESS;
		}
	}
	pcm->data_size = 0;
	return dk_aac_decoder::ERR_CODE_FAIL;
}

/*dk_aac_decoder::ERR_CODE faad2_decoder::initialize(dk_aac_decoder::configuration_t config, unsigned char * extra_data, int extra_data_size, int & samplerate, int & channels)
{
	_config = config;
	_extra_data_size = extra_data_size;
	_extra_data = static_cast<unsigned char*>(malloc(_extra_data_size));
	memcpy(_extra_data, extra_data, _extra_data_size);


	_aac_decoder = faacDecOpen();
	if (!_aac_decoder)
		return dk_aac_decoder::ERR_CODE_FAILED;
	_aac_config = faacDecGetCurrentConfiguration(_aac_decoder);
	if (!_aac_config)
		return dk_aac_decoder::ERR_CODE_FAILED;

	switch (config.output_format)
	{
	case dk_aac_decoder::FORMAT_TYPE_16BIT :
		_aac_config->outputFormat = FAAD_FMT_16BIT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_24BIT:
		_aac_config->outputFormat = FAAD_FMT_24BIT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_32BIT:
		_aac_config->outputFormat = FAAD_FMT_32BIT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_FLOAT:
		_aac_config->outputFormat = FAAD_FMT_16BIT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_FIXED:
		_aac_config->outputFormat = FAAD_FMT_FLOAT;
		break;
	case dk_aac_decoder::FORMAT_TYPE_DOUBLE:
		_aac_config->outputFormat = FAAD_FMT_FIXED;
		break;
	default :
		_aac_config->outputFormat = FAAD_FMT_DOUBLE;
		break;
	}

	//_aac_config->downMatrix = _config.mix_down;
	faacDecSetConfiguration(_aac_decoder, _aac_config);

	if (faacDecInit2(_aac_decoder, _extra_data, _extra_data_size, &_config.samplerate, &_config.channels) < 0)
		return dk_aac_decoder::ERR_CODE_FAILED;

	if (_config.mix_down)
	{
		_config.channels = 2; //TODO : check with mono
	}

	mp4AudioSpecificConfig info;
	AudioSpecificConfig(_extra_data, _extra_data_size, &info);

	samplerate = _config.samplerate;
	channels = _config.channels;

	_calc_frames = 0;
	_bytes_consumed = 0;
	_decoded_frames = 0;


	// Buffer Size for decoded PCM: 1s of 192kHz 32-bit with 8 channels
	// 192000 (Samples) * 4 (Bytes per Sample) * 8 (channels)
	//#define LAV_AUDIO_BUFFER_SIZE 6144000
	_buffer_size = 192000 * 4 * 8;
	_buffer = static_cast<unsigned char*>(malloc(_buffer_size));

	return dk_aac_decoder::ERR_CODE_SUCCESS;
}

dk_aac_decoder::ERR_CODE faad2_decoder::release(void)
{
	if (_aac_decoder)
	{
		faacDecClose(_aac_decoder);
		_aac_decoder = 0;
	}

	if (_extra_data)
	{
		free(_extra_data);
		_extra_data = 0;
	}

	if (_buffer)
	{
		free(_buffer);
		_buffer = 0;
	}

	_extra_data_size = 0;

	return dk_aac_decoder::ERR_CODE_SUCCESS;
}

dk_aac_decoder::ERR_CODE faad2_decoder::decode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int & osize)
{
	if (!_aac_decoder)
		return dk_aac_decoder::ERR_CODE_FAILED;

	short * out_samples = (short *)faacDecDecode(_aac_decoder, &_aac_frame_info, input, isize);
	if (_aac_frame_info.error)
	{
		osize = 0;
		return dk_aac_decoder::ERR_CODE_FAILED;
	}

	_calc_frames++;
	_decoded_frames++;
	_bytes_consumed += isize;
	if (_calc_frames == 43)
	{
		_config.bitrate = (int)((_bytes_consumed * 8) / (_decoded_frames / 43.07));
		_calc_frames = 0;
	}

	if (!_aac_frame_info.error && out_samples)
	{
		int channel_index = _aac_frame_info.channels - 1;
		if (chmap[channel_index][0])
		{
			// dshow remapping
			short *dst_buffer = (short*)output;
			for (unsigned int i = 0; i < _aac_frame_info.samples; i += _aac_frame_info.channels, out_samples += _aac_frame_info.channels)
			{
				for (unsigned int j = 1; j <= _aac_frame_info.channels; j++)
				{
					*dst_buffer++ = out_samples[chmap[channel_index][j]];
				}
			}
		}
		else
		{
			memcpy(output, out_samples, _aac_frame_info.samples * sizeof(short));
		}
	}
	else
	{
		osize = 0;
		return dk_aac_decoder::ERR_CODE_FAILED;
	}

	osize = _aac_frame_info.samples * sizeof(short);

	return dk_aac_decoder::ERR_CODE_SUCCESS;
}*/