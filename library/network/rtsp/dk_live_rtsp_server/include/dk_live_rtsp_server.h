#ifndef _DK_LIVE_RTSP_SERVER_H_
#define _DK_LIVE_RTSP_SERVER_H_

#include <dk_rtsp_base.h>

class EXP_CLASS dk_live_rtsp_server : public dk_rtsp_base
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