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
	class EXP_MEDIA_BUFFERING_CLASS media_buffering
	{
	public:
#if defined(WITH_SINGLETON)
		static media_buffering & instance(void);
#else
		media_buffering(void);
		virtual ~media_buffering(void);
#endif


		int32_t push_video(const uint8_t * data, size_t size, long long timestamp);
		int32_t pop_video(uint8_t * data, size_t & size, long long & timestamp);

		int32_t set_video_submedia_type(int32_t mt);
		int32_t set_vps(uint8_t * vps, size_t size);
		int32_t set_sps(uint8_t * sps, size_t size);
		int32_t set_pps(uint8_t * pps, size_t size);
		int32_t set_video_width(int32_t width);
		int32_t set_video_height(int32_t height);

		int32_t get_video_submedia_type(int32_t & mt);
		int32_t get_vps(uint8_t * vps, size_t & size);
		int32_t get_sps(uint8_t * sps, size_t & size);
		int32_t get_pps(uint8_t * pps, size_t & size);
		int32_t get_video_width(int32_t & width);
		int32_t get_video_height(int32_t & height);

		const uint8_t * get_vps(size_t & size);
		const uint8_t * get_sps(size_t & size);
		const uint8_t * get_pps(size_t & size);


		int32_t push_audio(const uint8_t * data, size_t size, long long timestamp);
		int32_t pop_audio(uint8_t * data, size_t & size, long long & timestamp);

		int32_t set_audio_submedia_type(int32_t mt);
		int32_t set_configstr(uint8_t * configstr, size_t size);
		int32_t set_audio_samplerate(int32_t samplerate);
		int32_t set_audio_bitdepth(int32_t bitdepth);
		int32_t set_audio_channels(int32_t channels);

		int32_t get_audio_submedia_type(int32_t & mt);
		int32_t get_configstr(uint8_t * configstr, size_t & size);
		int32_t get_audio_samplerate(int32_t & samplerate);
		int32_t get_audio_bitdepth(int32_t & bitdepth);
		int32_t get_audio_channels(int32_t & channels);


	private:
#if defined(WITH_SINGLETON)
		media_buffering(void);
		virtual ~media_buffering(void);
#endif
		media_buffering(const media_buffering & clone);

	private:
		video_buffer * _vbuffer;
		audio_buffer * _abuffer;
	};
}
