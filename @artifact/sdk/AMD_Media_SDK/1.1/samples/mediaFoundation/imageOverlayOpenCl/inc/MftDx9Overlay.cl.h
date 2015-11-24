#ifndef MFTOVERLAY_CL_H

const char OVERLAY_PROGRAM_NAME[] = "overlay"; 

const char OVERLAY_OPENCL_SCRIPT[] = "\
const sampler_t overlayImageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;                 \
                                                                                                                            \
__kernel void                                                                                                               \
overlay(__read_only image2d_t overlay, int overlayWidth, int overlayHeight,                                                 \
        __write_only image2d_t output, int outputWidth, int outputHeight)                                                   \
{                                                                                                                           \
    const int ix = get_global_id(0);                                                                                        \
    const int iy = get_global_id(1);                                                                                        \
	                                                                                                                        \
 	int2 locate;                                                                                                            \
    int2 locate_input;                                                                                                      \
    int2 locate_output;                                                                                                     \
	float4 color;                                                                                                           \
    int2 min_val;                                                                                                           \
    int2 max_val;                                                                                                           \
                                                                                                                            \
    min_val.x = 0;                                                                                                          \
    min_val.y = 0;                                                                                                          \
    max_val.x = overlayWidth - 1;                                                                                           \
    max_val.y = overlayHeight - 1;                                                                                          \
                                                                                                                            \
    locate.x = ix;                                                                                                          \
	locate.y = iy;                                                                                                          \
                                                                                                                            \
    locate = clamp(locate, min_val, max_val);                                                                               \
	                                                                                                                        \
    locate_input.x = locate.x;                                                                                              \
    locate_input.y = locate.y;                                                                                              \
    color = read_imagef(overlay, overlayImageSampler, locate_input);                                                        \
                                                                                                                            \
    locate_output.x = locate.x;																								\
    locate_output.y = (outputHeight - overlayHeight) + locate.y;															\
                                                                                                                            \
	write_imagef(output, locate_output, color);                                                                             \
                                                                                                                            \
}                                                                                                                           \
";

#endif
