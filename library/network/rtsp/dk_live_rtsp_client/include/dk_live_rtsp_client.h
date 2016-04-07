#ifndef _DK_LIVE_RTSP_CLIENT_H_
#define _DK_LIVE_RTSP_CLIENT_H_

#include <dk_rtsp_base.h>

class live_rtsp_client;
class EXP_CLASS dk_live_rtsp_client : public dk_rtsp_base
{
	friend class buffer_sink;
public:
	dk_live_rtsp_client(void);
	virtual ~dk_live_rtsp_client(void);
	dk_live_rtsp_client::error_code play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, int32_t recv_timeout, bool repeat = true);
	dk_live_rtsp_client::error_code stop(void);

	void set_sps(uint8_t * sps, size_t sps_size);
	void set_pps(uint8_t * pps, size_t pps_size);

	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);

	virtual void on_begin_video(dk_live_rtsp_client::vsubmedia_type smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp) = 0;
	virtual void on_recv_video(dk_live_rtsp_client::vsubmedia_type smt, const uint8_t * data, size_t data_size, long long timestamp) = 0;
	virtual void on_begin_audio(dk_live_rtsp_client::asubmedia_type smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp) = 0;
	virtual void on_recv_audio(dk_live_rtsp_client::asubmedia_type smt, const uint8_t * data, size_t data_size, long long timestamp) = 0;

	bool ignore_sdp(void);

private:
	void process( void );
#if !defined(WIN32)
    static void* process_cb( void * param );
#else
	static unsigned __stdcall process_cb( void * param );
#endif
#if !defined(WIN32)
	pthread_t _worker;
#else
	void * _worker;
#endif

private:
	char _url[260];
	char _username[260];
	char _password[260];
	int32_t _transport_option;
	int32_t _recv_option;
	int32_t _recv_timeout;
	bool _repeat;

	live_rtsp_client * _live;
    bool _kill;
	bool _ignore_sdp;

	uint8_t _sps[100];
	uint8_t _pps[100];
	int32_t _sps_size;
	int32_t _pps_size;
};

#endif // dk_live_rtsp_client_H
