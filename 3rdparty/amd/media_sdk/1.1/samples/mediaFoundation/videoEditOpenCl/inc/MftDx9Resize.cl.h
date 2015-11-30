#ifndef MFTDX9RESIZER_CL_H

const char RESIZER_PROGRAM_NAME[] = "resize"; 

const char RESIZER_OPENCL_SCRIPT[] = "\
__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;            \
                                                                                                                            \
__kernel void                                                                                                               \
resize( __read_only image2d_t imageIn, __write_only image2d_t imageOut, int wIn, int hIn, int wOut, int hOut )              \
{                                                                                                                           \
	const int ix = get_global_id(0);                                                                                        \
	const int iy = get_global_id(1);                                                                                        \
	                                                                                                                        \
	int2 locateOut;                                                                                                         \
	locateOut.x = ix;                                                                                                       \
	locateOut.y = iy;                                                                                                       \
	                                                                                                                        \
	float2 locateIn;                                                                                                        \
	locateIn.x = (ix * wIn) / ((float)wOut);                                                                                \
	locateIn.y = (iy * hIn) / ((float)hOut);                                                                                \
	                                                                                                                        \
	float4 color_src = read_imagef(imageIn, imageSampler, locateIn);                                                        \
	                                                                                                                        \
	write_imagef(imageOut, locateOut, color_src);                                                                           \
}                                                                                                                           \
";

#endif