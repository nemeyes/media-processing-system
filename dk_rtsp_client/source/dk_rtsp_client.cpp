#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#endif
#include "dk_rtsp_client.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "live_media_wrapper.h"

dk_rtsp_client::dk_rtsp_client( void )
	: _ignore_sdp(true)
{
	WSADATA wsd;
	WSAStartup( MAKEWORD(2,2), &wsd );
}

dk_rtsp_client::~dk_rtsp_client( void )
{
	WSACleanup();
}

dk_rtsp_client::ERROR_CODE dk_rtsp_client::play(const char * url, const char * username, const char * password, int transport_option, int recv_option, bool repeat)
{
    if( !url || strlen(url)<1 )
		return ERROR_CODE_FAIL;

	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	if (strlen(url)>0)
		strcpy(_url, url);
	if (strlen(username)>0)
		strcpy(_username, username);
	if (strlen(password)>0)
		strcpy(_password, password);
	_transport_option = transport_option;
	_recv_option = recv_option;
	_repeat = repeat;

#if defined(WIN32)
	unsigned int thread_id;
	_worker = (HANDLE)::_beginthreadex(0, 0, dk_rtsp_client::process_cb, this, 0, &thread_id);
#else
	pthread_create( &_worker, 0, &dk_rtsp_client::process_cb, this );
#endif
	return dk_rtsp_client::ERROR_CODE_SUCCESS;
}

dk_rtsp_client::ERROR_CODE dk_rtsp_client::stop(void)
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
#if defined(WIN32)
	::WaitForSingleObject( _worker, INFINITE );
#else
	pthread_join(_worker, 0);
#endif
	return dk_rtsp_client::ERROR_CODE_SUCCESS;
}

unsigned char * dk_rtsp_client::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

unsigned char * dk_rtsp_client::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

void dk_rtsp_client::set_sps(unsigned char * sps, size_t sps_size)
{
	memset(_sps, 0x00, sizeof(_sps));
	memcpy(_sps, sps, sps_size);
	_sps_size = sps_size;
}

void dk_rtsp_client::set_pps(unsigned char * pps, size_t pps_size)
{
	memset(_pps, 0x00, sizeof(_pps));
	memcpy(_pps, pps, pps_size);
	_pps_size = pps_size;
}

bool dk_rtsp_client::ignore_sdp(void)
{
	return _ignore_sdp;
}

void dk_rtsp_client::process( void )
{
	do
	{
		TaskScheduler * sched = BasicTaskScheduler::createNew();
		UsageEnvironment * env = BasicUsageEnvironment::createNew(*sched);
		if (strlen(_username) > 0 && strlen(_password) > 0)
			_live = live_media_wrapper::createNew(this, *env, _url, _username, _password, _transport_option, _recv_option, 0, &_kill);
		else
			_live = live_media_wrapper::createNew(this, *env, _url, 0, 0, _transport_option, _recv_option, 0, &_kill);

		_kill = false;
		live_media_wrapper::continue_after_client_creation(_live);
		env->taskScheduler().doEventLoop((char*)&_kill);


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

	} while (_repeat);
}

#if defined(WIN32)
unsigned __stdcall dk_rtsp_client::process_cb(void * param)
{
	dk_rtsp_client * self = static_cast<dk_rtsp_client*>(param);
	self->process();
	return 0;
}
#else
void* dk_rtsp_client::process_cb(void * param)
{
	dk_rtsp_client * self = static_cast<dk_rtsp_client*>(param);
	self->process();
	return 0;
}
#endif