#pragma once

#include <windows.h>
#include <cstdlib>
#include <cstdint>


// 44100 (Samples) * 2 (Bytes per Sample) * 8 (channels)
// 44100*2*8
#define MAX_AUDIO_FRAME_SIZE 705600

#include "dk_media_buffering.h"

typedef struct _dk_circular_buffer_t dk_circular_buffer_t;
class dk_audio_buffer
{
public:
	typedef struct _buffer_t
	{
		long long pts;
		size_t amount;
		_buffer_t * prev;
		_buffer_t * next;
	} buffer_t;


	dk_audio_buffer(size_t buffer_size = MAX_AUDIO_FRAME_SIZE);
	~dk_audio_buffer(void);

	dk_media_buffering::ERR_CODE push(const uint8_t * data, size_t size, long long pts);
	dk_media_buffering::ERR_CODE pop(uint8_t * data, size_t & size, long long & pts);

	dk_media_buffering::ERR_CODE set_submedia_type(dk_media_buffering::AUDIO_SUBMEDIA_TYPE mt);
	dk_media_buffering::ERR_CODE set_configstr(uint8_t * configstr, size_t size);
	dk_media_buffering::ERR_CODE set_samplerate(int32_t samplerate);
	dk_media_buffering::ERR_CODE set_bitdepth(int32_t bitdepth);
	dk_media_buffering::ERR_CODE set_channels(int32_t channels);

	dk_media_buffering::ERR_CODE get_submedia_type(dk_media_buffering::AUDIO_SUBMEDIA_TYPE & mt);
	dk_media_buffering::ERR_CODE get_configstr(uint8_t * configstr, size_t & size);
	dk_media_buffering::ERR_CODE get_samplerate(int32_t & samplerate);
	dk_media_buffering::ERR_CODE get_bitdepth(int32_t & bitdepth);
	dk_media_buffering::ERR_CODE get_channels(int32_t & channels);
	

private:
	dk_media_buffering::ERR_CODE init(buffer_t *  buffer);

private:
	dk_media_buffering::AUDIO_SUBMEDIA_TYPE _mt;
	uint8_t _configstr[200];
	size_t _configstr_size;
	int32_t _samplerate;
	int32_t _bitdepth;
	int32_t _channels;

	buffer_t * _root;


	int32_t _begin;
	int32_t _end;

	uint8_t * _buffer;
	int32_t _buffer_size;

	CRITICAL_SECTION _mutex;

	dk_circular_buffer_t * _cbuffer;
};