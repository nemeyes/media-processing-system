
#include "dk_audio_base.h"
#include <dk_auto_lock.h>
#include <dk_circular_buffer.h>


#define MAX_CHANNELS	8
#define MAX_SAMPLE_RATE	48000
#define MAX_BIT_DEPTH	16
#define MAX_AUDIO_SIZE	MAX_CHANNELS*MAX_SAMPLE_RATE*MAX_BIT_DEPTH*sizeof(int16_t) / 8

dk_audio_base::dk_audio_base(void)
{
	::InitializeCriticalSection(&_mutex);
	_aqueue = dk_circular_buffer_create(MAX_AUDIO_SIZE);
	_root = static_cast<abuffer_t*>(malloc(sizeof(abuffer_t)));
	init(_root);
}

dk_audio_base::~dk_audio_base(void)
{
	dk_circular_buffer_destroy(_aqueue);
	::DeleteCriticalSection(&_mutex);
}

dk_audio_base::err_code dk_audio_base::push(uint8_t * bs, size_t size, long long pts)
{
	dk_audio_base::err_code status = dk_audio_base::err_code_success;
	dk_auto_lock lock(&_mutex);
	if (bs && size > 0)
	{
		abuffer_t * abuffer = _root;
		abuffer->amount = MAX_AUDIO_SIZE;
		//move to tail
		do
		{
			if (!abuffer->next)
				break;
			abuffer = abuffer->next;
		} while (1);

		abuffer->next = static_cast<abuffer_t*>(malloc(sizeof(abuffer_t)));
		init(abuffer->next);
		abuffer->next->prev = abuffer;
		abuffer = abuffer->next;

		abuffer->amount = size;
		abuffer->pts = pts;
		int32_t result = dk_circular_buffer_write(_aqueue, bs, abuffer->amount);
		if (result == -1)
		{
			if (abuffer->prev)
				abuffer->prev->next = nullptr;
			free(abuffer);
			abuffer = nullptr;
			status = dk_audio_base::err_code_fail;
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

dk_audio_base::err_code dk_audio_base::pop(uint8_t * bs, size_t & size, long long & pts)
{
	dk_audio_base::err_code status = dk_audio_base::err_code_success;
	size = 0;
	dk_auto_lock lock(&_mutex);
	abuffer_t * abuffer = _root->next;
	if (abuffer)
	{
		_root->next = abuffer->next;
		if (_root->next)
			_root->next->prev = _root;

		int32_t result = dk_circular_buffer_read(_aqueue, bs, abuffer->amount);
		if (result == -1)
			status = dk_audio_base::err_code_fail;

		size = abuffer->amount;
		pts = abuffer->pts;
		//wchar_t debug[500];
		//_snwprintf_s(debug, sizeof(debug), L"<<pop audio data[%zu]\n", abuffer->amount);
		//OutputDebugString(debug);

		free(abuffer);
	}
	return status;
}

dk_audio_base::err_code dk_audio_base::init(abuffer_t * buffer)
{
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return dk_audio_base::err_code_success;
}


dk_audio_decoder::_configuration_t::_configuration_t(void)
	: samplerate(48000)
	, channels(2)
	, bitdepth(16)
	, bitrate(0)
	, extradata_size(0)
{
	memset(extradata, 0x00, sizeof(extradata));
}

dk_audio_decoder::_configuration_t::_configuration_t(const dk_audio_decoder::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	channels = clone.channels;
	bitdepth = clone.bitdepth;
	bitrate = clone.bitrate;
	extradata_size = clone.extradata_size;
	memcpy(extradata, clone.extradata, extradata_size);
}

dk_audio_decoder::_configuration_t dk_audio_decoder::_configuration_t::operator=(const dk_audio_decoder::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	channels = clone.channels;
	bitdepth = clone.bitdepth;
	bitrate = clone.bitrate;
	extradata_size = clone.extradata_size;
	memcpy(extradata, clone.extradata, extradata_size);
	return (*this);
}

dk_audio_decoder::dk_audio_decoder(void)
{

}

dk_audio_decoder::~dk_audio_decoder(void)
{

}

dk_audio_decoder::err_code dk_audio_decoder::initialize_decoder(void * config)
{
	return err_code_not_implemented;
}

dk_audio_decoder::err_code dk_audio_decoder::release_decoder(void)
{
	return err_code_not_implemented;
}

dk_audio_decoder::err_code dk_audio_decoder::decode(dk_audio_entity_t * encoded, dk_audio_entity_t * pcm)
{
	return err_code_not_implemented;
}

dk_audio_encoder::dk_audio_encoder(void)
{

}

dk_audio_encoder::~dk_audio_encoder(void)
{

}

dk_audio_encoder::err_code dk_audio_encoder::initialize_encoder(void * config)
{
	return err_code_not_implemented;
}

dk_audio_encoder::err_code dk_audio_encoder::release_encoder(void)
{
	return err_code_not_implemented;
}

dk_audio_encoder::err_code dk_audio_encoder::encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded)
{
	return err_code_not_implemented;
}

dk_audio_encoder::err_code dk_audio_encoder::encode(dk_audio_entity_t * pcm)
{
	return err_code_not_implemented;
}

dk_audio_encoder::err_code dk_audio_encoder::get_queued_data(dk_audio_entity_t * encoded)
{
	return err_code_not_implemented;
}

uint8_t * dk_audio_encoder::extradata(void)
{
	return nullptr;
}

size_t dk_audio_encoder::extradata_size(void)
{
	return 0;
}

dk_audio_renderer::_configuration_t::_configuration_t(void)
	: samplerate(0)
	, bitdepth(0)
	, channels(0)
{

}

dk_audio_renderer::_configuration_t::_configuration_t(const dk_audio_renderer::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	bitdepth = clone.bitdepth;
	channels = clone.channels;
}

dk_audio_renderer::_configuration_t & dk_audio_renderer::_configuration_t::operator=(const dk_audio_renderer::_configuration_t & clone)
{
	samplerate = clone.samplerate;
	bitdepth = clone.bitdepth;
	channels = clone.channels;
	return (*this);
}

dk_audio_renderer::dk_audio_renderer(void)
{

}

dk_audio_renderer::~dk_audio_renderer(void)
{

}

dk_audio_renderer::err_code dk_audio_renderer::initialize_renderer(void * config)
{
	return err_code_not_implemented;
}

dk_audio_renderer::err_code dk_audio_renderer::release_renderer(void)
{
	return err_code_not_implemented;
}

dk_audio_renderer::err_code dk_audio_renderer::render(dk_audio_renderer::dk_audio_entity_t * pcm)
{
	return err_code_not_implemented;
}