#ifndef _DK_RTMP_CLIENT_H_
#define _DK_RTMP_CLIENT_H_

#include <dk_basic_type.h>

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

namespace debuggerking
{
	class rtmp_core;
	class EXP_CLASS rtmp_client : public foundation
	{
	public:
		typedef enum _rtmp_state
		{
			state_stopped = 0,
			state_paused,
			state_subscribing,
			state_begin_publishing,
			state_publishing,
		} rtmp_state;

		rtmp_client(bool split_thread = false);
		virtual ~rtmp_client(void);

		uint8_t * get_sps(size_t & sps_size);
		uint8_t * get_pps(size_t & pps_size);
		void set_sps(uint8_t * sps, size_t sps_size);
		void set_pps(uint8_t * pps, size_t pps_size);
		void clear_sps(void);
		void clear_pps(void);

		rtmp_client::rtmp_state state(void);

		int32_t subscribe_begin(const char * url, const char * username, const char * password, int32_t recv_option, bool repeat = true);
		int32_t subscribe_end(void);
		virtual void on_begin_video(int32_t smt, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp);
		virtual void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);
		virtual void on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp);
		virtual void on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);

		int32_t publish_begin(int32_t vsmt, int32_t asmt, const char * url, const char * username, const char * password);
		int32_t publish_end(void);
		int32_t publish_video(uint8_t * bitstream, size_t nb, long long timestamp);
		int32_t publish_audio(uint8_t * bitstream, size_t nb, bool configstr, long long timestamp);

	private:
		bool _split_thread = false;
		rtmp_core * _core;
	};
};

#endif
