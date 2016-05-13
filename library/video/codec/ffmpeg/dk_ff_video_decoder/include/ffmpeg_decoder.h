#ifndef _FFMPEG_DECODER_H_
#define _FFMPEG_DECODER_H_

#include "dk_ff_video_decoder.h"

struct AVPacket;
struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct SwsContext;

namespace debuggerking
{
	class ffmpeg_core
	{
	public:
		ffmpeg_core(ff_video_decoder * front);
		virtual ~ffmpeg_core(void);

		int32_t initialize_decoder(ff_video_decoder::configuration_t * config);
		int32_t release_decoder(void);
		int32_t decode(ff_video_decoder::entity_t * encoded, ff_video_decoder::entity_t * decoded);
		int32_t decode(ff_video_decoder::entity_t * encoded);
		int32_t get_queued_data(ff_video_decoder::entity_t * decoded);

	private:
		ff_video_decoder::configuration_t * _config;
		ff_video_decoder * _front;
		AVPacket * _av_packet;
		AVFormatContext * _av_format_ctx;
		AVCodecContext * _av_codec_ctx;
		AVCodec * _av_codec;
		AVFrame * _av_frame;
		AVFrame * _av_video_frame;
		SwsContext * _sws_ctx;
		uint8_t * _buffer4resize;
		int	 _stride;

		bool _insert_start_code;
		//uint8_t * _buffer4start_code;
	};
};













#endif