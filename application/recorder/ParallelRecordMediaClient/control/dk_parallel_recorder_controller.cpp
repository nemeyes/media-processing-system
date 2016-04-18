#include <winsock2.h>
#include <windows.h>
#include "dk_parallel_recorder_controller.h"
#include "dk_parallel_recorder.h"
#include "commands_client.h"

dk_parallel_recorder_controller::dk_parallel_recorder_controller(parallel_recorder_t * parallel_recorder)
	: _parallel_recorder(parallel_recorder)
{
	add_command(new ic::get_years_res_cmd(this));
	add_command(new ic::get_months_res_cmd(this));
	add_command(new ic::get_days_res_cmd(this));
	add_command(new ic::get_hours_res_cmd(this));
	add_command(new ic::get_minutes_res_cmd(this));
	add_command(new ic::get_seconds_res_cmd(this));
}

dk_parallel_recorder_controller::~dk_parallel_recorder_controller(void)
{

}

void dk_parallel_recorder_controller::get_years(const char * uuid, int years[], int capacity, int & size)
{
	if (_parallel_recorder->waiting_response)
		return;
	if (!uuid && strlen(uuid) < 1)
		return;

	ic::CMD_GET_YEARS_REQ_T req;
	memcpy(req.uuid, uuid, sizeof(req.uuid));

	data_request(SERVER_UUID, CMD_GET_YEARS_REQUEST, (char*)&req, sizeof(req));
	_parallel_recorder->waiting_response = true;

	for (int i = 0; i < 100; i++)
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

void dk_parallel_recorder_controller::get_months(const char * uuid, int year, int months[], int capacity, int & size)
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

	for (int i = 0; i < 100; i++)
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

void dk_parallel_recorder_controller::get_days(const char * uuid, int year, int month, int days[], int capacity, int & size)
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

	for (int i = 0; i < 100; i++)
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

void dk_parallel_recorder_controller::set_years(const char * uuid, int years[], int size)
{
	memcpy(_uuid, uuid, sizeof(uuid));

	_nyears = size;
	for (int index = 0; index < _nyears; index++)
	{
		_years[index] = years[index];
	}
	_parallel_recorder->waiting_response = false;
}

void dk_parallel_recorder_controller::set_months(const char * uuid, int year, int months[], int size)
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

void dk_parallel_recorder_controller::set_days(const char * uuid, int year, int month, int days[], int size)
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

void dk_parallel_recorder_controller::assoc_completion_callback(void)
{
	if (_parallel_recorder)
		_parallel_recorder->connected = true;
}

void dk_parallel_recorder_controller::leave_completion_callback(void)
{
	if (_parallel_recorder)
		_parallel_recorder->connected = false;
}