#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dk_colorspace_converter.h"
//#include "colorspace.h"
#include "cpu_features.h"
//#include <Simd/SimdLib.h>

unsigned char dk_colorspace_converter::_buffer[8294400]; //1920*1080*4

/*dk_colorspace_converter::dk_colorspace_converter(void)
	: _bflip(0)
	, _cvt_fn(0)
{
	colorspace_init();
}

dk_colorspace_converter::~dk_colorspace_converter(void)
{
	release();
}

void dk_colorspace_converter::initialize(dk_colorspace_converter::DK_COLOR_SPACE in, dk_colorspace_converter::DK_COLOR_SPACE out, bool flip)
{	
	if ((in == dk_colorspace_converter::COLOR_SPACE_RGB32) && (out == dk_colorspace_converter::COLOR_SPACE_YV12))
	{
		if (media::hasMMX())
			_cvt_fn = bgra_to_yv12_mmx;
		else
			_cvt_fn = bgra_to_yv12;
	}
	else if ((in == dk_colorspace_converter::COLOR_SPACE_RGB24) && (out == dk_colorspace_converter::COLOR_SPACE_YV12))
	{
		if (media::hasMMX())
			_cvt_fn = bgr_to_yv12_mmx;
		else
			_cvt_fn = bgr_to_yv12;
	}
	else if ((in == dk_colorspace_converter::COLOR_SPACE_YV12) && (out == dk_colorspace_converter::COLOR_SPACE_RGB32))
	{
		if (media::hasMMX())
			_cvt_fn = yv12_to_bgra_mmx;
		else
			_cvt_fn = yv12_to_bgra;
	}
	else if ((in == dk_colorspace_converter::COLOR_SPACE_YV12) && (out == dk_colorspace_converter::COLOR_SPACE_RGB24))
	{
		if (media::hasMMX())
			_cvt_fn = yv12_to_bgr_mmx;
		else
			_cvt_fn = yv12_to_bgr;
		//_cvt_fn = yv12_to_bgr;
	}

	_ics = in;
	_ocs = out;
	_bflip = flip;
}

void dk_colorspace_converter::release(void)
{
	_cvt_fn = 0;
}

void dk_colorspace_converter::convert(int width, int height, unsigned char * src, int src_stride, unsigned char * dst, int dst_stride)
{
	unsigned char * y_plane = 0;
	unsigned char * u_plane = 0;
	unsigned char * v_plane = 0;
	unsigned char * rgb = 0;

	if ((_ics == dk_colorspace_converter::COLOR_SPACE_RGB32) || (_ics == dk_colorspace_converter::COLOR_SPACE_RGB24))
	{
		rgb = src;
		y_plane = dst;
		u_plane = y_plane + dst_stride*height;
		v_plane = u_plane + (dst_stride*height >> 2);
		(*_cvt_fn)(rgb, src_stride, y_plane, v_plane, u_plane, dst_stride, dst_stride >> 1, width, height, _bflip);
	}
	else
	{
		y_plane = src;
		u_plane = y_plane + src_stride*height;
		v_plane = u_plane + (src_stride*height >> 2);
		rgb = dst;
		(*_cvt_fn)(rgb, dst_stride, y_plane, v_plane, u_plane, src_stride, src_stride >> 1, width, height, _bflip);
	}


}*/

/*void dk_colorspace_converter::convert_rgb32_to_yv12(int width, int height, unsigned char * rgba, int stride,
													unsigned char * y, unsigned char * u, unsigned char * v, int y_stride, int uv_stride, bool bflip)
{
	bgra_to_yv12_mmx(rgba, width * 4, y, u, v, y_stride, uv_stride, width, height, bflip);
	//if (bflip)
	//	flip(width, height, rgba);
	//media::ConvertRGB32ToYUV_SSE2(rgba, y, u, v, width, height, stride, y_stride, uv_stride);
	//if(media::hasSSE2())
	//{
	//	media::ConvertRGB32ToYUV_SSE2(rgba, y, u, v, width, height, stride, y_stride, uv_stride);
	//}
	//else
	//{
	//	media::ConvertRGB32ToYUV_C(rgba, y, u, v, width, height, stride, y_stride, uv_stride);
	//}
}*/

void dk_colorspace_converter::flip(unsigned int width, unsigned int height, int stride, unsigned char * pixels)
{
	unsigned char * row = _buffer;// (unsigned char *)malloc(stride);
	unsigned char * low = pixels;
	unsigned char * high = &pixels[(height - 1) * stride];

	for (; low < high; low += stride, high -= stride)
	{
		memcpy(row, low, stride);
		memcpy(low, high, stride);
		memcpy(high, row, stride);
	}
	//free(row);
}

/*
void dk_colorspace_converter::flip(unsigned int width, unsigned int height, unsigned char * pixels)
{
	const size_t stride = width * 4;
	unsigned char * row = (unsigned char *)malloc(stride);
	unsigned char * low = pixels;
	unsigned char * high = &pixels[(height - 1) * stride];

	for (; low < high; low += stride, high -= stride)
	{
		memcpy(row, low, stride);
		memcpy(low, high, stride);
		memcpy(high, row, stride);
	}
	free(row);
}
*/

void dk_colorspace_converter::convert_rgba_to_rgba(unsigned int width, unsigned int height, unsigned char * src, int src_stride, unsigned char * dst, int dst_stride, bool flip)
{
	if (flip)
	{
		unsigned char * source = src + (height - 1)*src_stride;
		unsigned char * destination = dst;
		unsigned int bytes_row = width << 2;
		for (unsigned int h = 0; h < height; h++)
		{
			memcpy(destination, source, bytes_row);
			destination += dst_stride;
			source -= src_stride;
		}
	}
	else
	{
		unsigned char * source = src;
		unsigned char * destination = dst;
		unsigned int bytes_row = width << 2;
		for (unsigned int h = 0; h < height; h++)
		{
			memcpy(destination, source, bytes_row);
			destination += dst_stride;
			source += src_stride;
		}
	}
}

void dk_colorspace_converter::convert_rgba_to_yv12(unsigned int width, unsigned int height, unsigned char * bgra, int bgra_stride, unsigned char * y, int y_stride, unsigned char * u, int u_stride, unsigned char * v, int v_stride, bool flip)
{
	//bgra_to_yv12_mmx(bgra, width * 4, y, u, v, y_stride, u_stride, width, height, flip);

	if (flip)
		dk_colorspace_converter::flip(width, height, width*4, bgra);
	//media::ConvertRGB32ToYUV_SSE2(bgra, y, u, v, width, height, y_stride, y_stride, u_stride);
	//if(media::hasSSE2())
	//{
	//	media::ConvertRGB32ToYUV_SSE2(bgra, y, u, v, width, height, y_stride, y_stride, u_stride);
	//}
	//else
	//{
	//	media::ConvertRGB32ToYUV_C(bgra, y, u, v, width, height, y_stride, y_stride, u_stride);
	//}

	SimdBgraToYuv420p(bgra, width, height, bgra_stride, y, y_stride, u, u_stride, v, v_stride);
}