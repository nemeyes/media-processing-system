#include "dk_audio_buffer.h"
#include "dk_circular_buffer.h"
#include <dk_auto_lock.h>

dk_audio_buffer::dk_audio_buffer(size_t buffer_size)
	: _root(0)
	, _begin(0)
	, _end(0)
	, _buffer_size(buffer_size)
	, _mt(dk_media_buffering::AUDIO_SUBMEDIA_TYPE_UNKNOWN)
{
	::InitializeCriticalSection(&_mutex);

	_cbuffer = dk_circular_buffer_create(buffer_size);

	_root = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
	init(_root);
}

dk_audio_buffer::~dk_audio_buffer(void)
{
	while (_root->next)
	{
		buffer_t * buffer = _root->next;
		_root->next = buffer->next;
		free(buffer);
	}

	if (_root)
		free(_root);

	dk_circular_buffer_destroy(_cbuffer);
	::DeleteCriticalSection(&_mutex);
}

dk_media_buffering::ERR_CODE dk_audio_buffer::push(const uint8_t * data, size_t size, long long pts)
{
	dk_media_buffering::ERR_CODE status = dk_media_buffering::ERR_CODE_SUCCESS;
	if (data && size > 0)
	{
		dk_auto_lock lock(&_mutex);
		buffer_t * buffer = _root;
		buffer->amount = MAX_AUDIO_FRAME_SIZE;

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

dk_media_buffering::ERR_CODE dk_audio_buffer::pop(uint8_t * data, size_t & size, long long & pts)
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
		pts = buffer->pts;
		//wchar_t debug[500];
		//_snwprintf_s(debug, sizeof(debug), L"<<pop video data[%zu]\n", vbuffer->amount);
		//OutputDebugString(debug);
		free(buffer);
	}
	return status;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::set_submedia_type(dk_media_buffering::AUDIO_SUBMEDIA_TYPE mt)
{
	_mt = mt;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::set_configstr(uint8_t * configstr, size_t size)
{
	_configstr_size = size;
	memcpy(_configstr, configstr, _configstr_size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::set_samplerate(int32_t samplerate)
{
	_samplerate = samplerate;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::set_bitdepth(int32_t bitdepth)
{
	_bitdepth = bitdepth;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::set_channels(int32_t channels)
{
	_channels = channels;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::get_submedia_type(dk_media_buffering::AUDIO_SUBMEDIA_TYPE & mt)
{
	mt =_mt;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::get_configstr(uint8_t * configstr, size_t & size)
{
	size = _configstr_size;
	memcpy(configstr, _configstr, size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::get_samplerate(int32_t & samplerate)
{
	samplerate = _samplerate;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::get_bitdepth(int32_t & bitdepth)
{
	bitdepth = _bitdepth;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::get_channels(int32_t & channels)
{
	channels = _channels;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_audio_buffer::init(buffer_t * buffer)
{
	buffer->pts = 0;
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return dk_media_buffering::ERR_CODE_SUCCESS;
}


