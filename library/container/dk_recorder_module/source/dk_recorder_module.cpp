#include "dk_recorder_module.h"
#include "recorder_module.h"

#include <boost/date_time/local_time/local_time.hpp>
#include <dk_log4cplus_logger.h>

debuggerking::recorder_module::recorder_module(long long chunk_size_bytes)
	: _core(nullptr)
{
	_core = new module_core(this, chunk_size_bytes);
}

debuggerking::recorder_module::~recorder_module(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

bool debuggerking::recorder_module::seek(const char * single_recorder_file_path, long long timestamp, bool read)
{
	if (_core)
	{
		if (read)
			return _core->seek4r(single_recorder_file_path, timestamp);
		else
			return _core->seek4w(single_recorder_file_path, timestamp);
	}
		
	return false;
}

void debuggerking::recorder_module::write(uint8_t * data, size_t data_size, long long timestamp)
{
	if (_core)
		_core->write(data, data_size, timestamp);
}

void debuggerking::recorder_module::read(uint8_t * data, size_t & data_size, long long & interval, long long & timestamp)
{
	if (_core)
		_core->read(data, data_size, interval, timestamp);
}

void debuggerking::recorder_module::get_years(const char * content_path, const char * uuid, int years[], int capacity, int & size)
{
	module_core::get_years(content_path, uuid, years, capacity, size);
}

void debuggerking::recorder_module::get_months(const char * content_path, const char * uuid, int year, int months[], int capacity, int & size)
{
	module_core::get_months(content_path, uuid, year, months, capacity, size);
}

void debuggerking::recorder_module::get_days(const char * content_path, const char * uuid, int year, int month, int days[], int capacity, int & size)
{
	module_core::get_days(content_path, uuid, year, month, days, capacity, size);
}

void debuggerking::recorder_module::get_hours(const char * content_path, const char * uuid, int year, int month, int day, int hours[], int capacity, int & size)
{
	module_core::get_hours(content_path, uuid, year, month, day, hours, capacity, size);
}

void debuggerking::recorder_module::get_minutes(const char * content_path, const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size)
{
	module_core::get_minutes(content_path, uuid, year, month, day, hour, minutes, capacity, size);
}

void debuggerking::recorder_module::get_seconds(const char * content_path, const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size)
{
	module_core::get_seconds(content_path, uuid, year, month, day, hour, minute, seconds, capacity, size);
}

const uint8_t * debuggerking::recorder_module::get_vps(size_t & vps_size) const
{
	const uint8_t * vps = nullptr;
	if (_core)
		vps = _core->get_vps(vps_size);
	return vps;
}

const uint8_t * debuggerking::recorder_module::get_sps(size_t & sps_size) const
{
	const uint8_t * sps = nullptr;
	if (_core)
		sps = _core->get_sps(sps_size);
	return sps;
}

const uint8_t * debuggerking::recorder_module::get_pps(size_t & pps_size) const
{
	const uint8_t * pps = nullptr;
	if (_core)
		pps = _core->get_pps(pps_size);
	return pps;
}

void debuggerking::recorder_module::set_file_position(HANDLE file, uint32_t offset, uint32_t flag)
{
	if (file != NULL && file != INVALID_HANDLE_VALUE)
	{
		::SetFilePointer(file, offset, NULL, flag);
	}
}

long long debuggerking::recorder_module::get_file_size(HANDLE file)
{
	if (file == NULL || file == INVALID_HANDLE_VALUE)
		return 0;
	LARGE_INTEGER filesize = { 0 };
	::GetFileSizeEx(file, &filesize);
	long long estimated_filesize = 0;
	estimated_filesize = filesize.HighPart;
	estimated_filesize <<= 32;
	estimated_filesize |= filesize.LowPart;
	return estimated_filesize;
}

long long debuggerking::recorder_module::get_elapsed_msec_from_epoch(void)
{
	boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
	boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration elapsed = current_time - epoch;
	long long elapsed_millsec = elapsed.total_milliseconds();
	return elapsed_millsec;
}

void debuggerking::recorder_module::get_time_from_elapsed_msec_from_epoch(long long elapsed_time, char * time_string, int32_t time_string_size)
{
	boost::posix_time::time_duration elapsed = boost::posix_time::millisec(elapsed_time);
	boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
	boost::posix_time::ptime current_time = epoch + elapsed;

	std::string tmp_time = boost::posix_time::to_simple_string(current_time);
	strcpy_s(time_string, time_string_size, tmp_time.c_str());
}

void debuggerking::recorder_module::get_time_from_elapsed_msec_from_epoch(long long elapsed_time, int32_t & year, int32_t & month, int32_t & day, int32_t & hour, int32_t & minute, int32_t & second)
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


bool debuggerking::recorder_module::is_vps(int32_t smt, uint8_t nal_unit_type)
{
	// VPS NAL units occur in H.265 only:
	return smt == recorder_module::video_submedia_type_t::hevc && nal_unit_type == 32;
}

bool debuggerking::recorder_module::is_sps(int32_t smt, uint8_t nal_unit_type)
{
	return smt == recorder_module::video_submedia_type_t::h264 ? nal_unit_type == 7 : nal_unit_type == 33;
}

bool debuggerking::recorder_module::is_pps(int32_t smt, uint8_t nal_unit_type)
{
	return smt == recorder_module::video_submedia_type_t::h264 ? nal_unit_type == 8 : nal_unit_type == 34;
}

bool debuggerking::recorder_module::is_idr(int32_t smt, uint8_t nal_unit_type)
{
	return smt == recorder_module::video_submedia_type_t::h264 ? nal_unit_type == 5 : nal_unit_type == 34;
}

bool debuggerking::recorder_module::is_vcl(int32_t smt, uint8_t nal_unit_type)
{
	return smt == recorder_module::video_submedia_type_t::h264 ? (nal_unit_type <= 5 && nal_unit_type > 0) : (nal_unit_type <= 31);
}