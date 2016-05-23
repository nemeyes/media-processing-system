#ifndef _FFMPEG_TSMUXER_H_
#define _FFMPEG_TSMUXER_H_

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavfilter/avfilter.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libswresample/swresample.h>
	#include <libswscale/swscale.h>
}

#include "dk_ff_tsmuxer.h"

namespace debuggerking
{
	class ffmpeg_tsmuxer
	{
	public:
		ffmpeg_tsmuxer(ff_tsmuxer * front);
		~ffmpeg_tsmuxer(void);

		int32_t initialize(ff_tsmuxer::configuration_t * config);
		int32_t release(void);
		ff_tsmuxer::tsmuxer_state state(void);
		int32_t put_video_stream(const uint8_t * buffer, size_t nb, int64_t timestamp, bool keyframe);

	private:
		static int32_t on_write_packet(void * opaque, uint8_t * buffer, int32_t size);

	private:
		ff_tsmuxer * _front;
		ff_tsmuxer::configuration_t * _config;

		ff_tsmuxer::tsmuxer_state _state;

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
};


#endif