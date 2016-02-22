#pragma once
#include <cstdint>

#if defined(WIN32)
#include <windows.h>
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#define EXP_CLASS
#endif

class dk_video_buffer;
class dk_audio_buffer;
class EXP_CLASS dk_media_buffering
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;

	typedef enum _VIDEO_SUBMEDIA_TYPE
	{
		VIDEO_SUBMEDIA_TYPE_UNKNOWN = -1,
		VIDEO_SUBMEDIA_TYPE_AVC,
		VIDEO_SUBMEDIA_TYPE_HEVC,
		VIDEO_SUBMEDIA_TYPE_MPEG4,
		VIDEO_SUBMEDIA_TYPE_JPEG,
	} VIDEO_SUBMEDIA_TYPE;

	typedef enum _AUDIO_SUBMEDIA_TYPE
	{
		AUDIO_SUBMEDIA_TYPE_UNKNOWN = -1,
		AUDIO_SUBMEDIA_TYPE_AAC,
		AUDIO_SUBMEDIA_TYPE_MP3,
		AUDIO_SUBMEDIA_TYPE_AC3,
		AUDIO_SUBMEDIA_TYPE_CELT,
	} AUDIO_SUBMEDIA_TYPE;

	static dk_media_buffering & instance(void);

	
	dk_media_buffering::ERR_CODE push_video(const uint8_t * data, size_t size, long long timestamp);
	dk_media_buffering::ERR_CODE pop_video(uint8_t * data, size_t & size, long long & timestamp);

	dk_media_buffering::ERR_CODE set_video_submedia_type(dk_media_buffering::VIDEO_SUBMEDIA_TYPE mt);
	dk_media_buffering::ERR_CODE set_vps(uint8_t * vps, size_t size);
	dk_media_buffering::ERR_CODE set_sps(uint8_t * sps, size_t size);
	dk_media_buffering::ERR_CODE set_pps(uint8_t * pps, size_t size);
	dk_media_buffering::ERR_CODE set_video_width(int32_t width);
	dk_media_buffering::ERR_CODE set_video_height(int32_t height);

	dk_media_buffering::ERR_CODE get_video_submedia_type(dk_media_buffering::VIDEO_SUBMEDIA_TYPE & mt);
	dk_media_buffering::ERR_CODE get_vps(uint8_t * vps, size_t & size);
	dk_media_buffering::ERR_CODE get_sps(uint8_t * sps, size_t & size);
	dk_media_buffering::ERR_CODE get_pps(uint8_t * pps, size_t & size);
	dk_media_buffering::ERR_CODE get_video_width(int32_t & width);
	dk_media_buffering::ERR_CODE get_video_height(int32_t & height);


	
	dk_media_buffering::ERR_CODE push_audio(const uint8_t * data, size_t size, long long timestamp);
	dk_media_buffering::ERR_CODE pop_audio(uint8_t * data, size_t & size, long long & timestamp);

	dk_media_buffering::ERR_CODE set_audio_submedia_type(dk_media_buffering::AUDIO_SUBMEDIA_TYPE mt);
	dk_media_buffering::ERR_CODE set_configstr(uint8_t * configstr, size_t size);
	dk_media_buffering::ERR_CODE set_audio_samplerate(int32_t samplerate);
	dk_media_buffering::ERR_CODE set_audio_bitdepth(int32_t bitdepth);
	dk_media_buffering::ERR_CODE set_audio_channels(int32_t channels);

	dk_media_buffering::ERR_CODE get_audio_submedia_type(dk_media_buffering::AUDIO_SUBMEDIA_TYPE & mt);
	dk_media_buffering::ERR_CODE get_configstr(uint8_t * configstr, size_t & size);
	dk_media_buffering::ERR_CODE get_audio_samplerate(int32_t & samplerate);
	dk_media_buffering::ERR_CODE get_audio_bitdepth(int32_t & bitdepth);
	dk_media_buffering::ERR_CODE get_audio_channels(int32_t & channels);


private:
	dk_media_buffering(void);
	~dk_media_buffering(void);
	dk_media_buffering(const dk_media_buffering & clone);

private:
	dk_video_buffer * _vbuffer;
	dk_audio_buffer * _abuffer;
};