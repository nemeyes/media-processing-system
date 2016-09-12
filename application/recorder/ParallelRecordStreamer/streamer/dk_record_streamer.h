#ifndef _DK_RECORD_STREAMER_H_
#define _DK_RECORD_STREAMER_H_

#include <cstdint>
#include <dk_ipc_server.h>

namespace debuggerking
{
	class recorder_rtsp_server;
	class record_streamer : public ic::dk_ipc_server
	{
	public:
		record_streamer(void);
		virtual ~record_streamer(void);
		//static dk_streamer_service & instance(void);

		bool start(void);
		bool stop(void);

		void get_years(const char * uuid, int years[], int capacity, int & size);
		void get_months(const char * uuid, int year, int months[], int capacity, int & size);
		void get_days(const char * uuid, int year, int month, int days[], int capacity, int & size);
		void get_hours(const char * uuid, int year, int month, int day, int hours[], int capacity, int & size);
		void get_minutes(const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size);
		void get_seconds(const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size);

		const char * retrieve_contents_path(void);
		const char * retrieve_config_path(void);

	#if defined(WITH_RTSP_SERVER)
		int32_t get_rtsp_server_port_number(void);
	#endif

	private:
		void assoc_completion_callback(const char * uuid);
		void leave_completion_callback(const char * uuid);

	private:
		char _contents_path[260];
		char _config_path[260];

		bool _run;
	#if defined(WITH_RTSP_SERVER)
		int32_t _rtsp_server_port_number;
		recorder_rtsp_server * _rtsp_server;
	#endif
	};
};

#endif