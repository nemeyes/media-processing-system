#ifndef _DK_CUDA_UTILS_H_
#define _DK_CUDA_UTILS_H_

#if defined (_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#include <limits.h>

#define FALSE 0
#define TRUE  1
#define INFINITE UINT_MAX
#define stricmp strcasecmp
#define FILE_BEGIN               SEEK_SET
#define INVALID_SET_FILE_POINTER (-1)
#define INVALID_HANDLE_VALUE     ((void *)(-1))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef void* HANDLE;
typedef void* HINSTANCE;
typedef unsigned long DWORD, *LPWORD;
typedef DWORD FILE_SIZE;
#endif

inline bool sleep(unsigned int mSec)
{
#if defined (_WIN32)
    Sleep(mSec);
#else
    usleep(mSec * 1000);
#endif
    return true;
}

inline bool query_performance_frequency(unsigned long long *freq)
{
    *freq = 0;
#if defined (_WIN32)
    LARGE_INTEGER lfreq;
    if (!QueryPerformanceFrequency(&lfreq)) {
        return false;
    }
    *freq = lfreq.QuadPart;
#else
    // We use system's  gettimeofday() to return timer ticks in uSec
    *freq = 1000000000;
#endif
    return true;
}

#define SEC_TO_NANO_ULL(sec)    ((unsigned long long)sec * 1000000000)
#define MICRO_TO_NANO_ULL(sec)  ((unsigned long long)sec * 1000)

inline bool query_performance_counter(unsigned long long *counter)
{
    *counter = 0;
#if defined (_WIN32)
    LARGE_INTEGER lcounter;
    if (!QueryPerformanceCounter(&lcounter)) {
        return false;
    }
    *counter = lcounter.QuadPart;
#else
    struct timeval tv;
    int ret;

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) {
        return false;
    }

    *counter = SEC_TO_NANO_ULL(tv.tv_sec) + MICRO_TO_NANO_ULL(tv.tv_usec);
#endif
    return true;
}

#if !defined(_WIN32)
__inline bool operator==(const GUID &guid1, const GUID &guid2)
{
     if (guid1.Data1    == guid2.Data1 &&
         guid1.Data2    == guid2.Data2 &&
         guid1.Data3    == guid2.Data3 &&
         guid1.Data4[0] == guid2.Data4[0] &&
         guid1.Data4[1] == guid2.Data4[1] &&
         guid1.Data4[2] == guid2.Data4[2] &&
         guid1.Data4[3] == guid2.Data4[3] &&
         guid1.Data4[4] == guid2.Data4[4] &&
         guid1.Data4[5] == guid2.Data4[5] &&
         guid1.Data4[6] == guid2.Data4[6] &&
         guid1.Data4[7] == guid2.Data4[7])
    {
        return true;
    }

    return false;
}
__inline bool operator!=(const GUID &guid1, const GUID &guid2)
{
    return !(guid1 == guid2);
}
#endif
#endif

#define PRINTERR(message, ...) \
    fprintf(stderr, "%s line %d: " message, __FILE__, __LINE__, ##__VA_ARGS__)
