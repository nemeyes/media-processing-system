#ifndef _MPEG2TS_DEMUXER_H_
#define _MPEG2TS_DEMUXER_H_

#include "dk_mpeg2ts_demuxer.h"


#define TS_PACKET_LENGTH 188
#define TS_BUFFER_SIZE	TS_PACKET_LENGTH*10

#define TS_PACKET_HEADER_LENGTH	4

class mpeg2ts_demuxer
{
public:
	typedef struct _ts_packet_t
	{
		union _ts_packet_header_t
		{
			uint32_t	sync_byte						:8;
			uint32_t	transport_error_indicator		:1;
			uint32_t	payload_unit_start_indicator	:1;
			uint32_t	transport_priority				:1;
			uint32_t	PID								:13;
			uint32_t	transport_scrambling_control	:2;
			uint32_t	adaptation_field_control		:2;
			uint32_t	continuity_counter				:4;
		} header;
	} ts_packet_t;


	mpeg2ts_demuxer(void);
	~mpeg2ts_demuxer(void);

	dk_mpeg2ts_demuxer::ERR_CODE initialize(void);
	dk_mpeg2ts_demuxer::ERR_CODE release(void);

	dk_mpeg2ts_demuxer::ERR_CODE demultiplexing(uint8_t * buffer, size_t nb);



private:
	dk_mpeg2ts_demuxer::ERR_CODE parse_ts_packet(uint8_t * buffer);



	uint8_t _ts_buffer[TS_PACKET_LENGTH];
	size_t _ts_buffer_pos;
};








#endif