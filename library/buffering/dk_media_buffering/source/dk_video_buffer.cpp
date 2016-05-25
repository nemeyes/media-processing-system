#include "dk_video_buffer.h"
#include "dk_circular_buffer.h"
#include <dk_auto_lock.h>

debuggerking::video_buffer::video_buffer(size_t buffer_size)
	: _root(0)
	, _begin(0)
	, _end(0)
	, _buffer_size(buffer_size)
	, _mt(video_buffer::video_submedia_type_t::unknown)
{
	::InitializeCriticalSection(&_mutex);
	
	_cbuffer = circular_buffer_t::create(buffer_size);

	_root = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
	init(_root);
}

debuggerking::video_buffer::~video_buffer(void)
{
	while (_root->next)
	{
		buffer_t * buffer = _root->next;
		_root->next = buffer->next;
		free(buffer);
	}

	if(_root)
		free(_root);

	circular_buffer_t::destroy(_cbuffer);
	::DeleteCriticalSection(&_mutex);
}

int32_t debuggerking::video_buffer::push(const uint8_t * data, size_t size, long long timestamp)
{
	int32_t status = video_buffer::err_code_t::success;
	if (data && size > 0)
	{
		auto_lock lock(&_mutex);
		buffer_t * buffer = _root;
		buffer->amount = video_buffer::max_media_value_t::max_video_size;

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
			status = video_buffer::err_code_t::fail;
		}
	}
	return status;
}

int32_t debuggerking::video_buffer::pop(uint8_t * data, size_t & size, long long & timestamp)
{
	int32_t status = video_buffer::err_code_t::success;
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
			status = video_buffer::err_code_t::fail;
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

int32_t debuggerking::video_buffer::set_submedia_type(int32_t mt)
{
	_mt = mt;
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::set_vps(uint8_t * vps, size_t size)
{
	_vps_size = size;
	memcpy(_vps, vps, size);
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::set_sps(uint8_t * sps, size_t size)
{
	_sps_size = size;
	memcpy(_sps, sps, size);
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::set_pps(uint8_t * pps, size_t size)
{
	_pps_size = size;
	memcpy(_pps, pps, size);
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::set_width(int32_t width)
{
	_width = width;
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::set_height(int32_t height)
{
	_height = height;
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::get_submedia_type(int32_t & mt)
{
	mt = _mt;
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::get_vps(uint8_t * vps, size_t & size)
{
	size = _vps_size;
	memcpy(vps, _vps, _vps_size);
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::get_sps(uint8_t * sps, size_t & size)
{
	size = _sps_size;
	memcpy(sps, _sps, _sps_size);
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::get_pps(uint8_t * pps, size_t & size)
{
	size = _pps_size;
	memcpy(pps, _pps, _pps_size);
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::get_width(int32_t & width)
{
	width = _width;
	return video_buffer::err_code_t::success;
}

int32_t debuggerking::video_buffer::get_height(int32_t & height)
{
	height = _height;
	return video_buffer::err_code_t::success;
}

const uint8_t * debuggerking::video_buffer::get_vps(size_t & size) const
{
	size = _vps_size;
	return _vps;
}

const uint8_t * debuggerking::video_buffer::get_sps(size_t & size) const
{
	size = _sps_size;
	return _sps;
}

const uint8_t * debuggerking::video_buffer::get_pps(size_t & size) const
{
	size = _pps_size;
	return _pps;
}

int32_t debuggerking::video_buffer::init(video_buffer::buffer_t * buffer)
{
	buffer->timestamp = 0;
	buffer->amount = 0;
	buffer->next = nullptr;	
	buffer->prev = nullptr;
	return video_buffer::err_code_t::success;
}


