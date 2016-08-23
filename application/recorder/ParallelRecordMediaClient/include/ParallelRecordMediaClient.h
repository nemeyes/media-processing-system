#ifndef _PARALLEL_RECORD_MEDIA_CLIENT_H_
#define _PARALLEL_RECORD_MEDIA_CLIENT_H_

#include <windows.h>

#if defined(_DEBUG)
#include <vld.h>
#endif

#if defined(EXPORT_PARALLEL_RECORD_MEDIA_CLIENT_LIB)
#define EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS __declspec(dllexport)
#else
#define EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS __declspec(dllimport)
#endif

#define PRMC_FAIL		-1
#define PRMC_SUCCESS	0

typedef void(__stdcall *PRMC_PlayTimeCallback)(int index, int year, int month, int day, int hour, int minute, int second);
typedef void(__stdcall *PRMC_ExportBeginCallback)(int index);
typedef void(__stdcall *PRMC_ExportEndCallback)(int index);


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

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Add(const wchar_t * url, const wchar_t * uuid, HWND hwnd, PRMC_PlayTimeCallback cb = NULL);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Remove(const wchar_t * url, int index);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Play(const wchar_t * url, int index, int year, int month, int day, int hour, int minute, int second, float scale, int duration, bool repeat);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Stop(const wchar_t * url, int index);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Resume(const wchar_t * url, int index);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_Pause(const wchar_t * url, int index);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_EnableOSD(const wchar_t * url, int index, bool enable);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_SetOSDPosition(const wchar_t * url, int index, int x, int y);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_AddExport(const wchar_t * url, const wchar_t * uuid, const wchar_t * export_file_path, PRMC_ExportBeginCallback scb = NULL, PRMC_ExportEndCallback ecb = NULL);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_RemoveExport(const wchar_t * url, int index);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_PlayExport(const wchar_t * url, int index, 
																		int begin_year, int begin_month, int begin_day, int begin_hour, int begin_minute, int begin_second, 
																		int end_year, int end_month, int end_day, int end_hour, int end_minute, int end_second);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_StopExport(const wchar_t * url, int index);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_RTSP_Add(const wchar_t * url, int port_number, const wchar_t * username, const wchar_t * password, HWND hwnd);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_RTSP_Remove(int index);

extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_RTSP_Play(int index, bool repeat);
extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_RTSP_Stop(int index);


extern "C" int EXP_PARALLEL_RECORD_MEDIA_CLIENT_CLASS PRMC_FullScreen(const wchar_t * url, int index, bool enable);


#endif