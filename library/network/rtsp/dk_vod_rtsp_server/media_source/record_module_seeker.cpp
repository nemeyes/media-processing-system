#include "record_module_seeker.h"
#include <dk_record_module.h>
#include <string>
#include <boost/date_time/local_time/local_time.hpp>
#include <dk_log4cplus_logger.h>

record_module_seeker::record_module_seeker(void)
	: _record_module(nullptr)
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
		}
		else
		{
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

					//get_time_from_elapsed_msec_from_epoch
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