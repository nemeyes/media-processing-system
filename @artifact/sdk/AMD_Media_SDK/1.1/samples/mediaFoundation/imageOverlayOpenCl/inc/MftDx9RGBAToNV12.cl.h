#ifndef MFTRGBATONV12_CL_H

const char RGBATONV12_PROGRAM_NAME[] = "colorconvertrgbatonv12"; 

const char RGBATONV12_OPENCL_SCRIPT[] = "                                                                                   \
const sampler_t colorConvertRGBAToNV12ImageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;  \
                                                                                                                            \
float4 read_img(__read_only image2d_t img, uint x, uint y, int2 min_val, int2 max_val)                                      \
{                                                                                                                           \
    int2 locate;                                                                                                            \
    locate.x = x;                                                                                                           \
    locate.y = y;                                                                                                           \
                                                                                                                            \
    locate = clamp(locate, min_val, max_val);                                                                               \
                                                                                                                            \
    return (read_imagef(img, colorConvertRGBAToNV12ImageSampler, locate));                                                  \
}                                                                                                                           \
                                                                                                                            \
float read_and_generate_Y(__read_only image2d_t img, int x, int y, int2 min_val, int2 max_val)                              \
{                                                                                                                           \
    float4 rgb_pixel = read_img(img, x, y, min_val, max_val);                                                               \
	float Y1;																												\
                                                                                                                            \
    int R = (int)(rgb_pixel.x * 255);                                                                                       \
    int G = (int)(rgb_pixel.y * 255);                                                                                       \
    int B = (int)(rgb_pixel.z * 255);                                                                                       \
                                                                                                                            \
    int Y = clamp(((((66 * R) + (129 * G) + (25 * B) + 128) >> 8) + 16), 0, 255);                                           \
	Y1 = (float)Y / 255.0f;																									\
    return (Y1);																											\
}                                                                                                                           \
                                                                                                                            \
float read_and_generate_UV(__read_only image2d_t img, int x, int y, int2 min_val, int2 max_val, uint U_or_V)                \
{                                                                                                                           \
    float4 rgb_pixel = read_img(img, x, y, min_val, max_val);                                                               \
	float U1, V1;																											\
                                                                                                                            \
    int R = (int)(rgb_pixel.x * 255);                                                                                       \
    int G = (int)(rgb_pixel.y * 255);                                                                                       \
    int B = (int)(rgb_pixel.z * 255);                                                                                       \
                                                                                                                            \
    if (0 == U_or_V) {                                                                                                      \
    int U = clamp(((((-38 * R) - (74 * G) + (112 * B) + 128) >> 8) + 128), 0, 255);                                         \
	U1 = (float)U / 255.0f;																									\
        return (U1);																										\
    } else {                                                                                                                \
    int V = clamp(((((112 * R) - (94 * G) - (18 * B) + 128) >> 8) + 128), 0, 255);                                          \
	V1 = (float)V / 255.0f;																									\
        return (V1);																										\
    }                                                                                                                       \
}                                                                                                                           \
                                                                                                                            \
__kernel void                                                                                                               \
colorconvertrgbatonv12(__read_only image2d_t imageInRGB, int inputWidth, int inputHeight,                                   \
        __write_only image2d_t imageOutLuma, __write_only image2d_t imageOutChroma)                                         \
{                                                                                                                           \
    const int ix = get_global_id(0);                                                                                        \
    const int iy = get_global_id(1);                                                                                        \
	                                                                                                                        \
	float4 color_src;                                                                                                       \
    int2 min_val;                                                                                                           \
    int2 max_val;                                                                                                           \
	color_src.x = 0.0f;																										\
	color_src.y = 0.0f;																										\
	color_src.z = 0.0f;																										\
	color_src.w = 1.0f;																										\
                                                                                                                            \
                                    /* Generate Y Pixels */                                                                 \
                                                                                                                            \
    min_val.x = 0;                                                                                                          \
    min_val.y = 0;                                                                                                          \
    max_val.x = inputWidth - 1;                                                                                             \
    max_val.y = inputHeight - 1;                                                                                            \
                                                                                                                            \
    int out_x = ix * 2;                                                                                                     \
    int out_y = iy * 2;                                                                                                     \
    color_src.x = read_and_generate_Y(imageInRGB, ix * 2, inputHeight - 1 - (iy * 2), min_val, max_val);					\
    write_imagef(imageOutLuma, (int2)(out_x, out_y), color_src);                                                            \
                                                                                                                            \
    out_x = (ix * 2) + 1;                                                                                                   \
    out_y = iy * 2;                                                                                                         \
    color_src.x = read_and_generate_Y(imageInRGB, (ix * 2) + 1, inputHeight - 1 - (iy * 2), min_val, max_val);				\
    write_imagef(imageOutLuma, (int2)(out_x, out_y), color_src);                                                            \
                                                                                                                            \
    out_x = ix * 2;                                                                                                         \
    out_y = (iy * 2) + 1;                                                                                                   \
    color_src.x = read_and_generate_Y(imageInRGB, (ix * 2), inputHeight - 2 - (iy * 2), min_val, max_val);					\
    write_imagef(imageOutLuma, (int2)(out_x, out_y), color_src);                                                            \
                                                                                                                            \
    out_x = (ix * 2) + 1;                                                                                                   \
    out_y = (iy * 2) + 1;                                                                                                   \
    color_src.x = read_and_generate_Y(imageInRGB, (ix * 2) + 1, inputHeight - 2 - (iy * 2), min_val, max_val);				\
    write_imagef(imageOutLuma, (int2)(out_x, out_y), color_src);                                                            \
                                                                                                                            \
                                                                                                                            \
                                    /* Generate UV Pixels */                                                                \
                                                                                                                            \
    out_x = ix;                                                                                                             \
    out_y = iy;																											    \
                                                                                                                            \
    color_src.x = read_and_generate_UV(imageInRGB, ix * 2, inputHeight - 1 - (iy * 2), min_val, max_val, 0 /* It means U */);	\
    color_src.y = read_and_generate_UV(imageInRGB, ix * 2, inputHeight - 1 - (iy * 2), min_val, max_val, 1 /* It means V */);   \
    write_imagef(imageOutChroma, (int2)(out_x, out_y), color_src);                                                          \
                                                                                                                            \
}                                                                                                                           \
";

#endif
