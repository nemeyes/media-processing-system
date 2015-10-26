#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dk_simd_colorspace_converter.h"

#include <Simd/SimdLib.h>

uint8_t dk_simd_colorspace_converter::_buffer[8294400]; //1920*1080*4

void dk_simd_colorspace_converter::flip(int width, int height, int stride, uint8_t * pixels)
{
	uint8_t * row = _buffer;// (unsigned char *)malloc(stride);
	uint8_t * low = pixels;
	uint8_t * high = &pixels[(height - 1) * stride];

	for (; low < high; low += stride, high -= stride)
	{
		memcpy(row, low, stride);
		memcpy(low, high, stride);
		memcpy(high, row, stride);
	}
}

void dk_simd_colorspace_converter::convert_rgba_to_rgba(int width, int height, uint8_t * src, int src_stride, unsigned char * dst, int dst_stride, bool flip)
{
	if (flip)
	{
		uint8_t * source = src + (height - 1)*src_stride;
		uint8_t * destination = dst;
		int bytes_row = width << 2;
		for (int h = 0; h < height; h++)
		{
			memcpy(destination, source, bytes_row);
			destination += dst_stride;
			source -= src_stride;
		}
	}
	else
	{
		uint8_t * source = src;
		uint8_t * destination = dst;
		int bytes_row = width << 2;
		for (int h = 0; h < height; h++)
		{
			memcpy(destination, source, bytes_row);
			destination += dst_stride;
			source += src_stride;
		}
	}
}

void dk_simd_colorspace_converter::convert_rgba_to_yv12(int width, int height, uint8_t * src, int src_stride, uint8_t * dst, int dst_stride, bool flip)
{
	int uv_stride = dst_stride >> 1;
	uint8_t * y = dst;
	uint8_t * v = y + (height)* dst_stride;
	uint8_t * u = v + (height >> 1) * uv_stride;

	if (flip)
		dk_simd_colorspace_converter::flip(width, height, src_stride, src);
	SimdBgraToYuv420p(src, width, height, src_stride, y, dst_stride, u, uv_stride, v, uv_stride);
}

/*
void dk_simd_colorspace_converter::convert_rgba_to_yv12(int width, int height, uint8_t * bgra, int bgra_stride, uint8_t * y, int y_stride, uint8_t * u, int u_stride, uint8_t * v, int v_stride, bool flip)
{
	if (flip)
		dk_simd_colorspace_converter::flip(width, height, width * 4, bgra);
	SimdBgraToYuv420p(bgra, width, height, bgra_stride, y, y_stride, u, u_stride, v, v_stride);
}
*/