#ifndef _DK_RTSP_CLIENT_H_
#define _DK_RTSP_CLIENT_H_

#include <cstdint>
#if !defined(WIN32)
#include <pthread.h>
#define EXP_CLASS
#else
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#endif

class rtsp_client;
class EXP_CLASS dk_rtsp_client
{
	friend class buffer_sink;
public:
	typedef enum _MEDIA_TYPE_T
	{
		MEDIA_TYPE_VIDEO = 0,
		MEDIA_TYPE_AUDIO
	} MEDIA_TYPE_T;


	typedef enum _VIDEO_SUBMEDIA_TYPE_T
	{
		UNKNOWN_VIDEO_TYPE = -1,
		SUBMEDIA_TYPE_JPEG = 0,
		SUBMEDIA_TYPE_MPEG4,
		SUBMEDIA_TYPE_H264,
		SUBMEDIA_TYPE_HEVC,
	} VIDEO_SUBMEDIA_TYPE_T;

	typedef enum _AUDIO_SUBMEDIA_TYPE_T
	{
		UNKNOWN_AUDIO_TYPE = -1,
		SUBMEDIA_TYPE_MP3 = 0,
		SUBMEDIA_TYPE_AAC = 1,
		SUBMEDIA_TYPE_CELT,
	} AUDIO_SUBMEDIA_TYPE_T;

	typedef enum _ERROR_CODE
	{
		ERROR_CODE_SUCCESS = 0,
		ERROR_CODE_FAIL
	} ERROR_CODE;

	typedef enum _RECV_OPTION_T
	{
		RECV_AUDIO_VIDEO = 0,
		RECV_VIDEO,
		RECV_AUDIO
	} RECV_OPTION_T;

	typedef enum _TRANSPORT_OPTION_T
	{
		RTP_OVER_UDP = 0,
		RTP_OVER_TCP,
		RTP_OVER_HTTP
	} TRANSPORT_OPTION_T;

	typedef enum _FOCUS_OPTION_T
	{
		FOCUS_ON_NOTHING = 0,
		FOCUS_ON_VIDEO,
		FOCUS_ON_AUDIO
	} FOCUS_OPTION_T;

	dk_rtsp_client(void);
	~dk_rtsp_client(void);
	dk_rtsp_client::ERROR_CODE play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, int32_t recv_timeout, bool repeat = true);
	dk_rtsp_client::ERROR_CODE stop(void);
	
	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);

	virtual void on_begin_video(dk_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp) = 0;
	virtual void on_recv_video(dk_rtsp_client::VIDEO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long timestamp) = 0;
	virtual void on_begin_audio(dk_rtsp_client::AUDIO_SUBMEDIA_TYPE_T smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp) = 0;
	virtual void on_recv_audio(dk_rtsp_client::AUDIO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long timestamp) = 0;

	bool ignore_sdp(void);
private:
	void set_sps(uint8_t * sps, size_t sps_size);
	void set_pps(uint8_t * pps, size_t pps_size);

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

	rtsp_client * _live;
    bool _kill;
	bool _ignore_sdp;

	uint8_t _sps[100];
	uint8_t _pps[100];
	int32_t _sps_size;
	int32_t _pps_size;
};

#endif // dk_rtsp_client_H
