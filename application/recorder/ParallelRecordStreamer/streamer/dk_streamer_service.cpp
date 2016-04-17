#include "dk_streamer_service.h"
#include <dk_time_helper.h>
#include <dk_misc_helper.h>
#include <tinyxml.h>
#include <dk_vod_rtsp_server.h>
#include <dk_log4cplus_logger.h>

#include "commands_server.h"

dk_streamer_service::dk_streamer_service(void)
	: ic::dk_ipc_server("53E04C75-2AB0-4D76-AF12-84F9C80254AA")
	, _rtsp_server(nullptr)
	, _is_run(false)
{
	add_command(new ic::get_years_req_cmd(this));
	add_command(new ic::get_months_req_cmd(this));
	add_command(new ic::get_days_req_cmd(this));
	add_command(new ic::get_hours_req_cmd(this));
	add_command(new ic::get_minutes_req_cmd(this));
	add_command(new ic::get_seconds_req_cmd(this));

	_rtsp_server = new dk_vod_rtsp_server();
	memset(_config_path, 0x00, sizeof(_config_path));
}

dk_streamer_service::~dk_streamer_service(void)
{
	stop_streaming();
	if (_rtsp_server)
		delete _rtsp_server;
	_rtsp_server = nullptr;
}

dk_streamer_service & dk_streamer_service::instance(void)
{
	static dk_streamer_service _instance;
	return _instance;
}

bool dk_streamer_service::start_streaming(void)
{
	if (_is_run)
	{
		stop_streaming();
		_is_run = false;
	}

	std::string config_path = retrieve_config_path();
	if (config_path.size() < 1)
		return false;
	if (::GetFileAttributesA(config_path.c_str()) == INVALID_FILE_ATTRIBUTES)
		return false;

	std::string config_filepath = config_path + "record_streamer.xml";
	if (::GetFileAttributesA(config_filepath.c_str()) == INVALID_FILE_ATTRIBUTES)
		return false;

	TiXmlDocument document;
	document.LoadFile(config_filepath.c_str());

	TiXmlElement * root_elem = document.FirstChildElement("streamer");
	if (!root_elem)
		return false;

	TiXmlElement * control_elem = root_elem->FirstChildElement("control_server");
	if (!control_elem)
		return false;

	const char * str_control_port_number = nullptr;
	int32_t control_port_number = 15000;
	const char * control_username = nullptr;
	const char * control_password = nullptr;

	TiXmlElement * control_port_number_elem = control_elem->FirstChildElement("port_number");
	if (control_port_number_elem)
	{
		str_control_port_number = control_port_number_elem->GetText();
		if (str_control_port_number && strlen(str_control_port_number) > 0)
			control_port_number = atoi(str_control_port_number);
	}

	TiXmlElement * control_username_elem = control_elem->FirstChildElement("username");
	if (control_username_elem)
		control_username = control_username_elem->GetText();

	TiXmlElement * control_password_elem = control_elem->FirstChildElement("password");
	if (control_password_elem)
		control_password = control_password_elem->GetText();


	TiXmlElement * rtsp_elem = root_elem->FirstChildElement("rtsp_server");
	if (!rtsp_elem)
		return false;

	const char * str_rtsp_port_number = nullptr;
	int32_t rtsp_port_number = 554;
	const char * rtsp_username = nullptr;
	const char * rtsp_password = nullptr;

	TiXmlElement * rtsp_port_number_elem = rtsp_elem->FirstChildElement("port_number");
	if (rtsp_port_number_elem)
	{
		str_rtsp_port_number = rtsp_port_number_elem->GetText();
		if (str_rtsp_port_number && strlen(str_rtsp_port_number) > 0)
			rtsp_port_number = atoi(str_rtsp_port_number);
	}

	TiXmlElement * rtsp_username_elem = rtsp_elem->FirstChildElement("username");
	if (rtsp_username_elem)
		rtsp_username = rtsp_username_elem->GetText();

	TiXmlElement * rtsp_password_elem = rtsp_elem->FirstChildElement("password");
	if (rtsp_password_elem)
		rtsp_password = rtsp_password_elem->GetText();


	TiXmlElement * media_sources_elem = root_elem->FirstChildElement("media_sources");
	if (!media_sources_elem)
		return false;

	TiXmlElement * media_source_elem = media_sources_elem->FirstChildElement("media_source");
	while (media_source_elem)
	{
		TiXmlElement * url_elem = media_source_elem->FirstChildElement("url");
		TiXmlElement * username_elem = media_source_elem->FirstChildElement("username");
		TiXmlElement * password_elem = media_source_elem->FirstChildElement("password");

		const char * uuid = media_source_elem->Attribute("uuid");
		uuid = ::CharUpperA((char*)uuid);
		const char * url = url_elem->GetText();
		const char * username = username_elem->GetText();
		const char * password = password_elem->GetText();


		media_source_elem = media_source_elem->NextSiblingElement();
	}

	start(nullptr, control_port_number);
	dk_log4cplus_logger::instance().make_system_info_log("parallel.record.streamer", "start control server[port number=%d]", control_port_number);
	if (rtsp_username && strlen(rtsp_username)>0 && rtsp_password && strlen(rtsp_password)>0)
	{
		if (_rtsp_server)
		{
			_rtsp_server->start(rtsp_port_number, (char*)rtsp_username, (char*)rtsp_password);
			dk_log4cplus_logger::instance().make_system_info_log("parallel.record.streamer", "start rtsp server[port number=%d]", rtsp_port_number);
		}
	}
	else
	{
		if (_rtsp_server)
		{
			_rtsp_server->start(rtsp_port_number, nullptr, nullptr);
			dk_log4cplus_logger::instance().make_system_info_log("parallel.record.streamer", "start rtsp server[port number=%d]", rtsp_port_number);
		}
	}

	_is_run = true;
	return true;
}

bool dk_streamer_service::stop_streaming(void)
{
	if (_is_run)
	{
		if (_rtsp_server)
		{
			dk_log4cplus_logger::instance().make_system_info_log("parallel.record.streamer", "stop rtsp server");
			_rtsp_server->stop();
		}
		dk_log4cplus_logger::instance().make_system_info_log("parallel.record.streamer", "stop control server");
		stop();
		_is_run = false;
		return true;
	}
	else
	{
		return false;
	}
}

const char * dk_streamer_service::retrieve_config_path(void)
{
	char * module_path = nullptr;
	dk_misc_helper::retrieve_absolute_module_path("ParallelRecordStreamer.exe", &module_path);
	if (module_path && strlen(module_path)>0)
	{
		_snprintf_s(_config_path, sizeof(_config_path), "%s%s", module_path, "config\\");
		free(module_path);
	}
	return _config_path;
}

void dk_streamer_service::assoc_completion_callback(const char * uuid)
{

}

void dk_streamer_service::leave_completion_callback(const char * uuid)
{

}