#include "celt_encoder.h"


celt_encoder::celt_encoder(dk_celt_encoder * front)
	: _front(front)
	, _encoder(nullptr)
	, _resampler(nullptr)
	, _skip(0)
	, _buffer_pos(0)
	, _buffer4queue(nullptr)
	, _buffer4queue_size(0)
{
	_buffer = (int16_t*)malloc(10 * 2 * 48000 * sizeof(int16_t));
}

celt_encoder::~celt_encoder(void)
{
	if (_buffer)
	{
		free(_buffer);
		_buffer = nullptr;
	}
}

dk_celt_encoder::err_code celt_encoder::initialize_encoder(dk_celt_encoder::configuration_t * config)
{
	release_encoder();
	_config = *config;
	_framesize = _config.framesize * _config.codingrate / 1000;

	int32_t err;
	_encoder = opus_encoder_create(config->codingrate, config->channels, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &err);
	if (err != OPUS_OK || _encoder == NULL)
		return dk_celt_encoder::err_code_fail;

	err = opus_encoder_ctl(_encoder, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_ARG));
	if (err != OPUS_OK)
	{
		opus_encoder_destroy(_encoder);
		return dk_celt_encoder::err_code_fail;
	}

	err = opus_encoder_ctl(_encoder, OPUS_SET_VBR(1));
	if (err != OPUS_OK)
	{
		opus_encoder_destroy(_encoder);
		return dk_celt_encoder::err_code_fail;
	}

	err = opus_encoder_ctl(_encoder, OPUS_SET_VBR_CONSTRAINT(1));
	if (err != OPUS_OK)
	{
		opus_encoder_destroy(_encoder);
		return dk_celt_encoder::err_code_fail;
	}

	err = opus_encoder_ctl(_encoder, OPUS_SET_BITRATE(_config.bitrate));
	if (err != OPUS_OK)
	{
		opus_encoder_destroy(_encoder);
		return dk_celt_encoder::err_code_fail;
	}

	err = opus_encoder_ctl(_encoder, OPUS_SET_COMPLEXITY(_config.complexity));
	if (err != OPUS_OK)
	{
		opus_encoder_destroy(_encoder);
		return dk_celt_encoder::err_code_fail;
	}

	_buffer4queue_size = _config.bitrate >> 3;
	_buffer4queue = (uint8_t*)malloc(_buffer4queue_size);

	if (_config.samplerate != _config.codingrate)
	{
		return setup_resampler(_config.samplerate, _config.codingrate, _config.channels, _config.complexity);
	}

	return dk_celt_encoder::err_code_success;
}

dk_celt_encoder::err_code celt_encoder::release_encoder(void)
{
	clear_resampler();

	if (_buffer4queue)
	{
		free(_buffer4queue);
		_buffer4queue = nullptr;
		_buffer4queue_size = 0;
	}

	if (_encoder)
	{
		opus_encoder_destroy(_encoder);
		_encoder = nullptr;
	}

	_skip = 0;
	_buffer_pos = 0;
	return dk_celt_encoder::err_code_success;
}

dk_celt_encoder::err_code celt_encoder::encode(dk_celt_encoder::dk_audio_entity_t * pcm, dk_celt_encoder::dk_audio_entity_t * encoded)
{
	int16_t * intermediate = (int16_t*)pcm->data;
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
		int32_t bytes_written = opus_encode(_encoder, cur_buf, (int)_framesize, (uint8_t*)encoded->data + odata_position, (int)odata_capacity);
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
	return dk_celt_encoder::err_code_success;
}

dk_celt_encoder::err_code celt_encoder::encode(dk_celt_encoder::dk_audio_entity_t * pcm)
{
	int16_t * intermediate = (int16_t*)pcm->data;
	size_t isamples = pcm->data_size >> 1;
	if (_resampler)
	{
		size_t osamples				= 0;
		intermediate				= _resampler->outbuffers;
		int32_t		obuffer_size	= _resampler->outbuffer_size >> 1;
		int16_t *	pcmbuffer		= _resampler->inbuffers;
		int32_t		pcmbuffer_pos	= 0;
		int32_t		pcm_framesize	= _config.framesize * _resampler->samplerate / 1000;
		int32_t		pcm_step		= pcm_framesize *_resampler->channels;
		while (pcmbuffer_pos < isamples)
		{
			memcpy(pcmbuffer, (int16_t*)pcm->data + pcmbuffer_pos, pcm_step << 1);

			uint32_t inlength = pcm_framesize;
			uint32_t outlength = obuffer_size - osamples;
			pcmbuffer_pos += pcm_step;
			speex_resampler_process_interleaved_int(_resampler->resampler, pcmbuffer, &inlength, intermediate + osamples*_resampler->channels, &outlength);
			osamples += outlength;
			if (inlength == 0)
			{
				for (int32_t i = osamples; i<_framesize*_resampler->channels; i++)
					intermediate[i] = 0;

				pcm->data_size = (osamples*_resampler->channels) << 1;
				isamples = osamples*_resampler->channels;
				break;
			}
			//for (int32_t i = 0; i<_resampler->channels*((*inbuffer_pos) - (long int)inlength); i++)
			//	pcmbuffer[i] = pcmbuffer[i + _resampler->channels*inlength];
			//(*inbuffer_pos) -= inlength;
		}
		pcm->data_size = (osamples*_resampler->channels) << 1;
		isamples = osamples*_resampler->channels;
	}


	memcpy(_buffer + _buffer_pos, intermediate, pcm->data_size);
	_buffer_pos += isamples;

	size_t cur_sample = 0;
	int16_t	* cur_buf = _buffer;

	while (cur_sample + (_framesize * _config.channels) <= _buffer_pos)
	{
		int32_t bytes_written = opus_encode(_encoder, cur_buf, (int)_framesize, (uint8_t*)_buffer4queue, (int)_buffer4queue_size);
		if (bytes_written>1)
		{
			if (_front)
				_front->push(_buffer4queue, bytes_written, 0);
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
	return dk_celt_encoder::err_code_success;
}

dk_celt_encoder::err_code celt_encoder::get_queued_data(dk_celt_encoder::dk_audio_entity_t * encoded)
{
	if (_front)
	{
		return _front->pop((uint8_t*)encoded->data, encoded->data_size, encoded->pts);
	}
	else
	{
		return dk_celt_encoder::err_code_fail;
	}
}

dk_celt_encoder::err_code celt_encoder::setup_resampler(int32_t samplerate, int32_t codingrate, int32_t channels, int32_t complexity)
{
	int32_t err;
	if (_resampler)
	{
		clear_resampler();
	}

	_resampler = static_cast<resampler_t*>(calloc(1, sizeof(resampler_t)));
	_resampler->inbuffer_size =  5760 * 2;
	_resampler->inbuffer_pos = 0;
	_resampler->outbuffer_size = 48000 * sizeof(int16_t)*channels;//_framesize*sizeof(int16_t)*_resampler->channels;
	_resampler->outbuffer_pos = 0;
	_resampler->channels = channels;
	_resampler->samplerate = samplerate;
	_resampler->codingrate = codingrate;
	_resampler->done = 0;
	_resampler->resampler = speex_resampler_init(channels, samplerate, codingrate, codingrate == 48000 ? (complexity + 1) / 2 : 5, &err);
	_skip += speex_resampler_get_output_latency(_resampler->resampler);
	_resampler->inbuffers = static_cast<int16_t*>(malloc(sizeof(int16_t)*_resampler->inbuffer_size*_resampler->channels));
	_resampler->outbuffers = static_cast<int16_t*>(malloc(_resampler->outbuffer_size));

	if (err != OPUS_OK)
		return dk_celt_encoder::err_code_fail;
	else
		return dk_celt_encoder::err_code_success;
}

dk_celt_encoder::err_code celt_encoder::clear_resampler(void)
{
	if (_resampler)
	{
		speex_resampler_destroy(_resampler->resampler);

		free(_resampler->inbuffers);
		free(_resampler->outbuffers);
		free(_resampler);
		_resampler = nullptr;
	}
	return dk_celt_encoder::err_code_success;
}