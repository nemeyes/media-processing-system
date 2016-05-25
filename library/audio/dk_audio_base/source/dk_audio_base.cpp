
#include "dk_audio_base.h"
#include <dk_auto_lock.h>
#include <dk_circular_buffer.h>

debuggerking::audio_base::_configuration_t::_configuration_t(void)
	: mode(audio_base::mode_t::none)
	, buffer_size(audio_base::max_media_value_t::max_video_size)
{

}

debuggerking::audio_base::_configuration_t::_configuration_t(const audio_base::_configuration_t & clone)
{
	mode = clone.mode;
}

debuggerking::audio_base::_configuration_t & debuggerking::audio_base::_configuration_t::operator=(const audio_base::_configuration_t & clone)
{
	mode = clone.mode;
	return (*this);
}


debuggerking::audio_base::audio_base(void)
{

}

debuggerking::audio_base::~audio_base(void)
{

}

int32_t debuggerking::audio_base::initialize(configuration_t * config)
{
	_config = config;
	if (_config->mode == audio_base::mode_t::sync)
	{
		::InitializeCriticalSection(&_mutex);
		_queue = circular_buffer_t::create(_config->buffer_size);
		_root = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
		init(_root);
	}
	return audio_base::err_code_t::success;
}

int32_t debuggerking::audio_base::release(void)
{
	if (_config->mode == audio_base::mode_t::sync)
	{
		audio_base::buffer_t * buffer = _root->next;
		while (buffer)
		{
			buffer->prev->next = buffer->next;
			free(buffer);
			buffer = nullptr;
		}
		free(_root);
		_root = nullptr;

		circular_buffer_t::destroy(_queue);
		::DeleteCriticalSection(&_mutex);
	}
	_config->mode = audio_base::mode_t::none;
	return audio_base::err_code_t::success;
}

int32_t debuggerking::audio_base::push(uint8_t * bs, size_t size, long long timestamp)
{
	if (_config->mode != audio_base::mode_t::sync)
		return audio_base::err_code_t::unsupported_function;

	int32_t status = audio_base::err_code_t::success;
	auto_lock lock(&_mutex);
	if (bs && size > 0)
	{
		buffer_t * buffer = _root;
		buffer->amount = _config->buffer_size;
		//move to tail
		do
		{
			if (!buffer->next)
				break;
			buffer = buffer->next;
		} while (1);

		buffer->next = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
		init(buffer->next);
		buffer->next->prev = buffer;
		buffer = buffer->next;

		buffer->amount = size;
		buffer->timestamp = timestamp;
		int32_t result = circular_buffer_t::write(_queue, bs, buffer->amount);
		if (result == -1)
		{
			if (buffer->prev)
				buffer->prev->next = nullptr;
			free(buffer);
			buffer = nullptr;
			status = audio_base::err_code_t::fail;
		}
	}
	return status;
}

int32_t debuggerking::audio_base::pop(uint8_t * bs, size_t & size, long long & timestamp)
{
	if (_config->mode != audio_base::mode_t::sync)
		return audio_base::err_code_t::unsupported_function;

	int32_t status = audio_base::err_code_t::success;
	size = 0;
	auto_lock lock(&_mutex);
	buffer_t * buffer = _root->next;
	if (buffer)
	{
		_root->next = buffer->next;
		if (_root->next)
			_root->next->prev = _root;

		int32_t result = circular_buffer_t::read(_queue, bs, buffer->amount);
		if (result == -1)
			status = audio_base::err_code_t::fail;

		size = buffer->amount;
		timestamp = buffer->timestamp;
		free(buffer);
		buffer = nullptr;
	}
	return status;
}

int32_t debuggerking::audio_base::init(audio_base::buffer_t * buffer)
{
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return audio_base::err_code_t::success;
}

void debuggerking::audio_base::set_extradata(uint8_t * extradata, size_t extradata_size)
{
	_extradata_size = extradata_size;
	memmove(_extradata, extradata, _extradata_size);
}

uint8_t * debuggerking::audio_base::get_extradata(size_t & extradata_size)
{
	extradata_size = _extradata_size;
	return _extradata;
}

debuggerking::audio_decoder::_configuration_t::_configuration_t(void)
	: samplerate(48000)
	, channels(2)
	, bitdepth(16)
	, bitrate(0)
	, extradata_size(0)
{
	memset(extradata, 0x00, sizeof(extradata));
}

debuggerking::audio_decoder::_configuration_t::_configuration_t(const audio_decoder::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	channels = clone.channels;
	bitdepth = clone.bitdepth;
	bitrate = clone.bitrate;
	extradata_size = clone.extradata_size;
	memcpy(extradata, clone.extradata, extradata_size);
}

debuggerking::audio_decoder::_configuration_t debuggerking::audio_decoder::_configuration_t::operator=(const audio_decoder::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	channels = clone.channels;
	bitdepth = clone.bitdepth;
	bitrate = clone.bitrate;
	extradata_size = clone.extradata_size;
	memcpy(extradata, clone.extradata, extradata_size);
	return (*this);
}

debuggerking::audio_decoder::audio_decoder(void)
{

}

debuggerking::audio_decoder::~audio_decoder(void)
{

}

int32_t debuggerking::audio_decoder::initialize_decoder(void * config)
{
	return audio_decoder::err_code_t::not_implemented;
}

int32_t debuggerking::audio_decoder::release_decoder(void)
{
	return audio_decoder::err_code_t::not_implemented;
}

int32_t debuggerking::audio_decoder::decode(audio_decoder::entity_t * encoded, audio_decoder::entity_t * pcm)
{
	return audio_decoder::err_code_t::not_implemented;
}

int32_t debuggerking::audio_decoder::decode(audio_decoder::entity_t * encoded)
{
	return audio_decoder::err_code_t::not_implemented;
}

int32_t debuggerking::audio_decoder::get_queued_data(audio_decoder::entity_t * pcm)
{
	return audio_decoder::err_code_t::not_implemented;
}

debuggerking::audio_encoder::_configuration_t::_configuration_t(void)
	: samplerate(48000)
	, channels(2)
	, bitrate(128000)
{
}

debuggerking::audio_encoder::_configuration_t::_configuration_t(const audio_encoder::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	channels = clone.channels;
	bitrate = clone.bitrate;
}

debuggerking::audio_encoder::_configuration_t debuggerking::audio_encoder::_configuration_t::operator=(const audio_encoder::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	channels = clone.channels;
	bitrate = clone.bitrate;
	return (*this);
}

debuggerking::audio_encoder::audio_encoder(void)
{

}

debuggerking::audio_encoder::~audio_encoder(void)
{

}

int32_t debuggerking::audio_encoder::initialize_encoder(void * config)
{
	return audio_encoder::err_code_t::not_implemented;
}

int32_t debuggerking::audio_encoder::release_encoder(void)
{
	return audio_encoder::err_code_t::not_implemented;
}

int32_t debuggerking::audio_encoder::encode(audio_encoder::entity_t * pcm, audio_encoder::entity_t * encoded)
{
	return audio_encoder::err_code_t::not_implemented;
}

int32_t debuggerking::audio_encoder::encode(audio_encoder::entity_t * pcm)
{
	return audio_encoder::err_code_t::not_implemented;
}

int32_t debuggerking::audio_encoder::get_queued_data(audio_encoder::entity_t * encoded)
{
	return audio_encoder::err_code_t::not_implemented;
}

debuggerking::audio_renderer::_configuration_t::_configuration_t(void)
	: samplerate(0)
	, bitdepth(0)
	, channels(0)
{

}

debuggerking::audio_renderer::_configuration_t::_configuration_t(const audio_renderer::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	bitdepth = clone.bitdepth;
	channels = clone.channels;
}

debuggerking::audio_renderer::_configuration_t & debuggerking::audio_renderer::_configuration_t::operator=(const audio_renderer::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	bitdepth = clone.bitdepth;
	channels = clone.channels;
	return (*this);
}

debuggerking::audio_renderer::audio_renderer(void)
{

}

debuggerking::audio_renderer::~audio_renderer(void)
{

}

int32_t debuggerking::audio_renderer::initialize_renderer(void * config)
{
	return audio_renderer::err_code_t::not_implemented;
}

int32_t debuggerking::audio_renderer::release_renderer(void)
{
	return audio_renderer::err_code_t::not_implemented;
}

int32_t debuggerking::audio_renderer::render(audio_renderer::entity_t * pcm)
{
	return audio_renderer::err_code_t::not_implemented;
}