#include "dk_video_base.h"
#include <dk_circular_buffer.h>
#include <dk_auto_lock.h>

#define MAX_VIDEO_SIZE	1024*1024*2

dk_video_base::dk_video_base(bool use_builtin_queue)
	: _use_builtin_queue(use_builtin_queue)
{
	if (_use_builtin_queue)
	{
		::InitializeCriticalSection(&_mutex);
		_vqueue = dk_circular_buffer_create(MAX_VIDEO_SIZE);
		_root = static_cast<vbuffer_t*>(malloc(sizeof(vbuffer_t)));
		init(_root);
	}
}

dk_video_base::~dk_video_base(void)
{
	if (_use_builtin_queue)
	{
		vbuffer_t * vbuffer = _root->next;
		while (vbuffer)
		{
			vbuffer->prev->next = vbuffer->next;
			free(vbuffer);
			vbuffer = nullptr;
		}
		free(_root);
		_root = nullptr;

		dk_circular_buffer_destroy(_vqueue);
		::DeleteCriticalSection(&_mutex);
	}
}

dk_video_base::ERR_CODE dk_video_base::push(uint8_t * bs, size_t size)
{
	if (!_use_builtin_queue)
		return dk_video_base::ERR_CODE_UNSUPPORTED_FUNCTION;

	dk_video_base::ERR_CODE status = dk_video_base::ERR_CODE_SUCCESS;
	dk_auto_lock lock(&_mutex);
	if (bs && size > 0)
	{
		vbuffer_t * vbuffer = _root;
		vbuffer->amount = MAX_VIDEO_SIZE;
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
		int32_t result = dk_circular_buffer_write(_vqueue, bs, vbuffer->amount);
		if (result == -1)
		{
			if (vbuffer->prev)
				vbuffer->prev->next = nullptr;
			free(vbuffer);
			vbuffer = nullptr;
			status = dk_video_base::ERR_CODE_FAIL;
		}
		//else
		//{
		//	wchar_t debug[500];
		//	_snwprintf_s(debug, sizeof(debug), L">>push audio data[%zu]\n", abuffer->amount);
		//	OutputDebugString(debug);
		//}
	}
	return status;
}

dk_video_base::ERR_CODE dk_video_base::pop(uint8_t * bs, size_t & size)
{
	if (!_use_builtin_queue)
		return dk_video_base::ERR_CODE_UNSUPPORTED_FUNCTION;

	dk_video_base::ERR_CODE status = dk_video_base::ERR_CODE_SUCCESS;
	size = 0;
	dk_auto_lock lock(&_mutex);
	vbuffer_t * vbuffer = _root->next;
	if (vbuffer)
	{
		vbuffer->prev->next = vbuffer->next;
		int32_t result = dk_circular_buffer_read(_vqueue, bs, vbuffer->amount);
		if (result == -1)
		{
			status = dk_video_base::ERR_CODE_FAIL;
		}
		else
		{
			size = vbuffer->amount;
		}
		free(vbuffer);
		vbuffer = nullptr;
	}
	return status;
}

dk_video_base::ERR_CODE dk_video_base::init(vbuffer_t * buffer)
{
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	buffer->pts = 0;
	return dk_video_base::ERR_CODE_SUCCESS;
}

dk_video_decoder::dk_video_decoder(void)
{

}

dk_video_decoder::~dk_video_decoder(void)
{

}

dk_video_decoder::ERR_CODE dk_video_decoder::initialize_decoder(void * config)
{
	return ERR_CODE_SUCCESS;
}

dk_video_decoder::ERR_CODE dk_video_decoder::release_decoder(void)
{
	return ERR_CODE_SUCCESS;
}

dk_video_decoder::ERR_CODE dk_video_decoder::decode(dk_video_entity_t * bitstream, dk_video_entity_t * decoded)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_decoder::ERR_CODE dk_video_decoder::decode(dk_video_entity_t * bitstream)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_decoder::ERR_CODE dk_video_decoder::get_queued_data(dk_video_entity_t * bitstream)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_encoder::dk_video_encoder(bool use_builtin_queue)
	: dk_video_base(use_builtin_queue)
{

}

dk_video_encoder::~dk_video_encoder(void)
{

}

dk_video_encoder::ENCODER_STATE dk_video_encoder::state(void)
{
	return dk_video_encoder::ENCODER_STATE_NONE;
}

dk_video_encoder::ERR_CODE dk_video_encoder::initialize_encoder(void * config)
{
	return ERR_CODE_SUCCESS;
}

dk_video_encoder::ERR_CODE dk_video_encoder::release_encoder(void)
{
	return ERR_CODE_SUCCESS;
}

dk_video_encoder::ERR_CODE dk_video_encoder::encode(dk_video_entity_t * input, dk_video_entity_t * bitstream)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_encoder::ERR_CODE dk_video_encoder::encode(dk_video_entity_t * input)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_encoder::ERR_CODE dk_video_encoder::get_queued_data(dk_video_entity_t * bitstream)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_encoder::ERR_CODE dk_video_encoder::encode_async(dk_video_encoder::dk_video_entity_t * input)
{
	return dk_video_encoder::ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_encoder::ERR_CODE dk_video_encoder::check_encoding_finish(void)
{
	return dk_video_encoder::ERR_CODE_ENCODING_UNDER_PROCESSING;
}

const int dk_video_encoder::next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end)
{
	int i;
	*nal_start = 0;
	*nal_end = 0;

	i = 0;
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0 || bitstream[i + 3] != 0x01))
	{
		i++;
		if (i + 4 >= size)
			return 0;
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
		i++;

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
		return 0;/* error, should never happen */

	i += 3;
	*nal_start = i;
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01))
	{
		i++;
		if (i + 3 >= size)
		{
			*nal_end = size;
			return -1;
		}
	}

	*nal_end = i;
	return (*nal_end - *nal_start);
}

dk_video_renderer::dk_video_renderer(void)
{

}

dk_video_renderer::~dk_video_renderer(void)
{

}

dk_video_renderer::ERR_CODE dk_video_renderer::initialize_renderer(void * config)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_renderer::ERR_CODE dk_video_renderer::release_renderer(void)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_renderer::ERR_CODE dk_video_renderer::render(dk_video_entity_t * decoded)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}
