#pragma once

#include <cstdint>
#include <string>
#include <atlstr.h>
#include <winternl.h>

#define TICKSPERSEC					10000000
#define TICKSPERMSEC				10000
#define SECSPERDAY					86400
#define SECSPERHOUR					3600
#define SECSPERMIN					60
#define MINSPERHOUR					60
#define HOURSPERDAY					24
#define EPOCHWEEKDAY				1  /* Jan 1, 1601 was Monday */
#define DAYSPERWEEK					7
#define MONSPERYEAR					12
#define DAYSPERQUADRICENTENNIUM		(365 * 400 + 97)
#define DAYSPERNORMALQUADRENNIUM	(365 * 4 + 1)
#define SECS_1601_TO_1970			((369 * 365 + 89) * (ULONGLONG)SECSPERDAY)
#define TICKS_1601_TO_1970			(SECS_1601_TO_1970 * TICKSPERSEC)
#define SECS_1601_TO_1980			((379 * 365 + 91) * (ULONGLONG)SECSPERDAY)
#define TICKS_1601_TO_1980			(SECS_1601_TO_1980 * TICKSPERSEC)

class dk_time_helper
{
public:
	static void check_time(void)
	{
		SYSTEMTIME utc_system_time = { 0 };
		SYSTEMTIME local_system_time1 = { 0 };
		SYSTEMTIME local_system_time = { 0 };
		unsigned long elapsed_utc_time_seconds = 0;

		::GetSystemTime(&utc_system_time);

		TIME_ZONE_INFORMATION tz;
		::GetTimeZoneInformation(&tz);
		::TzSpecificLocalTimeToSystemTime(&tz, &utc_system_time, &local_system_time1);

		convert_utc_system_time_to_elapsed_utc_time_seconds(utc_system_time, elapsed_utc_time_seconds);
		convert_elasped_utc_time_to_local_system_time(elapsed_utc_time_seconds, local_system_time);
	}

	static bool convert_utc_system_time_to_elapsed_utc_time_seconds(SYSTEMTIME & utc_system_time, unsigned long & elapsed_utc_time_seconds)
	{
		FILETIME utc_file_time = { 0 };
		::SystemTimeToFileTime(&utc_system_time, &utc_file_time);

		LARGE_INTEGER utc_li_time;
		utc_li_time.LowPart = utc_file_time.dwLowDateTime;
		utc_li_time.HighPart = utc_file_time.dwHighDateTime;

		dk_time_helper::RtlTimeToSecondsSince1970(&utc_li_time, &elapsed_utc_time_seconds);

		return true;
	}

	static unsigned long get_elapsed_utc_time_seconds(void)
	{
		SYSTEMTIME utc_system_time = { 0 };
		::GetSystemTime(&utc_system_time);

		FILETIME utc_file_time = { 0 };
		::SystemTimeToFileTime(&utc_system_time, &utc_file_time);

		LARGE_INTEGER utc_li_time;
		utc_li_time.LowPart = utc_file_time.dwLowDateTime;
		utc_li_time.HighPart = utc_file_time.dwHighDateTime;

		ULONG elasped_utc_time_seconds = 0;
		dk_time_helper::RtlTimeToSecondsSince1970(&utc_li_time, &elasped_utc_time_seconds);

		return elasped_utc_time_seconds;
	}

	/*
	static unsigned long get_elapsed_utc_time_seconds(void)
	{
		SYSTEMTIME utc_system_time = { 0 };
		::GetSystemTime(&utc_system_time);

		FILETIME utc_file_time = { 0 };
		::SystemTimeToFileTime(&utc_system_time, &utc_file_time);

		LARGE_INTEGER utc_li_time;
		utc_li_time.LowPart = utc_file_time.dwLowDateTime;
		utc_li_time.HighPart = utc_file_time.dwHighDateTime;

		ULONG elasped_utc_time_seconds = 0;
		dk_time_helper::RtlTimeToSecondsSince1970(&utc_li_time, &elasped_utc_time_seconds);

		return elasped_utc_time_seconds;
	}
	*/

	static void convert_elasped_utc_time_to_local_system_time(unsigned long elapsed_utc_time, SYSTEMTIME & local_system_time)
	{
		LARGE_INTEGER utc_li_time;
		dk_time_helper::RtlSecondsSince1970ToTime(elapsed_utc_time, &utc_li_time);
		FILETIME utc_file_time = { 0 };
		utc_file_time.dwLowDateTime = utc_li_time.LowPart;
		utc_file_time.dwHighDateTime = utc_li_time.HighPart;


		SYSTEMTIME utc_system_time = { 0 };
		::FileTimeToSystemTime(&utc_file_time, &utc_system_time);

		TIME_ZONE_INFORMATION tz;
		::GetTimeZoneInformation(&tz);
		::TzSpecificLocalTimeToSystemTime(&tz, &utc_system_time, &local_system_time);
	}

	static void convert_elasped_utc_time_to_utc_system_time(unsigned long elapsed_utc_time, SYSTEMTIME & utc_system_time)
	{
		LARGE_INTEGER utc_li_time;
		RtlSecondsSince1970ToTime(elapsed_utc_time, &utc_li_time);

		FILETIME utc_file_time = { 0 };
		utc_file_time.dwLowDateTime = utc_li_time.LowPart;
		utc_file_time.dwHighDateTime = utc_li_time.HighPart;

		::FileTimeToSystemTime(&utc_file_time, &utc_system_time);
	}

	static bool convert_local_string_time_to_utc_time(const char * time, SYSTEMTIME & utc_time)
	{
		if (strlen(time) != 14) //20160320154030 : 2015-03-20 15::40::30
			return false;

		char seek_year[5] = { 0 };
		char seek_month[3] = { 0 };
		char seek_day[3] = { 0 };
		char seek_hour[3] = { 0 };
		char seek_minute[3] = { 0 };
		char seek_second[3] = { 0 };
		int32_t index = 0;
		memcpy(seek_year, time + index, 4);
		index += 4;
		memcpy(seek_month, time + index, 2);
		index += 2;
		memcpy(seek_day, time + index, 2);
		index += 2;
		memcpy(seek_hour, time + index, 2);
		index += 2;
		memcpy(seek_minute, time + index, 2);
		index += 2;
		memcpy(seek_second, time + index, 2);

		int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
		sscanf_s(seek_year, "%d", &year);
		sscanf_s(seek_month, "%d", &month);
		sscanf_s(seek_day, "%d", &day);
		sscanf_s(seek_hour, "%d", &hour);
		sscanf_s(seek_minute, "%d", &minute);
		sscanf_s(seek_second, "%d", &second);

		SYSTEMTIME local_time = { 0 };
		local_time.wYear = year;
		local_time.wMonth = month;
		local_time.wDay = day;
		local_time.wHour = hour;
		local_time.wMinute = minute;
		local_time.wSecond = second;
		local_time.wMilliseconds = 0;

		TIME_ZONE_INFORMATION tz;
		::GetTimeZoneInformation(&tz);
		::TzSpecificLocalTimeToSystemTime(&tz, &local_time, &utc_time);
		return true;
	}

	static bool convert_string_time_to_time(const char * time, int32_t & year, int32_t & month, int32_t & day, int32_t & hour, int32_t & minute, int32_t & second)
	{
		if (strlen(time) != 14) //20160320154030 : 2015-03-20 15::40::30
			return false;

		char seek_year[5] = { 0 };
		char seek_month[3] = { 0 };
		char seek_day[3] = { 0 };
		char seek_hour[3] = { 0 };
		char seek_minute[3] = { 0 };
		char seek_second[3] = { 0 };
		int32_t index = 0;
		memcpy(seek_year, time + index, 4);
		index += 4;
		memcpy(seek_month, time + index, 2);
		index += 2;
		memcpy(seek_day, time + index, 2);
		index += 2;
		memcpy(seek_hour, time + index, 2);
		index += 2;
		memcpy(seek_minute, time + index, 2);
		index += 2;
		memcpy(seek_second, time + index, 2);

		sscanf_s(seek_year, "%d", &year);
		sscanf_s(seek_month, "%d", &month);
		sscanf_s(seek_day, "%d", &day);
		sscanf_s(seek_hour, "%d", &hour);
		sscanf_s(seek_minute, "%d", &minute);
		sscanf_s(seek_second, "%d", &second);

		return true;
	}

private:

	static bool RtlTimeToSecondsSince1970(const LARGE_INTEGER * time, LPDWORD seconds)
	{
		ULONGLONG tmp = time->QuadPart / TICKSPERSEC - SECS_1601_TO_1970;
		if (tmp > 0xffffffff) 
			return false;
		*seconds = tmp;
		return true;
	}

	static void RtlSecondsSince1970ToTime(DWORD seconds, LARGE_INTEGER * time)
	{
		time->QuadPart = seconds * (ULONGLONG)TICKSPERSEC + TICKS_1601_TO_1970;
	}
};