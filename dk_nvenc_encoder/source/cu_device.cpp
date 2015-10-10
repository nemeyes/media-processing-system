#include "cu_device.h"

#if defined(WIN32)
HINSTANCE cu_device::load_cuda_driver_api(void)
{
	return LoadLibrary(L"nvcuda.dll");
}

void(*cu_device::get_cuda_driver_process_address(HMODULE lib, const char * name))(void)
{
	return (void(*)(void)) GetProcAddress(lib, name);
}

bool cu_device::free_cuda_driver_api(HMODULE lib)
{
	return FreeLibrary(lib) == TRUE ? true : false;
}

#else
void* cu_device::load_cuda_driver_api(void)
{
	return dlopen("libcuda.so", RTLD_NOW);
}
void(cu_device::*get_cuda_driver_process_address(void * lib, const char * name))(void)
{
	return (void(*)(void)) dlsym(lib, (const char *)name);
}

bool cu_device::free_cuda_driver_api(void * lib)
{
	return dlclose(lib) == 0 ? true : false;
}

#endif