#include "dk_mpeg2ts_saver.h"
#include <dk_fileio.h>

dk_mpeg2ts_saver::dk_mpeg2ts_saver(void)
	: dk_ff_mpeg2ts_muxer()
{
	_file = ::open_file_write("test.ts");
}

dk_mpeg2ts_saver::~dk_mpeg2ts_saver(void)
{
	::close_file(_file);
}

dk_mpeg2ts_saver::ERR_CODE dk_mpeg2ts_saver::on_recv_ts_stream(uint8_t * ts, size_t stream_size)
{
#if 0
	static int index = 1;
	char filename_prefix[100] = { 0, };
	_snprintf_s(filename_prefix, sizeof(filename_prefix), "test_%d.ts", index);

	HANDLE handle = ::open_file_write(filename_prefix);
	DWORD nbytes = 0;
	::WriteFile(handle, ts, stream_size, &nbytes, NULL);
	::close_file(handle);

	index++;
#else
	DWORD nbytes = 0;
	::WriteFile(_file, ts, stream_size, &nbytes, NULL);
#endif
	return dk_mpeg2ts_saver::ERR_CODE_SUCCESS;
}