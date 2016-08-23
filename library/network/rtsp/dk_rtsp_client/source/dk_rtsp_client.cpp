#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#endif
#include "dk_rtsp_client.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "rtsp_client.h"
#include <dk_log4cplus_logger.h>
#pragma comment(lib,"dk_log4cplus_logger_1.2.0.lib")

debuggerking::rtsp_client::rtsp_client(void)
	: _ignore_sdp(true)
{
	WSADATA wsd;
	WSAStartup( MAKEWORD(2,2), &wsd );
#if defined(DEBUG)
	log4cplus_logger::create("config/log.properties");
#endif
}

debuggerking::rtsp_client::~rtsp_client(void)
{
#if defined(DEBUG)
	log4cplus_logger::destroy();
#endif
	WSACleanup();
}

int32_t debuggerking::rtsp_client::play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, int32_t recv_timeout, float scale, bool repeat)
{
    if( !url || strlen(url)<1 )
		return rtsp_client::err_code_t::fail;

	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	if (strlen(url)>0)
		strcpy(_url, url);
	if (username && strlen(username)>0)
		strcpy(_username, username);
	if (password && strlen(password)>0)
		strcpy(_password, password);
	_transport_option = transport_option;
	_recv_option = recv_option;
	_recv_timeout = recv_timeout;
	_scale = scale;
	_repeat = repeat;

#if defined(WIN32)
	unsigned int thread_id;
	_worker = (HANDLE)::_beginthreadex(0, 0, rtsp_client::process_cb, this, 0, &thread_id);
#else
	pthread_create( &_worker, 0, &dk_live_rtsp_client::process_cb, this );
#endif
	return rtsp_client::err_code_t::success;
}

int32_t debuggerking::rtsp_client::stop(void)
{
	if (!_kill )
	{
		_repeat = false;
		_kill = true;
		if (_live)
		{
			_live->close();
		}
		else
		{
			_kill = true;
		}
	}
#if defined(WIN32)
	if (_worker != NULL && _worker!=INVALID_HANDLE_VALUE)
	{
		if(::WaitForSingleObject(_worker, INFINITE)==WAIT_OBJECT_0)
		{
			::CloseHandle(_worker);
			_worker = INVALID_HANDLE_VALUE;
		}
	}
#else
	pthread_join(_worker, 0);
#endif
	return rtsp_client::err_code_t::success;
}

int32_t debuggerking::rtsp_client::pause(void)
{
	if (!_kill)
	{
		if (_live)
		{
			_live->start_pausing_session();
		}
	}
	return rtsp_client::err_code_t::success;
}

uint8_t * debuggerking::rtsp_client::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * debuggerking::rtsp_client::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

void debuggerking::rtsp_client::set_sps(uint8_t * sps, size_t sps_size)
{
	memset(_sps, 0x00, sizeof(_sps));
	memcpy(_sps, sps, sps_size);
	_sps_size = sps_size;
}

void debuggerking::rtsp_client::set_pps(uint8_t * pps, size_t pps_size)
{
	memset(_pps, 0x00, sizeof(_pps));
	memcpy(_pps, pps, pps_size);
	_pps_size = pps_size;
}

bool debuggerking::rtsp_client::ignore_sdp(void)
{
	return _ignore_sdp;
}

void debuggerking::rtsp_client::process(void)
{
	do
	{
		log4cplus_logger::make_info_log("parallel.record.recorder", "connecting to the camera[%s]", _url);
		TaskScheduler * sched = BasicTaskScheduler::createNew();
		UsageEnvironment * env = BasicUsageEnvironment::createNew(*sched);
		if (strlen(_username) > 0 && strlen(_password) > 0)
			_live = rtsp_core::createNew(this, *env, _url, _username, _password, _transport_option, _recv_option, _recv_timeout, _scale, 0, &_kill);
		else
			_live = rtsp_core::createNew(this, *env, _url, 0, 0, _transport_option, _recv_option, _recv_timeout, _scale, 0, &_kill);
		log4cplus_logger::make_info_log("parallel.record.recorder", "connected to the camera[%s]", _url);

		_kill = false;
		rtsp_core::continue_after_client_creation(_live);
		env->taskScheduler().doEventLoop((char*)&_kill);

		log4cplus_logger::make_info_log("parallel.record.recorder", "disconnected from the camera[%s]", _url);

		if (env)
		{
			env->reclaim();
			env = 0;
		}
		if (sched)
		{
			delete sched;
			sched = 0;
		}

		_sps_size = 0;
		_pps_size = 0;
		memset(_sps, 0x00, sizeof(_sps));
		memset(_pps, 0x00, sizeof(_pps));
	} while (_repeat);
}

#if defined(WIN32)
unsigned __stdcall debuggerking::rtsp_client::process_cb(void * param)
{
	rtsp_client * self = static_cast<rtsp_client*>(param);
	self->process();
	return 0;
}
#else
void* debuggerking::live_rtsp_client::process_cb(void * param)
{
	rtsp_client * self = static_cast<rtsp_client*>(param);
	self->process();
	return 0;
}
#endif