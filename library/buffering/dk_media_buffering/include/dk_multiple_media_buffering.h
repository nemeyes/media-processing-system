#pragma once
#include <dk_basic_type.h>

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

namespace debuggerking
{
	class video_buffer;
	class audio_buffer;
	class EXP_MEDIA_BUFFERING_CLASS multiple_media_buffering : public foundation
	{
	public:
#if defined(WITH_SINGLETON)
		static multiple_media_buffering & instance(void);
#else
		multiple_media_buffering(void);
		virtual ~multiple_media_buffering(void);
#endif

		int32_t create(const char * id);
		int32_t destroy(const char * id);

		int32_t push_video(const char * id, const uint8_t * data, size_t size, long long timestamp);
		int32_t pop_video(const char * id, uint8_t * data, size_t & size, long long & timestamp);

		int32_t set_video_submedia_type(const char * id, int32_t mt);
		int32_t set_vps(const char * id, uint8_t * vps, size_t size);
		int32_t set_sps(const char * id, uint8_t * sps, size_t size);
		int32_t set_pps(const char * id, uint8_t * pps, size_t size);
		int32_t set_video_width(const char * id, int32_t width);
		int32_t set_video_height(const char * id, int32_t height);

		int32_t get_video_submedia_type(const char * id, int32_t & mt);
		int32_t get_vps(const char * id, uint8_t * vps, size_t & size);
		int32_t get_sps(const char * id, uint8_t * sps, size_t & size);
		int32_t get_pps(const char * id, uint8_t * pps, size_t & size);
		int32_t get_video_width(const char * id, int32_t & width);
		int32_t get_video_height(const char * id, int32_t & height);

		const uint8_t * get_vps(const char * id, size_t & size);
		const uint8_t * get_sps(const char * id, size_t & size);
		const uint8_t * get_pps(const char * id, size_t & size);


		int32_t push_audio(const char * id, const uint8_t * data, size_t size, long long timestamp);
		int32_t pop_audio(const char * id, uint8_t * data, size_t & size, long long & timestamp);

		int32_t set_audio_submedia_type(const char * id, int32_t mt);
		int32_t set_configstr(const char * id, uint8_t * configstr, size_t size);
		int32_t set_audio_samplerate(const char * id, int32_t samplerate);
		int32_t set_audio_bitdepth(const char * id, int32_t bitdepth);
		int32_t set_audio_channels(const char * id, int32_t channels);

		int32_t get_audio_submedia_type(const char * id, int32_t & mt);
		int32_t get_configstr(const char * id, uint8_t * configstr, size_t & size);
		int32_t get_audio_samplerate(const char * id, int32_t & samplerate);
		int32_t get_audio_bitdepth(const char * id, int32_t & bitdepth);
		int32_t get_audio_channels(const char * id, int32_t & channels);


	private:
#if defined(WITH_SINGLETON)
		multiple_media_buffering(void);
		virtual ~multiple_media_buffering(void);
#endif
		multiple_media_buffering(const multiple_media_buffering & clone);

		video_buffer * get_video_buffer(const char * id);
		audio_buffer * get_audio_buffer(const char * id);

	private:
		CRITICAL_SECTION _vmutex;
		CRITICAL_SECTION _amutex;
	};
};
