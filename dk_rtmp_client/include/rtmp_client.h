#ifndef _RTMP_CLIENT_H_
#define _RTMP_CLIENT_H_

#include "dk_rtmp_client.h"

namespace RTMP_LIB
{
	class CRTMP;
}

class rtmp_client
{
public:
	rtmp_client(dk_rtmp_client * front);
	~rtmp_client(void);
	dk_rtmp_client::ERROR_CODE play(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat = true);
	dk_rtmp_client::ERROR_CODE stop(void);

	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);

private:
	void set_sps(uint8_t * sps, size_t sps_size);
	void set_pps(uint8_t * pps, size_t pps_size);

	void process(void);
#if !defined(WIN32)
	static void* process_cb(void * param);
#else
	static unsigned __stdcall process_cb(void * param);
#endif
#if !defined(WIN32)
	pthread_t _worker;
#else
	void * _worker;
#endif

private:
	dk_rtmp_client * _front;

	char _url[260];
	char _username[260];
	char _password[260];
	int32_t _recv_option;
	bool _repeat;

	uint8_t _sps[100];
	uint8_t _pps[100];
	int32_t _sps_size;
	int32_t _pps_size;


	uint8_t * _buffer;
	int32_t _buffer_size;


	bool _is_first_idr_rcvd;
	/*
	int32_t _protocol;
	char _host[512];
	uint32_t _port;
	char _playpath[512];
	char _app[512];
	char _tc_url[512];
	uint8_t * _swf_hash;
	int32_t _swf_size;
	char _flash_version[200];
	char _subscribe_path[512];
	long int _timeout;

	RTMP_LIB::CRTMP * _librtmp;
	*/
};

#endif // _DK_RTMP_CLIENT_H_
