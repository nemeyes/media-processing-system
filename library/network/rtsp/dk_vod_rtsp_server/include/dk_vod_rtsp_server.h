#ifndef _DK_VOD_RTSP_SERVER_H_
#define _DK_VOD_RTSP_SERVER_H_

#include <cstdint>
#include <winsock2.h>
#include <windows.h>
#if !defined(WIN32)
#include <pthread.h>
#define EXP_CLASS
#else
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#endif

class RTSPServer;
class EXP_CLASS dk_vod_rtsp_server
{
public:
	dk_vod_rtsp_server(void);
	virtual ~dk_vod_rtsp_server(void);

	void start(int32_t port_number, char * username, char * password);
	void stop(void);

	bool add_live_media_source(const char * uuid, const char * url, const char * username, const char * password);
	bool remove_live_media_source(const char * uuid);

private:
	unsigned static __stdcall process_cb(void * param);
	void process(void);

private:
	RTSPServer * _rtsp_server;

	bool _bstop;
	HANDLE _thread;

	char _username[100];
	char _password[100];
	int32_t _port_number;
};












#endif