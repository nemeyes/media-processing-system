#include "dk_vod_rtsp_server.h"
#include <process.h>
#include <BasicUsageEnvironment.hh>
#include "vod_rtsp_server.h"
#include "live_media_source_entity.h"
#include <dk_log4cplus_logger.h>

debuggerking::vod_rtsp_server::vod_rtsp_server(void)
	: _port_number(554)
	, _thread(INVALID_HANDLE_VALUE)
{
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	log4cplus_logger::create("config/log.properties");
}

debuggerking::vod_rtsp_server::~vod_rtsp_server(void)
{
	log4cplus_logger::destroy();
}

int32_t debuggerking::vod_rtsp_server::add_live_media_source(const char * uuid, const char * url, const char * username, const char * password)
{
	return live_media_source_entity::instance().add_live_media_source(uuid, url, username, password);
}

int32_t debuggerking::vod_rtsp_server::remove_live_media_source(const char * uuid)
{
	return live_media_source_entity::instance().remove_live_media_source(uuid);
}

int32_t debuggerking::vod_rtsp_server::start(int32_t port_number, char * username, char * password)
{
	stop();
	_port_number = port_number;
	if (username && strlen(username) > 0)
		strncpy(_username, username, sizeof(_username));
	if (password && strlen(password) > 0)
		strncpy(_password, password, sizeof(_password));

	unsigned int thrd_addr;
	_bstop = false;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, &vod_rtsp_server::process_cb, this, CREATE_SUSPENDED, &thrd_addr);
	//::SetThreadPriority(_thread, THREAD_PRIORITY_HIGHEST);
	::ResumeThread(_thread);
	return vod_rtsp_server::err_code_t::success;
}

int32_t debuggerking::vod_rtsp_server::stop(void)
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
	return vod_rtsp_server::err_code_t::success;
}

unsigned debuggerking::vod_rtsp_server::process_cb(void * param)
{
	vod_rtsp_server * self = static_cast<vod_rtsp_server*>(param);
	self->process();
	return 0;
}

void debuggerking::vod_rtsp_server::process(void)
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
	_rtsp_server = vod_rtsp_core::createNew(*env, rtspServerPortNum, auth);
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