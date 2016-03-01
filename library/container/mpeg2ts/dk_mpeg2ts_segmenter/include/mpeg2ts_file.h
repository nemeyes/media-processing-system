#ifndef _MPEG2TS_FILE_H_
#define _MPEG2TS_FILE_H_

#include "dk_mpeg2ts_segmenter.h"
#include "mpeg2ts_segment.h"
#include "mpeg2ts_constants.h"

class mpeg2ts_file
{
public:
	mpeg2ts_file(void);
	~mpeg2ts_file(void);

	dk_mpeg2ts_segmenter::ERR_CODE initialize(wchar_t * filepath);
	dk_mpeg2ts_segmenter::ERR_CODE release(void);
	mpeg2ts_segment * next_segment(void);

	int64_t file_size(void);
	int64_t segment_count(void);

private:
	mpeg2ts_segment * get_segment(int64_t segment_number);

	HANDLE _ts_file;
	uint8_t _ts_buffer[TS_PACKET_SIZE];

	int64_t _file_size;
	int64_t _segment_count;
	int64_t _segment_number;



};



#endif