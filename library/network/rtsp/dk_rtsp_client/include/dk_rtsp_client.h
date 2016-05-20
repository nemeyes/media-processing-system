#ifndef _DK_RTSP_CLIENT_H_
#define _DK_RTSP_CLIENT_H_

#include <dk_rtsp_base.h>

namespace debuggerking
{
	class rtsp_core;
	class EXP_CLASS rtsp_client : public rtsp_base
	{
		friend class buffer_sink;
	public:
		rtsp_client(void);
		virtual ~rtsp_client(void);
		int32_t play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, int32_t recv_timeout, float scale = 1.f, bool repeat = true);
		int32_t stop(void);

		void set_sps(uint8_t * sps, size_t sps_size);
		void set_pps(uint8_t * pps, size_t pps_size);

		uint8_t * get_sps(size_t & sps_size);
		uint8_t * get_pps(size_t & pps_size);

		virtual void on_begin_video(int32_t smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp) = 0;
		virtual void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp) = 0;
		virtual void on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp) = 0;
		virtual void on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp) = 0;

		bool ignore_sdp(void);

	private:
		void process( void );
	#if !defined(WIN32)
		static void* process_cb( void * param );
	#else
		static unsigned __stdcall process_cb( void * param );
	#endif
	#if !defined(WIN32)
		pthread_t _worker;
	#else
		void * _worker;
	#endif

	private:
		char _url[260];
		char _username[260];
		char _password[260];
		int32_t _transport_option;
		int32_t _recv_option;
		int32_t _recv_timeout;
		float _scale;
		bool _repeat;

		rtsp_core * _live;
		bool _kill;
		bool _ignore_sdp;

		uint8_t _sps[100];
		uint8_t _pps[100];
		int32_t _sps_size;
		int32_t _pps_size;
	};
};

#endif // dk_live_rtsp_client_H
