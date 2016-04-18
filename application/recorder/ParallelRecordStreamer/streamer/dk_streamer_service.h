#ifndef _DK_STREAMER_SERVICE_H_
#define _DK_STREAMER_SERVICE_H_

#include <cstdint>
#include <dk_ipc_server.h>

class dk_log4cplus_logger;
class dk_vod_rtsp_server;
class dk_streamer_service : public ic::dk_ipc_server
{
public:
	dk_streamer_service(void);
	virtual ~dk_streamer_service(void);
	//static dk_streamer_service & instance(void);

	bool start_streaming(void);
	bool stop_streaming(void);

	void get_years(const char * uuid, int years[], int capacity, int & size);
	void get_months(const char * uuid, int year, int months[], int capacity, int & size);
	void get_days(const char * uuid, int year, int month, int days[], int capacity, int & size);
	void get_hours(const char * uuid, int year, int month, int day, int hours[], int capacity, int & size);
	void get_minutes(const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size);
	void get_seconds(const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size);

	const char * retrieve_storage_path(void);
	const char * retrieve_config_path(void);

private:
	void assoc_completion_callback(const char * uuid);
	void leave_completion_callback(const char * uuid);

	void get_time_from_elapsed_msec_from_epoch(long long elapsed_time, int & year, int & month, int & day, int & hour, int & minute, int & second);
private:
	char _storage_path[260];
	char _config_path[260];
	dk_vod_rtsp_server * _rtsp_server;
	bool _is_run;

	dk_log4cplus_logger * _logger;
};


#endif