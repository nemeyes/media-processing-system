#include "dk_live_rtsp_server.h"
#include <process.h>
#include <BasicUsageEnvironment.hh>
#include "rtsp_server.h"

dk_live_rtsp_server::dk_live_rtsp_server(void)
{

}

dk_live_rtsp_server::~dk_live_rtsp_server(void)
{

}

void dk_live_rtsp_server::start(void)
{
	if (_thread != INVALID_HANDLE_VALUE)
	{
		_bstop = true;
		::WaitForSingleObjectEx(_thread, INFINITE, FALSE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}

	unsigned int thrd_addr;
	_bstop = false;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, &dk_live_rtsp_server::process_cb, this, CREATE_SUSPENDED, &thrd_addr);
	//::SetThreadPriority(_thread, THREAD_PRIORITY_HIGHEST);
	::ResumeThread(_thread);
}

void dk_live_rtsp_server::stop(void)
{
	if (_thread != INVALID_HANDLE_VALUE)
	{
		_bstop = true;
		::WaitForSingleObjectEx(_thread, INFINITE, FALSE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}
}

unsigned dk_live_rtsp_server::process_cb(void * param)
{
	dk_live_rtsp_server * self = static_cast<dk_live_rtsp_server*>(param);
	self->process();
	return 0;
}

void dk_live_rtsp_server::process(void)
{
	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	UserAuthenticationDatabase* authDB = NULL;
	/*
	#ifdef ACCESS_CONTROL
	authDB								= new UserAuthenticationDatabase;
	authDB->addUserRecord( "nemeyes", "7224" );
	#endif
	*/

	// Create the RTSP server.  Try first with the default port number (554),
	// and then with the alternative port number (8554):
	RTSPServer* rtspServer;
	portNumBits rtspServerPortNum = 554;
	rtspServer = rtsp_server::createNew(*env, rtspServerPortNum, authDB);
	if (rtspServer == NULL)
	{
		rtspServerPortNum = 8554;
		rtspServer = rtsp_server::createNew(*env, rtspServerPortNum, authDB);
	}
	if (rtspServer == NULL)
	{
		return;
	}


	//char log[100] = { 0 };
	//_snprintf(log, sizeof(log), "RTSP server is started with portnumber [%d]", rtspServerPortNum);
	//logger::instance().make_system_info_log(log);

	if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080))
	{
		//memset(log, 0x00, sizeof(log));
		//_snprintf(log, sizeof(log), "port number %d is used for optional RTSP-over-HTTP tunneling", rtspServer->httpServerPortNum());
		//logger::instance().make_system_info_log(log);
	}
	else
	{
		//logger::instance().make_system_warn_log("RTSP-over-HTTP tunneling is not available.");
	}
	env->taskScheduler().doEventLoop(((char*)&_bstop)); // does not return
}