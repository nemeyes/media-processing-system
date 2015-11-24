#ifndef MFTCOPY_CL_H

const char COPY_PROGRAM_NAME[] = "copy"; 

const char COPY_OPENCL_SCRIPT[] = "\
const sampler_t copyImageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;                    \
                                                                                                                            \
__kernel void                                                                                                               \
copy(__read_only image2d_t input, int inputWidth, int inputHeight,                                                          \
        __write_only image2d_t output, int outputWidth, int outputHeight)                                                   \
{                                                                                                                           \
    const int ix = get_global_id(0);                                                                                        \
    const int iy = get_global_id(1);                                                                                        \
	                                                                                                                        \
 	int2 locate;                                                                                                            \
	float4 color;                                                                                                           \
                                                                                                                            \
	locate.x = ix;                                                                                                          \
	locate.y = iy;                                                                                                          \
    color = read_imagef(input, copyImageSampler, locate);                                                                   \
                                                                                                                            \
    int2 locate_out;                                                                                                        \
    locate_out.x = ix;                                                                                                      \
    locate_out.y = iy;                                                                                                      \
	write_imagef(output, locate_out, color);                                                                                \
                                                                                                                            \
}                                                                                                                           \
";

#endif
