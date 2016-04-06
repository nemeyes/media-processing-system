#include "dk_mpeg2ts_recorder.h"
#if defined(WITH_MPEG2TS)
#include <dk_time_helper.h>
#include <dk_fileio.h>
#include <stdio.h>
#include <stdlib.h>
#include <winternl.h>

dk_mpeg2ts_recorder::dk_mpeg2ts_recorder(const char * storage, const char * uuid)
	: dk_ff_mpeg2ts_muxer()
	, _file(INVALID_HANDLE_VALUE)
{
	char folder[MAX_PATH] = { 0 };
	char filepath[MAX_PATH] = { 0 };
	
	_snprintf_s(folder, MAX_PATH, "%s%s", storage, uuid);
	if (::GetFileAttributesA(folder) == INVALID_FILE_ATTRIBUTES)
		::CreateDirectoryA(folder, NULL);

	dk_time_helper::check_time();
	unsigned long elasped_utc_time_seconds = dk_time_helper::get_elapsed_utc_time_seconds();
	_snprintf_s(filepath, MAX_PATH, "%s%s\\%lu.ts", storage, uuid, elasped_utc_time_seconds);
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
#endif