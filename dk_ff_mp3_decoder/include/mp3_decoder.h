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

class mp3_decoder
{
public:
	mp3_decoder(void);
	~mp3_decoder(void);

	dk_ff_mp3_decoder::ERR_CODE initialize_decoder(dk_ff_mp3_decoder::configuration_t * config);
	dk_ff_mp3_decoder::ERR_CODE release_decoder(void);

	dk_ff_mp3_decoder::ERR_CODE decode(dk_audio_entity_t * encoded, dk_audio_entity_t * pcm);
private:
	dk_ff_mp3_decoder::configuration_t _config;


	AVCodec * _codec;
	AVCodecContext * _context;
	AVPacket _pkt;
	AVFrame * _decoded_frame;
	//uint8_t _inbuffer[AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
};

#endif