#pragma once
#include <cstdint>

#if defined(WIN32)
#include <windows.h>
#if defined(EXPORT_MEDIA_BUFFERING_LIB)
#define EXP_MEDIA_BUFFERING_CLASS __declspec(dllexport)
#else
#define EXP_MEDIA_BUFFERING_CLASS __declspec(dllimport)
#endif
#else
#define EXP_MEDIA_BUFFERING_CLASS
#endif

#include "define.h"

class dk_video_buffer;
class dk_audio_buffer;
class EXP_MEDIA_BUFFERING_CLASS dk_media_buffering
{
public:
	static dk_media_buffering & instance(void);

	
	buffering::err_code push_video(const uint8_t * data, size_t size, long long timestamp);
	buffering::err_code pop_video(uint8_t * data, size_t & size, long long & timestamp);

	buffering::err_code set_video_submedia_type(buffering::vsubmedia_type mt);
	buffering::err_code set_vps(uint8_t * vps, size_t size);
	buffering::err_code set_sps(uint8_t * sps, size_t size);
	buffering::err_code set_pps(uint8_t * pps, size_t size);
	buffering::err_code set_video_width(int32_t width);
	buffering::err_code set_video_height(int32_t height);

	buffering::err_code get_video_submedia_type(buffering::vsubmedia_type & mt);
	buffering::err_code get_vps(uint8_t * vps, size_t & size);
	buffering::err_code get_sps(uint8_t * sps, size_t & size);
	buffering::err_code get_pps(uint8_t * pps, size_t & size);
	buffering::err_code get_video_width(int32_t & width);
	buffering::err_code get_video_height(int32_t & height);


	
	buffering::err_code push_audio(const uint8_t * data, size_t size, long long timestamp);
	buffering::err_code pop_audio(uint8_t * data, size_t & size, long long & timestamp);

	buffering::err_code set_audio_submedia_type(buffering::asubmedia_type mt);
	buffering::err_code set_configstr(uint8_t * configstr, size_t size);
	buffering::err_code set_audio_samplerate(int32_t samplerate);
	buffering::err_code set_audio_bitdepth(int32_t bitdepth);
	buffering::err_code set_audio_channels(int32_t channels);

	buffering::err_code get_audio_submedia_type(buffering::asubmedia_type & mt);
	buffering::err_code get_configstr(uint8_t * configstr, size_t & size);
	buffering::err_code get_audio_samplerate(int32_t & samplerate);
	buffering::err_code get_audio_bitdepth(int32_t & bitdepth);
	buffering::err_code get_audio_channels(int32_t & channels);


private:
	dk_media_buffering(void);
	~dk_media_buffering(void);
	dk_media_buffering(const dk_media_buffering & clone);

private:
	dk_video_buffer * _vbuffer;
	dk_audio_buffer * _abuffer;
};