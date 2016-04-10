#ifndef _DK_MEDIA_PLAYER_FRAMEWORK_H_
#define _DK_MEDIA_PLAYER_FRAMEWORK_H_

#include <windows.h>

#if defined(EXPORT_MEDIA_PLAYER_FRAMEWORK_LIB)
#define EXP_MEDIA_PLAYER_FRAMEWORK_CLASS __declspec(dllexport)
#else
#define EXP_MEDIA_PLAYER_FRAMEWORK_CLASS __declspec(dllimport)
#endif


void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS dmpf_initialize(void);
void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS dmpf_release(void);

int EXP_MEDIA_PLAYER_FRAMEWORK_CLASS dmpf_rtsp_source_add(const char * url, const char * username, const char * password, int transport_option, int recv_option, bool repeat, HWND hwnd);
void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS dmpf_rtsp_source_remove(int id);

void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS dmpf_rtsp_source_play(int id);
void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS dmpf_rtsp_source_stop(int id);

void EXP_MEDIA_PLAYER_FRAMEWORK_CLASS dmpf_fullscreen(bool enable);


#endif