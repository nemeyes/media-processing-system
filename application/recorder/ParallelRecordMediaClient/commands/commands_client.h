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


	class get_years_res_cmd : public parallel_record_client_cmd
	{
	public:
		get_years_res_cmd(dk_parallel_recorder_controller * prsc)
			: parallel_record_client_cmd(prsc, CMD_GET_YEARS_RESPONSE) {}
		virtual ~get_years_res_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_YEARS_REQ_T req;
			CMD_GET_YEARS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));


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
			CMD_GET_MONTHS_REQ_T req;
			CMD_GET_MONTHS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));


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
			CMD_GET_DAYS_REQ_T req;
			CMD_GET_DAYS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));


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
			CMD_GET_HOURS_REQ_T req;
			CMD_GET_HOURS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));


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
			CMD_GET_MINUTES_REQ_T req;
			CMD_GET_MINUTES_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));



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
			CMD_GET_SECONDS_REQ_T req;
			CMD_GET_SECONDS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));



		}
	};
};




















#endif