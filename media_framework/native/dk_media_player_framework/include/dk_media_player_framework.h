#ifndef _DK_MEDIA_PLAYER_FRAMEWORK_H_
#define _DK_MEDIA_PLAYER_FRAMEWORK_H_

#include <windows.h>

#if defined(EXPORT_MEDIA_PLAYER_FRAMEWORK_LIB)
#define EXP_MEDIA_PLAYER_FRAMEWORK_CLASS __declspec(dllexport)
#else
#define EXP_MEDIA_PLAYER_FRAMEWORK_CLASS __declspec(dllimport)
#endif

#define RTSP_RECEIVER	0
#define RTMP_RECEIVER	1

extern "C" void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS MediaClient_Initialize(HWND hwnd);
extern "C" void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS MediaClient_Release(void);

extern "C" int EXP_MEDIA_PLAYER_FRAMEWORK_CLASS MediaClient_Add(int type, const wchar_t * url, const wchar_t * username, const wchar_t * password, bool repeat, HWND hwnd);
extern "C" void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS MediaClient_Remove(int index);

extern "C" void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS MediaClient_Play(int index);
extern "C" void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS MediaClient_Stop(int index);

extern "C" void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS MediaClient_FullScreen(int index, bool enable);


#endif