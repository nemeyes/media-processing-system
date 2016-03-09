#pragma once
#include <windows.h>
#include <cstdint>
#include <dk_ff_mpeg2ts_muxer.h>


class dk_mpeg2ts_recorder : public dk_ff_mpeg2ts_muxer
{
public:
	dk_mpeg2ts_recorder(const char * id);
	~dk_mpeg2ts_recorder(void);

	long long get_file_size(void);
	dk_mpeg2ts_recorder::ERR_CODE recv_ts_stream_callback(uint8_t * ts, size_t stream_size);

private:
	HANDLE _file;

};