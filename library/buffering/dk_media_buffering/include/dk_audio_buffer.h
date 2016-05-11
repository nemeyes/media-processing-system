#pragma once

#include <dk_basic_type.h>
#include <windows.h>

namespace debuggerking
{
	typedef struct _circular_buffer_t circular_buffer_t;
	class audio_buffer : public foundation
	{
	public:
		typedef struct _buffer_t
		{
			long long timestamp;
			size_t amount;
			_buffer_t * prev;
			_buffer_t * next;
		} buffer_t;


		audio_buffer(size_t buffer_size = max_media_value_t::max_audio_size);
		~audio_buffer(void);

		int32_t push(const uint8_t * data, size_t size, long long pts);
		int32_t pop(uint8_t * data, size_t & size, long long & pts);

		int32_t set_submedia_type(int32_t mt);
		int32_t set_configstr(uint8_t * configstr, size_t size);
		int32_t set_samplerate(int32_t samplerate);
		int32_t set_bitdepth(int32_t bitdepth);
		int32_t set_channels(int32_t channels);

		int32_t get_submedia_type(int32_t & mt);
		int32_t get_configstr(uint8_t * configstr, size_t & size);
		int32_t get_samplerate(int32_t & samplerate);
		int32_t get_bitdepth(int32_t & bitdepth);
		int32_t get_channels(int32_t & channels);


	private:
		int32_t init(buffer_t *  buffer);

	private:
		int32_t _mt;
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

		circular_buffer_t * _cbuffer;
	};
};