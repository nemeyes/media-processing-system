#ifndef _RTMP_CLIENT_H_
#define _RTMP_CLIENT_H_

#include "dk_rtmp_client.h"

typedef struct RTMPPacket RTMPPacket;
class rtmp_client
{
public:
	rtmp_client(dk_rtmp_client * front);
	~rtmp_client(void);
	dk_rtmp_client::ERROR_CODE play(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat = true);
	dk_rtmp_client::ERROR_CODE stop(void);

	void process_video(const RTMPPacket * packet);
	void process_audio(const RTMPPacket * packet);

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

	char * _buffer;
	int32_t _buffer_size;

	//bool _first;
	bool _rcv_first_idr;
	bool _change_sps;
	bool _change_pps;


	/*
	uint8_t extradata[200] = { 0 };
	size_t extradata_size = 0;
	struct timeval presentation_time = { 0, 0 };
	uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
	*/

};

#endif // _DK_RTMP_CLIENT_H_
