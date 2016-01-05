#pragma once

#include <windows.h>
#include <cstdlib>
#include <cstdint>
#define MAX_VIDEO_FRAME_SIZE 1024*1024*2

#include "dk_media_buffering.h"

typedef struct _dk_circular_buffer_t dk_circular_buffer_t;
class dk_video_buffer
{
public:
	typedef struct _buffer_t
	{
		size_t amount;
		_buffer_t * prev;
		_buffer_t * next;
	} buffer_t;


	dk_video_buffer(size_t buffer_size = MAX_VIDEO_FRAME_SIZE);
	~dk_video_buffer(void);

	dk_media_buffering::ERR_CODE push(uint8_t * data, size_t size);
	dk_media_buffering::ERR_CODE pop(uint8_t * data, size_t & size);

	dk_media_buffering::ERR_CODE set_vps(uint8_t * vps, size_t size);
	dk_media_buffering::ERR_CODE set_sps(uint8_t * sps, size_t size);
	dk_media_buffering::ERR_CODE set_pps(uint8_t * pps, size_t size);
	dk_media_buffering::ERR_CODE get_vps(uint8_t * sps, size_t & size);
	dk_media_buffering::ERR_CODE get_sps(uint8_t * sps, size_t & size);
	dk_media_buffering::ERR_CODE get_pps(uint8_t * pps, size_t & size);

private:
	dk_media_buffering::ERR_CODE init(buffer_t *  buffer);

private:
	uint8_t _vps[200];
	uint8_t _sps[200];
	uint8_t _pps[200];
	size_t _vps_size;
	size_t _sps_size;
	size_t _pps_size;

	buffer_t * _root;


	int32_t _begin;
	int32_t _end;

	uint8_t * _buffer;
	int32_t _buffer_size;

	CRITICAL_SECTION _mutex;

	dk_circular_buffer_t * _cbuffer;
};