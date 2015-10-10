#ifndef _DK_COLORSPACE_CONVERTER_H_
#define _DK_COLORSPACE_CONVERTER_H_

#include <stdint.h>

typedef void(*convert_function)(uint8_t * x_ptr, int x_stride, uint8_t * y_src, uint8_t * v_src, uint8_t * u_src, int y_stride, int uv_stride, int width, int height, int vflip);

#if defined(EXPORT_LIB)
class __declspec(dllexport) dk_colorspace_converter
#else
class __declspec(dllimport) dk_colorspace_converter
#endif
{
public:
	typedef enum _DK_COLOR_SPACE
	{
		COLOR_SPACE_YV12,
		COLOR_SPACE_NV12,
		COLOR_SPACE_RGB24,
		COLOR_SPACE_RGB32
	} DK_COLOR_SPACE;

	

	dk_colorspace_converter(void);
	~dk_colorspace_converter(void);

	void initialize(dk_colorspace_converter::DK_COLOR_SPACE in, dk_colorspace_converter::DK_COLOR_SPACE out, bool flip);
	void release(void);

	void convert(int width, int height, unsigned char * src, int src_stride, unsigned char * dst, int dst_stride);


	void convert_rgb32_to_yv12(int width, int height, unsigned char * rgba, int stride,
							   unsigned char * y, unsigned char * u, unsigned char * v, int y_stride, int uv_stride, bool flip=false);


	void convert_bgra_to_yv12_ssse3(const uint8_t * bgra, uint8_t * y, uint8_t * u, uint8_t * v, int width, int height, int bgrastride, int ystride);


	static void flip(unsigned int width, unsigned int height, int stride, unsigned char * pixels);
	static void convert_rgba_to_rgba(unsigned int width, unsigned int height, unsigned char * src, int src_stride, unsigned char * dst, int dst_stride, bool flip=false);
	static void convert_rgba_to_yv12(unsigned int width, unsigned int height, unsigned char * bgra, int bgra_stride, unsigned char * y, int y_stride, unsigned char * u, int u_stride, unsigned char * v, int v_stride, bool flip = false);
private:
	static unsigned char _buffer[8294400]; //1920*1080*4

	DK_COLOR_SPACE _ics;
	DK_COLOR_SPACE _ocs;
	bool _bflip;
	convert_function _cvt_fn;
};













#endif