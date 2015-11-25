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

	uint8_t * ibuff = _ts_buffer;
	size_t ibuff_size = _ts_buffer_pos + nb;
	size_t ibuff_pos = 0;
	while ((ibuff_size - ibuff_pos) >= TS_PACKET_LENGTH)
	{
		ts_packet_t ts_packet;
		if (ibuff[0] != 0x47)
			return dk_mpeg2ts_demuxer::ERR_CODE_FAILED;

		ts_packet.header.sync_byte = ibuff[0];
		//1000 0000
		ts_packet.header.transport_error_indicator = (ibuff[1] & 0x80) >> 7;
		ts_packet.header.payload_unit_start_indicator = (ibuff[1] & 0x40) >> 6;
		ts_packet.header.transport_priority = (ibuff[1] & 0x20) >> 5;
		ts_packet.header.PID = ((ibuff[1] & 0x1F) << 8) | ibuff[2];
		if (ts_packet.header.PID == NULL_PACKET)
		{

		}
		ts_packet.header.transport_scrambling_control = (ibuff[3] & 0xC0) >> 4;
		ts_packet.header.adaptation_field_control = (ibuff[3] & 0x30) >> 4;
		ts_packet.header.continuity_counter = (ibuff[3] & 0x0F);




		ibuff += TS_PACKET_HEADER_LENGTH;





		ibuff_pos += TS_PACKET_LENGTH;
	}

	if (ibuff_size > ibuff_pos)
	{
		_ts_buffer_pos = ibuff_size - ibuff_pos;
		memmove(_ts_buffer, _ts_buffer + ibuff_pos, _ts_buffer_pos);
	}
	return dk_mpeg2ts_demuxer::ERR_CODE_SUCCESS;
}