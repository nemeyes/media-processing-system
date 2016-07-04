#include "recorder_module.h"
#include <boost/date_time/local_time/local_time.hpp>
#include <dk_log4cplus_logger.h>
#include <string>
#include <vector>
#include <map>

#define WBUFFER_SIZE 1024*1024*2

typedef struct _recorder_module_seek_info_t
{
	long long start_time;
	long long end_time;
	char recorder_file[MAX_PATH];
	_recorder_module_seek_info_t(void)
	{}

	_recorder_module_seek_info_t(const _recorder_module_seek_info_t & clone)
	{
		start_time = clone.start_time;
		end_time = clone.end_time;
		strncpy_s(recorder_file, clone.recorder_file, sizeof(recorder_file));
	}

	_recorder_module_seek_info_t & operator=(const _recorder_module_seek_info_t & clone)
	{
		start_time = clone.start_time;
		end_time = clone.end_time;
		strncpy_s(recorder_file, clone.recorder_file, sizeof(recorder_file));
		return (*this);
	}

} recorder_module_seek_info_t;

bool compare_timestamp(long long first, long long second)
{
	return first > second;
}

bool compare_seek_info(recorder_module_seek_info_t first, recorder_module_seek_info_t second)
{
	return first.start_time > second.start_time;
}



/*
dk_recorder_module * _front;
long long _last_read_timestamp;
char _last_accessed_file[MAX_PATH];
char _single_recorder_file_path[MAX_PATH];
*/
debuggerking::module_core::module_core(recorder_module * front, long long chunk_size_bytes)
	: _front(front)
	, _chunk_size_bytes(chunk_size_bytes)
	, _record_file(INVALID_HANDLE_VALUE)
	, _vps_size(0)
	, _sps_size(0)
	, _pps_size(0)
	, _wbuffer(nullptr)
	, _wbuffer_size(0)
	, _wrecv_idr(false)
	, _windex(0)
	, _last_w_timestamp(0)
	, _rindex(0)
	, _last_r_timestamp(0)
{
	memset(_single_recorder_file_path, 0x00, sizeof(_single_recorder_file_path));
	memset(_vps, 0x00, sizeof(_vps));
	memset(_sps, 0x00, sizeof(_sps));
	memset(_sps, 0x00, sizeof(_sps));
	memset(_last_accessed_r_file_name, 0x00, sizeof(_last_accessed_r_file_name));
}

debuggerking::module_core::~module_core(void)
{
	close_recorder_file(_record_file);
	_record_file = INVALID_HANDLE_VALUE;
}

bool debuggerking::module_core::seek4w(const char * single_recorder_file_path, long long timestamp)
{
	if (!single_recorder_file_path || strlen(single_recorder_file_path) < 1)
		return false;

	memset(_single_recorder_file_path, 0x00, sizeof(_single_recorder_file_path));
	strncpy_s(_single_recorder_file_path, single_recorder_file_path, sizeof(_single_recorder_file_path));

	_wbuffer_size = WBUFFER_SIZE;
	if (_wbuffer)
	{
		free(_wbuffer);
		_wbuffer = nullptr;
	}
	_wbuffer = static_cast<uint8_t*>(malloc(_wbuffer_size));
	_wrecv_idr = false;
	_windex = 0;

	bool create_new_one = false;
	///////////////////////////////////////////////////////////////////
	char single_recorder_file_search_path[MAX_PATH] = { 0 };
	_snprintf_s(single_recorder_file_search_path, sizeof(single_recorder_file_search_path), "%s\\*", _single_recorder_file_path);


	if (_record_file && _record_file != INVALID_HANDLE_VALUE)
		::CloseHandle(_record_file);
	_record_file = INVALID_HANDLE_VALUE;
	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_recorder_file_search_path, &wfd);
	if (bfind == INVALID_HANDLE_VALUE)
	{
		create_new_one = true;
	}
	else
	{
		char recorded_filename[MAX_PATH] = { 0 };
		std::vector<long long> sorted_filenames;
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
					sorted_filenames.push_back(timestamp);

				}
			}
		} while (::FindNextFileA(bfind, &wfd));
		std::sort(sorted_filenames.begin(), sorted_filenames.end(), compare_timestamp);
		//get latest file
		if (sorted_filenames.size() > 0)
		{
			long long ts = sorted_filenames[0];
			_snprintf_s(recorded_filename, sizeof(recorded_filename), "%s\\%lld.dat", single_recorder_file_path, ts);
			_record_file = ::CreateFileA(recorded_filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			long long st = 0, et = 0;
			get_start_end_time(_record_file, st, et);
			if (et > st)
			{
				create_new_one = true;
				::CloseHandle(_record_file);
				_record_file = INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			create_new_one = true;
		}
	}

	::FindClose(bfind);
	//////////////////////////////////////////////////////////////////

	if (!create_new_one)
	{
		_windex = recorder_module::get_file_size(_record_file);
	}
	else
	{
		char filepath[MAX_PATH] = { 0 };
		if (::GetFileAttributesA(_single_recorder_file_path) == INVALID_FILE_ATTRIBUTES)
			::CreateDirectoryA(_single_recorder_file_path, NULL);

#if defined(WITH_MILLISECOND)
		if (timestamp == 0)
			timestamp = recorder_module::get_elapsed_millisec_from_epoch();
#else
		if (timestamp == 0)
			timestamp = recorder_module::get_elapsed_microsec_from_epoch();
#endif

		_snprintf_s(filepath, MAX_PATH, "%s\\%lld.dat", _single_recorder_file_path, timestamp);
		_record_file = ::CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		log4cplus_logger::instance()->make_info_log("parallel.record.recorder", "create new recorded file[%s]", filepath);
	}

	if (_record_file == NULL || _record_file == INVALID_HANDLE_VALUE)
		return false;
	else
		return true;
}

bool debuggerking::module_core::seek4r(const char * single_recorder_file_path, long long seek_time)
{
	if (!single_recorder_file_path || strlen(single_recorder_file_path) < 1)
		return false;

	memset(_single_recorder_file_path, 0x00, sizeof(_single_recorder_file_path));
	strncpy_s(_single_recorder_file_path, single_recorder_file_path, sizeof(_single_recorder_file_path));

	char single_recorder_file_search_path[MAX_PATH] = { 0 };
	_snprintf_s(single_recorder_file_search_path, sizeof(single_recorder_file_search_path), "%s\\*", _single_recorder_file_path);

	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_recorder_file_search_path, &wfd);
	if (bfind == INVALID_HANDLE_VALUE)
		return false;

	std::map<std::string, recorder_module_seek_info_t>::iterator seek_iter;
	std::map<std::string, recorder_module_seek_info_t> seek_infos;

	std::vector<recorder_module_seek_info_t>::iterator sorted_seek_iter;
	std::vector<recorder_module_seek_info_t> sorted_seek_infos;
	do
	{
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char * recorded_file_name = &wfd.cFileName[0];
			char single_recorder_file_name[MAX_PATH] = { 0 };
			_snprintf_s(single_recorder_file_name, sizeof(single_recorder_file_name), "%s\\%s", _single_recorder_file_path, recorded_file_name);

			long long start_time = 0, end_time = 0;

			_record_file = open_recorder_file(single_recorder_file_name);
			if (_record_file == INVALID_HANDLE_VALUE)
				continue;

			get_start_end_time(_record_file, start_time, end_time);

			char time_string[100] = { 0 };
			int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
			boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
#if defined(WITH_MILLISECOND)
			recorder_module::get_time_from_elapsed_millisec_from_epoch(start_time, year, month, day, hour, minute, second);
#else
			recorder_module::get_time_from_elapsed_microsec_from_epoch(start_time, year, month, day, hour, minute, second);
#endif
			_snprintf_s(time_string, sizeof(time_string), "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.000", year, month, day, hour, minute, second);
			boost::posix_time::ptime start_ptime = boost::posix_time::time_from_string(time_string);
			boost::posix_time::time_duration elapsed_start_time = start_ptime - epoch;
#if defined(WITH_MILLISECOND)
			start_time = elapsed_start_time.total_milliseconds();
#else
			start_time = elapsed_start_time.total_microseconds();
#endif

			memset(time_string, 0x00, sizeof(time_string));
#if defined(WITH_MILLISECOND)
			recorder_module::get_time_from_elapsed_millisec_from_epoch(end_time, year, month, day, hour, minute, second);
#else
			recorder_module::get_time_from_elapsed_microsec_from_epoch(end_time, year, month, day, hour, minute, second);
#endif
			_snprintf_s(time_string, sizeof(time_string), "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.000", year, month, day, hour, minute, second);
			boost::posix_time::ptime end_ptime = boost::posix_time::time_from_string(time_string);
			boost::posix_time::time_duration elapsed_end_time = end_ptime - epoch;
#if defined(WITH_MILLISECOND)
			end_time = elapsed_end_time.total_milliseconds();
#else
			end_time = elapsed_end_time.total_microseconds();
#endif

			recorder_module_seek_info_t seek_info;
			seek_info.start_time = start_time;
			seek_info.end_time = end_time;
			strncpy_s(seek_info.recorder_file, recorded_file_name, sizeof(seek_info.recorder_file));
			sorted_seek_infos.push_back(seek_info);

			close_recorder_file(_record_file);
			_record_file = INVALID_HANDLE_VALUE;
		}
	} while (::FindNextFileA(bfind, &wfd));
	::FindClose(bfind);
	std::sort(sorted_seek_infos.begin(), sorted_seek_infos.end(), compare_seek_info);

	long long min_start_time = _I64_MAX;
	long long max_end_time = _I64_MIN;
	for (sorted_seek_iter = sorted_seek_infos.begin(); sorted_seek_iter != sorted_seek_infos.end(); sorted_seek_iter++)
	{
		recorder_module_seek_info_t seek_info = (*sorted_seek_iter);
		if (seek_info.start_time <= seek_time && seek_time <= seek_info.end_time)
		{
			char single_recorder_file_name[MAX_PATH] = { 0 };
			_snprintf_s(single_recorder_file_name, sizeof(single_recorder_file_name), "%s\\%s", _single_recorder_file_path, seek_info.recorder_file);

			_record_file = open_recorder_file(single_recorder_file_name);
			if (_record_file != INVALID_HANDLE_VALUE)
			{
				if (seek(_record_file, seek_time))
					break;
				else
				{
					close_recorder_file(_record_file);
					_record_file = INVALID_HANDLE_VALUE;
				}
			}
		}

		if (min_start_time > seek_info.start_time)
			min_start_time = seek_info.start_time;
		if (max_end_time < seek_info.end_time)
			max_end_time = seek_info.end_time;
	}

	if (_record_file == INVALID_HANDLE_VALUE)
	{
		char single_recorder_file_name[MAX_PATH] = { 0 };
		if (seek_time < min_start_time)
		{
			for (sorted_seek_iter = sorted_seek_infos.begin(); sorted_seek_iter != sorted_seek_infos.end(); sorted_seek_iter++)
			{
				recorder_module_seek_info_t seek_info = (*sorted_seek_iter);
				if (seek_info.start_time == min_start_time)
				{
					_snprintf_s(single_recorder_file_name, sizeof(single_recorder_file_name), "%s\\%s", _single_recorder_file_path, seek_info.recorder_file);

					_record_file = open_recorder_file(single_recorder_file_name);
					if (_record_file != INVALID_HANDLE_VALUE)
					{
						if (seek(_record_file, seek_time))
							break;
						else
						{
							close_recorder_file(_record_file);
							_record_file = INVALID_HANDLE_VALUE;
						}
					}
				}
			}
		}
		else if (seek_time > max_end_time)
		{
			for (sorted_seek_iter = sorted_seek_infos.begin(); sorted_seek_iter != sorted_seek_infos.end(); sorted_seek_iter++)
			{
				recorder_module_seek_info_t seek_info = (*sorted_seek_iter);
				if (seek_info.end_time == max_end_time)
				{
					_snprintf_s(single_recorder_file_name, sizeof(single_recorder_file_name), "%s\\%s", _single_recorder_file_path, seek_info.recorder_file);
					_record_file = open_recorder_file(single_recorder_file_name);
					if (_record_file != INVALID_HANDLE_VALUE)
					{
						if (seek(_record_file, seek_time))
							break;
						else
						{
							close_recorder_file(_record_file);
							_record_file = INVALID_HANDLE_VALUE;
						}
					}
				}
			}
		}
		else
		{
			for (sorted_seek_iter = sorted_seek_infos.begin(); sorted_seek_iter != sorted_seek_infos.end(); sorted_seek_iter++)
			{
				recorder_module_seek_info_t seek_info = (*sorted_seek_iter);
				if (seek_info.start_time > seek_time)
				{
					_snprintf_s(single_recorder_file_name, sizeof(single_recorder_file_name), "%s\\%s", _single_recorder_file_path, seek_info.recorder_file);
					_record_file = open_recorder_file(single_recorder_file_name);
					if (_record_file != INVALID_HANDLE_VALUE)
					{
						if (seek(_record_file, seek_time))
							break;
						else
						{
							close_recorder_file(_record_file);
							_record_file = INVALID_HANDLE_VALUE;
						}
					}
				}
			}
		}
	}

	if (_record_file != INVALID_HANDLE_VALUE)
		return true;
	else
		return false;
}

void debuggerking::module_core::write(uint8_t * nalu, size_t nalu_size, long long timestamp)
{
	if (_record_file == NULL || _record_file == INVALID_HANDLE_VALUE)
		return;

#if defined(WITH_MILLISECOND)
	if (timestamp == 0)
		timestamp = recorder_module::get_elapsed_millisec_from_epoch();
#else
	if (timestamp == 0)
		timestamp = recorder_module::get_elapsed_microsec_from_epoch();
#endif

	long long file_size = recorder_module::get_file_size(_record_file);
	if (file_size == 0)
	{
		write_header_time(_record_file, timestamp, -1);
		_windex += 2 * sizeof(long long);//skip file information header
	}

	uint8_t * data = nalu + 4;
	size_t data_size = nalu_size - 4;

	size_t saved_sps_size = 0;
	const uint8_t * saved_sps = get_sps(saved_sps_size);
	size_t saved_pps_size = 0;
	const uint8_t * saved_pps = get_pps(saved_pps_size);

	bool is_sps = recorder_module::is_sps(recorder_module::video_submedia_type_t::h264, data[0] & 0x1F);
	bool is_pps = recorder_module::is_pps(recorder_module::video_submedia_type_t::h264, data[0] & 0x1F);
	bool is_idr = recorder_module::is_idr(recorder_module::video_submedia_type_t::h264, data[0] & 0x1F);
	if (is_sps)
	{
		if (saved_sps_size < 1 || !saved_sps)
		{
			//write_bitstream(dk_record_module::nalu_type_sps, data, data_size, current_time);
			set_sps(data, data_size);
			_wrecv_idr = false;
		}
		else
		{
			if (memcmp(saved_sps, data, saved_sps_size))
			{
				//write_bitstream(dk_record_module::nalu_type_sps, data, data_size, current_time);
				set_sps(data, data_size);
				_wrecv_idr = false;
			}
		}
	}
	else if (is_pps)
	{
		if (saved_pps_size < 1 || !saved_pps)
		{
			//write_bitstream(dk_record_module::nalu_type_pps, data, data_size, current_time);
			set_pps(data, data_size);
			_wrecv_idr = false;
		}
		else
		{
			if (memcmp(saved_pps, data, saved_pps_size))
			{
				//write_bitstream(dk_record_module::nalu_type_pps, data, data_size, current_time);
				set_pps(data, data_size);
				_wrecv_idr = false;
			}
		}
	}
	else if (is_idr)
	{
		if (file_size >= _chunk_size_bytes)
		{
			if (_record_file && _record_file!=INVALID_HANDLE_VALUE)
			{
				write_header_time(_record_file, -1, _last_w_timestamp);
				::CloseHandle(_record_file);
				_record_file = INVALID_HANDLE_VALUE;
			}

			char single_record_file_path[MAX_PATH] = { 0 };
			strncpy_s(single_record_file_path, _single_recorder_file_path, sizeof(single_record_file_path));
			seek4w(single_record_file_path, timestamp);
			long long file_size = recorder_module::get_file_size(_record_file);
			if (file_size == 0)
			{
				write_header_time(_record_file, timestamp, -1);
				_windex += 2 * sizeof(long long);//skip file information header
			}
		}

		if (!_wrecv_idr)
			_wrecv_idr = true;

		saved_sps = get_sps(saved_sps_size);
		saved_pps = get_pps(saved_pps_size);
		if ((saved_sps && saved_sps_size>0) && (saved_pps && saved_pps_size>0))
		{
			write_bitstream(_record_file, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size, timestamp);
			//write_bitstream(dk_record_module::nalu_type_sps, saved_sps, saved_sps_size, timestamp);
			//write_bitstream(dk_record_module::nalu_type_pps, saved_pps, saved_pps_size, timestamp);
		}
		else
		{
			write_bitstream(_record_file, nullptr, 0, nullptr, 0, data, data_size, timestamp);
			//write_bitstream(dk_record_module::nalu_type_idr, data, data_size, timestamp);
		}
	}
	else if (_wrecv_idr)
	{
		write_bitstream(_record_file, nullptr, 0, nullptr, 0, data, data_size, timestamp);
		//write_bitstream(dk_record_module::nalu_type_vcl, data, data_size, timestamp);
	}

	_last_w_timestamp = timestamp;
}

void debuggerking::module_core::read(uint8_t * data, size_t & data_size, long long & interval, long long & timestamp)
{
	data_size = 0;
	timestamp = 0;
	if (_record_file != INVALID_HANDLE_VALUE)
	{
		bool read_end = is_read_end(_record_file);
		if (!read_end)
		{
			uint8_t nalu_type;
			read(_record_file, nalu_type, data, data_size, interval, timestamp);

			//if (data_size>0)
			//	log4cplus_logger::make_debug_log("parallel.record.streamer", "nalu type : [%s], nalu size : [%d]", nalu_type == 0 ? "sps" : (nalu_type == 1 ? "pps" : (nalu_type == 2 ? "idr" : "vcl")), (int)data_size);
			//strncpy_s(_last_accessed_file, _record_module->get_filename(), sizeof(_last_accessed_file));
		}
		else
		{
			close_recorder_file(_record_file);
			_record_file = INVALID_HANDLE_VALUE;

			char single_recorder_file_search_path[MAX_PATH] = { 0 };
			_snprintf_s(single_recorder_file_search_path, sizeof(single_recorder_file_search_path), "%s\\*", _single_recorder_file_path);
			HANDLE bfind = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATAA wfd;
			bfind = ::FindFirstFileA(single_recorder_file_search_path, &wfd);
			if (bfind == INVALID_HANDLE_VALUE)
			{
				return;
			}
			else
			{
				char * dot = (char*)strrchr(_last_accessed_r_file_name, '.');
				_strset_s(dot, strlen(dot) + 1, 0x00);
				_last_r_timestamp = atoll(_last_accessed_r_file_name);

				char recorded_filename[MAX_PATH] = { 0 };
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
							if (timestamp > _last_r_timestamp)
							{
								sorted_filenames.insert(std::make_pair(timestamp - _last_r_timestamp, wfd.cFileName));
							}
						}
					}
				} while (::FindNextFileA(bfind, &wfd));

				std::map<long long, std::string, std::less<long long>>::iterator iter = sorted_filenames.begin();
				if (iter != sorted_filenames.end())
				{
					_snprintf_s(recorded_filename, sizeof(recorded_filename), "%s\\%s", _single_recorder_file_path, iter->second.c_str());

					long long start_time = 0, end_time = 0;

					_record_file = open_recorder_file(recorded_filename);
					if (_record_file != INVALID_HANDLE_VALUE)
					{
						get_start_end_time(_record_file, start_time, end_time);
						seek(_record_file, start_time);
					}
				}
				else
				{
					return;
				}
			}
		}
	}
}

void debuggerking::module_core::get_years(const char * content_path, const char * uuid, int years[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = content_path;
	char single_media_source_path[MAX_PATH] = { 0 };
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
		char recorded_filename[MAX_PATH] = { 0 };
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
}

void debuggerking::module_core::get_months(const char * content_path, const char * uuid, int year, int months[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = content_path;
	char single_media_source_path[MAX_PATH] = { 0 };
	char single_media_source_search_path[MAX_PATH] = { 0 };
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
		char recorded_filename[MAX_PATH] = { 0 };
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
}

void debuggerking::module_core::get_days(const char * content_path, const char * uuid, int year, int month, int days[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = content_path;
	char single_media_source_path[MAX_PATH] = { 0 };
	char single_media_source_search_path[MAX_PATH] = { 0 };
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
		char recorded_filename[MAX_PATH] = { 0 };
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
}

void debuggerking::module_core::get_hours(const char * content_path, const char * uuid, int year, int month, int day, int hours[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = content_path;
	char single_media_source_path[MAX_PATH] = { 0 };
	char single_media_source_search_path[MAX_PATH] = { 0 };
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
		char recorded_filename[MAX_PATH] = { 0 };
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
					if (year == year1 && month == month1 && day == day1)
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
}

void debuggerking::module_core::get_minutes(const char * content_path, const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = content_path;
	char single_media_source_path[MAX_PATH] = { 0 };
	char single_media_source_search_path[MAX_PATH] = { 0 };
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
		char recorded_filename[MAX_PATH] = { 0 };
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


	char single_media_source_file_path[MAX_PATH] = { 0 };
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
}

void debuggerking::module_core::get_seconds(const char * content_path, const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size)
{
	///////////////////////////////////////////////////////////////////
	const char * contents = content_path;
	char single_media_source_path[MAX_PATH] = { 0 };
	char single_media_source_search_path[MAX_PATH] = { 0 };
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
		char recorded_filename[MAX_PATH] = { 0 };
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


	char single_media_source_file_path[MAX_PATH] = { 0 };
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
}

const uint8_t * debuggerking::module_core::get_vps(size_t & vps_size) const
{ 
	vps_size = _vps_size;
	return _vps;
}

const uint8_t * debuggerking::module_core::get_sps(size_t & sps_size) const
{
	sps_size = _sps_size;
	return _sps;
}

const uint8_t * debuggerking::module_core::get_pps(size_t & pps_size) const
{
	pps_size = _pps_size;
	return _pps;
}

void debuggerking::module_core::set_vps(const uint8_t * vps, size_t vps_size)
{
	_vps_size = vps_size;
	memcpy(_vps, vps, _vps_size);
}

void debuggerking::module_core::set_sps(const uint8_t * sps, size_t sps_size)
{
	_sps_size = sps_size;
	memcpy(_sps, sps, _sps_size);
}

void debuggerking::module_core::set_pps(const uint8_t * pps, size_t pps_size)
{
	_pps_size = pps_size;
	memcpy(_pps, pps, _pps_size);
}

/////////////////////////////////////////////
HANDLE debuggerking::module_core::open_recorder_file(const char * recorder_file_path)
{
	_rindex = 0;

	HANDLE f = ::CreateFileA(recorder_file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == NULL || f == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	char * slash = (char*)strrchr(recorder_file_path, '\\');
	char * recorder_file_name = slash + 1;
	strncpy_s(_last_accessed_r_file_name, recorder_file_name, sizeof(_last_accessed_r_file_name));

	uint8_t spspps[MAX_PATH] = { 0 };
	bool sps_found = false;
	bool pps_found = false;

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	uint32_t spspps_index = 2 * sizeof(long long);//skip file information header
	recorder_module::set_file_position(f, spspps_index, FILE_BEGIN);

	do
	{
		long long file_size = recorder_module::get_file_size(f);
		if (file_size <= spspps_index)
			break;

		uint8_t nalu_type = 0;
		long long nalu_timestamp = 0;
		uint32_t nalu_size = 0;

		/*
		[file information    ][nalu information                                    ]
		[8byte     ][8byte   ][1byte frame type][8byte    ][4byte    ][nalu-size byte]
		[start-time][end-time][keyframe        ][timestamp][nalu-size][nalu          ]
		*/

		buf = (void*)&nalu_type;
		bytes_to_read = sizeof(nalu_type);
		bytes_read = 0;
		::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
		spspps_index += bytes_read;

		buf = (void*)&nalu_timestamp;
		bytes_to_read = sizeof(nalu_timestamp);
		bytes_read = 0;
		::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
		spspps_index += bytes_read;

		buf = (void*)&nalu_size;
		bytes_to_read = sizeof(nalu_size);
		bytes_read = 0;
		::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
		spspps_index += bytes_read;

		if (nalu_type == recorder_module::nalu_type_t::sps)
		{
			memset(spspps, 0x00, sizeof(spspps));
			buf = (void*)spspps;
			bytes_to_read = nalu_size;
			bytes_read = 0;
			::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
			spspps_index += bytes_read;
			set_sps(spspps, nalu_size);
			sps_found = true;
		}
		else if (nalu_type == recorder_module::nalu_type_t::pps)
		{
			memset(spspps, 0x00, sizeof(spspps));
			buf = (void*)spspps;
			bytes_to_read = nalu_size;
			bytes_read = 0;
			::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
			spspps_index += bytes_read;
			set_pps(spspps, nalu_size);
			pps_found = true;
		}
		else
		{
			recorder_module::set_file_position(f, spspps_index + nalu_size, FILE_BEGIN);
		}

		if (sps_found && pps_found)
			break;

	} while (1);

	if (sps_found && pps_found)
		return f;
	else
		return INVALID_HANDLE_VALUE;
}

HANDLE debuggerking::module_core::open_recorder_file(const char * storage, const char * uuid, long long timestamp)
{
	_wbuffer_size = WBUFFER_SIZE;
	if (_wbuffer)
	{
		free(_wbuffer);
		_wbuffer = nullptr;
	}
	_wbuffer = static_cast<uint8_t*>(malloc(_wbuffer_size));
	_wrecv_idr = false;
	_windex = 0;

	bool create_new_one = false;
	///////////////////////////////////////////////////////////////////
	char single_recorder_file_path[MAX_PATH] = { 0 };
	char single_recorder_file_search_path[MAX_PATH] = { 0 };
	_snprintf_s(single_recorder_file_path, sizeof(single_recorder_file_path), "%s%s\\", storage, uuid);
	_snprintf_s(single_recorder_file_search_path, sizeof(single_recorder_file_search_path), "%s%s\\*", storage, uuid);


	HANDLE f = INVALID_HANDLE_VALUE;
	HANDLE bfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA wfd;
	bfind = ::FindFirstFileA(single_recorder_file_search_path, &wfd);
	if (bfind == INVALID_HANDLE_VALUE)
	{
		create_new_one = true;
	}
	else
	{
		char recorded_filename[MAX_PATH] = { 0 };
		std::vector<long long> sorted_filenames;
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
					sorted_filenames.push_back(timestamp);

				}
			}
		} while (::FindNextFileA(bfind, &wfd));
		std::sort(sorted_filenames.begin(), sorted_filenames.end(), compare_timestamp);
		//get latest file
		if (sorted_filenames.size() > 0)
		{
			long long ts = sorted_filenames[0];
			_snprintf_s(recorded_filename, sizeof(recorded_filename), "%s%s\\%lld.dat", storage, uuid, ts);
			f = ::CreateFileA(recorded_filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			long long st = 0, et = 0;
			get_start_end_time(f, st, et);
			if (et > st)
			{
				create_new_one = true;
				::CloseHandle(f);
				f = INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			create_new_one = true;
		}
	}

	::FindClose(bfind);
	//////////////////////////////////////////////////////////////////

	if (!create_new_one)
	{
		_windex = recorder_module::get_file_size(f);
	}
	else
	{
		char folder[MAX_PATH] = { 0 };
		char filepath[MAX_PATH] = { 0 };

		_snprintf_s(folder, MAX_PATH, "%s%s", storage, uuid);
		if (::GetFileAttributesA(folder) == INVALID_FILE_ATTRIBUTES)
			::CreateDirectoryA(folder, NULL);

#if defined(WITH_MILLISECOND)
		if (timestamp == 0)
			timestamp = recorder_module::get_elapsed_millisec_from_epoch();
#else
		if (timestamp == 0)
			timestamp = recorder_module::get_elapsed_microsec_from_epoch();
#endif

		_snprintf_s(filepath, MAX_PATH, "%s%s\\%lld.dat", storage, uuid, timestamp);
		f = ::CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (f == NULL || f == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;
	else
		return f;
}

void debuggerking::module_core::close_recorder_file(HANDLE f)
{
	if (f != NULL && f != INVALID_HANDLE_VALUE)
		::CloseHandle(f);

	_vps_size = 0;
	_sps_size = 0;
	_pps_size = 0;
	memset(_vps, 0x00, sizeof(_vps));
	memset(_sps, 0x00, sizeof(_sps));
	memset(_pps, 0x00, sizeof(_pps));

	_last_r_timestamp = 0;
	_rindex = 0;

	_wbuffer_size = 0;
	if (_wbuffer)
	{
		free(_wbuffer);
		_wbuffer = nullptr;
	}
	_wrecv_idr = false;
	_last_w_timestamp = 0;
	_windex = 0;
}

bool debuggerking::module_core::is_occupied(HANDLE f)
{
	long long stime = 0;
	long long etime = 0;
	read_header_time(f, stime, etime);
	if (etime == -1)
		return true;
	else
		return false;
}

bool debuggerking::module_core::is_read_end(HANDLE f)
{
	long long file_size = recorder_module::get_file_size(f);
	return _rindex >= file_size;
}

void debuggerking::module_core::get_start_end_time(HANDLE f, long long & start_time, long long & end_time)
{
	read_header_time(f, start_time, end_time);
}

long long debuggerking::module_core::get_start_time(HANDLE f)
{
	long long stime = 0;
	long long etime = 0;
	read_header_time(f, stime, etime);
	return stime;
}

long long debuggerking::module_core::get_end_time(HANDLE f)
{
	long long stime = 0;
	long long etime = 0;
	read_header_time(f, stime, etime);
	return etime;
}

bool debuggerking::module_core::write(HANDLE f, uint8_t * nalu, size_t nalu_size, long long timestamp)
{
	if (f == NULL || f == INVALID_HANDLE_VALUE)
		return false;

#if defined(WITH_MILLISECOND)
	if (timestamp == 0)
		timestamp = recorder_module::get_elapsed_millisec_from_epoch();
#else
	if (timestamp == 0)
		timestamp = recorder_module::get_elapsed_microsec_from_epoch();
#endif

	long long file_size = recorder_module::get_file_size(f);
	if (file_size == 0)
	{
		write_header_time(f, timestamp, -1);
		_windex += 2 * sizeof(long long);//skip file information header
	}

	uint8_t * data = nalu + 4;
	size_t data_size = nalu_size - 4;

	size_t saved_sps_size = 0;
	const uint8_t * saved_sps = get_sps(saved_sps_size);
	size_t saved_pps_size = 0;
	const uint8_t * saved_pps = get_pps(saved_pps_size);

	bool is_sps = recorder_module::is_sps(recorder_module::video_submedia_type_t::h264, data[0] & 0x1F);
	bool is_pps = recorder_module::is_pps(recorder_module::video_submedia_type_t::h264, data[0] & 0x1F);
	bool is_idr = recorder_module::is_idr(recorder_module::video_submedia_type_t::h264, data[0] & 0x1F);
	if (is_sps)
	{
		if (saved_sps_size < 1 || !saved_sps)
		{
			set_sps(data, data_size);
			_wrecv_idr = false;
		}
		else
		{
			if (memcmp(saved_sps, data, saved_sps_size))
			{
				set_sps(data, data_size);
				_wrecv_idr = false;
			}
		}
	}
	else if (is_pps)
	{
		if (saved_pps_size < 1 || !saved_pps)
		{
			set_pps(data, data_size);
			_wrecv_idr = false;
		}
		else
		{
			if (memcmp(saved_pps, data, saved_pps_size))
			{
				set_pps(data, data_size);
				_wrecv_idr = false;
			}
		}
	}
	else if (is_idr)
	{
		if (!_wrecv_idr)
			_wrecv_idr = true;

		saved_sps = get_sps(saved_sps_size);
		saved_pps = get_pps(saved_pps_size);
		if ((saved_sps && saved_sps_size>0) && (saved_pps && saved_pps_size>0))
		{
			write_bitstream(f, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size, timestamp);
		}
		else
		{
			write_bitstream(f, nullptr, 0, nullptr, 0, data, data_size, timestamp);
		}
	}
	else if (_wrecv_idr)
	{
		write_bitstream(f, nullptr, 0, nullptr, 0, data, data_size, timestamp);
	}

	_last_w_timestamp = timestamp;
	return true;
}

bool debuggerking::module_core::seek(HANDLE f, long long seek_timestamp)
{
	bool found_sps = false;

	_rindex = 0;
	uint32_t seek_index = 2 * sizeof(long long);

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	uint32_t prev_idr_index = 0;
	long long prev_idr_timestamp = 0;

	uint32_t next_idr_index = 0;
	long long next_idr_timestamp = 0;

	do
	{
		long long file_size = recorder_module::get_file_size(f);
		if (seek_index >= file_size)
			break;

		recorder_module::set_file_position(f, seek_index, FILE_BEGIN);

		uint8_t nalu_type = 0;
		long long nalu_timestamp = 0;
		uint32_t nalu_size = 0;

		//read frame type
		buf = (void*)&nalu_type;
		bytes_to_read = sizeof(uint8_t);
		bytes_read = 0;
		::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
		seek_index += bytes_read;

		//read timestramp
		buf = (void*)&nalu_timestamp;
		bytes_to_read = sizeof(long long);
		bytes_read = 0;
		::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
		seek_index += bytes_read;

		//read nalu size
		buf = (void*)&nalu_size;
		bytes_to_read = sizeof(uint32_t);
		bytes_read = 0;
		::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
		seek_index += bytes_read;
		seek_index += nalu_size;

		if (nalu_type == recorder_module::nalu_type_t::sps)
		{
			if (nalu_timestamp >= seek_timestamp)
			{
				_rindex = seek_index - (sizeof(uint8_t) + sizeof(long long) + sizeof(uint32_t) + nalu_size);
				found_sps = true;
				break;
			}
			/*else if (nalu_timestamp < seek_timestamp)
			{
				prev_idr_index = seek_index - (sizeof(uint8_t) + sizeof(long long) + sizeof(uint32_t) + nalu_size);
				prev_idr_timestamp = nalu_timestamp;
			}
			else if (nalu_timestamp > seek_timestamp)
			{
				next_idr_index = seek_index - (sizeof(uint8_t) + sizeof(long long) + sizeof(uint32_t) + nalu_size);
				next_idr_timestamp = nalu_timestamp;
				break;
			}*/
		}
	} while (1);

	return found_sps;
	/*
	if (_read_index == 0)
	{
		if ((seek_timestamp - prev_idr_timestamp) <= (next_idr_timestamp - seek_timestamp))
		{
			_read_index = prev_idr_index;
		}
		else
		{
			_read_index = next_idr_index;
		}
	}
	*/
}

void debuggerking::module_core::read(HANDLE f, uint8_t & nalu_type, uint8_t * data, size_t & data_size, long long & interval, long long & timestamp)
{
	//0x00 0x00 0x00 0x01 0x06 0x05 0x08 0xbc 0x97 0xb8 0x4d 0x96 0x9f 0x48 0xb9 0xbc 0xe4 0x7c 0x1c 0x1a 0x39 0x2f 0x37 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
	read_bitstream(f, nalu_type, data, data_size, timestamp);
	long long next_time_stamp = 0;
	read_next_bitstream_timestamp(f, next_time_stamp);
	interval = next_time_stamp - timestamp;
}

void debuggerking::module_core::write_bitstream(HANDLE f, const uint8_t * sps, size_t sps_size, const uint8_t * pps, size_t pps_size, uint8_t * nalu, size_t size, long long timestamp)
{
	uint8_t nalu_type = recorder_module::nalu_type_t::sps;
	long long nalu_timestamp = 0;
	uint32_t nalu_size = 0;

	size_t write_buffer_index = 0;
	if (sps && sps_size > 0)
	{
		//put nalu type
		nalu_type = recorder_module::nalu_type_t::sps;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_type, sizeof(nalu_type));
		write_buffer_index += sizeof(nalu_type);
		//put timestamp
		nalu_timestamp = timestamp;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_timestamp, sizeof(nalu_timestamp));
		write_buffer_index += sizeof(nalu_timestamp);
		//put nalu size
		nalu_size = sps_size;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_size, sizeof(nalu_size));
		write_buffer_index += sizeof(nalu_size);
		memmove(_wbuffer + write_buffer_index, sps, sps_size);
		write_buffer_index += sps_size;
	}

	if (pps && pps_size > 0)
	{
		//put nalu type
		nalu_type = recorder_module::nalu_type_t::pps;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_type, sizeof(nalu_type));
		write_buffer_index += sizeof(nalu_type);
		//put timestamp
		nalu_timestamp = timestamp;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_timestamp, sizeof(nalu_timestamp));
		write_buffer_index += sizeof(nalu_timestamp);
		//put nalu size
		nalu_size = pps_size;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_size, sizeof(nalu_size));
		write_buffer_index += sizeof(nalu_size);
		memmove(_wbuffer + write_buffer_index, pps, pps_size);
		write_buffer_index += pps_size;
	}

	if ((nalu[0] & 0x1F) == 0x05) //idr
	{
		//put nalu type
		nalu_type = recorder_module::nalu_type_t::idr;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_type, sizeof(nalu_type));
		write_buffer_index += sizeof(nalu_type);
		//put timestamp
		nalu_timestamp = timestamp;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_timestamp, sizeof(nalu_timestamp));
		write_buffer_index += sizeof(nalu_timestamp);
		//put nalu size
		nalu_size = size;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_size, sizeof(nalu_size));
		write_buffer_index += sizeof(nalu_size);
		memmove(_wbuffer + write_buffer_index, nalu, nalu_size);
		write_buffer_index += nalu_size;
	}
	else
	{
		//put nalu type
		nalu_type = recorder_module::nalu_type_t::vcl;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_type, sizeof(nalu_type));
		write_buffer_index += sizeof(nalu_type);
		//put timestamp
		nalu_timestamp = timestamp;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_timestamp, sizeof(nalu_timestamp));
		write_buffer_index += sizeof(nalu_timestamp);
		//put nalu size
		nalu_size = size;
		memmove(_wbuffer + write_buffer_index, (void*)&nalu_size, sizeof(nalu_size));
		write_buffer_index += sizeof(nalu_size);
		memmove(_wbuffer + write_buffer_index, nalu, nalu_size);
		write_buffer_index += nalu_size;
	}

	void * buff = 0;
	unsigned long bytes_to_writes = 0;
	unsigned long bytes_writes = 0;

	recorder_module::set_file_position(f, _windex, FILE_BEGIN);
	buff = _wbuffer;
	bytes_to_writes = write_buffer_index;
	bytes_writes = 0;
	::WriteFile(f, buff, bytes_to_writes, &bytes_writes, NULL);
	_windex += bytes_writes;
}

//make nalu information
void debuggerking::module_core::write_bitstream(HANDLE f, uint8_t nalu_type, uint8_t * nalu, size_t nalu_size, long long timestamp)
{
	void * buff = 0;
	unsigned long bytes_to_writes = 0;
	unsigned long bytes_writes = 0;

	recorder_module::set_file_position(f, _windex, FILE_BEGIN);

	//write nalu_type
	buff = &nalu_type;
	bytes_to_writes = sizeof(uint8_t);
	bytes_writes = 0;
	::WriteFile(f, buff, bytes_to_writes, &bytes_writes, NULL);
	_windex += bytes_writes;

	//write timestamp
	buff = &timestamp;
	bytes_to_writes = sizeof(timestamp);
	bytes_writes = 0;
	::WriteFile(f, buff, bytes_to_writes, &bytes_writes, NULL);
	_windex += bytes_writes;

	//write nalu size
	uint32_t size = nalu_size;
	buff = &size;
	bytes_to_writes = sizeof(uint32_t);
	bytes_writes = 0;
	::WriteFile(f, buff, bytes_to_writes, &bytes_writes, NULL);
	_windex += bytes_writes;

	//write nalu
	buff = nalu;
	bytes_to_writes = nalu_size;
	bytes_writes = 0;
	::WriteFile(f, buff, bytes_to_writes, &bytes_writes, NULL);
	_windex += bytes_writes;

	//char time[260] = { 0 };
	//get_time_from_elapsed_msec_from_epoch(timestamp, time, sizeof(time));
	//dk_log4cplus_logger::instance().make_system_debug_log("parallel.record.recorder", "time is %s", time);
}

void debuggerking::module_core::write_header_time(HANDLE f, long long start_time, long long end_time)
{
	if (f == NULL || f == INVALID_HANDLE_VALUE)
		return;

	recorder_module::set_file_position(f, 0, FILE_BEGIN);
	if (start_time>0)
	{
		void * buff = &start_time;
		unsigned long bytes_to_writes = sizeof(long long);
		unsigned long bytes_writes = 0;
		recorder_module::set_file_position(f, 0, FILE_BEGIN);
		::WriteFile(f, buff, bytes_to_writes, &bytes_writes, NULL);
	}

	if (end_time > 0)
	{
		void * buff = &end_time;
		unsigned long bytes_to_writes = sizeof(long long);
		unsigned long bytes_writes = 0;
		recorder_module::set_file_position(f, sizeof(long long), FILE_BEGIN);
		::WriteFile(f, buff, bytes_to_writes, &bytes_writes, NULL);
	}
}

void debuggerking::module_core::read_header_time(HANDLE f, long long & start_time, long long & end_time)
{
	if (f == NULL || f == INVALID_HANDLE_VALUE)
		return;

	recorder_module::set_file_position(f, 0, FILE_BEGIN);

	long long stime;
	long long etime;

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	buf = (void*)&stime;
	bytes_to_read = sizeof(long long);
	bytes_read = 0;
	::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
	if (stime == 0)
		start_time = -1;
	else
		start_time = stime;

	buf = (void*)&etime;
	bytes_to_read = sizeof(long long);
	bytes_read = 0;
	::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
	if (etime == 0)
		end_time = -1;
	else
		end_time = etime;
}

void debuggerking::module_core::read_bitstream(HANDLE f, uint8_t & nalu_type, uint8_t * data, size_t & data_size, long long & timestamp)
{
	if (_rindex == 0)
		_rindex = 2 * sizeof(long long);

	recorder_module::set_file_position(f, _rindex, FILE_BEGIN);

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	//uint8_t nalu_type = 0;
	long long nalu_timestamp = 0;
	uint32_t nalu_size = 0;
	uint8_t * nalu = data;


	//read frame type
	buf = (void*)&nalu_type;
	bytes_to_read = sizeof(uint8_t);
	bytes_read = 0;
	::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
	_rindex += bytes_read;

	//read timestramp
	buf = (void*)&nalu_timestamp;
	bytes_to_read = sizeof(long long);
	bytes_read = 0;
	::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
	_rindex += bytes_read;

	//read nalu size
	buf = (void*)&nalu_size;
	bytes_to_read = sizeof(uint32_t);
	bytes_read = 0;
	::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
	_rindex += bytes_read;

	//read nalu size
#if 1
	buf = (void*)(nalu);
	bytes_to_read = nalu_size;
	bytes_read = 0;
	::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
	_rindex += bytes_read;

	data_size = nalu_size;
#else
	uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
	buf = (void*)(nalu + 4);
	bytes_to_read = nalu_size;
	bytes_read = 0;
	::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
	_rindex += bytes_read;
	memmove(nalu, start_code, sizeof(start_code));

	data_size = nalu_size + 4;
#endif
	timestamp = nalu_timestamp;
}

void debuggerking::module_core::read_next_bitstream_timestamp(HANDLE f, long long & timestamp)
{
	recorder_module::set_file_position(f, _rindex + 1, FILE_BEGIN);

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	//read timestramp
	buf = (void*)&timestamp;
	bytes_to_read = sizeof(long long);
	bytes_read = 0;
	::ReadFile(f, buf, bytes_to_read, &bytes_read, NULL);
}