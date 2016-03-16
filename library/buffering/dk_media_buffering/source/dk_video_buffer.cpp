#include "dk_video_buffer.h"
#include "dk_circular_buffer.h"
#include <dk_auto_lock.h>

dk_video_buffer::dk_video_buffer(size_t buffer_size)
	: _root(0)
	, _begin(0)
	, _end(0)
	, _buffer_size(buffer_size)
	, _mt(buffering::unknown_video_type)
{
	::InitializeCriticalSection(&_mutex);
	
	_cbuffer = dk_circular_buffer_create(buffer_size);

	_root = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
	init(_root);
}

dk_video_buffer::~dk_video_buffer(void)
{
	while (_root->next)
	{
		buffer_t * buffer = _root->next;
		_root->next = buffer->next;
		free(buffer);
	}

	if(_root)
		free(_root);

	dk_circular_buffer_destroy(_cbuffer);
	::DeleteCriticalSection(&_mutex);
}

buffering::err_code dk_video_buffer::push(const uint8_t * data, size_t size, long long pts)
{
	buffering::err_code status = buffering::err_code_success;
	if (data && size > 0)
	{
		dk_auto_lock lock(&_mutex);
		buffer_t * buffer = _root;
		buffer->amount = MAX_VIDEO_FRAME_SIZE;

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

		buffer->pts = pts;
		buffer->amount = size;
		int32_t result = dk_circular_buffer_write(_cbuffer, data, buffer->amount);
		if (result == -1)
		{
			if (buffer->prev)
				buffer->prev->next = nullptr;
			free(buffer);
			buffer = nullptr;
			status = buffering::err_code_failed;
		}
		//else
		//{
		//	wchar_t debug[500];
		//	_snwprintf_s(debug, sizeof(debug), L">>push video data[%zu]\n", vbuffer->amount);
		//	OutputDebugString(debug);
		//}
	}
	return status;
}

buffering::err_code dk_video_buffer::pop(uint8_t * data, size_t & size, long long & pts)
{
	buffering::err_code status = buffering::err_code_success;
	size = 0;
	dk_auto_lock lock(&_mutex);
	buffer_t * buffer = _root->next;
	if (buffer)
	{
		_root->next = buffer->next;
		if (_root->next)
			_root->next->prev = _root;

		int32_t result = dk_circular_buffer_read(_cbuffer, data, buffer->amount);
		if (result == -1)
			status = buffering::err_code_failed;

		size = buffer->amount;
		pts = buffer->pts;
		//wchar_t debug[500];
		//_snwprintf_s(debug, sizeof(debug), L"<<pop video data[%zu]\n", vbuffer->amount);
		//OutputDebugString(debug);
		free(buffer);
	}
	return status;
}

buffering::err_code dk_video_buffer::set_submedia_type(buffering::vsubmedia_type mt)
{
	_mt = mt;
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::set_vps(uint8_t * vps, size_t size)
{
	_vps_size = size;
	memcpy(_vps, vps, size);
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::set_sps(uint8_t * sps, size_t size)
{
	_sps_size = size;
	memcpy(_sps, sps, size);
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::set_pps(uint8_t * pps, size_t size)
{
	_pps_size = size;
	memcpy(_pps, pps, size);
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::set_width(int32_t width)
{
	_width = width;
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::set_height(int32_t height)
{
	_height = height;
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::get_submedia_type(buffering::vsubmedia_type & mt)
{
	mt = _mt;
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::get_vps(uint8_t * vps, size_t & size)
{
	size = _vps_size;
	memcpy(vps, _vps, _vps_size);
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::get_sps(uint8_t * sps, size_t & size)
{
	size = _sps_size;
	memcpy(sps, _sps, _sps_size);
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::get_pps(uint8_t * pps, size_t & size)
{
	size = _pps_size;
	memcpy(pps, _pps, _pps_size);
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::get_width(int32_t & width)
{
	width = _width;
	return buffering::err_code_success;
}

buffering::err_code dk_video_buffer::get_height(int32_t & height)
{
	height = _height;
	return buffering::err_code_success;
}

const uint8_t * dk_video_buffer::get_vps(size_t & size) const
{
	size = _vps_size;
	return _vps;
}

const uint8_t * dk_video_buffer::get_sps(size_t & size) const
{
	size = _sps_size;
	return _sps;
}

const uint8_t * dk_video_buffer::get_pps(size_t & size) const
{
	size = _pps_size;
	return _pps;
}

buffering::err_code dk_video_buffer::init(buffer_t * buffer)
{
	buffer->pts = 0;
	buffer->amount = 0;
	buffer->next = nullptr;	
	buffer->prev = nullptr;
	return buffering::err_code_success;
}


