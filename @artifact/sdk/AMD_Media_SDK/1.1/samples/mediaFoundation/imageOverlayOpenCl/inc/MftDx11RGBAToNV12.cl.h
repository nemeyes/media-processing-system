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
uint read_and_generate_Y(__read_only image2d_t img, int x, int y, int2 min_val, int2 max_val)                               \
{                                                                                                                           \
    float4 rgb_pixel = read_img(img, x, y, min_val, max_val);                                                               \
                                                                                                                            \
    int R = (int)(rgb_pixel.x * 255);                                                                                       \
    int G = (int)(rgb_pixel.y * 255);                                                                                       \
    int B = (int)(rgb_pixel.z * 255);                                                                                       \
                                                                                                                            \
    int Y = clamp(((((66 * R) + (129 * G) + (25 * B) + 128) >> 8) + 16), 0, 255);                                           \
    return ((uint)Y);                                                                                                       \
}                                                                                                                           \
                                                                                                                            \
uint read_and_generate_UV(__read_only image2d_t img, int x, int y, int2 min_val, int2 max_val, uint U_or_V)                 \
{                                                                                                                           \
    float4 rgb_pixel = read_img(img, x, y, min_val, max_val);                                                               \
                                                                                                                            \
    int R = (int)(rgb_pixel.x * 255);                                                                                       \
    int G = (int)(rgb_pixel.y * 255);                                                                                       \
    int B = (int)(rgb_pixel.z * 255);                                                                                       \
                                                                                                                            \
    if (0 == U_or_V) {                                                                                                      \
    int U = clamp(((((-38 * R) - (74 * G) + (112 * B) + 128) >> 8) + 128), 0, 255);                                         \
        return ((uint)U);                                                                                                   \
    } else {                                                                                                                \
    int V = clamp(((((112 * R) - (94 * G) - (18 * B) + 128) >> 8) + 128), 0, 255);                                          \
        return ((uint)V);                                                                                                   \
    }                                                                                                                       \
}                                                                                                                           \
                                                                                                                            \
__kernel void                                                                                                               \
colorconvertrgbatonv12(__read_only image2d_t imageIn, int inputWidth, int inputHeight,                                      \
        __write_only image2d_t imageOut, int outputWidth, int outputHeight)                                                 \
{                                                                                                                           \
    const int ix = get_global_id(0);                                                                                        \
    const int iy = get_global_id(1);                                                                                        \
	                                                                                                                        \
	uint4 color_src;                                                                                                        \
    int2 min_val;                                                                                                           \
    int2 max_val;                                                                                                           \
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
    color_src.x = read_and_generate_Y(imageIn, ix * 2, inputHeight - 1 - (iy * 2), min_val, max_val);                       \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src.x);                                                             \
                                                                                                                            \
    out_x = (ix * 2) + 1;                                                                                                   \
    out_y = iy * 2;                                                                                                         \
    color_src.x = read_and_generate_Y(imageIn, (ix * 2) + 1, inputHeight - 1 - (iy * 2), min_val, max_val);                 \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src.x);                                                             \
                                                                                                                            \
    out_x = ix * 2;                                                                                                         \
    out_y = (iy * 2) + 1;                                                                                                   \
    color_src.x = read_and_generate_Y(imageIn, (ix * 2), inputHeight - 2 - (iy * 2), min_val, max_val);                     \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src.x);                                                             \
                                                                                                                            \
    out_x = (ix * 2) + 1;                                                                                                   \
    out_y = (iy * 2) + 1;                                                                                                   \
    color_src.x = read_and_generate_Y(imageIn, (ix * 2) + 1, inputHeight - 2 - (iy * 2), min_val, max_val);                 \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src.x);                                                             \
                                                                                                                            \
                                                                                                                            \
                                    /* Generate UV Pixels */                                                                \
                                                                                                                            \
    out_x = ix * 2;                                                                                                         \
    out_y = inputHeight +  iy;                                                                                              \
                                                                                                                            \
    color_src.x = read_and_generate_UV(imageIn, ix * 2, inputHeight - 1 - (iy * 2), min_val, max_val, 0 /* It means U */);  \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src.x);                                                             \
                                                                                                                            \
    color_src.x = read_and_generate_UV(imageIn, ix * 2, inputHeight - 1 - (iy * 2), min_val, max_val, 1 /* It means V */);  \
    write_imageui(imageOut, (int2)(out_x + 1, out_y), color_src.x);                                                         \
                                                                                                                            \
}                                                                                                                           \
";

#endif
