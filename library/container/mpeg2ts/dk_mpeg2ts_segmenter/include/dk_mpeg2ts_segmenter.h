#ifndef _DK_MPEG2TS_SEGMENTER_H_
#define _DK_MPEG2TS_SEGMENTER_H_

#include <cstdint>
#include <vector>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LIB)
#  define EXP_DLL __declspec(dllexport)
# else
#  define EXP_DLL __declspec(dllimport)
# endif
#else
# define EXP_DLL
#endif

class m3u8_file;
class mpeg2ts_file;
class mpeg2ts_segment;
class EXP_DLL dk_mpeg2ts_segmenter
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAIL,
		ERR_CODE_SEGMENT_NOT_VALID,
		ERR_CODE_INCORRECT_TS_FILE_SIZE
	} ERR_CODE;

	typedef struct EXP_DLL _ts_packet_header_t
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

	typedef struct EXP_DLL _pes_packet_header_t
	{
		uint32_t	packet_start_code_prefix;
		uint8_t		stream_id;
		uint16_t	pes_packet_length;
		uint8_t		pes_scrambling_control;
		uint8_t		pes_priority;
		uint8_t		data_alignment_indicator;
		uint8_t		copyright;
		uint8_t		original_or_copy;
		uint8_t		pts_dts_flags;
		uint8_t		escr_flag;
		uint8_t		es_rate_flag;
		uint8_t		dsm_trick_mode_flag;
		uint8_t		additional_copy_info_flag;
		uint8_t		pes_crc_flag;
		uint8_t		pes_extension_flag;
		uint8_t		pes_header_data_length;
		uint64_t	pts;
		uint64_t	dts;
	} pes_packet_header_t;

	typedef struct EXP_DLL _program_association_section_t
	{
		uint8_t		table_id;
		uint8_t		section_syntax_indicator;
		uint16_t	section_length;
		uint16_t	transport_stream_id;
		uint8_t		version_number;
		uint8_t		current_next_indicator;
		uint8_t		section_number;
		uint8_t		last_section_number;
		
#if 0
		std::vector<uint16_t> network_pids;
		std::vector<uint16_t> program_map_pids;
#else
		uint16_t *	program_numbers;
		uint16_t *	network_pids;
		uint16_t *	program_map_pids;
#endif

	} program_association_section_t;

	dk_mpeg2ts_segmenter(void);
	~dk_mpeg2ts_segmenter(void);

	dk_mpeg2ts_segmenter::ERR_CODE initialize(wchar_t * ifile_path, wchar_t * ofile_name, wchar_t * opath, wchar_t * stream_id, int32_t duration);
	dk_mpeg2ts_segmenter::ERR_CODE release(void);

	dk_mpeg2ts_segmenter::ERR_CODE segment(void);


private:
	void copy_segments_to_file(wchar_t * ifile_path, wchar_t * ofile_path, int64_t from, int64_t to);
	void append_segments_to_file(wchar_t * ifile_path, wchar_t * ofile_path, int64_t from, int64_t to);

private:
	m3u8_file * _m3u8_file;
	mpeg2ts_file * _ts_file;
	wchar_t _ifile_path[500];
	//wchar_t _ofile_path[500];

	wchar_t _ofile_name[500];
	wchar_t _opath[500];
	wchar_t _stream_id[500];
	int32_t _duration;


};










#endif