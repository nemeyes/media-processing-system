#include "dk_device_check.h"
#include "dk_cuda_driver_api.h"
#include <stdio.h>
#include "nvEncodeAPI.h"

#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}
typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);

#define REQUIRED_MINIMUM_CUDA_DRIVER_VERSION 7000

dk_device_check::dk_device_check(void)
{

}

dk_device_check::~dk_device_check(void)
{

}

bool dk_device_check::is_cuda_supported(int & id)
{
	int  SMminor = 0, SMmajor = 0;
	int device_count = 0;
	dk_cuda_driver_api driver_api;
	if (!driver_api.load())
		return false;

	CUresult result = driver_api.init(0);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	result = driver_api.device_get_count(&device_count);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	if (device_count<1)
	{
		driver_api.free();
		return false;
	}

	size_t available_device_memory = 0;
	int device_id = -1;
	for (int index = 0; index < device_count; index++)
	{
		CUdevice tmp_device;
		size_t tmp_available_device_memory = 0;
		result = driver_api.device_get(&tmp_device, index);
		if (result != CUDA_SUCCESS)
			continue;

		int version = 0;
		result = driver_api.driver_get_version(&version);
		if (result != CUDA_SUCCESS)
			continue;
		if (version < REQUIRED_MINIMUM_CUDA_DRIVER_VERSION)
			continue;

		result = driver_api.device_total_memory(&tmp_available_device_memory, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;

		if (available_device_memory < tmp_available_device_memory)
			device_id = index;
	}

	if (device_id<0)
	{
		driver_api.free();
		return false;
	}

	id = device_id;
	driver_api.free();
	return true;
}

bool dk_device_check::is_cuda_npp_supported(void)
{
	int  SMminor = 0, SMmajor = 0;
	int device_count = 0;
	dk_cuda_driver_api driver_api;
	if (!driver_api.load())
		return false;

	CUresult result = driver_api.init(0);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	result = driver_api.device_get_count(&device_count);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	if (device_count<1)
	{
		driver_api.free();
		return false;
	}

	size_t available_device_memory = 0;
	int device_id = -1;
	for (int index = 0; index < device_count; index++)
	{
		CUdevice tmp_device;
		size_t tmp_available_device_memory = 0;
		result = driver_api.device_get(&tmp_device, index);
		if (result != CUDA_SUCCESS)
			continue;

		int version = 0;
		result = driver_api.driver_get_version(&version);
		if (result != CUDA_SUCCESS)
			continue;
		if (version < REQUIRED_MINIMUM_CUDA_DRIVER_VERSION)
			continue;

		result = driver_api.device_total_memory(&tmp_available_device_memory, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;

		if (available_device_memory < tmp_available_device_memory)
			device_id = index;
	}

	if (device_id<0)
	{
		driver_api.free();
		return false;
	}

	driver_api.free();
	return true;
}

bool dk_device_check::is_cuda_nvenc_supported(void)
{
	int  SMminor = 0, SMmajor = 0;
	int device_count = 0;
	dk_cuda_driver_api driver_api;
	if (!driver_api.load())
		return false;

	CUresult result = driver_api.init(0);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	result = driver_api.device_get_count(&device_count);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	if (device_count < 1)
	{
		driver_api.free();
		return false;
	}

	size_t available_device_memory = 0;
	int device_id = -1;
	for (int index = 0; index < device_count; index++)
	{
		CUdevice tmp_device;
		size_t tmp_available_device_memory = 0;
		result = driver_api.device_get(&tmp_device, index);
		if (result != CUDA_SUCCESS)
			continue;

		int version = 0;
		result = driver_api.driver_get_version(&version);
		if (result != CUDA_SUCCESS)
			continue;
		if (version < REQUIRED_MINIMUM_CUDA_DRIVER_VERSION)
			continue;

		result = driver_api.device_compute_capability(&SMmajor, &SMminor, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;
		if (((SMmajor << 4) + SMminor) < 0x30)
			continue;
		result = driver_api.device_total_memory(&tmp_available_device_memory, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;

		if (available_device_memory < tmp_available_device_memory)
			device_id = index;
	}

	if (device_id < 0)
	{
		driver_api.free();
		return false;
	}

	printf("step 1 \n");
	CUdevice device;
	void * cu_context;
	CUcontext cu_current_context;
	result = driver_api.device_get(&device, device_id);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	printf("step 2 \n");
	result = driver_api.ctx_create((CUcontext*)(&cu_context), 0, device);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}
	result = driver_api.ctx_pop_current(&cu_current_context);
	if (result != CUDA_SUCCESS)
		goto fail0;

	printf("step 3 \n");
#if defined(WIN32)
#if defined (_WIN64)
	HINSTANCE cu_enc_inst = LoadLibrary(TEXT("nvEncodeAPI64.dll"));
#else
	HINSTANCE cu_enc_inst = LoadLibrary(TEXT("nvEncodeAPI.dll"));
#endif
#else
	void * cu_enc_inst = dlopen("nvEncodeAPI.so", RTLD_NOW);
#endif
	if (!cu_enc_inst)
		goto fail0;

	printf("step 4 \n");
	MYPROC nvEncodeAPICreateInstance; // function pointer to create instance in nvEncodeAPI
#if defined(_WIN32)
	nvEncodeAPICreateInstance = (MYPROC)GetProcAddress(cu_enc_inst, "NvEncodeAPICreateInstance");
#else
	nvEncodeAPICreateInstance = (MYPROC)dlsym(cu_enc_inst, "NvEncodeAPICreateInstance");
#endif

	if (nvEncodeAPICreateInstance == NULL)
		goto fail1;


	printf("step 5 \n");
	NV_ENCODE_API_FUNCTION_LIST * api;
	api = new NV_ENCODE_API_FUNCTION_LIST;
	if (api == NULL)
		goto fail1;

	printf("step 5-1 \n");
	memset(api, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
	api->version = NV_ENCODE_API_FUNCTION_LIST_VER;
	NVENCSTATUS status = nvEncodeAPICreateInstance(api);
	if (status != NV_ENC_SUCCESS)
	{
		printf("nvEncodeAPICreateInstance Failed [%d]", status);
		goto fail2;
	}

	printf("step 6 \n");
	void * encoder = 0;
	NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS params;
	memset(&params, 0, sizeof(params));
	SET_VER(params, NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS);
	params.device = cu_context;
	params.deviceType = NV_ENC_DEVICE_TYPE_CUDA;
	params.reserved = NULL;
	params.apiVersion = NVENCAPI_VERSION;
	__try
	{
		status = api->nvEncOpenEncodeSessionEx(&params, &encoder);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		//printf("exception occured \n");
		goto fail2;
	}

	if (status != NV_ENC_SUCCESS)
		goto fail2;

	printf("step 7 \n");
	api->nvEncDestroyEncoder(encoder);
	if (api)
		delete api;
	driver_api.ctx_destroy((CUcontext)cu_context);
	driver_api.free();

#if defined(WIN32)
	return FreeLibrary((HMODULE)cu_enc_inst) == TRUE ? true : false;
#else
	return dlclose(cu_enc_inst) == 0 ? true : false;
#endif

fail0:
	driver_api.ctx_destroy((CUcontext)cu_context);
	driver_api.free();
	return false;

fail1:
	driver_api.ctx_destroy((CUcontext)cu_context);
	driver_api.free();
#if defined(WIN32)
	FreeLibrary((HMODULE)cu_enc_inst);
#else
	dlclose(cu_enc_inst);
#endif
	return false;

fail2:
	if (api)
		delete api;
	driver_api.ctx_destroy((CUcontext)cu_context);
	driver_api.free();
#if defined(WIN32)
	FreeLibrary((HMODULE)cu_enc_inst);
#else
	dlclose(cu_enc_inst);
#endif
	return false;
}

bool dk_device_check::is_cuda_nvcuvid_supported(void)
{
	int  SMminor = 0, SMmajor = 0;
	int device_count = 0;
	dk_cuda_driver_api driver_api;
	if (!driver_api.load())
		return false;

	CUresult result = driver_api.init(0);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	int version = 0;
	driver_api.driver_get_version(&version);
	printf("CUDA Driver version is %d\n", version);

	result = driver_api.device_get_count(&device_count);
	if (result != CUDA_SUCCESS)
	{
		driver_api.free();
		return false;
	}

	if (device_count<1)
	{
		driver_api.free();
		return false;
	}

	size_t available_device_memory = 0;
	int device_id = -1;
	for (int index = 0; index < device_count; index++)
	{
		CUdevice tmp_device;
		size_t tmp_available_device_memory = 0;
		result = driver_api.device_get(&tmp_device, index);
		if (result != CUDA_SUCCESS)
			continue;

		result = driver_api.device_total_memory(&tmp_available_device_memory, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;

		if (available_device_memory < tmp_available_device_memory)
			device_id = index;
	}

	if (device_id<0)
	{
		driver_api.free();
		return false;
	}

	driver_api.free();


#if defined(WIN32)
	HANDLE cu_dec_inst = LoadLibrary(TEXT("nvcuvid.dll"));
#else
	void * cu_dec_inst = dlopen("nvcuvid.so", RTLD_NOW);
#endif
	if (!cu_dec_inst)
		return false;

#if defined(WIN32)
	return FreeLibrary((HMODULE)cu_dec_inst) == TRUE ? true : false;
#else
	return dlclose(cu_dec_inst) == 0 ? true : false;
#endif
	return true;
}

int dk_device_check::get_max_gflops_graphics_cuda_device_id(void)
{
	CUdevice current_device = 0, max_perf_device = 0;
	int device_count = 0, sm_per_multiproc = 0;
	int max_compute_perf = 0, best_SM_arch = 0;
	int major = 0, minor = 0, multiProcessorCount, clockRate;
	int bTCC = 0, version;
	char deviceName[256];

	dk_cuda_driver_api driver_api;
	if (!driver_api.load())
		return false;

	driver_api.device_get_count(&device_count);
	if (device_count <= 0)
		return -1;

	driver_api.driver_get_version(&version);

	// Find the best major SM Architecture GPU device that are graphics devices
	while (current_device < device_count) 
	{
		driver_api.device_get_name(deviceName, 256, current_device);
		driver_api.device_compute_capability(&major, &minor, current_device);

		if (version >= 3020) {
			driver_api.device_get_attribute(&bTCC, CU_DEVICE_ATTRIBUTE_TCC_DRIVER, current_device);
		}
		else 
		{
			// Assume a Tesla GPU is running in TCC if we are running CUDA 3.1
			if (deviceName[0] == 'T') bTCC = 1;
		}
		if (!bTCC) 
		{
			if (major > 0 && major < 9999) 
			{
				best_SM_arch = max(best_SM_arch, major);
			}
		}
		current_device++;
	}

	// Find the best CUDA capable GPU device
	current_device = 0;
	while (current_device < device_count) 
	{
		driver_api.device_get_attribute(&multiProcessorCount, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, current_device);
		driver_api.device_get_attribute(&clockRate, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, current_device);
		driver_api.device_compute_capability(&major, &minor, current_device);

		if (version >= 3020) 
		{
			driver_api.device_get_attribute(&bTCC, CU_DEVICE_ATTRIBUTE_TCC_DRIVER, current_device);
		}
		else 
		{
			// Assume a Tesla GPU is running in TCC if we are running CUDA 3.1
			if (deviceName[0] == 'T') bTCC = 1;
		}

		if (major == 9999 && minor == 9999) 
		{
			sm_per_multiproc = 1;
		}
		else 
		{
			sm_per_multiproc = convert_SMVer2_cores_driver_api(major, minor);
		}

		// If this is a Tesla based GPU and SM 2.0, and TCC is disabled, this is a contendor
		if (!bTCC) // Is this GPU running the TCC driver?  If so we pass on this
		{
			int compute_perf = multiProcessorCount * sm_per_multiproc * clockRate;
			if (compute_perf > max_compute_perf) {
				// If we find GPU with SM major > 2, search only these
				if (best_SM_arch > 2) {
					// If our device = dest_SM_arch, then we pick this one
					if (major == best_SM_arch) {
						max_compute_perf = compute_perf;
						max_perf_device = current_device;
					}
				}
				else {
					max_compute_perf = compute_perf;
					max_perf_device = current_device;
				}
			}

#ifdef _DEBUG
			driver_api.device_get_name(deviceName, 256, current_device);
			//bgLog((LOG_TRACE, 10, L"CUDA Device: %S, Compute: %d.%d, CUDA Cores: %d, Clock: %d MHz", deviceName, major, minor, multiProcessorCount * sm_per_multiproc, clockRate / 1000));
#endif
		}
		++current_device;
	}
	return max_perf_device;
}

int dk_device_check::convert_SMVer2_cores_driver_api(int major, int minor)
{
	// Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
	typedef struct {
		int SM; // 0xMm (hexidecimal notation), M = SM Major version, and m = SM minor version
		int Cores;
	} sSMtoCores;

	sSMtoCores nGpuArchCoresPerSM[] =
	{
		{ 0x10, 8 },
		{ 0x11, 8 },
		{ 0x12, 8 },
		{ 0x13, 8 },
		{ 0x20, 32 },
		{ 0x21, 48 },
		{ 0x30, 192 },
		{ 0x32, 192 },
		{ 0x35, 192 },
		{ 0x37, 192 },
		{ 0x50, 128 },
		{ 0x52, 128 },
		{ -1, -1 }
	};

	int index = 0;
	while (nGpuArchCoresPerSM[index].SM != -1) 
	{
		if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor)) 
		{
			return nGpuArchCoresPerSM[index].Cores;
		}
		index++;
	}
	//printf("MapSMtoCores undefined SMversion %d.%d!\n", major, minor);
	return -1;
}