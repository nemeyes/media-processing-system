#include "mpeg2ts_segment.h"
#include "mpeg2ts_constants.h"

mpeg2ts_segment::mpeg2ts_segment(void)
{

}

mpeg2ts_segment::mpeg2ts_segment(uint8_t * segment, uint32_t sequence)
{
	initialize(segment, sequence);
}

mpeg2ts_segment::~mpeg2ts_segment(void)
{
	release();
}

dk_mpeg2ts_segmenter::ERR_CODE mpeg2ts_segment::initialize(uint8_t * segment, uint64_t sequence)
{
	dk_mpeg2ts_segmenter::ERR_CODE err_code = dk_mpeg2ts_segmenter::ERR_CODE_FAIL;
	release();
	
	_data = nullptr;
	_sequence = 0;
	_stream_id = 0;
	_stream_number = 0;
	_pes_packet_length = 0;
	_is_video_stream = false;
	_is_audio_stream = false;
	_pts_defined = false;

	if (segment[0] != TS_PACKET_SYNC_BYTE)
		return dk_mpeg2ts_segmenter::ERR_CODE_SEGMENT_NOT_VALID;

	_data = segment;
	_sequence = sequence;

	err_code = parse_header();
	if (err_code != dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS)
		return err_code;

	if (pes_packet_found())
	{
		err_code = parse_pes_packet();
		if (err_code != dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS)
			return err_code;
	}

	if (program_association_section_found())
	{
		err_code = parse_program_association_section();
		if (err_code != dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS)
			return err_code;
	}

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_segmenter::ERR_CODE mpeg2ts_segment::release(void)
{
	if (_pas.program_numbers)
		free(_pas.program_numbers);
	if (_pas.network_pids)
		free(_pas.network_pids);
	if (_pas.program_map_pids)
		free(_pas.program_map_pids);

	//_pas.program_numbers = nullptr;
	//_pas.network_pids = nullptr;
	//_pas.program_map_pids = nullptr;

	memset(&_ts_packet_header, 0x00, sizeof(_ts_packet_header));
	memset(&_pes_packet_header, 0x00, sizeof(_pes_packet_header));

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

double mpeg2ts_segment::pts_to_second(void)
{
	return (double)_pes_packet_header.pts / TIMER_IN_HZ;
}

bool mpeg2ts_segment::pts_defined(void)
{
	return _pts_defined;
}

dk_mpeg2ts_segmenter::ts_packet_header_t * mpeg2ts_segment::ts_packet_header(void)
{
	return &_ts_packet_header;
}

dk_mpeg2ts_segmenter::pes_packet_header_t * mpeg2ts_segment::pes_packet_header(void)
{
	return &_pes_packet_header;
}

dk_mpeg2ts_segmenter::program_association_section_t * mpeg2ts_segment::program_association_section(void)
{
	return &_pas;
}

dk_mpeg2ts_segmenter::ERR_CODE mpeg2ts_segment::parse_header(void)
{
	_ts_packet_header.sync_byte = _data[0];
	//1000 0000
	_ts_packet_header.transport_error_indicator = (_data[1] & 0x80) >> 7;
	_ts_packet_header.payload_unit_start_indicator = (_data[1] & 0x40) >> 6;
	_ts_packet_header.transport_priority = (_data[1] & 0x20) >> 5;
	_ts_packet_header.pid = ((_data[1] & 0x1F) << 8) | _data[2];
	_ts_packet_header.transport_scrambling_control = (_data[3] & 0xC0) >> 4;
	_ts_packet_header.adaptation_field_control = (_data[3] & 0x30) >> 4;
	_ts_packet_header.continuity_counter = (_data[3] & 0x0F);

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_segmenter::ERR_CODE mpeg2ts_segment::parse_pes_packet(void)
{
	dk_mpeg2ts_segmenter::ERR_CODE err_code = dk_mpeg2ts_segmenter::ERR_CODE_FAIL;
	size_t offset = pes_packet_offset();

	_stream_id = _data[offset + PES_PACKET_STREAM_ID_OFFSET];
	_pes_packet_length = (_data[offset + PES_PACKET_LENGTH_OFFSET] & 0xff) << 8 | _data[offset + PES_PACKET_LENGTH_OFFSET + 1];
	_is_video_stream = (_stream_id & PES_PACKET_VIDEO_STREAM_ID_MASK) == PES_PACKET_VIDEO_STREAM_ID;
	_is_audio_stream = (_stream_id & PES_PACKET_AUDIO_STREAM_ID_MASK) == PES_PACKET_AUDIO_STREAM_ID;

	if (_is_video_stream)
		_stream_number = _stream_id & PES_PACKET_VIDEO_STREAM_NUMBER_MASK;

	if (_is_audio_stream)
		_stream_number = _stream_id & PES_PACKET_AUDIO_STREAM_NUMBER_MASK;


	//get 1st byte
	uint8_t info = _data[offset + PES_PACKET_STREAM_INFO_OFFSET + 0];
	_pes_packet_header.pes_scrambling_control = (info & 0x30) >> 4;
	_pes_packet_header.pes_priority = (info & 0x08) != 0;
	_pes_packet_header.data_alignment_indicator = (info & 0x04) != 0;
	_pes_packet_header.copyright = (info & 0x02) != 0;
	_pes_packet_header.original_or_copy = (info & 0x01);

	//get 2nd byte
	info = _data[offset + PES_PACKET_STREAM_INFO_OFFSET + 1];
	_pes_packet_header.pts_dts_flags = (info & 0xC0) >> 6;
	_pes_packet_header.escr_flag = (info & 0x20) != 0;
	_pes_packet_header.es_rate_flag = (info & 0x10) != 0;
	_pes_packet_header.dsm_trick_mode_flag = (info & 0x08) != 0;
	_pes_packet_header.additional_copy_info_flag = (info & 0x04) != 0;
	_pes_packet_header.pes_crc_flag = (info & 0x02) != 0;
	_pes_packet_header.pes_extension_flag = (info & 0x01) != 0;

	//get 3rd byte
	_pes_packet_header.pes_header_data_length = _data[offset + PES_PACKET_STREAM_INFO_OFFSET + 2];

	if ((_pes_packet_header.pts_dts_flags == PES_PACKET_PTS_ONLY_VALUE) ||
		(_pes_packet_header.pts_dts_flags == PES_PACKET_PTS_AND_DTS_VALUE))
	{

		//1st byte
		info = _data[offset + PES_PACKET_PTS_DST_OFFSET + 0];
		_pes_packet_header.pts = (info & 0x0E) << 29;

		//2nd byte(8bit)
		info = _data[offset + PES_PACKET_PTS_DST_OFFSET + 1];
		_pes_packet_header.pts |= (info << 22);

		//3rd byte(7bit)
		info = _data[offset + PES_PACKET_PTS_DST_OFFSET + 2];
		_pes_packet_header.pts |= ((info & 0xFE) << 14);

		//4th byte(8bit)
		info = _data[offset + PES_PACKET_PTS_DST_OFFSET + 3];
		_pes_packet_header.pts |= (info << 7);

		//5th byte(7bit)
		info = _data[offset + PES_PACKET_PTS_DST_OFFSET + 4];
		_pes_packet_header.pts |= (info >> 1);

		_pts_defined = true;
	}
	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_segmenter::ERR_CODE mpeg2ts_segment::parse_program_association_section(void)
{
	size_t offset = program_assocation_section_offset();

	//1st byte
	_pas.table_id = _data[offset + 0];

	//2nd byte
	uint8_t info = _data[offset + 1];
	_pas.section_syntax_indicator = (info & 0x80) != 0;
	_pas.section_length = (info & 0x0F) << 8;

	//3rd byte
	info = _data[offset + 2];
	_pas.section_length |= _data[offset + 2];
	int32_t number_of_programs = (_pas.section_length - 9) / 4;

	//4th & 5th bytes
	_pas.transport_stream_id = (_data[offset + 3] << 8) | _data[offset + 4];

	//6th byte
	info = _data[offset + 5];
	_pas.version_number = (info & 0x3E) >> 1;
	_pas.current_next_indicator = (info & 0x01);

	//7th byte
	_pas.section_number = offset + 6;
	_pas.last_section_number = offset + 7;

#if 0
	for (int32_t index = 0; index < number_of_programs; index++)
	{
		size_t base_offset = offset + 8 + index * 4;
		_pas.program_numbers[index] = (_data[base_offset] << 8) | _data[base_offset + 1];

		uint16_t pid = ((_data[base_offset + 2] << 8) | _data[base_offset + 3]) & 0x1FFF;
		//first 3bit are reserved
		if (_pas.program_numbers[index] == 0)
		{
			_pas.network_pids[index] = pid;
		}
		else
		{
			_pas.program_map_pids[index] = pid;
		}
	}
#else
	if (number_of_programs > 0)
	{
		_pas.program_numbers = (uint16_t*)malloc(number_of_programs*sizeof(uint16_t));
		_pas.network_pids = (uint16_t*)malloc(number_of_programs*sizeof(uint16_t));
		_pas.program_map_pids = (uint16_t*)malloc(number_of_programs*sizeof(uint16_t));
	}
	for (int32_t index = 0; index < number_of_programs; index++)
	{
		size_t base_offset = offset + 8 + index * 4;
		_pas.program_numbers[index] = (_data[base_offset] << 8) | _data[base_offset + 1];

		uint16_t pid = ((_data[base_offset + 2] << 8) | _data[base_offset + 3]) & 0x1FFF;
		//first 3bit are reserved
		if (_pas.program_numbers[index] == 0)
		{
			_pas.network_pids[index] = pid;
		}
		else
		{
			_pas.program_map_pids[index] = pid;
		}
	}
#endif	

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

bool mpeg2ts_segment::pes_packet_found(void)
{
	if (((_ts_packet_header.adaptation_field_control == TS_PACKET_ADAPTATION_FIELD_PAYLOAD_ONLY) || 
		(_ts_packet_header.adaptation_field_control == TS_PACKET_ADAPTATION_FIELD_WITH_PAYLOAD)) &&
		_ts_packet_header.payload_unit_start_indicator)
	{
		size_t offset = pes_packet_offset();

		uint32_t pes_prefix = 0;
		pes_prefix = (_data[offset + 0] & 0xFF) << 16 | (_data[offset + 1] & 0xFF) << 8 | (_data[offset + 2]);

		return pes_prefix == TS_PACKET_PES_PREFIX;
	}
	else
		return false;
}

bool mpeg2ts_segment::program_association_section_found(void)
{
	return _ts_packet_header.pid == 0;
}

size_t mpeg2ts_segment::pes_packet_offset(void)
{
	/*
	#    offset 0: packet_start_code_prefix = > 24 bits
	#    offset 3: stream_id = > 8 bits
	#    offset 4: PES_packet_length = > 16 bits
	*/
	_stream_id = _data[3];
	return TS_PACKET_HEADER_SIZE + adaptation_field_size();
}

size_t mpeg2ts_segment::adaptation_field_size(void)
{
	size_t length = adaptation_field_length();
	if (length != 0)
		return length + 1;
	else
		return 0;
}

size_t mpeg2ts_segment::adaptation_field_length(void)
{
	if ((_ts_packet_header.adaptation_field_control == TS_PACKET_ADAPTATION_FIELD_ONLY) ||
		(_ts_packet_header.adaptation_field_control == TS_PACKET_ADAPTATION_FIELD_WITH_PAYLOAD))
		                                               
	{
		/*
		# adaptation field present, the length is defined on the first byte after
		# the ts packet header(4 bytes)
		*/
		return (size_t)_data[TS_PACKET_ADAPTATION_FIELD_LENGTH_OFFSET];
	}
	else
	{
		return 0;
	}
}

size_t mpeg2ts_segment::program_assocation_section_offset(void)
{
	if (_ts_packet_header.payload_unit_start_indicator)
		return TS_PACKET_HEADER_SIZE + adaptation_field_size() + 1;
	else
		return TS_PACKET_HEADER_SIZE + adaptation_field_size();
}