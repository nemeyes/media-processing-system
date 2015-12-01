#include "mpeg2ts_demuxer.h"

mpeg2ts_demuxer::mpeg2ts_demuxer(void)
	: _ts_buffer_pos(0)
	, _ts_buffer_begin_pcr(0)
	, _ts_buffer_end_pcr(0)
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
		uint8_t * adapation_field = nullptr;
		int32_t adapation_field_length = 0;

		uint8_t * payload = nullptr;
		int32_t payload_length = 0;

		ts_packet_header_t ts_packet_header;
		if (ibuff[0] != 0x47)
			return dk_mpeg2ts_demuxer::ERR_CODE_FAILED;

		ts_packet_header.sync_byte = ibuff[0];
		//1000 0000
		ts_packet_header.transport_error_indicator = (ibuff[1] & 0x80) >> 7;
		ts_packet_header.payload_unit_start_indicator = (ibuff[1] & 0x40) >> 6;
		ts_packet_header.transport_priority = (ibuff[1] & 0x20) >> 5;
		ts_packet_header.pid = ((ibuff[1] & 0x1F) << 8) | ibuff[2];
		ts_packet_header.transport_scrambling_control = (ibuff[3] & 0xC0) >> 4;
		ts_packet_header.adaptation_field_control = (ibuff[3] & 0x30) >> 4;
		ts_packet_header.continuity_counter = (ibuff[3] & 0x0F);

		if (ts_packet_header.pid == NULL_PACKET)
		{
			adapation_field = nullptr;
			adapation_field_length = 0;
			payload = nullptr;
			payload_length = 0;

			continue;
		}

		switch (ts_packet_header.adaptation_field_control)
		{
		case 0:



		}


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