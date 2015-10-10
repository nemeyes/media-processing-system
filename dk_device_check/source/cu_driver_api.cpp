#if defined(WIN32)
#include <Windows.h>
#endif
#include "cu_driver_api.h"


typedef CUresult(CUDAAPI *cuInit_)(unsigned int flags);
typedef CUresult(CUDAAPI *cuDeviceGetCount_)(int * count);
typedef CUresult(CUDAAPI *cuDeviceComputeCapability_)(int * major, int * minor, CUdevice dev);
typedef CUresult(CUDAAPI *cuDeviceGet_)(CUdevice * device, int ordinal);
typedef CUresult(CUDAAPI *cuDeviceTotalMem_)(size_t* bytes, CUdevice dev);
typedef CUresult(CUDAAPI *cuCtxCreate_)(CUcontext* pctx, unsigned int  flags, CUdevice dev);
typedef CUresult(CUDAAPI *cuCtxPopCurrent_)(CUcontext *pctx);
typedef CUresult(CUDAAPI *cuCtxPushCurrent_)(CUcontext ctx);
typedef CUresult(CUDAAPI *cuCtxDestroy_)(CUcontext ctx);


typedef CUresult(CUDAAPI *cuMemcpyHtoD_)(CUdeviceptr dstDevice, const void* srcHost, size_t ByteCount);
typedef CUresult(CUDAAPI *cuMemcpy2D_)(const CUDA_MEMCPY2D* pCopy);
typedef CUresult(CUDAAPI *cuMemAlloc_)(CUdeviceptr* dptr, size_t bytesize);
typedef CUresult(CUDAAPI *cuMemAllocPitch_)(CUdeviceptr* dptr, size_t* pPitch, size_t WidthInBytes, size_t Height, unsigned int  ElementSizeBytes);
typedef CUresult(CUDAAPI *cuMemFree_)(CUdeviceptr dptr);

cu_driver_api::cu_driver_api(void)
{}

cu_driver_api::~cu_driver_api(void)
{}

bool cu_driver_api::load(void)
{
#if defined(WIN32)
	_cu_driver_inst = LoadLibrary(L"nvcuda.dll");
#else
	_cu_driver_inst = dlopen("libcuda.so", RTLD_NOW);
#endif
	if (_cu_driver_inst)
		return true;
	else
		return false;
}

bool cu_driver_api::free(void)
{
#if defined(WIN32)
	return FreeLibrary((HMODULE)_cu_driver_inst) == TRUE ? true : false;
#else
	return dlclose(_cu_driver_inst) == 0 ? true : false;
#endif
}

void(*cu_driver_api::get_cuda_driver_process_address(const char * name))(void)
{
#if defined(WIN32)
	return (void(*)(void)) GetProcAddress((HMODULE)_cu_driver_inst, name);
#else
	return (void(*)(void)) dlsym(_cu_driver_inst, (const char *)name);
#endif
}


CUresult cu_driver_api::init(unsigned int flags)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuInit_ myCuInit = 0;
#if defined(WIN32)
	myCuInit = (cuInit_)get_cuda_driver_process_address("cuInit");
#else
	myCuInit = (cuInit_)get_cuda_driver_process_address("cuInit");
#endif
	if (!myCuInit)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuInit)(flags);
}

CUresult cu_driver_api::deviceGetCount(int * count)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuDeviceGetCount_ myCuDeviceGetCount = 0;
#if defined(WIN32)
	myCuDeviceGetCount = (cuDeviceGetCount_)get_cuda_driver_process_address("cuDeviceGetCount");
#else
	myCuDeviceGetCount = (cuDeviceGetCount_)get_cuda_driver_process_address("cuDeviceGetCount");
#endif
	if (!myCuDeviceGetCount)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuDeviceGetCount)(count);
}

CUresult cu_driver_api::deviceComputeCapability(int * major, int * minor, CUdevice dev)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuDeviceComputeCapability_ myCuDeviceComputeCapability = 0;
#if defined(WIN32)
	myCuDeviceComputeCapability = (cuDeviceComputeCapability_)get_cuda_driver_process_address("cuDeviceComputeCapability");
#else
	myCuDeviceComputeCapability = (cuDeviceComputeCapability_)get_cuda_driver_process_address("cuDeviceComputeCapability");
#endif
	if (!myCuDeviceComputeCapability)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuDeviceComputeCapability)(major, minor, dev);
}

CUresult cu_driver_api::deviceGet(CUdevice * device, int ordinal)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuDeviceGet_ myCuDeviceGet = 0;
#if defined(WIN32)
	myCuDeviceGet = (cuDeviceGet_)get_cuda_driver_process_address("cuDeviceGet");
#else
	myCuDeviceGet = (cuDeviceGet_)get_cuda_driver_process_address("cuDeviceGet");
#endif
	if (!myCuDeviceGet)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuDeviceGet)(device, ordinal);
}

CUresult cu_driver_api::deviceTotalMem(size_t* bytes, CUdevice dev)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuDeviceTotalMem_ myCuDeviceTotalMem = 0;
#if defined(WIN32)
	myCuDeviceTotalMem = (cuDeviceTotalMem_)get_cuda_driver_process_address("cuDeviceTotalMem");
#else
	myCuDeviceTotalMem = (cuDeviceTotalMem_)get_cuda_driver_process_address("cuDeviceTotalMem");
#endif
	if (!myCuDeviceTotalMem)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuDeviceTotalMem)(bytes, dev);
}

CUresult cu_driver_api::ctxCreate(CUcontext* pctx, unsigned int  flags, CUdevice dev)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuCtxCreate_ myCuCtxCreate = 0;
#if defined(WIN32)
	myCuCtxCreate = (cuCtxCreate_)get_cuda_driver_process_address("cuCtxCreate");
#else
	myCuCtxCreate = (cuCtxCreate_)get_cuda_driver_process_address("cuCtxCreate");
#endif
	if (!myCuCtxCreate)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuCtxCreate)(pctx, flags, dev);
}

CUresult cu_driver_api::ctxPopCurrent(CUcontext *pctx)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuCtxPopCurrent_ myCuCtxPopCurrent = 0;
#if defined(WIN32)
	myCuCtxPopCurrent = (cuCtxPopCurrent_)get_cuda_driver_process_address("cuCtxPopCurrent");
#else
	myCuCtxPopCurrent = (cuCtxPopCurrent_)get_cuda_driver_process_address("cuCtxPopCurrent");
#endif
	if (!myCuCtxPopCurrent)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuCtxPopCurrent)(pctx);
}

CUresult cu_driver_api::ctxPushCurrent(CUcontext ctx)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuCtxPushCurrent_ myCuCtxPushCurrent = 0;
#if defined(WIN32)
	myCuCtxPushCurrent = (cuCtxPushCurrent_)get_cuda_driver_process_address("cuCtxPushCurrent");
#else
	myCuCtxPushCurrent = (cuCtxPushCurrent_)get_cuda_driver_process_address("cuCtxPushCurrent");
#endif
	if (!myCuCtxPushCurrent)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuCtxPushCurrent)(ctx);
}

CUresult cu_driver_api::ctxDestroy(CUcontext ctx)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuCtxDestroy_ myCuCtxDestroy = 0;
#if defined(WIN32)
	myCuCtxDestroy = (cuCtxDestroy_)get_cuda_driver_process_address("cuCtxDestroy");
#else
	myCuCtxDestroy = (cuCtxDestroy_)get_cuda_driver_process_address("cuCtxDestroy");
#endif
	if (!myCuCtxDestroy)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuCtxDestroy)(ctx);
}


CUresult cu_driver_api::memcpyHtoD(CUdeviceptr dstDevice, const void* srcHost, size_t ByteCount)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuMemcpyHtoD_ myCuMemcpyHtoD = 0;
#if defined(WIN32)
	myCuMemcpyHtoD = (cuMemcpyHtoD_)get_cuda_driver_process_address("cuMemcpyHtoD");
#else
	myCuMemcpyHtoD = (cuMemcpyHtoD_)get_cuda_driver_process_address("cuMemcpyHtoD");
#endif
	if (!myCuMemcpyHtoD)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuMemcpyHtoD)(dstDevice, srcHost, ByteCount);
}

CUresult cu_driver_api::memcpy2D(const CUDA_MEMCPY2D* pCopy)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuMemcpy2D_ myCuMemcpy2D = 0;
#if defined(WIN32)
	myCuMemcpy2D = (cuMemcpy2D_)get_cuda_driver_process_address("cuMemcpy2D");
#else
	myCuMemcpy2D = (cuMemcpy2D_)get_cuda_driver_process_address("cuMemcpy2D");
#endif
	if (!myCuMemcpy2D)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuMemcpy2D)(pCopy);
}

CUresult cu_driver_api::memAlloc(CUdeviceptr* dptr, size_t bytesize)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuMemAlloc_ myCuMemAlloc = 0;
#if defined(WIN32)
	myCuMemAlloc = (cuMemAlloc_)get_cuda_driver_process_address("cuMemAlloc");
#else
	myCuMemAlloc = (cuMemAlloc_)get_cuda_driver_process_address("cuMemAlloc");
#endif
	if (!myCuMemAlloc)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuMemAlloc)(dptr, bytesize);
}

CUresult cu_driver_api::memAllocPitch(CUdeviceptr* dptr, size_t* pPitch, size_t WidthInBytes, size_t Height, unsigned int  ElementSizeBytes)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuMemAllocPitch_ myCuMemAllocPitch = 0;
#if defined(WIN32)
	myCuMemAllocPitch = (cuMemAllocPitch_)get_cuda_driver_process_address("cuMemAllocPitch");
#else
	myCuMemAllocPitch = (cuMemAllocPitch_)get_cuda_driver_process_address("cuMemAllocPitch");
#endif
	if (!myCuMemAllocPitch)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuMemAllocPitch)(dptr, pPitch, WidthInBytes, Height, ElementSizeBytes);
}

CUresult cu_driver_api::memFree(CUdeviceptr dptr)
{
	if (!_cu_driver_inst)
		return CUDA_ERROR_UNKNOWN;
	cuMemFree_ myCuMemFree = 0;
#if defined(WIN32)
	myCuMemFree = (cuMemFree_)get_cuda_driver_process_address("cuMemFree");
#else
	myCuMemFree = (cuMemFree_)get_cuda_driver_process_address("cuMemFree");
#endif
	if (!myCuMemFree)
		return CUDA_ERROR_UNKNOWN;

	return (*myCuMemFree)(dptr);
}
