#ifndef MFTNV12TORGBA_CL_H

const char NV12TORGBA_PROGRAM_NAME[] = "colorconvertnv12torgba";

const char NV12TORGBA_OPENCL_SCRIPT[] = "                                                                                     \
const sampler_t colorConvertNV12ToRGBAImageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;    \
                                                                                                                              \
float4 read_image(__read_only image2d_t img, uint x, uint y, int2 min_val, int2 max_val)                                      \
{                                                                                                                             \
    int2 locate;                                                                                                              \
    locate.x = x;                                                                                                             \
    locate.y = y;                                                                                                             \
                                                                                                                              \
    locate = clamp(locate, min_val, max_val);                                                                                 \
                                                                                                                              \
    return (read_imagef(img, colorConvertNV12ToRGBAImageSampler, locate));                                                    \
}                                                                                                                             \
                                                                                                                              \
float4 generate_rgb(float y, float u, float v)                                                                                \
{                                                                                                                             \
    float4 rgb_pixel;                                                                                                         \
    int  r, g, b;                                                                                                             \
    int r1, g1, b1;                                                                                                           \
	int y1, u1, v1;																											  \
																															  \
	y1 = (int)(y * 255.0f);																								      \
	u1 = (int)(u * 255.0f);																									  \
	v1 = (int)(v * 255.0f);																									  \
                                                                                                                              \
    r = (int) (1.164f * (y1 - 16) + 1.596f * (v1 - 128));                                                                     \
    g = (int) (1.164f * (y1 - 16) - 0.813f * (v1 - 128) - 0.391f * (u1 - 128));                                               \
    b = (int) (1.164f * (y1 - 16) + 2.018f * (u1 - 128));                                                                     \
                                                                                                                              \
    r1 = (r < 0) ? (0) : ((r > 255) ? 255 : r);                                                                               \
    g1 = (g < 0) ? (0) : ((g > 255) ? 255 : g);                                                                               \
    b1 = (b < 0) ? (0) : ((b > 255) ? 255 : b);                                                                               \
                                                                                                                              \
    rgb_pixel.x = r1 / 255.0f;																								  \
    rgb_pixel.y = g1 / 255.0f;																								  \
    rgb_pixel.z = b1 / 255.0f;																								  \
    rgb_pixel.w = 1.0f;                                                                                                       \
                                                                                                                              \
    return (rgb_pixel);                                                                                                       \
}                                                                                                                             \
                                                                                                                              \
__kernel void                                                                                                                 \
colorconvertnv12torgba(__read_only image2d_t imageInLuma, __read_only image2d_t imageInChroma,								  \
        int inputWidth, int inputHeight, __write_only image2d_t imageOut)													  \
{                                                                                                                             \
    const int ix = get_global_id(0);                                                                                          \
    const int iy = get_global_id(1);                                                                                          \
	                                                                                                                          \
    int2 min_val;                                                                                                             \
    int2 max_val;                                                                                                             \
    float4 y1, y2, y3, y4;                                                                                                    \
    float4 rgb1, rgb2, rgb3, rgb4;                                                                                            \
    int2 min_val_chroma, max_val_chroma;                                                                                      \
	float4 uv;																									              \
                                                                                                                              \
    min_val.x = 0;                                                                                                            \
    min_val.y = 0;                                                                                                            \
    max_val.x = inputWidth - 1;                                                                                               \
    max_val.y = inputHeight - 1;                                                                                              \
                                                                                                                              \
    y1 = read_image(imageInLuma, (ix * 2), (iy * 2), min_val, max_val);                    		                              \
    y2 = read_image(imageInLuma, (ix * 2) + 1, (iy * 2), min_val, max_val);                                                   \
    y3 = read_image(imageInLuma, (ix * 2), (iy * 2) + 1, min_val, max_val);                                                   \
    y4 = read_image(imageInLuma, (ix * 2) + 1, (iy * 2) + 1, min_val, max_val);                                               \
                                                                                                                              \
    min_val_chroma.x = 0;                                                                                                     \
    min_val_chroma.y = 0;																									  \
	max_val_chroma.x = (inputWidth  / 2) - 1;                                                                                 \
    max_val_chroma.y = (inputHeight / 2) - 1;																				  \
                                                                                                                              \
    uv = read_image(imageInChroma, ix, iy, min_val_chroma, max_val_chroma);										              \
                                                                                                                              \
    rgb1 = generate_rgb(y1.x, uv.x, uv.y);                                                                                    \
    write_imagef(imageOut, (int2)(ix * 2, (inputHeight - 1) - (iy * 2)), rgb1);                                               \
                                                                                                                              \
    rgb2 = generate_rgb(y2.x, uv.x, uv.y);                                                                                    \
    write_imagef(imageOut, (int2)((ix * 2) + 1, (inputHeight - 1) - (iy * 2)), rgb2);                                         \
                                                                                                                              \
    rgb3 = generate_rgb(y3.x, uv.x, uv.y);                                                                                    \
    write_imagef(imageOut, (int2)(ix * 2, (inputHeight - 2) - (iy * 2)), rgb3);                                               \
                                                                                                                              \
    rgb4 = generate_rgb(y4.x, uv.x, uv.y);                                                                                    \
    write_imagef(imageOut, (int2)((ix * 2) + 1, (inputHeight - 2) - (iy * 2)), rgb4);                                         \
}                                                                                                                             \
";

#endif
