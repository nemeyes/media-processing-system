#ifndef _DK_FILE_DEMUXER_H_
#define _DK_FILE_DEMUXER_H_

#include <cstdint>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_FILE_DEMUXER_LIB)
#  define EXP_FILE_DEMUXER_CLASS __declspec(dllexport)
# else
#  define EXP_FILE_DEMUXER_CLASS __declspec(dllimport)
# endif
#else
# define EXP_FILE_DEMUXER_CLASS
#endif

class ff_demuxer;
class EXP_FILE_DEMUXER_CLASS dk_file_demuxer
{
public:
	typedef enum _state
	{
		state_stopped = 0,
	} state;


	typedef enum _media_type
	{
		media_type_video = 0,
		media_type_audio
	} media_type;

	typedef enum _video_submedia_type
	{
		unknown_video_type = -1,
		vsubmedia_type_jpeg = 0,
		vsubmedia_type_mpeg4,
		vsubmedia_type_h264,
		vsubmedia_type_hevc,
	} vsubmedia_type;

	typedef enum _asubmedia_type
	{
		unknown_audio_type = -1,
		asubmedia_type_mp3 = 0,
		asubmedia_type_aac,
	} asubmedia_type;

	typedef enum _err_code
	{
		ERR_CODE_SUCCESS = 0,
		ERR_CODE_FAIL
	} err_code;

	dk_file_demuxer(void);
	virtual ~dk_file_demuxer(void);

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