#ifndef _DK_RTMP_CLIENT_H_
#define _DK_RTMP_CLIENT_H_

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

class rtmp_client;
class EXP_CLASS dk_rtmp_client
{
public:
	typedef enum _MEDIA_TYPE_T
	{
		MEDIA_TYPE_VIDEO = 0,
		MEDIA_TYPE_AUDIO
	} MEDIA_TYPE_T;

	typedef enum _SUBMEDIA_TYPE_T
	{
		SUBMEDIA_TYPE_H264 = 0,
		SUBMEDIA_TYPE_JPEG,
	} SUBMEDIA_TYPE_T;

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

	dk_rtmp_client(void);
	~dk_rtmp_client(void);
	dk_rtmp_client::ERROR_CODE play(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat = true);
	dk_rtmp_client::ERROR_CODE stop(void);

	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);

	virtual void on_begin_media(MEDIA_TYPE_T mt, SUBMEDIA_TYPE_T smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, struct timeval presentation_time) = 0;
	virtual void on_recv_media(MEDIA_TYPE_T mt, SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, struct timeval presentation_time) = 0;

private:
	rtmp_client * _core;
};

#endif // _DK_RTMP_CLIENT_H_
