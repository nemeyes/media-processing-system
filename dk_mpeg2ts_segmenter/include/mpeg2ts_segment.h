#ifndef _MPEG2TS_SEGMENT_H_
#define _MPEG2TS_SEGMENT_H_

#include "dk_mpeg2ts_segmenter.h"

class mpeg2ts_segment
{
public:
	mpeg2ts_segment(void);
	mpeg2ts_segment(uint8_t * segment, uint32_t sequnece);
	~mpeg2ts_segment(void);

	dk_mpeg2ts_segmenter::ERR_CODE initialize(uint8_t * segment, uint64_t sequnece);
	dk_mpeg2ts_segmenter::ERR_CODE release(void);

	double pts_to_second(void);
	bool pts_defined(void);

	dk_mpeg2ts_segmenter::ts_packet_header_t * ts_packet_header(void);
	dk_mpeg2ts_segmenter::pes_packet_header_t * pes_packet_header(void);
	dk_mpeg2ts_segmenter::program_association_section_t * program_association_section(void);

private:
	dk_mpeg2ts_segmenter::ERR_CODE parse_header(void);
	dk_mpeg2ts_segmenter::ERR_CODE parse_pes_packet(void);
	dk_mpeg2ts_segmenter::ERR_CODE parse_program_association_section(void);

	bool pes_packet_found(void);
	bool program_association_section_found(void);

	size_t pes_packet_offset(void);
	size_t adaptation_field_size(void);
	size_t adaptation_field_length(void);
	size_t program_assocation_section_offset(void);

private:
	uint8_t * _data;
	uint64_t _sequence;
	uint8_t _stream_id;
	uint8_t _stream_number;
	size_t _pes_packet_length;
	bool _is_video_stream;
	bool _is_audio_stream;
	bool _pts_defined;

	dk_mpeg2ts_segmenter::ts_packet_header_t _ts_packet_header;
	dk_mpeg2ts_segmenter::pes_packet_header_t _pes_packet_header;
	dk_mpeg2ts_segmenter::program_association_section_t _pas;

};
















#endif