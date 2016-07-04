#ifndef _DK_RTSP_EXPORTOR_H_
#define _DK_RTSP_EXPORTOR_H_

#include <dk_base_receiver.h>
#include <dk_rtsp_client.h>
#include <dk_ff_tsmuxer.h>

namespace debuggerking
{
	class rtsp_exportor_status_callback
	{
	public:
		virtual void start(void) = 0;
		virtual void stop(void) = 0;
	};

	class rtsp_exportor : public base_receiver, public rtsp_client
	{
	public:
		rtsp_exportor(rtsp_exportor_status_callback * cb = nullptr);
		virtual ~rtsp_exportor(void);

		int32_t play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, bool repeat, char * export_file_path, int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second);
		int32_t stop(void);

		void on_begin_video(int32_t smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp);
		void on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);
		void on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp);
		void on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp);

	private:
		static unsigned __stdcall process_callback(void * param);
		void process(void);

	private:
		rtsp_exportor_status_callback * _cb;
		char _url[MAX_PATH];
		char _username[MAX_PATH];
		char _password[MAX_PATH];
		int32_t _transport_option;
		int32_t _recv_option;
		bool _repeat;
		char _ts_file_path[MAX_PATH];
		int32_t _year;
		int32_t _month;
		int32_t _day;
		int32_t _hour;
		int32_t _minute;
		int32_t _second;
		int32_t _millisec;

		int32_t _rcvd_year;
		int32_t _rcvd_month;
		int32_t _rcvd_day;
		int32_t _rcvd_hour;
		int32_t _rcvd_minute;
		int32_t _rcvd_second;
		int32_t _rcvd_millisec;

		ff_tsmuxer::configuration_t * _tsmuxer_config;
		ff_tsmuxer * _tsmuxer;

		//bool _idr_rcvd;
		bool _rcvd_timestamp;

		HANDLE _thread;
		bool _run;

	};
};

#endif
