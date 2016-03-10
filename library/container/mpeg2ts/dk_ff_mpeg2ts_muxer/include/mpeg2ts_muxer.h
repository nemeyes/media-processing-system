#ifndef _MPEG2TS_MUXER_H_
#define _MPEG2TS_MUXER_H_

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavfilter/avfilter.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libswresample/swresample.h>
	#include <libswscale/swscale.h>
}

#include "dk_ff_mpeg2ts_muxer.h"

class mpeg2ts_muxer
{
public:
	mpeg2ts_muxer(dk_ff_mpeg2ts_muxer * front);
	~mpeg2ts_muxer(void);

	dk_ff_mpeg2ts_muxer::ERR_CODE initialize(dk_ff_mpeg2ts_muxer::configuration_t * config);
	dk_ff_mpeg2ts_muxer::ERR_CODE release(void);
	dk_ff_mpeg2ts_muxer::STATE state(void);

	dk_ff_mpeg2ts_muxer::ERR_CODE put_video_stream(uint8_t * buffer, size_t nb, int64_t ts, bool keyframe);

private:
	static int32_t on_write_packet(void * opaque, uint8_t * buffer, int32_t size);

private:
	dk_ff_mpeg2ts_muxer * _front;
	dk_ff_mpeg2ts_muxer::configuration_t * _config;

	dk_ff_mpeg2ts_muxer::STATE _state;

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