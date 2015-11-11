#ifndef _MPEG2TS_DEMUXER_H_
#define _MPEG2TS_DEMUXER_H_

#include "dk_mpeg2ts_demuxer.h"

class mpeg2ts_demuxer
{
public:
	mpeg2ts_demuxer(void);
	~mpeg2ts_demuxer(void);

	dk_mpeg2ts_demuxer::ERR_CODE initialize(void);
	dk_mpeg2ts_demuxer::ERR_CODE release(void);

	dk_mpeg2ts_demuxer::ERR_CODE demultiplexing(uint8_t * buffer, size_t nb);



};








#endif