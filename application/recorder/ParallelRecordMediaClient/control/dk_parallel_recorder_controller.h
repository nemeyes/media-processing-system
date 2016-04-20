#ifndef _DK_PARALLEL_RECORDER_CONTROLLER_H_
#define _DK_PARALLEL_RECORDER_CONTROLLER_H_

#include <dk_ipc_client.h>
#include <dk_log4cplus_logger.h>


typedef struct _parallel_recorder_t parallel_recorder_t;
class dk_parallel_recorder_controller : public ic::dk_ipc_client
{
public:
	dk_parallel_recorder_controller(parallel_recorder_t * parallel_recorder);
	virtual ~dk_parallel_recorder_controller(void);

	//void get_rtsp_server_port_number(int & port_number);
	void get_years(const char * uuid, int years[], int capacity, int & size);
	void get_months(const char * uuid, int year, int months[], int capacity, int & size);
	void get_days(const char * uuid, int year, int month, int days[], int capacity, int & size);
	void get_hours(const char * uuid, int year, int month, int day, int hours[], int capacity, int & size);
	void get_minutes(const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size);
	void get_seconds(const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size);

	//void set_rtsp_server_port_number(int port_number);
	void set_years(const char * uuid, int years[], int size);
	void set_months(const char * uuid, int year, int months[], int size);
	void set_days(const char * uuid, int year, int month, int days[], int size);
	void set_hours(const char * uuid, int year, int month, int day, int hours[], int size);
	void set_minutes(const char * uuid, int year, int month, int day, int hour, int minutes[], int size);
	void set_seconds(const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int size);

	void get_rtsp_server_port_number_callback(int port_number);
private:
	void assoc_completion_callback(void);
	void leave_completion_callback(void);
	

private:
	parallel_recorder_t * _parallel_recorder;
	dk_log4cplus_logger * _logger;

	//int32_t _rtsp_server_port_number;

	char _uuid[64];
	int32_t _year;
	int32_t _month;
	int32_t _day;
	int32_t _hour;
	int32_t _minute;
	int32_t _second;

	int32_t _nyears;
	int32_t _years[10];
	int32_t _nmonths;
	int32_t _months[12];
	int32_t _ndays;
	int32_t _days[31];
	int32_t _nhours;
	int32_t _hours[24];
	int32_t _nminutes;
	int32_t _minutes[60];
	int32_t _nseconds;
	int32_t _seconds[60];
};













#endif