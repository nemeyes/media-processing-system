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

	static void flip(int width, int height, int stride, uint8_t * pixels);
	static void convert_rgba_to_rgba(int width, int height, uint8_t * src, int src_stride, uint8_t * dst, int dst_stride, bool flip = false);
	static void convert_rgba_to_yv12(int width, int height, uint8_t * bgra, int bgra_stride, uint8_t * y, int y_stride, uint8_t * u, int u_stride, uint8_t * v, int v_stride, bool flip = false);

private:
	dk_simd_colorspace_converter(void);
	~dk_simd_colorspace_converter(void);

private:
	static uint8_t _buffer[8294400]; //1920*1080*4

};

#endif