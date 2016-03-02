#ifndef _CUDA_DEVICE_H_
#define _CUDA_DEVICE_H_
#include "dk_cuda_utils.h"

class cuda_device
{
public:
#if defined(WIN32)
	static HINSTANCE load_cuda_driver_api(void);
	static void(*get_cuda_driver_process_address(HMODULE lib, const char * name))(void);
	static bool free_cuda_driver_api(HMODULE lib);
#else
	static void* load_cuda_driver_api(void);
	static void(*get_cuda_driver_process_address(void * lib, const char * name))(void);
	static bool free_cuda_driver_api(void * lib);
#endif
};






#endif