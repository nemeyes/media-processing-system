#include "dk_recorder_service.h"
#include <dk_time_helper.h>
#include <dk_misc_helper.h>
#include "dk_rtsp_recorder.h"
#include <tinyxml.h>

dk_recorder_service::_recorder_receiver_information_t::_recorder_receiver_information_t(void)
{
	memset(uuid, 0x00, sizeof(uuid));
	memset(url, 0x00, sizeof(url));
	memset(username, 0x00, sizeof(username));
	memset(password, 0x00, sizeof(password));
	recorder = nullptr;
}

dk_recorder_service::_recorder_receiver_information_t::_recorder_receiver_information_t(const dk_recorder_service::_recorder_receiver_information_t & clone)
{
	strncpy_s(uuid, clone.uuid, sizeof(uuid));
	strncpy_s(url, clone.url, sizeof(url));
	strncpy_s(username, clone.username, sizeof(username));
	strncpy_s(password, clone.password, sizeof(password));
	recorder = clone.recorder;
}

dk_recorder_service::_recorder_receiver_information_t dk_recorder_service::_recorder_receiver_information_t::operator=(const dk_recorder_service::_recorder_receiver_information_t & clone)
{
	strncpy_s(uuid, clone.uuid, sizeof(uuid));
	strncpy_s(url, clone.url, sizeof(url));
	strncpy_s(username, clone.username, sizeof(username));
	strncpy_s(password, clone.password, sizeof(password));
	recorder = clone.recorder;
	return (*this);
}

dk_recorder_service::dk_recorder_service(void)
{
	memset(_storage_path, 0x00, sizeof(_storage_path));
	memset(_config_path, 0x00, sizeof(_config_path));
}

dk_recorder_service::~dk_recorder_service(void)
{
	stop_recording();
}

dk_recorder_service & dk_recorder_service::instance(void)
{
	static dk_recorder_service _instance;
	return _instance;
}

bool dk_recorder_service::start_recording(void)
{
	std::string config_path = retrieve_config_path();
	if (config_path.size() < 1)
		return false;
	if (::GetFileAttributesA(config_path.c_str()) == INVALID_FILE_ATTRIBUTES)
		return false;

	std::string config_filepath = config_path + "mediasources.xml";
	if (::GetFileAttributesA(config_filepath.c_str()) == INVALID_FILE_ATTRIBUTES)
		return false;

	std::string storage_path = retrieve_storage_path();
	if (::GetFileAttributesA(storage_path.c_str()) == INVALID_FILE_ATTRIBUTES)
		CreateDirectoryA(storage_path.c_str(), NULL);

	TiXmlDocument document;
	document.LoadFile(config_filepath.c_str());
	TiXmlElement * root_elem = document.FirstChildElement("mediasources");
	if (!root_elem)
		return false;
	TiXmlElement * sub_elem = root_elem->FirstChildElement("mediasource");
	while (sub_elem)
	{
		TiXmlElement * url_elem = sub_elem->FirstChildElement("url");
		TiXmlElement * username_elem = sub_elem->FirstChildElement("username");
		TiXmlElement * password_elem = sub_elem->FirstChildElement("password");

		const char * uuid = sub_elem->Attribute("uuid");
		uuid = ::CharUpperA((char*)uuid);
		const char * url = url_elem->GetText();
		const char * username = username_elem->GetText();
		const char * password = password_elem->GetText();


		dk_recorder_service::recorder_receiver_information_t recv_info;
		strncpy_s(recv_info.uuid, uuid, sizeof(recv_info.uuid));
		strncpy_s(recv_info.url, url, sizeof(recv_info.url));
		if (username && strlen(username)>0)
			strncpy_s(recv_info.username, username, sizeof(recv_info.username));
		if (password && strlen(password)>0)
			strncpy_s(recv_info.password, password, sizeof(recv_info.password));
		recv_info.recorder = new dk_rtsp_recorder();
		recv_info.recorder->start_recording(recv_info.url, recv_info.username, recv_info.password, dk_rtsp_recorder::rtp_over_tcp, dk_rtsp_recorder::recv_video, storage_path.c_str(), recv_info.uuid);
		_receivers.push_back(recv_info);

		sub_elem = sub_elem->NextSiblingElement();
	}

	/*
	//camera 2
	{
		dk_recorder_service::recorder_receiver_information_t recv_info;
		std::string uuid = dk_misc_helper::generate_guid();

		strncpy_s(recv_info.uuid, uuid.c_str(), sizeof(recv_info.uuid));
		strncpy_s(recv_info.url, "rtsp://now.iptime.org/1/stream1", sizeof(recv_info.url));
		strncpy_s(recv_info.username, "", sizeof(recv_info.username));
		strncpy_s(recv_info.password, "", sizeof(recv_info.password));
		recv_info.recorder = new dk_rtsp_recorder();
		recv_info.recorder->start_recording(recv_info.url, recv_info.username, recv_info.password, dk_rtsp_recorder::rtp_over_tcp, dk_rtsp_recorder::recv_video, storage_path.c_str(), recv_info.uuid);
		_receivers.push_back(recv_info);
	}

	//camera 3
	{
		dk_recorder_service::recorder_receiver_information_t recv_info;
		std::string uuid = dk_misc_helper::generate_guid();

		strncpy_s(recv_info.uuid, uuid.c_str(), sizeof(recv_info.uuid));
		strncpy_s(recv_info.url, "rtsp://now.iptime.org/1/stream1", sizeof(recv_info.url));
		strncpy_s(recv_info.username, "", sizeof(recv_info.username));
		strncpy_s(recv_info.password, "", sizeof(recv_info.password));
		recv_info.recorder = new dk_rtsp_recorder();
		recv_info.recorder->start_recording(recv_info.url, recv_info.username, recv_info.password, dk_rtsp_recorder::rtp_over_tcp, dk_rtsp_recorder::recv_video, storage_path.c_str(), recv_info.uuid);
		_receivers.push_back(recv_info);
	}
	*/
	return true;
}

bool dk_recorder_service::stop_recording(void)
{
	std::vector<dk_recorder_service::recorder_receiver_information_t>::iterator iter;
	for (iter = _receivers.begin(); iter != _receivers.end(); iter++)
	{
		dk_rtsp_recorder * recorder = (iter)->recorder;
		recorder->stop_recording();
		delete recorder;
		(iter)->recorder = nullptr;
	}
	_receivers.clear();
	return true;
}

const char * dk_recorder_service::retrieve_storage_path(void)
{
#if 1
	//std::string module_path = dk_misc_helper::retrieve_absolute_module_path("ParallelRecordServer.exe");
	char * module_path = nullptr;
	dk_misc_helper::retrieve_absolute_module_path("ParallelRecordServer.exe", &module_path);
	if(module_path && strlen(module_path)>0)
	{
		_snprintf_s(_storage_path, sizeof(_storage_path), "%s%s", module_path, "storage\\");
		free(module_path);
	}
#else
	HINSTANCE module_handle = ::GetModuleHandleA("ParallelRecordServer.exe");
	char module_path[260] = { 0 };
	char * module_name = module_path;
	module_name += GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
	if (module_name != module_path)
	{
		CHAR *slash = strrchr(module_path, '\\');
		if (slash != NULL)
		{
			module_name = slash + 1;
			_strset_s(module_name, sizeof(module_path), 0);
		}
		else
		{
			_strset_s(module_path, sizeof(module_path), 0);
		}
	}
	_snprintf_s(_storage_path, sizeof(_storage_path), "%s%s", module_path, "storage\\");
#endif
	return _storage_path;
}

const char * dk_recorder_service::retrieve_config_path(void)
{
#if 1
	char * module_path = nullptr;
	dk_misc_helper::retrieve_absolute_module_path("ParallelRecordServer.exe", &module_path);
	if (module_path && strlen(module_path)>0)
	{
		_snprintf_s(_config_path, sizeof(_config_path), "%s%s", module_path, "config\\");
		free(module_path);
	}
#else
	HINSTANCE module_handle = ::GetModuleHandleA("ParallelRecordServer.exe");
	char module_path[260] = { 0 };
	char * module_name = module_path;
	module_name += GetModuleFileNameA(module_handle, module_name, (sizeof(module_path) / sizeof(*module_path)) - (module_name - module_path));
	if (module_name != module_path)
	{
		char * slash = strrchr(module_path, '\\');
		if (slash != NULL)
		{
			module_name = slash + 1;
			_strset_s(module_name, sizeof(module_path), 0);
		}
		else
		{
			_strset_s(module_path, sizeof(module_path), 0);
		}
	}
	_snprintf_s(_config_path, sizeof(_config_path), "%s%s", module_path, "config\\");
#endif
	return _config_path;
}

/*
void dk_recorder_service::retrieve_receivers(std::vector<dk_recorder_service::recorder_receiver_information_t> * receivers)
{

	//camera 1
	{
		dk_recorder_service::recorder_receiver_information_t receiver_info;
		std::string uuid = dk_misc_helper::generate_guid();

		strncpy_s(receiver_info.uuid, uuid.c_str(), sizeof(receiver_info.uuid));
		strncpy_s(receiver_info.url, "rtsp://now.iptime.org/1/stream1", sizeof(receiver_info.url));
		strncpy_s(receiver_info.username, "", sizeof(receiver_info.username));
		strncpy_s(receiver_info.password, "", sizeof(receiver_info.password));
		receivers->push_back(receiver_info);
	}

	//camera 2
	{
		dk_recorder_service::recorder_receiver_information_t receiver_info;
		std::string uuid = dk_misc_helper::generate_guid();

		strncpy_s(receiver_info.uuid, uuid.c_str(), sizeof(receiver_info.uuid));
		strncpy_s(receiver_info.url, "rtsp://now.iptime.org/1/stream1", sizeof(receiver_info.url));
		strncpy_s(receiver_info.username, "", sizeof(receiver_info.username));
		strncpy_s(receiver_info.password, "", sizeof(receiver_info.password));
		receivers->push_back(receiver_info);
	}

	//camera 3
	{
		dk_recorder_service::recorder_receiver_information_t receiver_info;
		std::string uuid = dk_misc_helper::generate_guid();

		strncpy_s(receiver_info.uuid, uuid.c_str(), sizeof(receiver_info.uuid));
		strncpy_s(receiver_info.url, "rtsp://now.iptime.org/1/stream1", sizeof(receiver_info.url));
		strncpy_s(receiver_info.username, "", sizeof(receiver_info.username));
		strncpy_s(receiver_info.password, "", sizeof(receiver_info.password));
		receivers->push_back(receiver_info);
	}
}
*/