#include "celt_decoder.h"


debuggerking::celt_core::celt_core(celt_decoder * front)
	: _front(front)
	, _decoder(nullptr)
	, _buffer_pos(0)
{
	_buffer = (int16_t*)malloc(10 * 2 * 48000 * sizeof(int16_t));
}

debuggerking::celt_core::~celt_core(void)
{
	if (_buffer)
	{
		free(_buffer);
		_buffer = nullptr;
	}
}

int32_t debuggerking::celt_core::initialize_decoder(celt_decoder::configuration_t * config)
{
	release_decoder();
	_config = *config;

	_framesize = _config.framesize * _config.samplerate / 1000;

	int32_t err;
	_decoder = opus_decoder_create(config->samplerate, config->channels, &err);
	if (err != OPUS_OK || _decoder == NULL)
		return celt_decoder::err_code_t::fail;
	
	return celt_decoder::err_code_t::success;
}

int32_t debuggerking::celt_core::release_decoder(void)
{
	if (_decoder)
	{
		opus_decoder_destroy(_decoder);
		_decoder = nullptr;
	}
	return celt_decoder::err_code_t::success;
}

int32_t debuggerking::celt_core::decode(celt_decoder::entity_t * encoded, celt_decoder::entity_t * pcm)
{
	pcm->data_capacity = pcm->data_capacity / (_config.channels * sizeof(int16_t));
	int32_t bytes_written = opus_decode(_decoder, (uint8_t*)encoded->data, (opus_int32)encoded->data_size, (opus_int16*)pcm->data, (int)pcm->data_capacity, 0);
	if (bytes_written > 0)
	{
		pcm->data_size = bytes_written * _config.channels * sizeof(int16_t);
	}
	return celt_decoder::err_code_t::success;

	/*int16_t * intermediate = (int16_t*)encoded->data;
	size_t isamples = pcm->data_size / (sizeof(int16_t)*_config.channels);

	memcpy(_buffer + _buffer_pos, intermediate, pcm->data_size);
	_buffer_pos += isamples;

	size_t cur_sample = 0;
	int16_t	* cur_buf = _buffer;

	size_t odata_position = 0;
	size_t odata_capacity = encoded->data_capacity;
	encoded->data_size = 0;
	while (cur_sample + (_framesize * _config.channels) <= _buffer_pos)
	{
		int32_t bytes_written = opus_decode(_decoder, input, (int)isize, (opus_int16*)output, osize, 0);
		//int32_t bytes_written = opus_encode(_encoder, cur_buf, (int)_framesize, (uint8_t*)encoded->data + odata_position, (int)odata_capacity);
		if (bytes_written>1)
		{
			odata_position += bytes_written;
			odata_capacity -= bytes_written;
			encoded->data_size += bytes_written;
		}
		cur_sample += _framesize * _config.channels;
		cur_buf += _framesize * _config.channels;
	}

	if (cur_sample > 0)
	{
		int	left = _buffer_pos - cur_sample;
		if (left > 0)
			memcpy(_buffer, _buffer + cur_sample, left*sizeof(int16_t));
		_buffer_pos = left;
	}

	opus_decode(_decoder)
	return dk_celt_encoder::ERR_CODE_SUCCESS;*/
}

/*int32_t celt_decoder::decode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize)
{
	int32_t bytes_written = 0;
	bytes_written = opus_decode(_decoder, input, (int)isize, (opus_int16*)output, osize, 0);
	osize = 0;
	if (bytes_written > 0)
	{
		osize = bytes_written;
	}
	return celt_decoder::err_code_t::SUCCESS;
}*/

int32_t debuggerking::celt_core::decode(celt_decoder::entity_t * encoded)
{
	//pcm->data_capacity = pcm->data_capacity / (_config.channels * sizeof(int16_t));
	//int32_t bytes_written = opus_decode(_decoder, (uint8_t*)encoded->data, (opus_int32)encoded->data_size, (opus_int16*)pcm->data, (int)pcm->data_capacity, 0);
	//if (bytes_written > 0)
	//{
	//	pcm->data_size = bytes_written * _config.channels * sizeof(int16_t);
	//}
	return celt_decoder::err_code_t::success;
}

int32_t debuggerking::celt_core::get_queued_data(celt_decoder::entity_t * pcm)
{
	if (_front)
	{
		return _front->pop((uint8_t*)pcm->data, pcm->data_size, pcm->timestamp);
	}
	else
	{
		return celt_decoder::err_code_t::fail;
	}
}

