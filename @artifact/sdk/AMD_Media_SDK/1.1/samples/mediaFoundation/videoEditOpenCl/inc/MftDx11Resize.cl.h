#ifndef MFTRESIZER_CL_H
const char RESIZER_PROGRAM_NAME[] = "resize"; 

const char RESIZER_OPENCL_SCRIPT[] = "\
const sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;                         \
                                                                                                                            \
uint read_img(__read_only image2d_t img, uint x, uint y, int2 min_val, int2 max_val)                                        \
{                                                                                                                           \
    int2 locate;                                                                                                            \
    locate.x = x;                                                                                                           \
    locate.y = y;                                                                                                           \
                                                                                                                            \
    locate = clamp(locate, min_val, max_val);                                                                               \
                                                                                                                            \
    return (read_imageui(img, imageSampler, locate).x);                                                                     \
}                                                                                                                           \
                                                                                                                            \
uint read_and_interpolate_Y(__read_only image2d_t img, float x, float y, int2 min_val, int2 max_val)                        \
{                                                                                                                           \
    uint x1 = floor(x);                                                                                                     \
    uint y1 = floor(y);                                                                                                     \
    uint x2 = x1 + 1;                                                                                                       \
    uint y2 = y1 + 1;                                                                                                       \
                                                                                                                            \
    float weight_top_right    = (x - x1) * (y - y1);                                                                        \
    float weight_bottom_right = (x - x1) * (y2 - y);                                                                        \
    float weight_bottom_left  = (x2 - x) * (y2 - y);                                                                        \
    float weight_top_left     = (x2 - x) * (y - y1);                                                                        \
                                                                                                                            \
    uint color_top_left     = read_img(img, x1, y2, min_val, max_val);                                                      \
    uint color_bottom_left  = read_img(img, x1, y1, min_val, max_val);                                                      \
    uint color_bottom_right = read_img(img, x2, y1, min_val, max_val);                                                      \
    uint color_top_right    = read_img(img, x2, y2, min_val, max_val);                                                      \
                                                                                                                            \
    return (color_top_left * weight_top_left + color_bottom_left * weight_bottom_left +                                     \
            color_bottom_right * weight_bottom_right + color_top_right * weight_top_right);                                 \
}                                                                                                                           \
                                                                                                                            \
uint read_and_interpolate_UV(__read_only image2d_t img, float x, float y, int2 min_val, int2 max_val, uint U_or_V, int hIn) \
{                                                                                                                           \
    uint x1 = floor(x);                                                                                                     \
    uint y1 = floor(y);                                                                                                     \
    uint x2 = x1 + 1;                                                                                                       \
    uint y2 = y1 + 1;                                                                                                       \
                                                                                                                            \
    float weight_top_right    = (x - x1) * (y - y1);                                                                        \
    float weight_bottom_right = (x - x1) * (y2 - y);                                                                        \
    float weight_bottom_left  = (x2 - x) * (y2 - y);                                                                        \
    float weight_top_left     = (x2 - x) * (y - y1);                                                                        \
                                                                                                                            \
    x1 = x1 * 2 + U_or_V;                                                                                                   \
    y1 = y1 + hIn;                                                                                                          \
    x2 = x2 * 2 + U_or_V;                                                                                                   \
    y2 = y2 + hIn;                                                                                                          \
                                                                                                                            \
    uint color_top_left     = read_img(img, x1, y2, min_val, max_val);                                                      \
    uint color_bottom_left  = read_img(img, x1, y1, min_val, max_val);                                                      \
    uint color_bottom_right = read_img(img, x2, y1, min_val, max_val);                                                      \
    uint color_top_right    = read_img(img, x2, y2, min_val, max_val);                                                      \
                                                                                                                            \
    return (color_top_left * weight_top_left + color_bottom_left * weight_bottom_left +                                     \
            color_bottom_right * weight_bottom_right + color_top_right * weight_top_right);                                 \
}                                                                                                                           \
                                                                                                                            \
__kernel void                                                                                                               \
resize(__read_only image2d_t imageIn, __write_only image2d_t imageOut, int wIn, int hIn, int wOut, int hOut)                \
{                                                                                                                           \
    const int ix = get_global_id(0);                                                                                        \
    const int iy = get_global_id(1);                                                                                        \
                                                                                                                            \
    float xScale = wIn / (float)wOut;                                                                                       \
    float yScale = hIn / (float)hOut;                                                                                       \
    uint4 color_src;                                                                                                        \
    int2 min_val;                                                                                                           \
    int2 max_val;                                                                                                           \
                                                                                                                            \
                                                                                                                            \
                                        /* Process Y plane */                                                               \
    min_val.x = 0;                                                                                                          \
    min_val.y = 0;                                                                                                          \
    max_val.x = wIn - 1;                                                                                                    \
    max_val.y = hIn - 1;                                                                                                    \
                                                                                                                            \
                                                                                                                            \
    int out_x = ix * 2;                                                                                                     \
    int out_y = iy * 2;                                                                                                     \
    color_src.x = read_and_interpolate_Y(imageIn, (out_x * xScale), (out_y * yScale), min_val, max_val);                    \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src);                                                               \
                                                                                                                            \
    out_x = ix * 2;                                                                                                         \
    out_y = iy * 2 + 1;                                                                                                     \
    color_src.x = read_and_interpolate_Y(imageIn, (out_x * xScale), (out_y * yScale), min_val, max_val);                    \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src);                                                               \
                                                                                                                            \
    out_x = ix * 2 + 1;                                                                                                     \
    out_y = iy * 2;                                                                                                         \
    color_src.x = read_and_interpolate_Y(imageIn, (out_x * xScale), (out_y * yScale), min_val, max_val);                    \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src);                                                               \
                                                                                                                            \
    out_x = ix * 2 + 1;                                                                                                     \
    out_y = iy * 2 + 1;                                                                                                     \
    color_src.x = read_and_interpolate_Y(imageIn, (out_x * xScale), (out_y * yScale), min_val, max_val);                    \
    write_imageui(imageOut, (int2)(out_x, out_y), color_src);                                                               \
                                                                                                                            \
                                                                                                                            \
                                      /* Process UV plane */                                                                \
    min_val.x = 0;                                                                                                          \
    min_val.y = hIn;                                                                                                        \
    max_val.x = wIn - 2;                                                                                                    \
    max_val.y = hIn + hIn / 2 - 1;                                                                                          \
                                                                                                                            \
    out_x = ix * 2;                                                                                                         \
    out_y = iy + hOut;                                                                                                      \
                                                                                                                            \
    color_src.x = read_and_interpolate_UV(imageIn, (ix * xScale), (iy * yScale), min_val, max_val, 0 /* It means U */, hIn);\
    write_imageui(imageOut, (int2)(out_x, out_y), color_src);                                                               \
                                                                                                                            \
                                                                                                                            \
    max_val.x = wIn - 1;                                                                                                    \
                                                                                                                            \
    color_src.x = read_and_interpolate_UV(imageIn, (ix * xScale), (iy * yScale), min_val, max_val, 1 /* It means V */, hIn);\
    write_imageui(imageOut, (int2)(out_x + 1, out_y), color_src);                                                           \
}                                                                                                                           \
";

#endif