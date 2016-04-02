#include "dk_vod_rtsp_server.h"
#include <process.h>
#include <BasicUsageEnvironment.hh>
#include "vod_rtsp_server.h"

dk_vod_rtsp_server::dk_vod_rtsp_server(void)
	: _port_number(554)
	, _thread(INVALID_HANDLE_VALUE)
{
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
}

dk_vod_rtsp_server::~dk_vod_rtsp_server(void)
{

}

void dk_vod_rtsp_server::start(int32_t port_number, char * username, char * password)
{
	stop();
	_port_number = port_number;
	if (username && strlen(username) > 0)
		strncpy(_username, username, sizeof(_username));
	if (password && strlen(password) > 0)
		strncpy(_password, password, sizeof(_password));

	unsigned int thrd_addr;
	_bstop = false;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, &dk_vod_rtsp_server::process_cb, this, CREATE_SUSPENDED, &thrd_addr);
	//::SetThreadPriority(_thread, THREAD_PRIORITY_HIGHEST);
	::ResumeThread(_thread);
}

void dk_vod_rtsp_server::stop(void)
{
	if (_thread != INVALID_HANDLE_VALUE)
	{
#if 0
		_bstop = ~0;
		::WaitForSingleObjectEx(_thread, INFINITE, FALSE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
#else
		_bstop = ~0;
		::WaitForSingleObjectEx(_thread, INFINITE, FALSE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
		//Medium::close(_rtsp_server);
#endif
	}
}

unsigned dk_vod_rtsp_server::process_cb(void * param)
{
	dk_vod_rtsp_server * self = static_cast<dk_vod_rtsp_server*>(param);
	self->process();
	return 0;
}

void dk_vod_rtsp_server::process(void)
{
	TaskScheduler * scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment * env = BasicUsageEnvironment::createNew(*scheduler);
	UserAuthenticationDatabase * auth = NULL;
	if (strlen(_username)>0 && strlen(_password)>0)
	{
		auth = new UserAuthenticationDatabase;
		auth->addUserRecord(_username, _password);
	}

	portNumBits rtspServerPortNum = _port_number;
	_rtsp_server = vod_rtsp_server::createNew(*env, rtspServerPortNum, auth);
	if (_rtsp_server == NULL)
		return;

	//rtspServer->setUpTunnelingOverHTTP(80);

	env->taskScheduler().doEventLoop((char*)&_bstop); // does not return	

	Medium::close(_rtsp_server);
	if (auth)
	{
		delete auth;
		auth = nullptr;
	}
	if (env)
	{
		env->reclaim();
		env = nullptr;
	}
	if (scheduler)
	{
		delete scheduler;
		scheduler = nullptr;
	}
}