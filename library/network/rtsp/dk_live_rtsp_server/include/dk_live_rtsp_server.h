#ifndef _DK_LIVE_RTSP_SERVER_H_
#define _DK_LIVE_RTSP_SERVER_H_

#include <cstdint>
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


class dk_live_rtsp_server
{
public:
	dk_live_rtsp_server(void);
	virtual ~dk_live_rtsp_server(void);

	void start(void);
	void stop(void);

private:
	unsigned static __stdcall process_cb(void * param);
	void process(void);

private:
	bool _bstop;
	HANDLE _thread;
};












#endif