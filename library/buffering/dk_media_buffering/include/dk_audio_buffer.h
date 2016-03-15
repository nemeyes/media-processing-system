#pragma once

#include "define.h"


// 44100 (Samples) * 2 (Bytes per Sample) * 8 (channels)
// 44100*2*8
#define MAX_AUDIO_FRAME_SIZE 705600

#include "define.h"

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

	buffering::err_code push(const uint8_t * data, size_t size, long long pts);
	buffering::err_code pop(uint8_t * data, size_t & size, long long & pts);

	buffering::err_code set_submedia_type(buffering::asubmedia_type mt);
	buffering::err_code set_configstr(uint8_t * configstr, size_t size);
	buffering::err_code set_samplerate(int32_t samplerate);
	buffering::err_code set_bitdepth(int32_t bitdepth);
	buffering::err_code set_channels(int32_t channels);

	buffering::err_code get_submedia_type(buffering::asubmedia_type & mt);
	buffering::err_code get_configstr(uint8_t * configstr, size_t & size);
	buffering::err_code get_samplerate(int32_t & samplerate);
	buffering::err_code get_bitdepth(int32_t & bitdepth);
	buffering::err_code get_channels(int32_t & channels);
	

private:
	buffering::err_code init(buffer_t *  buffer);

private:
	buffering::asubmedia_type _mt;
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