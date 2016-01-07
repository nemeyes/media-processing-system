#ifndef _DK_RTMP_CLIENT_H_
#define _DK_RTMP_CLIENT_H_

#include <cstdint>
#if !defined(WIN32)
#include <pthread.h>
#define EXP_CLASS
#else
#include <windows.h>
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
	typedef enum _STATE_T
	{
		STATE_STOPPED = 0,
		STATUE_PAUSED,
		STATE_SUBSCRIBING,
		STATE_PUBLISHING,
	} STATE_T;


	typedef enum _MEDIA_TYPE_T
	{
		MEDIA_TYPE_VIDEO = 0,
		MEDIA_TYPE_AUDIO
	} MEDIA_TYPE_T;

	typedef enum _VIDEO_SUBMEDIA_TYPE_T
	{
		UNKNOWN_VIDEO_TYPE = -1,
		SUBMEDIA_TYPE_JPEG = 1,
		SUBMEDIA_TYPE_SORENSON_H263,
		SUBMEDIA_TYPE_SCREEN_VIDEO,
		SUBMEDIA_TYPE_VP6,
		SUBMEDIA_TYPE_VP_WITH_ALPHA_CHANNEL,
		SUBMEDIA_TYPE_SCREEN_VIDEO_VERSION2,
		SUBMEDIA_TYPE_AVC,
	} VIDEO_SUBMEDIA_TYPE_T;

	typedef enum _AUDIO_SUBMEDIA_TYPE_T
	{
		UNKNOWN_AUDIO_TYPE = -1,
		SUBMEDIA_TYPE_LINEAR_PCM_PE = 0, //platform endian
		SUBMEDIA_TYPE_ADPCM,
		SUBMEDIA_TYPE_MP3,
		SUBMEDIA_TYPE_LINEAR_PCM_LE, //little endian
		SUBMEDIA_TYPE_NELLYMOSER_16KHZ,
		SUBMEDIA_TYPE_NELLYMOSER_8KHZ,
		SUBMEDIA_TYPE_NELLYMOSER,
		SUBMEDIA_TYPE_ALAW,
		SUBMEDIA_TYPE_MLAW,
		SUBMEDIA_TYPE_AAC = 10,
		SUBMEDIA_TYPE_SPEEX,
		SUBMEDIA_TYPE_MP3_8KHZ,
	} AUDIO_SUBMEDIA_TYPE_T;

	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS = 0,
		ERR_CODE_FAIL
	} ERR_CODE;

	typedef enum _RECV_OPTION_T
	{
		RECV_AUDIO_VIDEO = 0,
		RECV_VIDEO,
		RECV_AUDIO
	} RECV_OPTION_T;

	dk_rtmp_client(bool split_thread=false);
	virtual ~dk_rtmp_client(void);

	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);
	void set_sps(uint8_t * sps, size_t sps_size);
	void set_pps(uint8_t * pps, size_t pps_size);
	void clear_sps(void);
	void clear_pps(void);

	dk_rtmp_client::STATE_T state(void);

	dk_rtmp_client::ERR_CODE subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat = true);
	dk_rtmp_client::ERR_CODE subscribe_end(void);

	virtual void on_begin_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, struct timeval presentation_time) = 0;
	virtual void on_recv_video(dk_rtmp_client::VIDEO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, struct timeval presentation_time) = 0;
	
	virtual void on_begin_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, struct timeval presentation_time) = 0;
	virtual void on_recv_audio(dk_rtmp_client::AUDIO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, struct timeval presentation_time) = 0;

	dk_rtmp_client::ERR_CODE publish_begin(VIDEO_SUBMEDIA_TYPE_T vsmt, AUDIO_SUBMEDIA_TYPE_T asmt, const char * url, const char * username, const char * password);
	dk_rtmp_client::ERR_CODE publish_video(uint8_t * bitstream, size_t nb);
	dk_rtmp_client::ERR_CODE publish_audio(uint8_t * bitstream, size_t nb);
	dk_rtmp_client::ERR_CODE publish_end(void);

private:
	bool _split_thread = false;
	rtmp_client * _core;
};

#endif // _DK_RTMP_CLIENT_H_
