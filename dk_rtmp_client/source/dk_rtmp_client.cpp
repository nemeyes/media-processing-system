#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#endif
#include "dk_rtmp_client.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "rtmp.h"
#include "log.h"
#include "AMFObject.h"
#include "parseurl.h"

#define DEFAULT_FLASH_VER "LNX 10,0,22,87"


bool bCtrlC = false;

#if defined(_DEBUG)
int debuglevel = 1;
FILE *netstackdump = 0;
FILE *netstackdump_read = 0;
#endif

dk_rtmp_client::dk_rtmp_client(void)
{
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);

	_buffer_size = 1024 * 1024 * 2;//2MB
	_buffer = (uint8_t*)malloc(_buffer_size);
}

/*
dk_rtmp_client::dk_rtmp_client(void)
	: _protocol(RTMP_PROTOCOL_UNDEFINED)
	, _port(1935)
	, _swf_hash(nullptr)
	, _swf_size(0)
	, _timeout(120) //timeout connection after 120 seconds
{
	memset(_host, 0x00, sizeof(_host));
	memset(_playpath, 0x00, sizeof(_playpath));
	memset(_app, 0x00, sizeof(_app));
	memset(_tc_url, 0x00, sizeof(_tc_url));
	strcpy(_flash_version, DEFAULT_FLASH_VER);
	memset(_subscribe_path, 0x00, sizeof(_subscribe_path));

	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);
}
*/

dk_rtmp_client::~dk_rtmp_client(void)
{
	free(_buffer);
	WSACleanup();
}

dk_rtmp_client::ERROR_CODE dk_rtmp_client::play(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat)
{
	if (!url || strlen(url)<1)
		return ERROR_CODE_FAIL;

	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	if (strlen(url)>0)
		strcpy_s(_url, url);
	if (strlen(username)>0)
		strcpy_s(_username, username);
	if (strlen(password)>0)
		strcpy_s(_password, password);
	_recv_option = recv_option;
	_repeat = repeat;

#if defined(WIN32)
	unsigned int thread_id;
	_worker = (HANDLE)::_beginthreadex(0, 0, dk_rtmp_client::process_cb, this, 0, &thread_id);
#else
	pthread_create(&_worker, 0, &dk_rtmp_client::process_cb, this);
#endif
	return dk_rtmp_client::ERROR_CODE_SUCCESS;
}

dk_rtmp_client::ERROR_CODE dk_rtmp_client::stop(void)
{
	_repeat = false;
#if defined(WIN32)
	::WaitForSingleObject(_worker, INFINITE);
#else
	pthread_join(_worker, 0);
#endif
	return dk_rtmp_client::ERROR_CODE_SUCCESS;
}

uint8_t * dk_rtmp_client::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * dk_rtmp_client::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

void dk_rtmp_client::set_sps(uint8_t * sps, size_t sps_size)
{
	memset(_sps, 0x00, sizeof(_sps));
	memcpy(_sps, sps, sps_size);
	_sps_size = sps_size;
}

void dk_rtmp_client::set_pps(uint8_t * pps, size_t pps_size)
{
	memset(_pps, 0x00, sizeof(_pps));
	memcpy(_pps, pps, pps_size);
	_pps_size = pps_size;
}

void dk_rtmp_client::process(void)
{
	do
	{
#if defined(_DEBUG)
		netstackdump = fopen("netstackdump", "wb");
		netstackdump_read = fopen("netstackdump_read", "wb");
#endif
		//int32_t protocol = RTMP_PROTOCOL_UNDEFINED;
		//char host[512] = { 0 };
		//uint32_t port = 1935;
		//char _playpath[512] = { 0 };
		//char _app[512] = { 0 };
		//char _tc_url[512] = { 0 };
		char * _swf_hash = 0;
		int32_t _swf_size = 0;
		char _flash_version[512];
		strcpy(_flash_version, DEFAULT_FLASH_VER);
		char _subscribe_path[512] = { 0 };
		long int _timeout = 120;

		RTMP_LIB::CRTMP * rtmp = new RTMP_LIB::CRTMP();


		int32_t protocol = RTMP_PROTOCOL_UNDEFINED;
		char * host = 0;
		uint32_t port = 1935;
		char * sock_host = 0;
		char * playpath = 0;
		char * app = 0;
		char * tc_url = 0;
		char * swf_url = 0;
		char * page_url = 0;
		char * auth = 0;
		char * swf_hash = 0;
		int32_t swf_size = 0;
		char * flash_version = _strdup(DEFAULT_FLASH_VER);
		char * subscribe_path = 0;

		long int timeout = 120;
		uint32_t seek = 0;	 // seek position in resume mode, 0 otherwise
		uint32_t length = 0;
		double duration = 0.0;

		uint32_t buffer_time = 10 * 60 * 60 * 1000; // 10 hours as default

		if (!ParseUrl((char*)_url, &protocol, &host, &port, &playpath, &app))
		{
			if (!tc_url && app)
			{
				char str[512] = { 0 };
				_snprintf(str, 511, "%s://%s:%d/%s", RTMPProtocolStringsLower[protocol], host, port, app);
				tc_url = _strdup(str);
			}
			rtmp->SetupStream(protocol, host, port, sock_host, playpath, tc_url, swf_url, page_url, app, auth, swf_hash, swf_size, flash_version, subscribe_path, 0, 0, true, timeout);
		}


		bool first = true;
		while (!bCtrlC)
		{
			rtmp->SetBufferMS(buffer_time);
			if (first)
			{
				first = 0;
				LogPrintf("Connecting ...\n");

				if (!rtmp->Connect())
				{
					break;
				}

				Log(LOGINFO, "Connected...");

				if (!rtmp->ConnectStream(seek, length))
				{
					break;
				}
			}
			else
			{


			}

			LogPrintf("Starting Live Stream\n");




			RTMP_LIB::RTMPPacket packet;
			int32_t rtn_get_next_media_packet = rtmp->GetNextMediaPacket(packet);
			if (rtn_get_next_media_packet)
			{
				char * packet_body = packet.m_body;
				uint32_t packet_length = packet.m_nBodySize;

				//skip video info/command packets
				if (packet.m_packetType == 0x09 && packet_length == 2 && ((*packet_body & 0xF0) == 0x50))
					continue;

				if (packet.m_packetType == 0x09 && packet_length <= 5)
				{
					Log(LOGWARNING, "ignoring too small video packet: size: %d", packet_length);
					continue;
				}

				if (packet.m_packetType == 0x08 && packet_length <= 1)
				{
					Log(LOGWARNING, "ignoring too small audio packet: size: %d", packet_length);
					continue;
				}

#ifdef _DEBUG
				Log(LOGDEBUG, "type: %02X, size: %d, TS: %d ms, abs TS: %d", packet.m_packetType, packet_length, packet.m_nTimeStamp, packet.m_hasAbsTimestamp);
				if (packet.m_packetType == 0x09)
					Log(LOGDEBUG, "frametype: %02X", (*packet_body & 0xf0));
#endif

				uint32_t size = packet_length + ((packet.m_packetType == 0x08 || packet.m_packetType == 0x09 || packet.m_packetType == 0x12) ? 11 : 0) + (packet.m_packetType != 0x16 ? 4 : 0);


				uint8_t * ptr = _buffer;
				uint32_t timestamp = 0;// use to return timestamp of last processed packet

				// audio (0x08), video (0x09) or metadata (0x12) packets :
				// construct 11 byte header then add rtmp packet's data
				if (packet.m_packetType == 0x09)
				{

				}
				else if (packet.m_packetType == 0x08)
				{


				}
				else if (packet.m_packetType == 0x12)
				{

				}
			}
		}

		rtmp->Close();


		if (host)
			free(host);
		if (playpath)
			free(playpath);
		if (app)
			free(app);
		if (tc_url)
			free(tc_url);

#if defined(_DEBUG)
		if (netstackdump != 0)
			fclose(netstackdump);
		if (netstackdump_read != 0)
			fclose(netstackdump_read);
#endif
	} while (0/*_repeat*/);
}

#if defined(WIN32)
unsigned __stdcall dk_rtmp_client::process_cb(void * param)
{
	dk_rtmp_client * self = static_cast<dk_rtmp_client*>(param);
	self->process();
	return 0;
}
#else
void* dk_rtmp_client::process_cb(void * param)
{
	dk_rtmp_client * self = static_cast<dk_rtmp_client*>(param);
	self->process();
	return 0;
}
#endif