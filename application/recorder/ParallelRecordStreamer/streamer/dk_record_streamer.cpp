#include "dk_record_streamer.h"
#include <dk_time_helper.h>
#include <dk_misc_helper.h>
#include <tinyxml.h>
#include <boost/date_time/local_time/local_time.hpp>
#include <dk_log4cplus_logger.h>
#include <dk_recorder_module.h>

#if defined(WITH_RTSP_SERVER)
#include <dk_vod_rtsp_server.h>
#endif

#include "commands_server.h"

bool compare_object(long long first, long long second)
{
	return first > second;
}

debuggerking::record_streamer::record_streamer(void)
	: ic::dk_ipc_server("53E04C75-2AB0-4D76-AF12-84F9C80254AA")
	, _run(false)
#if defined(WITH_RTSP_SERVER)
	, _rtsp_server_port_number(554)
	, _rtsp_server(nullptr)
#endif
{
#if defined(WITH_RTSP_SERVER)
	add_command(new ic::get_rtsp_server_port_req_cmd(this));
#else
	add_command(new ic::begin_playback_req_cmd(this));
	add_command(new ic::end_playback_req_cmd(this));
#endif
	add_command(new ic::get_years_req_cmd(this));
	add_command(new ic::get_months_req_cmd(this));
	add_command(new ic::get_days_req_cmd(this));
	add_command(new ic::get_hours_req_cmd(this));
	add_command(new ic::get_minutes_req_cmd(this));
	add_command(new ic::get_seconds_req_cmd(this));

#if defined(WITH_RTSP_SERVER)
	_rtsp_server = new vod_rtsp_server();
#endif
	log4cplus_logger::create("config\\log.properties");
	memset(_config_path, 0x00, sizeof(_config_path));
}

debuggerking::record_streamer::~record_streamer(void)
{
	stop();
#if defined(WITH_RTSP_SERVER)
	if (_rtsp_server)
		delete _rtsp_server;
	_rtsp_server = nullptr;
#endif
	log4cplus_logger::destroy();
}


bool debuggerking::record_streamer::start(void)
{
	if (_run)
	{
		stop();
		_run = false;
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


#if defined(WITH_RTSP_SERVER)
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
#endif

	TiXmlElement * media_sources_elem = root_elem->FirstChildElement("media_sources");
	if (media_sources_elem)
	{
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
	}

	ic::dk_ipc_server::start(nullptr, control_port_number);
	log4cplus_logger::make_info_log("parallel.record.streamer", "start control server[port number=%d]", control_port_number);

#if defined(WITH_RTSP_SERVER)
	if (rtsp_username && strlen(rtsp_username)>0 && rtsp_password && strlen(rtsp_password)>0)
	{
		if (_rtsp_server)
		{
			_rtsp_server->start(rtsp_port_number, (char*)rtsp_username, (char*)rtsp_password);
			log4cplus_logger::make_info_log("parallel.record.streamer", "start rtsp server[port number=%d]", rtsp_port_number);
			_rtsp_server_port_number = rtsp_port_number;
		}
	}
	else
	{
		if (_rtsp_server)
		{
			_rtsp_server->start(rtsp_port_number, nullptr, nullptr);
			log4cplus_logger::make_info_log("parallel.record.streamer", "start rtsp server[port number=%d]", rtsp_port_number);
			_rtsp_server_port_number = rtsp_port_number;
		}
	}
#endif
	_run = true;
	return true;
}

bool debuggerking::record_streamer::stop(void)
{
	if (_run)
	{
#if defined(WITH_RTSP_SERVER)
		if (_rtsp_server)
		{
			log4cplus_logger::make_info_log("parallel.record.streamer", "stop rtsp server");
			_rtsp_server->stop();
		}
#endif
		log4cplus_logger::make_info_log("parallel.record.streamer", "stop control server");
		ic::dk_ipc_server::stop();
		_run = false;
		return true;
	}
	else
	{
		return false;
	}
}

const char * debuggerking::record_streamer::retrieve_contents_path(void)
{
	char * module_path = nullptr;
	debuggerking::misc_helper::retrieve_absolute_module_path("ParallelRecordServer.exe", &module_path);
	if (module_path && strlen(module_path)>0)
	{
		_snprintf_s(_contents_path, sizeof(_contents_path), "%s%s", module_path, "contents\\");
		free(module_path);
	}
	return _contents_path;
}

const char * debuggerking::record_streamer::retrieve_config_path(void)
{
	char * module_path = nullptr;
	debuggerking::misc_helper::retrieve_absolute_module_path("ParallelRecordStreamer.exe", &module_path);
	if (module_path && strlen(module_path)>0)
	{
		_snprintf_s(_config_path, sizeof(_config_path), "%s%s", module_path, "config\\");
		free(module_path);
	}
	return _config_path;
}

#if defined(WITH_RTSP_SERVER)
int32_t debuggerking::record_streamer::get_rtsp_server_port_number(void)
{
	return _rtsp_server_port_number;
}
#endif

void debuggerking::record_streamer::assoc_completion_callback(const char * uuid)
{

}

void debuggerking::record_streamer::leave_completion_callback(const char * uuid)
{

}

void debuggerking::record_streamer::get_years(const char * uuid, int years[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = retrieve_contents_path();
#if defined(WITH_RTSP_SERVER)
	char single_media_source_path[260] = { 0 };
	char single_media_source_search_path[260] = { 0 };
	_snprintf_s(single_media_source_path, sizeof(single_media_source_path), "%s%s\\", contents, uuid);
	_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s%s\\*", contents, uuid);

	std::vector<int> year_vector;
	std::vector<int>::iterator iter;
	int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
	if (bfind != INVALID_HANDLE_VALUE)
	{
		char recorded_filename[260] = { 0 };
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strncpy_s(recorded_filename, &wfd.cFileName[0], sizeof(recorded_filename));
				if (strcmp(recorded_filename, "INDEX"))
				{
					char * dot = (char*)strrchr(recorded_filename, '.');
					_strset_s(dot, strlen(dot) + 1, 0x00);

					long long timestamp = atoll(recorded_filename);
#if defined(WITH_MILLISECOND)
					recorder_module::get_time_from_elapsed_millisec_from_epoch(timestamp, year, month, day, hour, minute, second);
#else
					recorder_module::get_time_from_elapsed_microsec_from_epoch(timestamp, year, month, day, hour, minute, second);
#endif
					iter = std::find(year_vector.begin(), year_vector.end(), year);
					if (iter == year_vector.end())
					{
						year_vector.push_back(year);
					}					
				}
			}
		} while (::FindNextFileA(bfind, &wfd));
	}
	::FindClose(bfind);

	size = year_vector.size();
	int count = size > capacity ? capacity : size;
	for (int index = 0; index < count; index++)
	{
		years[index] = year_vector[index];
	}
#else
	dk_recorder_module::get_years(contents, uuid, years, capacity, size);
#endif
}

void debuggerking::record_streamer::get_months(const char * uuid, int year, int months[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = retrieve_contents_path();
#if defined(WITH_RTSP_SERVER)
	char single_media_source_path[260] = { 0 };
	char single_media_source_search_path[260] = { 0 };
	_snprintf_s(single_media_source_path, sizeof(single_media_source_path), "%s%s\\", contents, uuid);
	_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s%s\\*", contents, uuid);

	std::vector<int> month_vector;
	std::vector<int>::iterator iter;
	int32_t year1 = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
	if (bfind != INVALID_HANDLE_VALUE)
	{
		char recorded_filename[260] = { 0 };
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strncpy_s(recorded_filename, &wfd.cFileName[0], sizeof(recorded_filename));
				if (strcmp(recorded_filename, "INDEX"))
				{
					char * dot = (char*)strrchr(recorded_filename, '.');
					_strset_s(dot, strlen(dot) + 1, 0x00);

					long long timestamp = atoll(recorded_filename);
#if defined(WITH_MILLISECOND)
					recorder_module::get_time_from_elapsed_millisec_from_epoch(timestamp, year1, month, day, hour, minute, second);
#else
					recorder_module::get_time_from_elapsed_microsec_from_epoch(timestamp, year1, month, day, hour, minute, second);
#endif
					if (year == year1)
					{
						iter = std::find(month_vector.begin(), month_vector.end(), month);
						if (iter == month_vector.end())
						{
							month_vector.push_back(month);
						}

					}
				}
			}
		} while (::FindNextFileA(bfind, &wfd));
	}
	::FindClose(bfind);

	size = month_vector.size();
	int count = size > capacity ? capacity : size;
	for (int index = 0; index < count; index++)
	{
		months[index] = month_vector[index];
	}
#else
	dk_recorder_module::get_months(contents, uuid, year, months, capacity, size);
#endif
}

void debuggerking::record_streamer::get_days(const char * uuid, int year, int month, int days[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = retrieve_contents_path();
#if defined(WITH_RTSP_SERVER)
	char single_media_source_path[260] = { 0 };
	char single_media_source_search_path[260] = { 0 };
	_snprintf_s(single_media_source_path, sizeof(single_media_source_path), "%s%s\\", contents, uuid);
	_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s%s\\*", contents, uuid);

	std::vector<int> day_vector;
	std::vector<int>::iterator iter;
	int32_t year1 = 0, month1 = 0, day = 0, hour = 0, minute = 0, second = 0;

	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
	if (bfind != INVALID_HANDLE_VALUE)
	{
		char recorded_filename[260] = { 0 };
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strncpy_s(recorded_filename, &wfd.cFileName[0], sizeof(recorded_filename));
				if (strcmp(recorded_filename, "INDEX"))
				{
					char * dot = (char*)strrchr(recorded_filename, '.');
					_strset_s(dot, strlen(dot) + 1, 0x00);

					long long timestamp = atoll(recorded_filename);
#if defined(WITH_MILLISECOND)
					recorder_module::get_time_from_elapsed_millisec_from_epoch(timestamp, year1, month1, day, hour, minute, second);
#else
					recorder_module::get_time_from_elapsed_microsec_from_epoch(timestamp, year1, month1, day, hour, minute, second);
#endif
					if (year == year1 && month == month1)
					{
						iter = std::find(day_vector.begin(), day_vector.end(), day);
						if (iter == day_vector.end())
						{
							day_vector.push_back(day);
						}
					}
				}
			}
		} while (::FindNextFileA(bfind, &wfd));
	}
	::FindClose(bfind);

	size = day_vector.size();
	int count = size > capacity ? capacity : size;
	for (int index = 0; index < count; index++)
	{
		days[index] = day_vector[index];
	}
#else
	dk_recorder_module::get_days(contents, uuid, year, month, days, capacity, size);
#endif
}

void debuggerking::record_streamer::get_hours(const char * uuid, int year, int month, int day, int hours[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = retrieve_contents_path();
#if defined(WITH_RTSP_SERVER)
	char single_media_source_path[260] = { 0 };
	char single_media_source_search_path[260] = { 0 };
	_snprintf_s(single_media_source_path, sizeof(single_media_source_path), "%s%s\\", contents, uuid);
	_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s%s\\*", contents, uuid);

	std::vector<int> hour_vector;
	std::vector<int>::iterator iter;
	int32_t year1 = 0, month1 = 0, day1 = 0, hour = 0, minute = 0, second = 0;

	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
	if (bfind != INVALID_HANDLE_VALUE)
	{
		char recorded_filename[260] = { 0 };
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strncpy_s(recorded_filename, &wfd.cFileName[0], sizeof(recorded_filename));
				if (strcmp(recorded_filename, "INDEX"))
				{
					char * dot = (char*)strrchr(recorded_filename, '.');
					_strset_s(dot, strlen(dot) + 1, 0x00);

					long long timestamp = atoll(recorded_filename);
#if defined(WITH_MILLISECOND)
					recorder_module::get_time_from_elapsed_millisec_from_epoch(timestamp, year1, month1, day1, hour, minute, second);
#else
					recorder_module::get_time_from_elapsed_microsec_from_epoch(timestamp, year1, month1, day1, hour, minute, second);
#endif
					if (year == year1 && month == month1 && day==day1)
					{
						iter = std::find(hour_vector.begin(), hour_vector.end(), hour);
						if (iter == hour_vector.end())
						{
							hour_vector.push_back(hour);
						}
					}
				}
			}
		} while (::FindNextFileA(bfind, &wfd));
	}
	::FindClose(bfind);

	size = hour_vector.size();
	int count = size > capacity ? capacity : size;
	for (int index = 0; index < count; index++)
	{
		hours[index] = hour_vector[index];
	}
#else
	dk_recorder_module::get_hours(contents, uuid, year, month, day, hours, capacity, size);
#endif
}

void debuggerking::record_streamer::get_minutes(const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = retrieve_contents_path();
#if defined(WITH_RTSP_SERVER)
	char single_media_source_path[260] = { 0 };
	char single_media_source_search_path[260] = { 0 };
	_snprintf_s(single_media_source_path, sizeof(single_media_source_path), "%s%s\\", contents, uuid);
	_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s%s\\*", contents, uuid);

	std::vector<long long> gathered_timestamp;
	std::vector<long long>::iterator gathered_timestamp_iter;
	int32_t year1 = 0, month1 = 0, day1 = 0, hour1 = 0, minute = 0, second = 0;

	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
	if (bfind != INVALID_HANDLE_VALUE)
	{
		char recorded_filename[260] = { 0 };
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strncpy_s(recorded_filename, &wfd.cFileName[0], sizeof(recorded_filename));
				if (strcmp(recorded_filename, "INDEX"))
				{
					char * dot = (char*)strrchr(recorded_filename, '.');
					_strset_s(dot, strlen(dot) + 1, 0x00);

					long long timestamp = atoll(recorded_filename);
#if defined(WITH_MILLISECOND)
					recorder_module::get_time_from_elapsed_millisec_from_epoch(timestamp, year1, month1, day1, hour1, minute, second);
#else
					recorder_module::get_time_from_elapsed_microsec_from_epoch(timestamp, year1, month1, day1, hour1, minute, second);
#endif
					if (year == year1 && month == month1 && day == day1 && hour == hour1)
					{
						gathered_timestamp_iter = std::find(gathered_timestamp.begin(), gathered_timestamp.end(), timestamp);
						if (gathered_timestamp_iter == gathered_timestamp.end())
						{
							gathered_timestamp.push_back(timestamp);
						}
					}
				}
			}
		} while (::FindNextFileA(bfind, &wfd));
	}
	::FindClose(bfind);


	char single_media_source_file_path[260] = { 0 };
	std::vector<int> minute_vector;
	std::vector<int>::iterator iter;
	for (gathered_timestamp_iter = gathered_timestamp.begin(); gathered_timestamp_iter != gathered_timestamp.end(); gathered_timestamp_iter++)
	{
		_snprintf_s(single_media_source_file_path, sizeof(single_media_source_file_path), "%s%s\\%lld.dat", contents, uuid, *gathered_timestamp_iter);

		HANDLE file = ::CreateFileA(single_media_source_file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file != NULL && file != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER lifilesize = { 0 };
			::GetFileSizeEx(file, &lifilesize);
			long long filesize = 0;
			filesize = lifilesize.HighPart;
			filesize <<= 32;
			filesize |= lifilesize.LowPart;

			uint32_t seek_index = 2 * sizeof(long long); //skip first 16 byte

			void * buf = nullptr;
			unsigned long bytes_to_read = 0;
			unsigned long bytes_read = 0;

			uint32_t prev_idr_index = 0;
			long long prev_idr_timestamp = 0;

			uint32_t next_idr_index = 0;
			long long next_idr_timestamp = 0;


			do
			{
				if (seek_index >= filesize)
					break;

				::SetFilePointer(file, seek_index, NULL, FILE_BEGIN);

				uint8_t nalu_type = 0;
				long long nalu_timestamp = 0;
				uint32_t nalu_size = 0;

				//read frame type
				buf = (void*)&nalu_type;
				bytes_to_read = sizeof(uint8_t);
				bytes_read = 0;
				::ReadFile(file, buf, bytes_to_read, &bytes_read, NULL);
				seek_index += bytes_read;

				//read timestramp
				buf = (void*)&nalu_timestamp;
				bytes_to_read = sizeof(long long);
				bytes_read = 0;
				::ReadFile(file, buf, bytes_to_read, &bytes_read, NULL);
				seek_index += bytes_read;

				//read nalu size
				buf = (void*)&nalu_size;
				bytes_to_read = sizeof(uint32_t);
				bytes_read = 0;
				::ReadFile(file, buf, bytes_to_read, &bytes_read, NULL);
				seek_index += bytes_read;
				seek_index += nalu_size;

				if (nalu_type == 0) //sps
				{
#if defined(WITH_MILLISECOND)
					recorder_module::get_time_from_elapsed_millisec_from_epoch(nalu_timestamp, year1, month1, day1, hour1, minute, second);
#else
					recorder_module::get_time_from_elapsed_microsec_from_epoch(nalu_timestamp, year1, month1, day1, hour1, minute, second);
#endif
					if (year == year1 && month == month1 && day == day1 && hour == hour1)
					{
						iter = std::find(minute_vector.begin(), minute_vector.end(), minute);
						if (iter == minute_vector.end())
						{
							minute_vector.push_back(minute);
						}
					}

				}
			} while (1);
			::CloseHandle(file);
		}
	}

	size = minute_vector.size();
	int count = size > capacity ? capacity : size;
	for (int index = 0; index < count; index++)
	{
		minutes[index] = minute_vector[index];
	}
#else
	dk_recorder_module::get_minutes(contents, uuid, year, month, day, hour, minutes, capacity, size);
#endif
}

void debuggerking::record_streamer::get_seconds(const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = retrieve_contents_path();
#if defined(WITH_RTSP_SERVER)
	char single_media_source_path[260] = { 0 };
	char single_media_source_search_path[260] = { 0 };
	_snprintf_s(single_media_source_path, sizeof(single_media_source_path), "%s%s\\", contents, uuid);
	_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s%s\\*", contents, uuid);

	std::vector<long long> gathered_timestamp;
	std::vector<long long>::iterator gathered_timestamp_iter;
	int32_t year1 = 0, month1 = 0, day1 = 0, hour1 = 0, minute1 = 0, second = 0;

	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
	if (bfind != INVALID_HANDLE_VALUE)
	{
		char recorded_filename[260] = { 0 };
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strncpy_s(recorded_filename, &wfd.cFileName[0], sizeof(recorded_filename));
				if (strcmp(recorded_filename, "INDEX"))
				{
					char * dot = (char*)strrchr(recorded_filename, '.');
					_strset_s(dot, strlen(dot) + 1, 0x00);

					long long timestamp = atoll(recorded_filename);
#if defined(WITH_MILLISECOND)
					recorder_module::get_time_from_elapsed_millisec_from_epoch(timestamp, year1, month1, day1, hour1, minute1, second);
#else
					recorder_module::get_time_from_elapsed_microsec_from_epoch(timestamp, year1, month1, day1, hour1, minute1, second);
#endif
					if (year == year1 && month == month1 && day == day1 && hour == hour1)
					{
						gathered_timestamp_iter = std::find(gathered_timestamp.begin(), gathered_timestamp.end(), timestamp);
						if (gathered_timestamp_iter == gathered_timestamp.end())
						{
							gathered_timestamp.push_back(timestamp);
						}
					}
				}
			}
		} while (::FindNextFileA(bfind, &wfd));
	}
	::FindClose(bfind);


	char single_media_source_file_path[260] = { 0 };
	std::vector<int> second_vector;
	std::vector<int>::iterator iter;
	for (gathered_timestamp_iter = gathered_timestamp.begin(); gathered_timestamp_iter != gathered_timestamp.end(); gathered_timestamp_iter++)
	{
		_snprintf_s(single_media_source_file_path, sizeof(single_media_source_file_path), "%s%s\\%lld.dat", contents, uuid, *gathered_timestamp_iter);

		HANDLE file = ::CreateFileA(single_media_source_file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file != NULL && file != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER lifilesize = { 0 };
			::GetFileSizeEx(file, &lifilesize);
			long long filesize = 0;
			filesize = lifilesize.HighPart;
			filesize <<= 32;
			filesize |= lifilesize.LowPart;

			uint32_t seek_index = 2 * sizeof(long long); //skip first 16 byte

			void * buf = nullptr;
			unsigned long bytes_to_read = 0;
			unsigned long bytes_read = 0;

			uint32_t prev_idr_index = 0;
			long long prev_idr_timestamp = 0;

			uint32_t next_idr_index = 0;
			long long next_idr_timestamp = 0;


			do
			{
				if (seek_index >= filesize)
					break;

				::SetFilePointer(file, seek_index, NULL, FILE_BEGIN);

				uint8_t nalu_type = 0;
				long long nalu_timestamp = 0;
				uint32_t nalu_size = 0;

				//read frame type
				buf = (void*)&nalu_type;
				bytes_to_read = sizeof(uint8_t);
				bytes_read = 0;
				::ReadFile(file, buf, bytes_to_read, &bytes_read, NULL);
				seek_index += bytes_read;

				//read timestramp
				buf = (void*)&nalu_timestamp;
				bytes_to_read = sizeof(long long);
				bytes_read = 0;
				::ReadFile(file, buf, bytes_to_read, &bytes_read, NULL);
				seek_index += bytes_read;

				//read nalu size
				buf = (void*)&nalu_size;
				bytes_to_read = sizeof(uint32_t);
				bytes_read = 0;
				::ReadFile(file, buf, bytes_to_read, &bytes_read, NULL);
				seek_index += bytes_read;
				seek_index += nalu_size;

				if (nalu_type == 0) //sps
				{
#if defined(WITH_MILLISECOND)
					recorder_module::get_time_from_elapsed_millisec_from_epoch(nalu_timestamp, year1, month1, day1, hour1, minute1, second);
#else
					recorder_module::get_time_from_elapsed_microsec_from_epoch(nalu_timestamp, year1, month1, day1, hour1, minute1, second);
#endif
					if (year == year1 && month == month1 && day == day1 && hour == hour1 && minute == minute1)
					{
						iter = std::find(second_vector.begin(), second_vector.end(), second);
						if (iter == second_vector.end())
						{
							second_vector.push_back(second);
						}
					}

				}
			} while (1);
			::CloseHandle(file);
		}
	}

	size = second_vector.size();
	int count = size > capacity ? capacity : size;
	for (int index = 0; index < count; index++)
	{
		seconds[index] = second_vector[index];
	}
#else
	dk_recorder_module::get_seconds(contents, uuid, year, month, day, hour, minute, seconds, capacity, size);
#endif
}