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