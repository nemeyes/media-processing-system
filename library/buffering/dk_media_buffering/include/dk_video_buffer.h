#pragma once

#include <windows.h>
#include <cstdlib>
#include <cstdint>
#define MAX_VIDEO_FRAME_SIZE 1024*1024*6

#include "define.h"

typedef struct _dk_circular_buffer_t dk_circular_buffer_t;
class dk_video_buffer
{
public:
	typedef struct _buffer_t
	{
		long long timestamp;
		size_t amount;
		_buffer_t * prev;
		_buffer_t * next;
	} buffer_t;


	dk_video_buffer(size_t buffer_size = MAX_VIDEO_FRAME_SIZE);
	~dk_video_buffer(void);

	buffering::err_code push(const uint8_t * data, size_t size, long long timestamp);
	buffering::err_code pop(uint8_t * data, size_t & size, long long & timestamp);


	buffering::err_code set_submedia_type(buffering::vsubmedia_type mt);
	buffering::err_code set_vps(uint8_t * vps, size_t size);
	buffering::err_code set_sps(uint8_t * sps, size_t size);
	buffering::err_code set_pps(uint8_t * pps, size_t size);
	buffering::err_code set_width(int32_t width);
	buffering::err_code set_height(int32_t height);

	buffering::err_code get_submedia_type(buffering::vsubmedia_type & timestamp);
	buffering::err_code get_vps(uint8_t * sps, size_t & size);
	buffering::err_code get_sps(uint8_t * sps, size_t & size);
	buffering::err_code get_pps(uint8_t * pps, size_t & size);
	buffering::err_code get_width(int32_t & width);
	buffering::err_code get_height(int32_t & height);

	const uint8_t * get_vps(size_t & size) const;
	const uint8_t * get_sps(size_t & size) const;
	const uint8_t * get_pps(size_t & size) const;

private:
	buffering::err_code init(buffer_t *  buffer);

private:
	buffering::vsubmedia_type _mt;
	uint8_t _vps[200];
	uint8_t _sps[200];
	uint8_t _pps[200];
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

	dk_circular_buffer_t * _cbuffer;
};