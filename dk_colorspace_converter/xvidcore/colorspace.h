
#include <windows.h>
#include <stdio.h>

#ifndef MIN
#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#endif
#ifndef MAX
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
#endif

#define int8_t   char
#define uint8_t  unsigned char
#define int16_t  short
#define uint16_t unsigned short
#define int32_t  int
#define uint32_t unsigned int
#define int64_t  __int64
#define uint64_t unsigned __int64

#define pthread_t				HANDLE
#define pthread_create(t,u,f,d) *(t)=CreateThread(NULL,0,f,d,0,NULL)
#define pthread_join(t,s)		{ WaitForSingleObject(t,INFINITE); \
									CloseHandle(t); } 
#define sched_yield()			Sleep(0);
static __inline int pthread_num_processors_np()
{
	DWORD p_aff, s_aff, r = 0;
	GetProcessAffinityMask(GetCurrentProcess(), (PDWORD_PTR)&p_aff, (PDWORD_PTR)&s_aff);
	for (; p_aff != 0; p_aff >>= 1) r += p_aff & 1;
	return r;
}

/*****************************************************************************
*  Some things that are only architecture dependant
****************************************************************************/

#if defined(ARCH_IS_32BIT)
#    define CACHE_LINE 64
#    define ptr_t uint32_t
#    define intptr_t int32_t
#    define _INTPTR_T_DEFINED
#    if defined(_MSC_VER) && _MSC_VER >= 1300 && !defined(__INTEL_COMPILER)
#        include <stdarg.h>
#    else
#        define uintptr_t uint32_t
#    endif
#elif defined(ARCH_IS_64BIT)
#    define CACHE_LINE  64
#    define ptr_t uint64_t
#    define intptr_t int64_t
#    define _INTPTR_T_DEFINED
#    if defined (_MSC_VER) && _MSC_VER >= 1300 && !defined(__INTEL_COMPILER)
#        include <stdarg.h>
#    else
#        define uintptr_t uint64_t
#    endif
#else
#    error You are trying to compile Xvid without defining address bus size.
#endif

#define snprintf _snprintf
#define vsnprintf _vsnprintf

/*----------------------------------------------------------------------------
| msvc x86 specific macros/functions
*---------------------------------------------------------------------------*/
#if defined(ARCH_IS_IA32)
#	define BSWAP(a) __asm mov eax,a __asm bswap eax __asm mov a, eax
	static __inline int64_t read_counter(void)
	{
		int64_t ts;
		uint32_t ts1, ts2;
		__asm {
			rdtsc
				mov ts1, eax
				mov ts2, edx
		}
		ts = ((uint64_t)ts2 << 32) | ((uint64_t)ts1);
		return ts;
	}
#elif defined(ARCH_IS_X86_64)
#	include <intrin.h>
#   define BSWAP(a) ((a) = _byteswap_ulong(a))
	static __inline int64_t read_counter(void) { return __rdtsc(); }
/*----------------------------------------------------------------------------
| msvc GENERIC (plain C only) - Probably alpha or some embedded device
*---------------------------------------------------------------------------*/
#elif defined(ARCH_IS_GENERIC)
#	define BSWAP(a) \
			((a) = (((a) & 0xff) << 24)  | (((a) & 0xff00) << 8) | \
			(((a) >> 8) & 0xff00) | (((a) >> 24) & 0xff))

#	include <time.h>
	static __inline int64_t read_counter(void)
	{
		return (int64_t)clock();
	}

/*----------------------------------------------------------------------------
| msvc Not given architecture - This is probably an user who tries to build
| Xvid the wrong way.
*---------------------------------------------------------------------------*/
#else
#	error You are trying to compile Xvid without defining the architecture type.
#endif

extern "C"
{
	void colorspace_init(void);

	void bgr_to_yv12_mmx(uint8_t * x_ptr,
		int x_stride,
		uint8_t * y_src,
		uint8_t * v_src,
		uint8_t * u_src,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		int vflip);

	void bgra_to_yv12_mmx(uint8_t * x_ptr,
		int x_stride,
		uint8_t * y_src,
		uint8_t * v_src,
		uint8_t * u_src,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		int vflip);

	void yv12_to_bgr_mmx(uint8_t * x_ptr,
		int x_stride,
		uint8_t * y_src,
		uint8_t * v_src,
		uint8_t * u_src,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		int vflip);

	void yv12_to_bgra_mmx(uint8_t * x_ptr,
		int x_stride,
		uint8_t * y_src,
		uint8_t * v_src,
		uint8_t * u_src,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		int vflip);




	//////////////
	void bgr_to_yv12(uint8_t * x_ptr,
		int x_stride,
		uint8_t * y_src,
		uint8_t * v_src,
		uint8_t * u_src,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		int vflip);;

	void bgra_to_yv12(uint8_t * x_ptr,
		int x_stride,
		uint8_t * y_src,
		uint8_t * v_src,
		uint8_t * u_src,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		int vflip);;

	void yv12_to_bgr(uint8_t * x_ptr,
		int x_stride,
		uint8_t * y_src,
		uint8_t * v_src,
		uint8_t * u_src,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		int vflip);;

	void yv12_to_bgra(uint8_t * x_ptr,
		int x_stride,
		uint8_t * y_src,
		uint8_t * v_src,
		uint8_t * u_src,
		int y_stride,
		int uv_stride,
		int width,
		int height,
		int vflip);
}
