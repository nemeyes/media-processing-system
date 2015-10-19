#ifndef _FF_MPEG2TS_MUXER_CORE_H_
#define _FF_MPEG2TS_MUXER_CORE_H_

extern "C"
{
	/*
	avcodec.lib
	avfilter.lib
	avformat.lib
	avutil.lib
	swresample.lib
	swscale.lib
	*/
	#include <libavcodec/avcodec.h>
	#include <libavfilter/avfilter.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libswresample/swresample.h>
	#include <libswscale/swscale.h>
}

#include "dk_ff_mpeg2ts_muxer.h"

class ff_mpeg2ts_muxer_core
{
public:
	ff_mpeg2ts_muxer_core(void);
	~ff_mpeg2ts_muxer_core(void);

	dk_ff_mpeg2ts_muxer::ERR_CODE initialize(dk_ff_mpeg2ts_muxer::configuration_t config);
	dk_ff_mpeg2ts_muxer::ERR_CODE release(void);
	dk_ff_mpeg2ts_muxer::ERR_CODE put_video_stream(unsigned char * buffer, size_t nb, long long pts, bool keyframe);

private:
	AVOutputFormat * m_pFmt;
	AVFormatContext * m_pFormatCtx;
	AVStream * m_pVideoStream;

	unsigned char _extra_data[500];
	int _extra_data_size;

	bool m_bInited;
	int m_nProcessedFramesNum;
};



#endif