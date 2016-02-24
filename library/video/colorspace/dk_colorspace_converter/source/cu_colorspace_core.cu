#include "cuda_runtime.h"
#include "device_launch_parameters.h"

__device__ __forceinline__ void rgb_to_y(const unsigned char b, const unsigned char g, const unsigned char uchar r, const unsigned char & y)
{
	y = static_cast<uchar>(((int)(30 * r) + (int)(59 * g) + (int)(11 * b)) / 100);
}

__device__ __forceinline__ void rgb_to_yuv(const unsigned char b, const unsigned char g, const unsigned char r, unsigned char & y, unsigned char & u, unsigned char & v)
{
	rgb_to_y(b, g, r, y);
	u = static_cast<uchar>(((int)(-17 * r) - (int)(33 * g) + (int)(50 * b) + 12800) / 100);
	v = static_cast<uchar>(((int)(50 * r) - (int)(42 * g) - (int)(8 * b) + 12800) / 100);
}

__global__ void convert_rgba_to_yv12_kernel(int width, int height)
{
	unsigned int x = (blockIdx.x * blockDim.x + threadIdx.x) * 2;
	unsigned int y = (blockIdx.y * blockDim.y + threadIdx.y) * 2;

	if ((x + 1) >= width || (y + 1) > height)
		return;




	// load RGB values
	unsigned int R = input[(x + y * width) * RGBcomponentCount];
	unsigned int G = input[(x + y * width) * RGBcomponentCount + 1];
	unsigned int B = input[(x + y * width) * RGBcomponentCount + 2];

	// NTSC standard
	// wikipedia : http://en.wikipedia.org/wiki/YUV#Y.27UV420p_.28and_Y.27V12_or_YV12.29_to_RGB888_conversion
	float Y = 0.299 * R + 0.587 * G + 0.114 * B; 
	if (Y>255) Y = 255;
	float U = -0.147 * R - 0.289 * G + 0.436 * B + 128;
	if (U>255) U = 255;
	float V = 0.615 * R - 0.515 * G - 0.100 * B + 128;
	if (V>255) V = 255;
	output[x + y * width] = Y; // Y-components of all pixels
	output[(y / 2) * (width / 2) + (x / 2) + dst_size] = U; // U-components of all pixels
	output[(y / 2) * (width / 2) + (x / 2) + dst_size + (dst_size / 4)] = V; // V-components of all pixels
}

void cu_convert_rgba_to_yv12(int width, int height, unsigned char * rgba, unsigned char * yv12);
{
	unsigned char * cu_rgba;
	unsigned char * cu_yv12;

	cudaMalloc((void**)&cu_rgba, 4 * width*height);
	cudaMalloc((void**)&cu_yv12, 1.5*width*height);
	cudaMemcpy(cu_rgba, rgba, 4 * width*height, cudaMemcpyHostToDevice);



	cudaMemcpy(yv12, cu_yv12, 1.5 * width*height, cudaMemcpyDeviceToHost);
	cudaFree(cu_rgb32);
	cudaFree(cu_yv12);
}