#if !defined(_DK_SIMD_COLORSPACE_CONVERTER_H_)
#define _DK_SIMD_COLORSPACE_CONVERTER_H_

#include <cstdint>

#if defined(EXPORT_LIB)
class __declspec(dllexport) dk_simd_colorspace_converter
#else
class __declspec(dllimport) dk_simd_colorspace_converter
#endif
{
public:
	typedef enum _COLOR_SPACE
	{
		COLOR_SPACE_YV12,
		COLOR_SPACE_NV12,
		COLOR_SPACE_RGB24,
		COLOR_SPACE_RGB32
	} COLOR_SPACE;

	static void flip(int32_t width, int32_t height, int32_t stride, uint8_t * pixels);
	static void convert_rgba_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip = false);
	static void convert_rgba_to_yv12(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip = false);


	static void convert_i420_to_rgba(int32_t width, int32_t height, uint8_t * y, int32_t y_stride, uint8_t * u, int32_t u_stride, uint8_t * v, int32_t v_stride, 
									 uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip = false);
	static void convert_i420_to_rgb(int32_t width, int32_t height, uint8_t * y, int32_t y_stride, uint8_t * u, int32_t u_stride, uint8_t * v, int32_t v_stride,
									uint8_t * dst, int32_t dst_stride, bool flip = false);
	static void convert_i420_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip = false);
	static void convert_i420_to_rgb(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip = false);

	static void convert_yv12_to_rgba(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, uint8_t alpha, bool flip = false);
	static void convert_yv12_to_rgb(int32_t width, int32_t height, uint8_t * src, int32_t src_stride, uint8_t * dst, int32_t dst_stride, bool flip = false);

private:
	dk_simd_colorspace_converter(void);
	~dk_simd_colorspace_converter(void);

private:
	static uint8_t _buffer[8294400]; //1920*1080*4

};

#endif