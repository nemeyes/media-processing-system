#include "dk_video_buffer.h"
#include "dk_circular_buffer.h"
#include <dk_auto_lock.h>

dk_video_buffer::dk_video_buffer(size_t buffer_size)
	: _root(0)
	, _begin(0)
	, _end(0)
	, _buffer_size(buffer_size)
	, _mt(dk_media_buffering::VIDEO_SUBMEDIA_TYPE_UNKNOWN)
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

dk_media_buffering::ERR_CODE dk_video_buffer::push(const uint8_t * data, size_t size, long long timestamp)
{
	dk_media_buffering::ERR_CODE status = dk_media_buffering::ERR_CODE_SUCCESS;
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

		buffer->timestamp = timestamp;
		buffer->amount = size;
		int32_t result = dk_circular_buffer_write(_cbuffer, data, buffer->amount);
		if (result == -1)
		{
			if (buffer->prev)
				buffer->prev->next = nullptr;
			free(buffer);
			buffer = nullptr;
			status = dk_media_buffering::ERR_CODE_FAILED;
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

dk_media_buffering::ERR_CODE dk_video_buffer::pop(uint8_t * data, size_t & size, long long & timestamp)
{
	dk_media_buffering::ERR_CODE status = dk_media_buffering::ERR_CODE_SUCCESS;
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
			status = dk_media_buffering::ERR_CODE_FAILED;

		size = buffer->amount;
		timestamp = buffer->timestamp;
		//wchar_t debug[500];
		//_snwprintf_s(debug, sizeof(debug), L"<<pop video data[%zu]\n", vbuffer->amount);
		//OutputDebugString(debug);
		free(buffer);
	}
	return status;
}

dk_media_buffering::ERR_CODE dk_video_buffer::set_submedia_type(dk_media_buffering::VIDEO_SUBMEDIA_TYPE mt)
{
	_mt = mt;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::set_vps(uint8_t * vps, size_t size)
{
	_vps_size = size;
	memcpy(_vps, vps, size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::set_sps(uint8_t * sps, size_t size)
{
	_sps_size = size;
	memcpy(_sps, sps, size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::set_pps(uint8_t * pps, size_t size)
{
	_pps_size = size;
	memcpy(_pps, pps, size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::set_width(int32_t width)
{
	_width = width;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::set_height(int32_t height)
{
	_height = height;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::get_submedia_type(dk_media_buffering::VIDEO_SUBMEDIA_TYPE & mt)
{
	mt = _mt;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::get_vps(uint8_t * vps, size_t & size)
{
	size = _vps_size;
	memcpy(vps, _vps, _vps_size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::get_sps(uint8_t * sps, size_t & size)
{
	size = _sps_size;
	memcpy(sps, _sps, _sps_size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::get_pps(uint8_t * pps, size_t & size)
{
	size = _pps_size;
	memcpy(pps, _pps, _pps_size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::get_width(int32_t & width)
{
	width = _width;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::get_height(int32_t & height)
{
	height = _height;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_video_buffer::init(buffer_t * buffer)
{
	buffer->amount = 0;
	buffer->next = nullptr;	
	buffer->prev = nullptr;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}


