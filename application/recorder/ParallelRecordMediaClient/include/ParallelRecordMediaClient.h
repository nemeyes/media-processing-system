#ifndef _PARALLEL_RECORD_MEDIA_CLIENT_H_
#define _PARALLEL_RECORD_MEDIA_CLIENT_H_

#include <windows.h>

#if defined(EXPORT_PARALLEL_RECORD_MEDIA_CLIENT_LIB)
#define EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS __declspec(dllexport)
#else
#define EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS __declspec(dllimport)
#endif

#define PRMC_FAIL		-1
#define PRMC_SUCCESS	0

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Initialize(HWND hwnd);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Release(void);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Connect(const wchar_t * url, int port_number, const wchar_t * username, const wchar_t * password);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Disconnect(const wchar_t * url);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_GetYears(const wchar_t * url, const wchar_t * uuid, int years[], int capacity, int & size);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_GetMonths(const wchar_t * url, const wchar_t * uuid, int year, int months[], int capacity, int & size);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_GetDays(const wchar_t * url, const wchar_t * uuid, int year, int month, int days[], int capacity, int & size);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_GetHours(const wchar_t * url, const wchar_t * uuid, int year, int month, int day, int hours[], int capacity, int & size);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_GetMinutes(const wchar_t * url, const wchar_t * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_GetSeconds(const wchar_t * url, const wchar_t * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Add(const wchar_t * url, const wchar_t * uuid, HWND hwnd);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Remove(const wchar_t * url, int index);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Play(const wchar_t * url, int index, int year, int month, int day, int hour, int minute, int second, bool repeat);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Stop(const wchar_t * url, int index);


extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_FullScreen(const wchar_t * url, int index, bool enable);


#endif