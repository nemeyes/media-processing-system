#include "mpeg2ts_demuxer.h"

mpeg2ts_demuxer::mpeg2ts_demuxer(void)
	: _ts_buffer_pos(0)
{

}

mpeg2ts_demuxer::~mpeg2ts_demuxer(void)
{

}

dk_mpeg2ts_demuxer::ERR_CODE mpeg2ts_demuxer::initialize(void)
{

	return dk_mpeg2ts_demuxer::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_demuxer::ERR_CODE mpeg2ts_demuxer::release(void)
{

	return dk_mpeg2ts_demuxer::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_demuxer::ERR_CODE mpeg2ts_demuxer::demultiplexing(uint8_t * buffer, size_t nb)
{
	if (_ts_buffer_pos + nb > TS_BUFFER_SIZE)
		return dk_mpeg2ts_demuxer::ERR_CODE_FAILED;

	memcpy(_ts_buffer + _ts_buffer_pos, buffer, nb);

	uint8_t * inbuffer = _ts_buffer;
	size_t inbuffer_size = _ts_buffer_pos + nb;
	size_t inbuffer_pos = 0;
	while ((inbuffer_size - inbuffer_pos) >= TS_PACKET_LENGTH)
	{
		ts_packet_t ts_packet;
		memcpy(&ts_packet.header, inbuffer, TS_PACKET_HEADER_LENGTH);
		inbuffer += TS_PACKET_HEADER_LENGTH;





		inbuffer_pos += TS_PACKET_LENGTH;
	}

	if (inbuffer_size > inbuffer_pos)
	{
		_ts_buffer_pos = inbuffer_size - inbuffer_pos;
		memmove(_ts_buffer, _ts_buffer + inbuffer_pos, _ts_buffer_pos);
	}
	return dk_mpeg2ts_demuxer::ERR_CODE_SUCCESS;
}