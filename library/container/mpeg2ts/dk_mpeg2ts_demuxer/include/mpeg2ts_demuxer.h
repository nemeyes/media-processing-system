#ifndef _MPEG2TS_DEMUXER_H_
#define _MPEG2TS_DEMUXER_H_

#include "dk_mpeg2ts_demuxer.h"


#define TS_PACKET_LENGTH 188
#define TS_BUFFER_SIZE	TS_PACKET_LENGTH*10

#define TS_PACKET_HEADER_LENGTH	4

class mpeg2ts_demuxer
{
public:

	typedef enum _PID_TABLE
	{
		PAT = 0x0000, //Program Association Table
		CAT = 0x0001, //Conditional Access Table
		TSDT = 0x0002, //Transport Stream Description Table
		NULL_PACKET = 0x1FFF
	} PID_TABLE;

	typedef struct _adaptation_field_t
	{
		uint8_t adaptation_field_length;
		uint8_t discontinuity_indicator;
		uint8_t random_access_indicator;
		uint8_t elementary_stream_priority_indicator;
		uint8_t PCR_flag;
		uint8_t OPCR_flag;
		uint8_t splicing_point_flag;
		uint8_t transport_private_data_flag;
		uint8_t adaptation_field_extension_flag;
		union _pcr_t
		{
			uint64_t program_clock_reference_base		: 33;
			uint64_t reserved1							: 6;
			uint64_t program_clock_reference_extension	: 9;
		} PCR;
		union _opcr_t
		{
			uint64_t original_program_clock_reference_base		: 33;
			uint64_t reserved2									: 6;
			uint64_t original_program_clock_reference_extensin	: 9;
		} OPCR;

		uint8_t splice_countdown;
		uint8_t transport_private_data_length;
		uint8_t * private_data_byte;

		uint8_t adaptation_field_extension_legnth;
		uint8_t ltw_flag;
		uint8_t piecewise_rate_flag;
		uint8_t seamless_splice_flag;
		uint8_t reserved3;

		uint8_t ltw_valid_flag;
		uint16_t ltw_offset;

		uint8_t reserved4;
		uint32_t piecewise_rate;

		uint8_t splice_type;
		uint8_t DTS_next_AU_32_30;
		uint8_t marker_bit3;
		uint8_t DTS_next_AU_29_15;
		uint8_t marker_bit2;
		uint8_t DTS_next_AU_14_0;
		uint8_t marker_bit1;

	} adaptation_field_t;


	typedef struct _ts_packet_header_t
	{
		uint8_t		sync_byte;
		uint8_t		transport_error_indicator;
		uint8_t		payload_unit_start_indicator;
		uint8_t		transport_priority;
		uint16_t	pid;
		uint8_t		transport_scrambling_control;
		uint8_t		adaptation_field_control;
		uint8_t		continuity_counter;
	} ts_packet_header_t;

	mpeg2ts_demuxer(void);
	~mpeg2ts_demuxer(void);

	dk_mpeg2ts_demuxer::ERR_CODE initialize(void);
	dk_mpeg2ts_demuxer::ERR_CODE release(void);

	dk_mpeg2ts_demuxer::ERR_CODE demultiplexing(uint8_t * buffer, size_t nb);



private:
	dk_mpeg2ts_demuxer::ERR_CODE parse_ts_packet(uint8_t * buffer);



	uint8_t _ts_buffer[TS_PACKET_LENGTH];
	size_t _ts_buffer_pos;


	uint64_t _ts_buffer_begin_pcr;
	uint64_t _ts_buffer_end_pcr;
};








#endif