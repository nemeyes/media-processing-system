#include "dk_mpeg2ts_recorder.h"
#include <dk_fileio.h>
#include <stdio.h>
#include <stdlib.h>
#include <winternl.h>

dk_mpeg2ts_recorder::dk_mpeg2ts_recorder(const char * id)
	: dk_ff_mpeg2ts_muxer()
	, _file(INVALID_HANDLE_VALUE)
{
	char filepath[MAX_PATH] = { 0 };
	
	unsigned long utc_elasped_time = dk_mpeg2ts_recorder::get_elapsed_utc_time();

	_snprintf_s(filepath, MAX_PATH, "%s-%lu.ts", id, utc_elasped_time);
	_file = ::CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file == INVALID_HANDLE_VALUE)
	{
		DWORD err = ::GetLastError();
	}
}

dk_mpeg2ts_recorder::~dk_mpeg2ts_recorder(void)
{
	::CloseHandle(_file);
}

long long dk_mpeg2ts_recorder::get_file_size(void)
{
	if (_file == NULL || _file == INVALID_HANDLE_VALUE)
		return 0;
	LARGE_INTEGER filesize = { 0 };
	::GetFileSizeEx(_file, &filesize);
	long long estimated_filesize = 0;
	estimated_filesize = filesize.HighPart;
	estimated_filesize <<= 32;
	estimated_filesize |= filesize.LowPart;
	return estimated_filesize;
}

dk_mpeg2ts_recorder::ERR_CODE dk_mpeg2ts_recorder::recv_ts_stream_callback(uint8_t * ts, size_t stream_size)
{
	DWORD nbytes = 0;
	::WriteFile(_file, ts, stream_size, &nbytes, NULL);
	return dk_mpeg2ts_recorder::ERR_CODE_SUCCESS;
}

unsigned long dk_mpeg2ts_recorder::get_elapsed_utc_time(void)
{
	SYSTEMTIME utc_systemtime = { 0 };
	::GetSystemTime(&utc_systemtime);

	FILETIME utc_filetime = { 0 };
	::SystemTimeToFileTime(&utc_systemtime, &utc_filetime);

	LARGE_INTEGER utc_time;
	utc_time.LowPart = utc_filetime.dwLowDateTime;
	utc_time.HighPart = utc_filetime.dwHighDateTime;

	ULONG utc_elasped_time;
	::RtlTimeToSecondsSince1970(&utc_time, &utc_elasped_time);

	return utc_elasped_time;
}
