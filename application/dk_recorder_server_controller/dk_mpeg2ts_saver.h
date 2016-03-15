#pragma once
#include <windows.h>
#include <cstdint>
#include <dk_ff_mpeg2ts_muxer.h>


class dk_mpeg2ts_saver : public dk_ff_mpeg2ts_muxer
{
public:
	dk_mpeg2ts_saver(void);
	~dk_mpeg2ts_saver(void);

	dk_mpeg2ts_saver::ERR_CODE recv_ts_stream_callback(uint8_t * ts, size_t stream_size);


private:
	HANDLE _file;

};