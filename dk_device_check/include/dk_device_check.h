#ifndef _DK_DEVICE_CHECK_H_
#define _DK_DEVICE_CHECK_H_

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LIB)
#  define EXP_DLL __declspec(dllexport)
# else
#  define EXP_DLL __declspec(dllimport)
# endif
#else
# define EXP_DLL
#endif

class EXP_DLL dk_device_check
{
public:
	dk_device_check(void);
	~dk_device_check(void);

	static bool is_cuda_supported(int & id);
	static bool is_cuda_npp_supported(void);
	static bool is_cuda_nvenc_supported(void);
	static bool is_cuda_nvcuvid_supported(void);
	static int get_max_gflops_graphics_cuda_device_id(void);

private:
	static int convert_SMVer2_cores_driver_api(int major, int minor);
};








#endif