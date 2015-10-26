#ifndef _DK_FF_DECODER_H_
#define _DK_FF_DECODER_H_

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavutil/opt.h>
	#include <libavutil/mathematics.h>
}

class dk_ff_decoder
{
public:
	dk_ff_decoder(void);
	~dk_ff_decoder(void);

	int initialize(LPSUBMEDIA_PROCESS_ELEMENT_T subpe);
	int release(LPSUBMEDIA_PROCESS_ELEMENT_T subpe);
	int process(LPSUBMEDIA_PROCESS_ELEMENT_T subpe);

private:
	AVPacket				*_av_packet;
	AVFormatContext			*_format_context;
	AVCodecContext			*_codec_context;
	AVCodec					*_codec;
	AVFrame					*_video_frame;
	unsigned char			*_buffer;
	int32_t					_number_of_bytes;

	AVFrame					*_yuv420p_frame;
	SwsContext				*_yuv420p_sws_context;
	uint32_t				_dst_width;
	uint32_t				_dst_height;

#if defined(DEBUG_DECODE_YUV)
	HANDLE					_file;
	unsigned char			*_sbuffer;
#endif
};

#endif