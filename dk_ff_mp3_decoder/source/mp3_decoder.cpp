#include "mp3_decoder.h"


mp3_decoder::mp3_decoder(void)
	: _codec(nullptr)
	, _context(nullptr)
	, _decoded_frame(nullptr)
{
	
}

mp3_decoder::~mp3_decoder(void)
{

}

dk_ff_mp3_decoder::ERR_CODE mp3_decoder::initialize_decoder(dk_ff_mp3_decoder::configuration_t * config)
{
	release_decoder();
	_config = *config;

	_codec = avcodec_find_decoder(AV_CODEC_ID_MP3);
	if (!_codec)
		return dk_ff_mp3_decoder::ERR_CODE_FAIL;

	_context = avcodec_alloc_context3(_codec);
	if (_context)
		return dk_ff_mp3_decoder::ERR_CODE_FAIL;
	_context->channels = config->channels;
	_context->sample_rate = config->samplerate;
	//_context->sample_fmt = 

	AVDictionary * avdic = nullptr;
	if (avcodec_open2(_context, _codec, &avdic)<0)
	{
		avcodec_close(_context);
		av_free(_context);
		_context = nullptr;
		_codec = nullptr;
	}

	av_init_packet(&_pkt);
	return dk_ff_mp3_decoder::ERR_CODE_SUCCESS;
}

dk_ff_mp3_decoder::ERR_CODE mp3_decoder::decode(dk_audio_entity_t * encoded, dk_audio_entity_t * pcm)
{
	pcm->data_size = 0;
	_pkt.data = static_cast<uint8_t*>(encoded->data);
	_pkt.size = encoded->data_size;

	while (_pkt.size > 0)
	{
		int32_t got_frame = 0;
		int32_t length = 0;

		if (!_decoded_frame) 
		{
			if (!(_decoded_frame = av_frame_alloc())) 
			{
				return dk_ff_mp3_decoder::ERR_CODE_FAIL;
			}
		}
		length  = avcodec_decode_audio4(_context, _decoded_frame, &got_frame, &_pkt);
		if (length < 0)
			return dk_ff_mp3_decoder::ERR_CODE_FAIL;
		if (got_frame)
		{
			int32_t data_size = av_get_bytes_per_sample(_context->sample_fmt);
			if (data_size < 0)
				return dk_ff_mp3_decoder::ERR_CODE_FAIL;

			for (int32_t i = 0; i < _decoded_frame->nb_samples; i++)
			{
				for (int32_t ch = 0; ch < _context->channels; ch++)
				{
					memcpy((uint8_t*)pcm->data + pcm->data_size, _decoded_frame->data[ch] + data_size*i, data_size);
					pcm->data_size += data_size;
				}
			}
		}
		_pkt.size -= length;
		_pkt.data += length;
		_pkt.dts = AV_NOPTS_VALUE;
		_pkt.pts = AV_NOPTS_VALUE;
	}
	return dk_ff_mp3_decoder::ERR_CODE_SUCCESS;
}

dk_ff_mp3_decoder::ERR_CODE mp3_decoder::release_decoder(void)
{
	if (_context)
	{
		avcodec_close(_context);
		av_free(_context);
		_context = nullptr;
		_codec = nullptr;
	}

	if (_decoded_frame)
	{
		av_frame_free(&_decoded_frame);
		_decoded_frame = nullptr;
	}
	return dk_ff_mp3_decoder::ERR_CODE_SUCCESS;
}