#ifndef _MP3_DECODER_H_
#define _MP3_DECODER_H_

#include "dk_ff_mp3_decoder.h"

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavfilter/avfilter.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libswresample/swresample.h>
	#include <libswscale/swscale.h>
}

#define AUDIO_INBUF_SIZE 20480

namespace debuggerking
{
	class ffmpeg_core
	{
	public:
		ffmpeg_core(ff_mp3_decoder * front);
		~ffmpeg_core(void);

		int32_t initialize_decoder(ff_mp3_decoder::configuration_t * config);
		int32_t release_decoder(void);
		int32_t decode(ff_mp3_decoder::entity_t * encoded, ff_mp3_decoder::entity_t * pcm);
		int32_t decode(audio_decoder::entity_t * encoded);
		int32_t get_queued_data(audio_decoder::entity_t * pcm);

	private:
		ff_mp3_decoder * _front;
		ff_mp3_decoder::configuration_t _config;

		AVCodec * _codec;
		AVCodecContext * _context;
		AVPacket _pkt;
		AVFrame * _decoded_frame;
		//uint8_t _inbuffer[AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
	};
};

#endif