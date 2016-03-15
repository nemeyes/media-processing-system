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
class EXP_MEDIA_BUFFERING_CLASS dk_multiple_media_buffering
{
public:
	static dk_multiple_media_buffering & instance(void);

	buffering::err_code create(const char * id);
	buffering::err_code destroy(const char * id);

	buffering::err_code push_video(const char * id, const uint8_t * data, size_t size, long long timestamp);
	buffering::err_code pop_video(const char * id, uint8_t * data, size_t & size, long long & timestamp);

	buffering::err_code set_video_submedia_type(const char * id, buffering::vsubmedia_type mt);
	buffering::err_code set_vps(const char * id, uint8_t * vps, size_t size);
	buffering::err_code set_sps(const char * id, uint8_t * sps, size_t size);
	buffering::err_code set_pps(const char * id, uint8_t * pps, size_t size);
	buffering::err_code set_video_width(const char * id, int32_t width);
	buffering::err_code set_video_height(const char * id, int32_t height);

	buffering::err_code get_video_submedia_type(const char * id, buffering::vsubmedia_type & mt);
	buffering::err_code get_vps(const char * id, uint8_t * vps, size_t & size);
	buffering::err_code get_sps(const char * id, uint8_t * sps, size_t & size);
	buffering::err_code get_pps(const char * id, uint8_t * pps, size_t & size);
	buffering::err_code get_video_width(const char * id, int32_t & width);
	buffering::err_code get_video_height(const char * id, int32_t & height);



	buffering::err_code push_audio(const char * id, const uint8_t * data, size_t size, long long timestamp);
	buffering::err_code pop_audio(const char * id, uint8_t * data, size_t & size, long long & timestamp);

	buffering::err_code set_audio_submedia_type(const char * id, buffering::asubmedia_type mt);
	buffering::err_code set_configstr(const char * id, uint8_t * configstr, size_t size);
	buffering::err_code set_audio_samplerate(const char * id, int32_t samplerate);
	buffering::err_code set_audio_bitdepth(const char * id, int32_t bitdepth);
	buffering::err_code set_audio_channels(const char * id, int32_t channels);

	buffering::err_code get_audio_submedia_type(const char * id, buffering::asubmedia_type & mt);
	buffering::err_code get_configstr(const char * id, uint8_t * configstr, size_t & size);
	buffering::err_code get_audio_samplerate(const char * id, int32_t & samplerate);
	buffering::err_code get_audio_bitdepth(const char * id, int32_t & bitdepth);
	buffering::err_code get_audio_channels(const char * id, int32_t & channels);


private:
	dk_multiple_media_buffering(void);
	~dk_multiple_media_buffering(void);
	dk_multiple_media_buffering(const dk_multiple_media_buffering & clone);

	dk_video_buffer * get_video_buffer(const char * id);
	dk_audio_buffer * get_audio_buffer(const char * id);

private:
	CRITICAL_SECTION _vmutex;
	CRITICAL_SECTION _amutex;
};