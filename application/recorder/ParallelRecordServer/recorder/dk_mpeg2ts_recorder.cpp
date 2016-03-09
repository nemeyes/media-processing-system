#include "dk_mpeg2ts_recorder.h"
#include <dk_fileio.h>
#include <stdio.h>
#include <stdlib.h>

dk_mpeg2ts_recorder::dk_mpeg2ts_recorder(const char * id)
	: dk_ff_mpeg2ts_muxer()
	, _file(INVALID_HANDLE_VALUE)
{
	char filepath[MAX_PATH] = { 0 };
	_snprintf_s(filepath, MAX_PATH, "%s.ts", id);
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
	return filesize.QuadPart;
}

dk_mpeg2ts_recorder::ERR_CODE dk_mpeg2ts_recorder::recv_ts_stream_callback(uint8_t * ts, size_t stream_size)
{
	DWORD nbytes = 0;
	::WriteFile(_file, ts, stream_size, &nbytes, NULL);
	return dk_mpeg2ts_recorder::ERR_CODE_SUCCESS;
}