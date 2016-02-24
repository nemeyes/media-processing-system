#ifndef _FFMPEG_DECODER_CORE_H_
#define _FFMPEG_DECODER_CORE_H_

#include "dk_ff_video_decoder.h"

struct AVPacket;
struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct SwsContext;
class ffmpeg_decoder
{
public:
	ffmpeg_decoder(dk_ff_video_decoder * front);
	~ffmpeg_decoder(void);

	dk_ff_video_decoder::ERR_CODE initialize_decoder(dk_ff_video_decoder::configuration_t * config);
	dk_ff_video_decoder::ERR_CODE release_decoder(void);
	dk_ff_video_decoder::ERR_CODE decode(dk_ff_video_decoder::dk_video_entity_t * encoded, dk_ff_video_decoder::dk_video_entity_t * decoded);
	dk_ff_video_decoder::ERR_CODE decode(dk_ff_video_decoder::dk_video_entity_t * encoded);
	dk_ff_video_decoder::ERR_CODE get_queued_data(dk_ff_video_decoder::dk_video_entity_t * decoded);

private:
	dk_ff_video_decoder::configuration_t * _config;
	dk_ff_video_decoder * _front;
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












#endif