#ifndef _FFMPEG_DECODER_CORE_H_
#define _FFMPEG_DECODER_CORE_H_

#include "dk_ff_video_decoder.h"

struct AVPacket;
struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct SwsContext;
class ffmpeg_decoder_core
{
public:
	ffmpeg_decoder_core(dk_ff_video_decoder * front);
	~ffmpeg_decoder_core(void);

	dk_ff_video_decoder::ERR_CODE initialize_decoder(dk_ff_video_decoder::configuration_t * config);
	dk_ff_video_decoder::ERR_CODE release_decoder(void);
	dk_ff_video_decoder::ERR_CODE decode(dk_video_entity_t * encoded, dk_video_entity_t * decoded);

private:
	dk_ff_video_decoder * _front;
	AVPacket * _av_packet;
	AVFormatContext * _av_format_ctx;
	AVCodecContext * _av_codec_ctx;
	AVCodec * _av_codec;
	AVFrame * _av_frame;
	AVFrame * _av_video_frame;
	SwsContext * _sws_ctx;
	uint8_t * _buffer;
	int	 _stride;
};












#endif