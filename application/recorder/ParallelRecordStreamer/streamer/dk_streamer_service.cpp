#include "dk_streamer_service.h"
#include <dk_time_helper.h>
#include <dk_misc_helper.h>
#include <tinyxml.h>
#include <dk_vod_rtsp_server.h>

dk_streamer_service::dk_streamer_service(void)
	: _rtsp_server(nullptr)
	, _is_run(false)
{
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

	TiXmlElement * rtsp_elem = root_elem->FirstChildElement("rtsp_server");
	if (!rtsp_elem)
		return false;

	const char * str_port_number = nullptr;
	int32_t port_number = 554;
	const char * username = nullptr;
	const char * password = nullptr;

	TiXmlElement * port_number_elem = rtsp_elem->FirstChildElement("port_number");
	if (port_number_elem)
	{
		str_port_number = port_number_elem->GetText();
		if (str_port_number && strlen(str_port_number) > 0)
			port_number = atoi(str_port_number);
	}

	TiXmlElement * username_elem = rtsp_elem->FirstChildElement("username");
	if (username_elem)
		username = username_elem->GetText();

	TiXmlElement * password_elem = rtsp_elem->FirstChildElement("password");
	if (password_elem)
		password = password_elem->GetText();


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

	if (username && strlen(username)>0 && password && strlen(password)>0)
	{
		if (_rtsp_server)
			_rtsp_server->start(port_number, (char*)username, (char*)password);
	}
	else
	{
		if (_rtsp_server)
			_rtsp_server->start(port_number, nullptr, nullptr);
	}

	_is_run = true;
	return true;
}

bool dk_streamer_service::stop_streaming(void)
{
	if (_is_run)
	{
		if (_rtsp_server)
			_rtsp_server->stop();

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