#include "dk_video_buffer.h"
#include "dk_circular_buffer.h"

class dk_video_auto_lock
{
public:
	dk_video_auto_lock(CRITICAL_SECTION * lock)
		: _lock(lock)
	{
		::EnterCriticalSection(_lock);
	}

	~dk_video_auto_lock(void)
	{
		::LeaveCriticalSection(_lock);
	}
private:
	CRITICAL_SECTION * _lock;
};

dk_video_buffer::dk_video_buffer(size_t buffer_size)
	: _root(0)
	, _begin(0)
	, _end(0)
	, _buffer_size(buffer_size)
{
	::InitializeCriticalSection(&_mutex);
	
	_cbuffer = dk_circular_buffer_create(buffer_size);

	_root = static_cast<vbuffer_t*>(malloc(sizeof(vbuffer_t)));
	init(_root);
}

dk_video_buffer::~dk_video_buffer(void)
{
	while (_root->next)
	{
		vbuffer_t * vbuffer = _root->next;
		_root->next = vbuffer->next;
		free(vbuffer);
	}

	if(_root)
		free(_root);

	dk_circular_buffer_destroy(_cbuffer);
	::DeleteCriticalSection(&_mutex);
}

dk_media_buffering::ERR_CODE dk_video_buffer::push(uint8_t * data, size_t size)
{
	dk_media_buffering::ERR_CODE status = dk_media_buffering::ERR_CODE_SUCCESS;
	if (data && size > 0)
	{
		dk_video_auto_lock lock(&_mutex);
		vbuffer_t * vbuffer = _root;
		vbuffer->amount = MAX_VIDEO_FRAME_SIZE;

		//move to tail
		do
		{
			if (!vbuffer->next)
				break;
			vbuffer = vbuffer->next;
		} while (1);

		vbuffer->next = static_cast<vbuffer_t*>(malloc(sizeof(vbuffer_t)));
		init(vbuffer->next);
		vbuffer->next->prev = vbuffer;
		vbuffer = vbuffer->next;

		vbuffer->amount = size;
		int32_t result = dk_circular_buffer_write(_cbuffer, data, vbuffer->amount);
		if (result == -1)
		{
			if (vbuffer->prev)
				vbuffer->prev->next = nullptr;
			free(vbuffer);
			vbuffer = nullptr;
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

dk_media_buffering::ERR_CODE dk_video_buffer::pop(uint8_t * data, size_t & size)
{
	dk_media_buffering::ERR_CODE status = dk_media_buffering::ERR_CODE_SUCCESS;
	size = 0;
	dk_video_auto_lock lock(&_mutex);
	vbuffer_t * vbuffer = _root->next;
	if (vbuffer)
	{
		_root->next = vbuffer->next;
		if (_root->next)
			_root->next->prev = _root;

		int32_t result = dk_circular_buffer_read(_cbuffer, data, vbuffer->amount);
		if (result == -1)
			status = dk_media_buffering::ERR_CODE_FAILED;

		size = vbuffer->amount;
		//wchar_t debug[500];
		//_snwprintf_s(debug, sizeof(debug), L"<<pop video data[%zu]\n", vbuffer->amount);
		//OutputDebugString(debug);
		free(vbuffer);
	}
	return status;
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

dk_media_buffering::ERR_CODE dk_video_buffer::init(vbuffer_t * buffer)
{
	buffer->amount = 0;
	buffer->next = nullptr;	
	buffer->prev = nullptr;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}


