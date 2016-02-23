#ifndef _DK_FILE_DEMUXER_H_
#define _DK_FILE_DEMUXER_H_

#include <cstdint>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LIB)
#  define EXP_DLL __declspec(dllexport)
# else
#  define EXP_DLL __declspec(dllimport)
# endif
#else
# define EXP_DLL
#endif

class ff_demuxer;
class EXP_DLL dk_file_demuxer
{
public:
	typedef enum _STATE_T
	{
		STATE_STOPPED = 0,
	} STATE_T;


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
		SUBMEDIA_TYPE_AAC,
	} AUDIO_SUBMEDIA_TYPE_T;

	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS = 0,
		ERR_CODE_FAIL
	} ERR_CODE;

	dk_file_demuxer(void);
	~dk_file_demuxer(void);

	dk_file_demuxer::ERR_CODE play(const char * filepath);
	dk_file_demuxer::ERR_CODE stop(void);

	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);
	void set_sps(uint8_t * sps, size_t sps_size);
	void set_pps(uint8_t * pps, size_t pps_size);

	virtual void on_begin_video(dk_file_demuxer::VIDEO_SUBMEDIA_TYPE_T smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long presentation_time) = 0;
	virtual void on_recv_video(dk_file_demuxer::VIDEO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long presentation_time) = 0;
	virtual void on_begin_audio(dk_file_demuxer::AUDIO_SUBMEDIA_TYPE_T smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long presentation_time) = 0;
	virtual void on_recv_audio(dk_file_demuxer::AUDIO_SUBMEDIA_TYPE_T smt, const uint8_t * data, size_t data_size, long long presentation_time) = 0;

private:
	ff_demuxer * _core;

	uint8_t _sps[100];
	uint8_t _pps[100];
	int32_t _sps_size;
	int32_t _pps_size;

};







#endif