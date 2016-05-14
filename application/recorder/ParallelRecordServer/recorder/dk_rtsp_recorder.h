#pragma once
#include <windows.h>
#include <cstdint>
#include "dk_bit_vector.h"
#include <dk_live_rtsp_client.h>
#if defined(WITH_MPEG2TS)
 #include "dk_mpeg2ts_recorder.h"
#else
 #include "dk_record_module.h"
#endif

#if defined(WITH_RELAY_LIVE)
 #define WITH_SERVER_PUBLISH
 #include <dk_shared_memory.h>
#endif

namespace debuggerking 
{
	class rtsp_recorder : public live_rtsp_client
	{
	public:
		rtsp_recorder(int32_t chunk_size_mb);
		virtual ~rtsp_recorder(void);

		void start_recording(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, int32_t recv_timeout, const char * storage, const char * uuid);
		void stop_recording(void);

		void on_begin_video(int32_t smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp);
		void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);

		void on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp);
		void on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);

	private:
		// A general bit copy operation:
		void parse_vui(CBitVector& bv, unsigned& num_units_in_tick, unsigned& time_scale, unsigned& fixed_frame_rate_flag, int* sar_width, int* sar_height);
		int parse_pps(uint8_t* data, int sizeOfSPS);
		int parse_sps(uint8_t* data, int sizeOfSPS, int *width, int *height, int* sar_width, int* sar_height);
		int parse_mpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
		int parse_jpeg(uint8_t* data, int size, int *width, int *height, int* sar_width, int* sar_height);
		void make_adts_header(uint8_t* data, int size, char audioObjectType, char samplingFreqIndex, char channelConfig);

		uint8_t * get_sps(size_t & sps_size);
		uint8_t * get_pps(size_t & pps_size);
		void set_sps(uint8_t * sps, size_t sps_size);
		void set_pps(uint8_t * pps, size_t pps_size);
		void clear_sps(void);
		void clear_pps(void);

	private:
		static long long get_elapsed_msec_from_epoch(void);
		static void get_time_from_elapsed_msec_from_epoch(long long elapsed_time, char * time_string, int time_string_size);

	private:
		char _storage[260];
		char _uuid[260];

		uint8_t _sps[200];
		size_t _sps_size;
		uint8_t _pps[200];
		size_t _pps_size;
	
	#if defined(WITH_MPEG2TS)
		dk_ff_mpeg2ts_muxer::configuration_t _config;
		dk_mpeg2ts_recorder * _mpeg2ts_recorder;
	#else
		dk_record_module * _file_recorder;
	#endif

	#if defined(WITH_RELAY_LIVE)
		ic::dk_shared_memory_server * _sm_server;
	#endif

		long long _chunk_size_bytes;
	};
};
