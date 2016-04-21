#include "record_module_seeker.h"
#include <dk_record_module.h>
#include <string>
#include <map>
#include <boost/date_time/local_time/local_time.hpp>
#include <dk_log4cplus_logger.h>

record_module_seeker::record_module_seeker(void)
	: _record_module(nullptr)
	, _last_read_timestamp(0)
{

}

record_module_seeker::~record_module_seeker(void)
{
	if (_record_module)
		delete _record_module;
	_record_module = nullptr;
}

bool record_module_seeker::seek(const char * single_media_source_path, long long seek_time)
{
	if (!single_media_source_path || strlen(single_media_source_path) < 1)
		return false;

	memset(_single_media_source_path, 0x00, sizeof(_single_media_source_path));
	strncpy_s(_single_media_source_path, single_media_source_path, sizeof(_single_media_source_path));

	char single_media_source_search_path[260] = { 0 };
	_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s\\*", _single_media_source_path);

	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
	if (bfind == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char * recorded_file_name = &wfd.cFileName[0];
			char single_ms[260] = { 0 };
			_snprintf_s(single_ms, sizeof(single_ms), "%s\\%s", _single_media_source_path, recorded_file_name);

			long long start_time = 0, end_time = 0;
			_record_module = new dk_record_module(single_ms);
			_record_module->get_start_end_time(start_time, end_time);

			char time_string[100] = { 0 };
			int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
			boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");

			get_time_from_elapsed_msec_from_epoch(start_time, year, month, day, hour, minute, second);
			_snprintf_s(time_string, sizeof(time_string), "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.000", year, month, day, hour, minute, second);
			boost::posix_time::ptime start_ptime = boost::posix_time::time_from_string(time_string);
			boost::posix_time::time_duration elapsed_start_time = start_ptime - epoch;
			start_time = elapsed_start_time.total_milliseconds();

			memset(time_string, 0x00, sizeof(time_string));
			get_time_from_elapsed_msec_from_epoch(end_time, year, month, day, hour, minute, second);
			_snprintf_s(time_string, sizeof(time_string), "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.000", year, month, day, hour, minute, second);
			boost::posix_time::ptime end_ptime = boost::posix_time::time_from_string(time_string);
			boost::posix_time::time_duration elapsed_end_time = end_ptime - epoch;
			end_time = elapsed_end_time.total_milliseconds();

			/*
			char str_seek_time[260] = { 0 };
			char str_start_time[260] = { 0 };
			char str_end_time[260] = { 0 };
			get_time_from_elapsed_msec_from_epoch(seek_time, str_seek_time, sizeof(str_seek_time));
			get_time_from_elapsed_msec_from_epoch(start_time, str_start_time, sizeof(str_start_time));
			get_time_from_elapsed_msec_from_epoch(end_time, str_end_time, sizeof(str_end_time));

			dk_log4cplus_logger::make_debug_log("parallel.record.streamer", "seek[%s], start[%s], end[%s]", str_seek_time, str_start_time, str_end_time);
			*/
			if (seek_time >= start_time && seek_time <= end_time)
			{
				_record_module->seek(seek_time);
				break;
			}

			if (_record_module)
				delete _record_module;
			_record_module = nullptr;
		}
	} while (::FindNextFileA(bfind, &wfd));

	::FindClose(bfind);

	if (_record_module)
		return true;
	else
		return false;
}

void record_module_seeker::read(uint8_t * data, size_t &data_size, long long & timestamp)
{
	data_size = 0;
	timestamp = 0;
	if (_record_module)
	{
		bool read_end = _record_module->is_read_end();
		if (!read_end)
		{
			dk_record_module::nalu_type type;
			_record_module->read(type, data, data_size, timestamp);

			if (data_size>0)
				dk_log4cplus_logger::make_debug_log("parallel.record.streamer", "nalu type : [%s]", type == 0 ? "sps" : (type == 1 ? "pps" : (type == 2 ? "idr" : "vcl")));
			strncpy_s(_last_filename, _record_module->get_filename(), sizeof(_last_filename));
			//_last_read_timestamp = timestamp;
		}
		else
		{
#if 1
			delete _record_module;
			_record_module = nullptr;

			char single_media_source_search_path[260] = { 0 };
			_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s\\*", _single_media_source_path);
			HANDLE bfind = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATAA wfd;
			bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
			if (bfind == INVALID_HANDLE_VALUE)
			{
				return;
			}
			else
			{
				char * dot = (char*)strrchr(_last_filename, '.');
				_strset_s(dot, strlen(dot) + 1, 0x00);
				_last_read_timestamp = atoll(_last_filename);

				char recorded_filename[260] = { 0 };
				std::map<long long, std::string, std::less<long long>> sorted_filenames;
				do
				{
					if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						strncpy_s(recorded_filename, &wfd.cFileName[0], sizeof(recorded_filename));
						if (strcmp(recorded_filename, "INDEX"))
						{
							dot = (char*)strrchr(recorded_filename, '.');
							_strset_s(dot, strlen(dot) + 1, 0x00);

							long long timestamp = atoll(recorded_filename);
							if (timestamp > _last_read_timestamp)
							{
								sorted_filenames.insert(std::make_pair(timestamp - _last_read_timestamp, wfd.cFileName));
							}
						}
					}
				} while (::FindNextFileA(bfind, &wfd));
				
				std::map<long long, std::string, std::less<long long>>::iterator iter = sorted_filenames.begin();
				if (iter != sorted_filenames.end())
				{
					_snprintf_s(recorded_filename, sizeof(recorded_filename), "%s\\%s", _single_media_source_path, iter->second.c_str());
					
					long long start_time = 0, end_time = 0;
					_record_module = new dk_record_module(recorded_filename);
					_record_module->get_start_end_time(start_time, end_time);
					_record_module->seek(start_time);
				}
				else
				{
					return;
				}
			}
#else
			long long prev_end_time = _record_module->get_end_time();
			delete _record_module;
			_record_module = nullptr;

			char single_media_source_search_path[260] = { 0 };
			_snprintf_s(single_media_source_search_path, sizeof(single_media_source_search_path), "%s\\*", _single_media_source_path);

			HANDLE bfind = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATAA wfd;
			bfind = ::FindFirstFileA(single_media_source_search_path, &wfd);
			if (bfind == INVALID_HANDLE_VALUE)
				return;
			do
			{
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					char * recorded_file_name = &wfd.cFileName[0];
					char single_ms[260] = { 0 };
					_snprintf_s(single_ms, sizeof(single_ms), "%s\\%s", _single_media_source_path, recorded_file_name);

					long long start_time = 0, end_time = 0;
					_record_module = new dk_record_module(single_ms);
					_record_module->get_start_end_time(start_time, end_time);


					char str_start_time[260] = { 0 };
					char str_end_time[260] = { 0 };
					get_time_from_elapsed_msec_from_epoch(start_time, str_start_time, sizeof(str_start_time));
					get_time_from_elapsed_msec_from_epoch(end_time, str_end_time, sizeof(str_end_time));
					dk_log4cplus_logger::instance().make_system_debug_log("parallel.record.streamer", "start time[%s], end time[%s]", str_start_time, str_end_time);
					if (prev_end_time >= start_time && prev_end_time <= end_time)
					{
						_record_module->seek(prev_end_time);
						break;
					}

					if (_record_module)
						delete _record_module;
					_record_module = nullptr;
				}
			} while (::FindNextFileA(bfind, &wfd));
			::FindClose(bfind);
#endif
		}
	}
}

uint8_t * record_module_seeker::get_sps(size_t & sps_size)
{
	uint8_t * sps = nullptr;
	sps_size = 0;
	if (_record_module)
	{
		sps = _record_module->get_sps(sps_size);
	}
	return sps;
}

uint8_t * record_module_seeker::get_pps(size_t & pps_size)
{
	uint8_t * pps = nullptr;
	pps_size = 0;
	if (_record_module)
	{
		pps = _record_module->get_pps(pps_size);
	}
	return pps;
}

void record_module_seeker::get_time_from_elapsed_msec_from_epoch(long long elapsed_time, char * time_string, int time_string_size)
{
	boost::posix_time::time_duration elapsed = boost::posix_time::millisec(elapsed_time);
	boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
	boost::posix_time::ptime current_time = epoch + elapsed;

	std::string tmp_time = boost::posix_time::to_simple_string(current_time);
	//strncpy_s(time_string, time_string_size, tmp_time.c_str(), (size_t)time_string_size);
	strcpy_s(time_string, time_string_size, tmp_time.c_str());
}

void record_module_seeker::get_time_from_elapsed_msec_from_epoch(long long elapsed_time, int & year, int & month, int & day, int & hour, int & minute, int & second)
{
	boost::posix_time::time_duration elapsed = boost::posix_time::millisec(elapsed_time);
	boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
	boost::posix_time::ptime current_time = epoch + elapsed;
	year = current_time.date().year();
	month = current_time.date().month();
	day = current_time.date().day();
	hour = current_time.time_of_day().hours();
	minute = current_time.time_of_day().minutes();
	second = current_time.time_of_day().seconds();
}