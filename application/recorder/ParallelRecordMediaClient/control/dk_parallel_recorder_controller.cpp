#include <winsock2.h>
#include <windows.h>
#include "dk_parallel_recorder_controller.h"
#include "dk_parallel_recorder.h"
#include "commands_client.h"

debuggerking::parallel_recorder_controller::parallel_recorder_controller(parallel_recorder_t * parallel_recorder)
	: _parallel_recorder(parallel_recorder)
{
#if defined(WITH_RTSP_SERVER)
	add_command(new ic::get_rtsp_server_port_res_cmd(this));
#else
	add_command(new ic::begin_playback_res_cmd(this));
	add_command(new ic::end_playback_res_cmd(this));
#endif
	add_command(new ic::get_years_res_cmd(this));
	add_command(new ic::get_months_res_cmd(this));
	add_command(new ic::get_days_res_cmd(this));
	add_command(new ic::get_hours_res_cmd(this));
	add_command(new ic::get_minutes_res_cmd(this));
	add_command(new ic::get_seconds_res_cmd(this));
}

debuggerking::parallel_recorder_controller::~parallel_recorder_controller(void)
{
	for (int32_t index = 0; (index < 100) && _parallel_recorder->connected; index++)
		::Sleep(30);
}

/*
void dk_parallel_recorder_controller::get_rtsp_server_port_number(int & port_number)
{
	if (_parallel_recorder->waiting_response)
		return;

	ic::CMD_GET_RTSP_SERVER_PORT_REQ_T req;

	data_request(SERVER_UUID, CMD_GET_RTSP_SERVER_PORT_REQUEST, (char*)&req, sizeof(req));
	_parallel_recorder->waiting_response = true;

	for (int i = 0; i < 100; i++)
	{
		if (!_parallel_recorder->waiting_response)
			break;
		::Sleep(10);
	}

	port_number = _rtsp_server_port_number;
}
*/

void debuggerking::parallel_recorder_controller::get_years(const char * uuid, int years[], int capacity, int & size)
{
	if (_parallel_recorder->waiting_response)
		return;
	if (!uuid && strlen(uuid) < 1)
		return;

	ic::CMD_GET_YEARS_REQ_T req;
	memcpy(req.uuid, uuid, sizeof(req.uuid));

	data_request(SERVER_UUID, CMD_GET_YEARS_REQUEST, (char*)&req, sizeof(req));
	_parallel_recorder->waiting_response = true;

	for (int i = 0; i < 1000; i++)
	{
		if (!_parallel_recorder->waiting_response)
			break;
		::Sleep(10);
	}

	size = capacity>_nyears ? _nyears : capacity;
	for (int index = 0; index < size; index++)
	{
		years[index] = _years[index];
	}
}

void debuggerking::parallel_recorder_controller::get_months(const char * uuid, int year, int months[], int capacity, int & size)
{
	if (_parallel_recorder->waiting_response)
		return;
	if (!uuid && strlen(uuid) < 1)
		return;

	ic::CMD_GET_MONTHS_REQ_T req;
	memcpy(req.uuid, uuid, sizeof(req.uuid));
	req.year = year;

	data_request(SERVER_UUID, CMD_GET_MONTHS_REQUEST, (char*)&req, sizeof(req));
	_parallel_recorder->waiting_response = true;

	for (int i = 0; i < 1000; i++)
	{
		if (!_parallel_recorder->waiting_response)
			break;
		::Sleep(10);
	}

	size = capacity>_nmonths ? _nmonths : capacity;
	for (int index = 0; index < size; index++)
	{
		months[index] = _months[index];
	}
}

void debuggerking::parallel_recorder_controller::get_days(const char * uuid, int year, int month, int days[], int capacity, int & size)
{
	if (_parallel_recorder->waiting_response)
		return;
	if (!uuid && strlen(uuid) < 1)
		return;

	ic::CMD_GET_DAYS_REQ_T req;
	memcpy(req.uuid, uuid, sizeof(req.uuid));
	req.year = year;
	req.month = month;

	data_request(SERVER_UUID, CMD_GET_DAYS_REQUEST, (char*)&req, sizeof(req));
	_parallel_recorder->waiting_response = true;

	for (int i = 0; i < 1000; i++)
	{
		if (!_parallel_recorder->waiting_response)
			break;
		::Sleep(10);
	}

	size = capacity>_ndays ? _ndays : capacity;
	for (int index = 0; index < size; index++)
	{
		days[index] = _days[index];
	}
}

void debuggerking::parallel_recorder_controller::get_hours(const char * uuid, int year, int month, int day, int hours[], int capacity, int & size)
{
	if (_parallel_recorder->waiting_response)
		return;
	if (!uuid && strlen(uuid) < 1)
		return;

	ic::CMD_GET_HOURS_REQ_T req;
	memcpy(req.uuid, uuid, sizeof(req.uuid));
	req.year = year;
	req.month = month;
	req.day = day;

	data_request(SERVER_UUID, CMD_GET_HOURS_REQUEST, (char*)&req, sizeof(req));
	_parallel_recorder->waiting_response = true;

	for (int i = 0; i < 1000; i++)
	{
		if (!_parallel_recorder->waiting_response)
			break;
		::Sleep(10);
	}

	size = capacity>_nhours ? _nhours : capacity;
	for (int index = 0; index < size; index++)
	{
		hours[index] = _hours[index];
	}
}

void debuggerking::parallel_recorder_controller::get_minutes(const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size)
{
	if (_parallel_recorder->waiting_response)
		return;
	if (!uuid && strlen(uuid) < 1)
		return;

	ic::CMD_GET_MINUTES_REQ_T req;
	memcpy(req.uuid, uuid, sizeof(req.uuid));
	req.year = year;
	req.month = month;
	req.day = day;
	req.hour = hour;

	data_request(SERVER_UUID, CMD_GET_MINUTES_REQUEST, (char*)&req, sizeof(req));
	_parallel_recorder->waiting_response = true;

	for (int i = 0; i < 1000; i++)
	{
		if (!_parallel_recorder->waiting_response)
			break;
		::Sleep(10);
	}

	size = capacity>_nminutes ? _nminutes : capacity;
	for (int index = 0; index < size; index++)
	{
		minutes[index] = _minutes[index];
	}
}

void debuggerking::parallel_recorder_controller::get_seconds(const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size)
{
	if (_parallel_recorder->waiting_response)
		return;
	if (!uuid && strlen(uuid) < 1)
		return;

	ic::CMD_GET_SECONDS_REQ_T req;
	memcpy(req.uuid, uuid, sizeof(req.uuid));
	req.year = year;
	req.month = month;
	req.day = day;
	req.hour = hour;
	req.minute = minute;

	data_request(SERVER_UUID, CMD_GET_SECONDS_REQUEST, (char*)&req, sizeof(req));
	_parallel_recorder->waiting_response = true;

	for (int i = 0; i < 1000; i++)
	{
		if (!_parallel_recorder->waiting_response)
			break;
		::Sleep(10);
	}

	size = capacity>_nseconds ? _nseconds : capacity;
	for (int index = 0; index < size; index++)
	{
		seconds[index] = _seconds[index];
	}
}

/*
void dk_parallel_recorder_controller::set_rtsp_server_port_number(int port_number)
{
	_rtsp_server_port_number = port_number;
	_parallel_recorder->waiting_response = false;
}
*/

void debuggerking::parallel_recorder_controller::set_years(const char * uuid, int years[], int size)
{
	memcpy(_uuid, uuid, sizeof(uuid));

	_nyears = size;
	for (int index = 0; index < _nyears; index++)
	{
		_years[index] = years[index];
	}
	_parallel_recorder->waiting_response = false;
}

void debuggerking::parallel_recorder_controller::set_months(const char * uuid, int year, int months[], int size)
{
	memcpy(_uuid, uuid, sizeof(uuid));
	_year = year;

	_nmonths = size;
	for (int index = 0; index < _nmonths; index++)
	{
		_months[index] = months[index];
	}
	_parallel_recorder->waiting_response = false;
}

void debuggerking::parallel_recorder_controller::set_days(const char * uuid, int year, int month, int days[], int size)
{
	memcpy(_uuid, uuid, sizeof(uuid));
	_year = year;
	_month = month;

	_ndays = size;
	for (int index = 0; index < _ndays; index++)
	{
		_days[index] = days[index];
	}
	_parallel_recorder->waiting_response = false;
}

void debuggerking::parallel_recorder_controller::set_hours(const char * uuid, int year, int month, int day, int hours[], int size)
{
	memcpy(_uuid, uuid, sizeof(uuid));
	_year = year;
	_month = month;
	_day = day;

	_nhours = size;
	for (int index = 0; index < _nhours; index++)
	{
		_hours[index] = hours[index];
	}
	_parallel_recorder->waiting_response = false;
}

void debuggerking::parallel_recorder_controller::set_minutes(const char * uuid, int year, int month, int day, int hour, int minutes[], int size)
{
	memcpy(_uuid, uuid, sizeof(uuid));
	_year = year;
	_month = month;
	_day = day;
	_hour = hour;

	_nminutes = size;
	for (int index = 0; index < _nminutes; index++)
	{
		_minutes[index] = minutes[index];
	}
	_parallel_recorder->waiting_response = false;
}

void debuggerking::parallel_recorder_controller::set_seconds(const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int size)
{
	memcpy(_uuid, uuid, sizeof(uuid));
	_year = year;
	_month = month;
	_day = day;
	_hour = hour;
	_minute = minute;

	_nseconds = size;
	for (int index = 0; index < _nseconds; index++)
	{
		_seconds[index] = seconds[index];
	}
	_parallel_recorder->waiting_response = false;
}

void debuggerking::parallel_recorder_controller::assoc_completion_callback(void)
{
	if (_parallel_recorder && !_parallel_recorder->connected)
	{
		ic::CMD_GET_RTSP_SERVER_PORT_REQ_T req;
		data_request(SERVER_UUID, CMD_GET_RTSP_SERVER_PORT_REQUEST, (char*)&req, sizeof(req));

		//log4cplus_logger::make_info_log("parallel.record.client", "connection completed");
		_parallel_recorder->connected = true;
	}
}

void debuggerking::parallel_recorder_controller::leave_completion_callback(void)
{
	if (_parallel_recorder && _parallel_recorder->connected)
	{
		//log4cplus_logger::make_info_log("parallel.record.client", "disconnection completed");

		_parallel_recorder->connected = false;
		_parallel_recorder->rtsp_port_number_received = false;
	}
}

void debuggerking::parallel_recorder_controller::get_rtsp_server_port_number_callback(int port_number)
{
	if (_parallel_recorder && _parallel_recorder->connected)
	{
		_parallel_recorder->rtsp_server_port_number = port_number;
		_parallel_recorder->rtsp_port_number_received = true;

		//log4cplus_logger::make_info_log("parallel.record.client", "port number[%d] callback received", _parallel_recorder->rtsp_server_port_number);
	}
}