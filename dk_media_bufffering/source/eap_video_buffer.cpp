#include "eap_video_buffer.h"
#include "eap_circular_buffer.h"

class eap_video_auto_lock
{
public:
	eap_video_auto_lock(CRITICAL_SECTION * lock)
		: _lock(lock)
	{
		::EnterCriticalSection(_lock);
	}

	~eap_video_auto_lock(void)
	{
		::LeaveCriticalSection(_lock);
	}
private:
	CRITICAL_SECTION * _lock;
};

eap_video_buffer::eap_video_buffer(size_t buffer_size)
	: _root(0)
	, _begin(0)
	, _end(0)
	, _buffer_size(buffer_size)
{
	::InitializeCriticalSection(&_mutex);
	
	_cbuffer = eap_circular_buffer_create(buffer_size);

	_root = static_cast<vbuffer_t*>(malloc(sizeof(vbuffer_t)));
	init(_root);
}

eap_video_buffer::~eap_video_buffer(void)
{
	if(_root)
		free(_root);

	eap_circular_buffer_destroy(_cbuffer);
	::DeleteCriticalSection(&_mutex);
}

eap_media_buffering::ERR_CODE eap_video_buffer::push_bitstream(uint8_t * bs, size_t size)
{
	eap_video_auto_lock lock(&_mutex);
	return push(bs, size);
}

eap_media_buffering::ERR_CODE eap_video_buffer::pop_bitstream(uint8_t * bs, size_t & size)
{
	eap_media_buffering::ERR_CODE status = eap_media_buffering::ERR_CODE_FAILED;
	eap_video_auto_lock lock(&_mutex);
	status = pop(bs, size);
	return status;
}

eap_media_buffering::ERR_CODE eap_video_buffer::set_vps(uint8_t * vps, size_t size)
{
	_vps_size = size;
	memcpy(_vps, vps, size);
	return eap_media_buffering::ERR_CODE_SUCCESS;
}

eap_media_buffering::ERR_CODE eap_video_buffer::set_sps(uint8_t * sps, size_t size)
{
	_sps_size = size;
	memcpy(_sps, sps, size);
	return eap_media_buffering::ERR_CODE_SUCCESS;
}

eap_media_buffering::ERR_CODE eap_video_buffer::set_pps(uint8_t * pps, size_t size)
{
	_pps_size = size;
	memcpy(_pps, pps, size);
	return eap_media_buffering::ERR_CODE_SUCCESS;
}

eap_media_buffering::ERR_CODE eap_video_buffer::get_vps(uint8_t * vps, size_t & size)
{
	size = _vps_size;
	memcpy(vps, _vps, _vps_size);
	return eap_media_buffering::ERR_CODE_SUCCESS;
}

eap_media_buffering::ERR_CODE eap_video_buffer::get_sps(uint8_t * sps, size_t & size)
{
	size = _sps_size;
	memcpy(sps, _sps, _sps_size);
	return eap_media_buffering::ERR_CODE_SUCCESS;
}

eap_media_buffering::ERR_CODE eap_video_buffer::get_pps(uint8_t * pps, size_t & size)
{
	size = _pps_size;
	memcpy(pps, _pps, _pps_size);
	return eap_media_buffering::ERR_CODE_SUCCESS;
}


eap_media_buffering::ERR_CODE eap_video_buffer::push(uint8_t * bs, size_t size)
{
	eap_media_buffering::ERR_CODE status = eap_media_buffering::ERR_CODE_SUCCESS;
	if (bs && size > 0)
	{
		vbuffer_t * vbuffer = _root;
		vbuffer->amount = MAX_VIDEO_FRAME_SIZE;
		do
		{
			if (vbuffer->amount < 1)
			{
				vbuffer->amount = size;
				int32_t result = eap_circular_buffer_write(_cbuffer, bs, vbuffer->amount);
				if (result == -1)
				{
					if(vbuffer->prev)
						vbuffer->prev->next = nullptr;
					free(vbuffer);
					vbuffer = nullptr;

					status = eap_media_buffering::ERR_CODE_FAILED;
				}
				break;
			}
			vbuffer->next = static_cast<vbuffer_t*>(malloc(sizeof(vbuffer_t)));
			init(vbuffer->next);
			vbuffer = vbuffer->next;

		} while (1);
	}
	return status;
}

eap_media_buffering::ERR_CODE eap_video_buffer::pop(uint8_t * bs, size_t & size)
{
	eap_media_buffering::ERR_CODE status = eap_media_buffering::ERR_CODE_SUCCESS;
	size = 0;
	if (_root->next)
	{
		vbuffer_t * vbuffer = _root->next;
		_root->next = vbuffer->next;

		int32_t result = eap_circular_buffer_read(_cbuffer, bs, vbuffer->amount);
		if (result == -1)
			status = eap_media_buffering::ERR_CODE_FAILED;

		size = vbuffer->amount;
		free(vbuffer);
	}
	return status;
}

eap_media_buffering::ERR_CODE eap_video_buffer::init(vbuffer_t * buffer)
{
	buffer->amount = 0;
	buffer->next = nullptr;	
	buffer->prev = nullptr;
	return eap_media_buffering::ERR_CODE_SUCCESS;
}


