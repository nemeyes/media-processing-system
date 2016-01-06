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
	ff_mpeg2ts_muxer_core(dk_ff_mpeg2ts_muxer * front);
	~ff_mpeg2ts_muxer_core(void);

	dk_ff_mpeg2ts_muxer::ERR_CODE initialize(dk_ff_mpeg2ts_muxer::configuration_t * config);
	dk_ff_mpeg2ts_muxer::ERR_CODE release(void);
	dk_ff_mpeg2ts_muxer::ERR_CODE put_video_stream(uint8_t * buffer, size_t nb, int64_t pts, bool keyframe);

private:
	dk_ff_mpeg2ts_muxer * _front;

	static int32_t on_write_packet(void * opaque, uint8_t * buffer, int32_t size);


	dk_ff_mpeg2ts_muxer::configuration_t _config;
	uint8_t * _avio_buffer;
	int32_t _avio_buffer_size;

	uint8_t * _buffered_ts_output;
	int32_t _buffered_ts_output_pos;
	int32_t _buffered_ts_output_size;

	AVOutputFormat * _ofmt;
	
	AVFormatContext * _format_ctx;
	AVStream * _vstream;
	AVIOContext * _avio_ctx;
	

	bool _is_initialized;
	int32_t _nframes;
};



#endif