#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cuda.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "dk_cuda_driver_api.h"

/*
__device__ int device_clip_byte(unsigned char x)
{
	if (x > 255)
		return 255;
	else if (x < 0)
		return 0;
	else
		return x;
}

__global__ void kernel_convert_rgb32_to_yv12(unsigned int width, unsigned int height, unsigned char * rgba, unsigned int rgba_pitch,
											 unsigned char * y_plane, unsigned char * u_plane, unsigned char * v_plane, unsigned int y_pitch, unsigned int uv_pitch)
{
	int x, y;
	x = blockIdx.x*blockDim.x + threadIdx.x;
	y = blockIdx.y*blockDim.y + threadIdx.y;

	if ((x < width) && (y < height))
	{
		const unsigned char * pixel = rgba + rgba_pitch * x;
		y_plane[x] = device_clip_byte(((pixel[2] * 66 + pixel[1] * 129 + pixel[0] * 25 + 128) >> 8) + 16);
		if (y % 2 == 0 && x % 2 == 0) 
		{
			u_plane[x / 2] = device_clip_byte(((pixel[2] * -38 + pixel[1] * -74 + pixel[0] * 112 + 128) >> 8) + 128);
			v_plane[x / 2] = device_clip_byte(((pixel[2] * 112 + pixel[1] * -94 + pixel[0] * -18 + 128) >> 8) + 128);
		}
	}
}

void convert_rgb32_to_yv12(unsigned int width, unsigned int height, unsigned char * rgba, unsigned int rgba_pitch, 
						   CUdeviceptr y_plane, CUdeviceptr u_plane, CUdeviceptr v_plane, unsigned int y_pitch, unsigned int uv_pitch)
{
#define BLOCK_X 32
#define BLOCK_Y 16
	dim3 block(BLOCK_X, BLOCK_Y, 1); //512 block
	dim3 grid((width + BLOCK_X - 1) / BLOCK_X, (height + BLOCK_Y - 1) / BLOCK_Y, 1);
#undef BLOCK_Y
#undef BLOCK_X




}
*/
__device__ unsigned char device_read_pixel_value(unsigned char * src, unsigned int width, int x, int y)
{
	return (unsigned char)src[y*width + x];
}

__device__ void device_put_pixel_value(unsigned char * dst, unsigned int width, int x, int y, unsigned char value)
{
	dst[y*width + x] = value;
}

__global__ void kernel_rotate( unsigned char * src, unsigned char * dst, int size_x, int size_y, float deg)
{
	int i = blockIdx.x * blockDim.x + threadIdx.x;// Kernel definition
	int j = blockIdx.y * blockDim.y + threadIdx.y;
	int xc = size_x - size_x / 2;
	int yc = size_y - size_y / 2;
	int newx = ((float)i - xc)*cos(deg) - ((float)j - yc)*sin(deg) + xc;
	int newy = ((float)i - xc)*sin(deg) + ((float)j - yc)*cos(deg) + yc;
	if (newx >= 0 && newx < size_x && newy >= 0 && newy < size_y)
	{
		device_put_pixel_value(dst, size_x, i, j, device_read_pixel_value(src, size_x, newx, newy));
	}
}

__global__ void kernel_interleave_uv(unsigned char *yuv_cb, unsigned char *yuv_cr, unsigned char *nv12_chroma, int chroma_width, int chroma_height, int cb_pitch, int cr_pitch, int nv12_pitch)
{
	int x, y;
	unsigned char *pCb;
	unsigned char *pCr;
	unsigned char *pDst;
	x = blockIdx.x*blockDim.x + threadIdx.x;
	y = blockIdx.y*blockDim.y + threadIdx.y;

	if ((x < chroma_width) && (y < chroma_height))
	{
		pCb = yuv_cb + (y*cb_pitch);
		pCr = yuv_cr + (y*cr_pitch);
		pDst = nv12_chroma + y*nv12_pitch;
		pDst[x << 1] = pCb[x];
		pDst[(x << 1) + 1] = pCr[x];
	}
}


#if defined(WITH_DYNAMIC_CUDA_LOAD)
void interleave_uv(void* driver_api, unsigned int width, unsigned int height, unsigned char * src, unsigned int src_pitch, CUdeviceptr dst, unsigned int dst_pitch, CUdeviceptr cb, CUdeviceptr cr)
#else
void interleave_uv(unsigned int width, unsigned int height, unsigned char * src, unsigned int src_pitch, CUdeviceptr dst, unsigned int dst_pitch, CUdeviceptr cb, CUdeviceptr cr)
#endif
{
	unsigned char * origin_yv12_y_plane = src;
	unsigned char * origin_yv12_v_plane = origin_yv12_y_plane + src_pitch*height;
	unsigned char * origin_yv12_u_plane = origin_yv12_v_plane + (src_pitch*height >> 2);

#if defined(WITH_DYNAMIC_CUDA_LOAD)
	dk_cuda_driver_api * driver_api_ = static_cast<dk_cuda_driver_api*>(driver_api);
#endif

	// copy luma
	CUDA_MEMCPY2D copy_param;
	memset(&copy_param, 0, sizeof(copy_param));
	copy_param.dstMemoryType = CU_MEMORYTYPE_DEVICE;
	copy_param.dstDevice = dst;
	copy_param.dstPitch = dst_pitch;
	copy_param.srcMemoryType = CU_MEMORYTYPE_HOST;
	copy_param.srcHost = origin_yv12_y_plane;
	copy_param.srcPitch = src_pitch;
	copy_param.WidthInBytes = width;
	copy_param.Height = height;
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	driver_api_->memcpy_2d(&copy_param);
	driver_api_->memcpy_host_to_device(cb, origin_yv12_u_plane, src_pitch*height >> 2);
	driver_api_->memcpy_host_to_device(cr, origin_yv12_v_plane, src_pitch*height >> 2);

#else
	cuMemcpy2D(&copy_param);
	cuMemcpyHtoD(cb, origin_yv12_u_plane, src_pitch*height >> 2);
	cuMemcpyHtoD(cr, origin_yv12_v_plane, src_pitch*height >> 2);
#endif
	unsigned int chroma_height = height >> 1;
	unsigned int chroma_width = width >> 1;
#define BLOCK_X 32
#define BLOCK_Y 16
	dim3 block(BLOCK_X, BLOCK_Y, 1); //512 block
	dim3 grid((chroma_width + BLOCK_X - 1) / BLOCK_X, (chroma_height + BLOCK_Y - 1) / BLOCK_Y, 1);
#undef BLOCK_Y
#undef BLOCK_X

	unsigned int chroma_pitch = src_pitch >> 1;
	CUdeviceptr dst_chroma = (CUdeviceptr)((unsigned char*)dst + dst_pitch*height);
	kernel_interleave_uv << <block, grid >> >((unsigned char*)cb, (unsigned char*)cr, (unsigned char*)dst_chroma, chroma_width, chroma_height, chroma_pitch, chroma_pitch, dst_pitch);

	//CUresult result = cuStreamQuery(NULL);
}