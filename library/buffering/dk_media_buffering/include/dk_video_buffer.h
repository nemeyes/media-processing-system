#pragma once

#include <dk_basic_type.h>
#include <windows.h>

namespace debuggerking
{
	typedef struct _circular_buffer_t circular_buffer_t;
	class video_buffer : public foundation
	{
	public:
		typedef struct _buffer_t
		{
			long long timestamp;
			size_t amount;
			_buffer_t * prev;
			_buffer_t * next;
		} buffer_t;


		video_buffer(size_t buffer_size = max_media_value_t::max_video_size);
		~video_buffer(void);

		int32_t push(const uint8_t * data, size_t size, long long timestamp);
		int32_t pop(uint8_t * data, size_t & size, long long & timestamp);


		int32_t set_submedia_type(int32_t mt);
		int32_t set_vps(uint8_t * vps, size_t size);
		int32_t set_sps(uint8_t * sps, size_t size);
		int32_t set_pps(uint8_t * pps, size_t size);
		int32_t set_width(int32_t width);
		int32_t set_height(int32_t height);

		int32_t get_submedia_type(int32_t & mt);
		int32_t get_vps(uint8_t * sps, size_t & size);
		int32_t get_sps(uint8_t * sps, size_t & size);
		int32_t get_pps(uint8_t * pps, size_t & size);
		int32_t get_width(int32_t & width);
		int32_t get_height(int32_t & height);

		const uint8_t * get_vps(size_t & size) const;
		const uint8_t * get_sps(size_t & size) const;
		const uint8_t * get_pps(size_t & size) const;

	private:
		int32_t init(buffer_t *  buffer);

	private:
		int32_t _mt;
		uint8_t _vps[MAX_PATH];
		uint8_t _sps[MAX_PATH];
		uint8_t _pps[MAX_PATH];
		size_t _vps_size;
		size_t _sps_size;
		size_t _pps_size;
		int32_t _width;
		int32_t _height;

		buffer_t * _root;

		int32_t _begin;
		int32_t _end;

		uint8_t * _buffer;
		int32_t _buffer_size;

		CRITICAL_SECTION _mutex;

		circular_buffer_t * _cbuffer;
	};
};