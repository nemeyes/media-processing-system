#ifndef _DK_CUDA_DRIVER_API_H_
#define _DK_CUDA_DRIVER_API_H_

#include <cuda.h>

class dk_cuda_driver_api
{
public:
	//dk_cuda_driver_api(void);
	//~dk_cuda_driver_api(void);
	
	bool load(void);
	bool free(void);
	void(*get_cuda_driver_process_address(const char * name))(void);

	CUresult init(unsigned int flags);
	CUresult device_get_count(int * count);
	CUresult device_compute_capability(int * major, int * minor, CUdevice dev);
	CUresult device_get(CUdevice * device, int ordinal);
	CUresult driver_get_version(int * driver_version);
	CUresult device_get_name(char *name, int len, CUdevice dev);
	CUresult device_get_attribute(int *pi, CUdevice_attribute attrib, CUdevice dev);
//	typedef CUresult(CUDAAPI *cuDeviceGetName_)(char *name, int len, CUdevice dev);
//	typedef CUresult(CUDAAPI *cuDeviceGetAttribute_)(int *pi, CUdevice_attribute attrib, CUdevice dev);

	CUresult device_total_memory(size_t* bytes, CUdevice dev);
	CUresult ctx_create(CUcontext* pctx, unsigned int  flags, CUdevice dev);
	CUresult ctx_pop_current(CUcontext *pctx);
	CUresult ctx_push_current(CUcontext ctx);
	CUresult ctx_destroy(CUcontext ctx);

	

	CUresult memcpy_host_to_device(CUdeviceptr dstDevice, const void* srcHost, size_t ByteCount);
	CUresult memcpy_2d(const CUDA_MEMCPY2D* pCopy);
	CUresult mem_alloc(CUdeviceptr* dptr, size_t bytesize);
	CUresult mem_alloc_pitch(CUdeviceptr* dptr, size_t* pPitch, size_t WidthInBytes, size_t Height, unsigned int  ElementSizeBytes);
	CUresult mem_free(CUdeviceptr dptr);

private:
	//cu_driver_api(const cu_driver_api & clone);

private:
	void * _cu_driver_inst;
};













#endif