#ifndef _COMMANDS_CLIENT_H_
#define _COMMANDS_CLIENT_H_

#include <command.h>
#include <commands_payload.h>

#include "dk_parallel_recorder_controller.h"

namespace ic
{
	class parallel_record_client_cmd : public abstract_command
	{
	public:
		parallel_record_client_cmd(dk_parallel_recorder_controller * prsc, int32_t command_id)
			: abstract_command(command_id)
			, _prsc(prsc) {}
		virtual ~parallel_record_client_cmd(void) {}

	protected:
		dk_parallel_recorder_controller * _prsc;
	};

	class get_rtsp_server_port_res_cmd : public parallel_record_client_cmd
	{
	public:
		get_rtsp_server_port_res_cmd(dk_parallel_recorder_controller * prsc)
			: parallel_record_client_cmd(prsc, CMD_GET_RTSP_SERVER_PORT_RESPONSE) {}
		virtual ~get_rtsp_server_port_res_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_RTSP_SERVER_PORT_RES_T res;
			memset(&res, 0x00, sizeof(res));
			memcpy(&res, msg, sizeof(res));

			_prsc->get_rtsp_server_port_number_callback(res.port_number);
		}
	};

	class get_years_res_cmd : public parallel_record_client_cmd
	{
	public:
		get_years_res_cmd(dk_parallel_recorder_controller * prsc)
			: parallel_record_client_cmd(prsc, CMD_GET_YEARS_RESPONSE) {}
		virtual ~get_years_res_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_YEARS_RES_T res;
			memset(&res, 0x00, sizeof(res));
			memcpy(&res, msg, sizeof(res));

			_prsc->set_years(res.uuid, res.years, res.count);
		}
	};


	class get_months_res_cmd : public parallel_record_client_cmd
	{
	public:
		get_months_res_cmd(dk_parallel_recorder_controller * prsc)
			: parallel_record_client_cmd(prsc, CMD_GET_MONTHS_RESPONSE) {}
		virtual ~get_months_res_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_MONTHS_RES_T res;
			memset(&res, 0x00, sizeof(res));
			memcpy(&res, msg, sizeof(res));

			_prsc->set_months(res.uuid, res.year, res.months, res.count);
		}
	};

	class get_days_res_cmd : public parallel_record_client_cmd
	{
	public:
		get_days_res_cmd(dk_parallel_recorder_controller * prsc)
			: parallel_record_client_cmd(prsc, CMD_GET_DAYS_RESPONSE) {}
		virtual ~get_days_res_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_DAYS_RES_T res;
			memset(&res, 0x00, sizeof(res));
			memcpy(&res, msg, sizeof(res));

			_prsc->set_days(res.uuid, res.year, res.month, res.days, res.count);
		}
	};

	class get_hours_res_cmd : public parallel_record_client_cmd
	{
	public:
		get_hours_res_cmd(dk_parallel_recorder_controller * prsc)
			: parallel_record_client_cmd(prsc, CMD_GET_HOURS_RESPONSE) {}
		virtual ~get_hours_res_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_HOURS_RES_T res;
			memset(&res, 0x00, sizeof(res));
			memcpy(&res, msg, sizeof(res));

			_prsc->set_hours(res.uuid, res.year, res.month, res.day, res.hours, res.count);
		}
	};

	class get_minutes_res_cmd : public parallel_record_client_cmd
	{
	public:
		get_minutes_res_cmd(dk_parallel_recorder_controller * prsc)
			: parallel_record_client_cmd(prsc, CMD_GET_MINUTES_RESPONSE) {}
		virtual ~get_minutes_res_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_MINUTES_RES_T res;
			memset(&res, 0x00, sizeof(res));
			memcpy(&res, msg, sizeof(res));

			_prsc->set_minutes(res.uuid, res.year, res.month, res.day, res.hour, res.minutes, res.count);
		}
	};

	class get_seconds_res_cmd : public parallel_record_client_cmd
	{
	public:
		get_seconds_res_cmd(dk_parallel_recorder_controller * prsc)
			: parallel_record_client_cmd(prsc, CMD_GET_SECONDS_RESPONSE) {}
		virtual ~get_seconds_res_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_SECONDS_RES_T res;
			memset(&res, 0x00, sizeof(res));
			memcpy(&res, msg, sizeof(res));

			_prsc->set_seconds(res.uuid, res.year, res.month, res.day, res.hour, res.minute, res.seconds, res.count);
		}
	};
};




















#endif