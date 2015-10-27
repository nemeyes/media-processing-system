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
	ffmpeg_decoder_core(void);
	~ffmpeg_decoder_core(void);

	dk_ff_video_decoder::ERR_CODE initialize_decoder(dk_ff_video_decoder::CONFIGURATION_T * config);
	dk_ff_video_decoder::ERR_CODE release_decoder(void);
	dk_ff_video_decoder::ERR_CODE decode(DK_VIDEO_ENTITY_T * bitstream);

private:
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