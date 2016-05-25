#include "dk_audio_buffer.h"
#include "dk_circular_buffer.h"
#include <dk_auto_lock.h>

debuggerking::audio_buffer::audio_buffer(size_t buffer_size)
	: _root(0)
	, _begin(0)
	, _end(0)
	, _buffer_size(buffer_size)
	, _mt(audio_buffer::audio_submedia_type_t::unknown)
{
	::InitializeCriticalSection(&_mutex);

	_cbuffer = circular_buffer_t::create(buffer_size);

	_root = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
	init(_root);
}

debuggerking::audio_buffer::~audio_buffer(void)
{
	while (_root->next)
	{
		buffer_t * buffer = _root->next;
		_root->next = buffer->next;
		free(buffer);
	}

	if (_root)
		free(_root);

	circular_buffer_t::destroy(_cbuffer);
	::DeleteCriticalSection(&_mutex);
}

int32_t debuggerking::audio_buffer::push(const uint8_t * data, size_t size, long long timestamp)
{
	int32_t status = audio_buffer::err_code_t::success;
	if (data && size > 0)
	{
		auto_lock lock(&_mutex);
		buffer_t * buffer = _root;
		buffer->amount = audio_buffer::max_media_value_t::max_audio_size;

		//move to tail
		do
		{
			if (!buffer->next)
				break;
			buffer = buffer->next;
		} while (1);

		buffer_t * next = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
		init(next);
		next->prev = buffer;
		buffer->next = next;
		buffer = next;

		buffer->timestamp = timestamp;
		buffer->amount = size;
		int32_t result = circular_buffer_t::write(_cbuffer, data, buffer->amount);
		if (result == -1)
		{
			if (buffer->prev)
				buffer->prev->next = nullptr;
			free(buffer);
			buffer = nullptr;
			status = audio_buffer::err_code_t::fail;
		}
	}
	return status;
}

int32_t debuggerking::audio_buffer::pop(uint8_t * data, size_t & size, long long & timestamp)
{
	int32_t status = audio_buffer::err_code_t::success;
	size = 0;
	auto_lock lock(&_mutex);
	buffer_t * buffer = _root->next;
	if (buffer)
	{
		buffer_t * next = buffer->next;
		buffer_t * prev = buffer->prev;
		int32_t result = circular_buffer_t::read(_cbuffer, data, buffer->amount);
		if (result == -1)
		{
			status = audio_buffer::err_code_t::fail;
		}
		else
		{
			size = buffer->amount;
			timestamp = buffer->timestamp;
		}
		free(buffer);
		if (next)
		{
			buffer = next;
			buffer->prev = prev;
			buffer->prev->next = buffer;
		}
		else
		{
			_root->next = nullptr;
		}
	}
	return status;
}

int32_t debuggerking::audio_buffer::set_submedia_type(int32_t mt)
{
	_mt = mt;
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::set_configstr(uint8_t * configstr, size_t size)
{
	_configstr_size = size;
	memcpy(_configstr, configstr, _configstr_size);
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::set_samplerate(int32_t samplerate)
{
	_samplerate = samplerate;
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::set_bitdepth(int32_t bitdepth)
{
	_bitdepth = bitdepth;
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::set_channels(int32_t channels)
{
	_channels = channels;
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::get_submedia_type(int32_t & mt)
{
	mt =_mt;
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::get_configstr(uint8_t * configstr, size_t & size)
{
	size = _configstr_size;
	memcpy(configstr, _configstr, size);
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::get_samplerate(int32_t & samplerate)
{
	samplerate = _samplerate;
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::get_bitdepth(int32_t & bitdepth)
{
	bitdepth = _bitdepth;
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::get_channels(int32_t & channels)
{
	channels = _channels;
	return audio_buffer::err_code_t::success;
}

int32_t debuggerking::audio_buffer::init(audio_buffer::buffer_t * buffer)
{
	buffer->timestamp = 0;
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return audio_buffer::err_code_t::success;
}


