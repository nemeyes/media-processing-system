#ifndef _COMMANDS_SERVER_H_
#define _COMMANDS_SERVER_H_

#include <command.h>
#include <commands_payload.h>

#include "dk_streamer_service.h"

namespace ic
{
	class parallel_record_server_cmd : public abstract_command
	{
	public:
		parallel_record_server_cmd(dk_streamer_service * prss, int32_t command_id)
			: abstract_command(command_id)
			, _prss(prss) {}
		virtual ~parallel_record_server_cmd(void) {}

	protected:
		dk_streamer_service * _prss;
	};

	//CMD_GET_RTSP_SERVER_PORT_REQUEST
	class get_rtsp_server_port_req_cmd : public parallel_record_server_cmd
	{
	public:
		get_rtsp_server_port_req_cmd(dk_streamer_service * prss)
			: parallel_record_server_cmd(prss, CMD_GET_RTSP_SERVER_PORT_REQUEST) {}
		virtual ~get_rtsp_server_port_req_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_RTSP_SERVER_PORT_REQ_T req;
			CMD_GET_RTSP_SERVER_PORT_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));

			res.code = CMD_ERR_CODE_SUCCESS;
			res.port_number = _prss->get_rtsp_server_port_number();

			session->push_send_packet(session->uuid(), uuid(), CMD_GET_RTSP_SERVER_PORT_RESPONSE, reinterpret_cast<char*>(&res), sizeof(res));
		}
	};

	class get_years_req_cmd : public parallel_record_server_cmd
	{
	public:
		get_years_req_cmd(dk_streamer_service * prss)
			: parallel_record_server_cmd(prss, CMD_GET_YEARS_REQUEST) {}
		virtual ~get_years_req_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_YEARS_REQ_T req;
			CMD_GET_YEARS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));

			res.code = CMD_ERR_CODE_SUCCESS;
			memcpy(res.uuid, req.uuid, sizeof(res.uuid));
			_prss->get_years(res.uuid, res.years, sizeof(res.years) / sizeof(int), res.count);
			
			session->push_send_packet(session->uuid(), uuid(), CMD_GET_YEARS_RESPONSE, reinterpret_cast<char*>(&res), sizeof(res));
		}
	};


	class get_months_req_cmd : public parallel_record_server_cmd
	{
	public:
		get_months_req_cmd(dk_streamer_service * prss)
			: parallel_record_server_cmd(prss, CMD_GET_MONTHS_REQUEST) {}
		virtual ~get_months_req_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_MONTHS_REQ_T req;
			CMD_GET_MONTHS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));

			res.code = CMD_ERR_CODE_SUCCESS;
			memcpy(res.uuid, req.uuid, sizeof(res.uuid));
			res.year = req.year;
			_prss->get_months(res.uuid, res.year, res.months, sizeof(res.months) / sizeof(int), res.count);
			

			session->push_send_packet(session->uuid(), uuid(), CMD_GET_MONTHS_RESPONSE, reinterpret_cast<char*>(&res), sizeof(res));
		}
	};

	class get_days_req_cmd : public parallel_record_server_cmd
	{
	public:
		get_days_req_cmd(dk_streamer_service * prss)
			: parallel_record_server_cmd(prss, CMD_GET_DAYS_REQUEST) {}
		virtual ~get_days_req_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_DAYS_REQ_T req;
			CMD_GET_DAYS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));

			res.code = CMD_ERR_CODE_SUCCESS;
			memcpy(res.uuid, req.uuid, sizeof(res.uuid));
			res.year = req.year;
			res.month = req.month;
			_prss->get_days(res.uuid, res.year, res.month, res.days, sizeof(res.days) / sizeof(int), res.count);

			session->push_send_packet(session->uuid(), uuid(), CMD_GET_DAYS_RESPONSE, reinterpret_cast<char*>(&res), sizeof(res));
		}
	};

	class get_hours_req_cmd : public parallel_record_server_cmd
	{
	public:
		get_hours_req_cmd(dk_streamer_service * prss)
			: parallel_record_server_cmd(prss, CMD_GET_HOURS_REQUEST) {}
		virtual ~get_hours_req_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_HOURS_REQ_T req;
			CMD_GET_HOURS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));

			res.code = CMD_ERR_CODE_SUCCESS;
			memcpy(res.uuid, req.uuid, sizeof(res.uuid));
			res.year = req.year;
			res.month = req.month;
			res.day = req.day;
			_prss->get_hours(res.uuid, res.year, res.month, res.day, res.hours, sizeof(res.hours) / sizeof(int), res.count);

			session->push_send_packet(session->uuid(), uuid(), CMD_GET_HOURS_RESPONSE, reinterpret_cast<char*>(&res), sizeof(res));
		}
	};

	class get_minutes_req_cmd : public parallel_record_server_cmd
	{
	public:
		get_minutes_req_cmd(dk_streamer_service * prss)
			: parallel_record_server_cmd(prss, CMD_GET_MINUTES_REQUEST) {}
		virtual ~get_minutes_req_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_MINUTES_REQ_T req;
			CMD_GET_MINUTES_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));

			res.code = CMD_ERR_CODE_SUCCESS;
			memcpy(res.uuid, req.uuid, sizeof(res.uuid));
			res.year = req.year;
			res.month = req.month;
			res.day = req.day;
			res.hour = req.hour;
			_prss->get_minutes(res.uuid, res.year, res.month, res.day, res.hour, res.minutes, sizeof(res.minutes) / sizeof(int), res.count);

			session->push_send_packet(session->uuid(), uuid(), CMD_GET_MINUTES_RESPONSE, reinterpret_cast<char*>(&res), sizeof(res));
		}
	};

	class get_seconds_req_cmd : public parallel_record_server_cmd
	{
	public:
		get_seconds_req_cmd(dk_streamer_service * prss)
			: parallel_record_server_cmd(prss, CMD_GET_SECONDS_REQUEST) {}
		virtual ~get_seconds_req_cmd(void) {}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_GET_SECONDS_REQ_T req;
			CMD_GET_SECONDS_RES_T res;
			memset(&req, 0x00, sizeof(req));
			memset(&res, 0x00, sizeof(res));
			memcpy(&req, msg, sizeof(req));

			res.code = CMD_ERR_CODE_SUCCESS;
			memcpy(res.uuid, req.uuid, sizeof(res.uuid));
			res.year = req.year;
			res.month = req.month;
			res.day = req.day;
			res.hour = req.hour;
			res.minute = req.minute;
			_prss->get_seconds(res.uuid, res.year, res.month, res.day, res.hour, res.minute, res.seconds, sizeof(res.seconds) / sizeof(int), res.count);

			session->push_send_packet(session->uuid(), uuid(), CMD_GET_SECONDS_RESPONSE, reinterpret_cast<char*>(&res), sizeof(res));
		}
	};
};












#endif