#include <windows.h>
#include <process.h>
#include "ffmpeg_decoder_core.h"

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavutil/opt.h>
	#include <libavutil/mathematics.h>
}

ffmpeg_decoder_core::ffmpeg_decoder_core(void)
{

}

ffmpeg_decoder_core::~ffmpeg_decoder_core(void)
{

}

dk_ff_video_decoder::ERR_CODE ffmpeg_decoder_core::initialize_decoder(dk_ff_video_decoder::CONFIGURATION_T * config)
{
	switch (config->ist)
	{
		case dk_ff_video_decoder::SUBMEDIA_TYPE_H264:
		case dk_ff_video_decoder::SUBMEDIA_TYPE_H264_BP:
		case dk_ff_video_decoder::SUBMEDIA_TYPE_H264_HP:
		case dk_ff_video_decoder::SUBMEDIA_TYPE_H264_MP:
		case dk_ff_video_decoder::SUBMEDIA_TYPE_H264_EP:
		{
			_av_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
			break;
		}
		case dk_ff_video_decoder::SUBMEDIA_TYPE_MP4V:
		case dk_ff_video_decoder::SUBMEDIA_TYPE_MP4V_SP:
		case dk_ff_video_decoder::SUBMEDIA_TYPE_MP4V_ASP:
		{
			_av_codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
			break;
		}
		case dk_ff_video_decoder::SUBMEDIA_TYPE_MJPEG:
		{
			_av_codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
			break;
		}
	}

	if (!_av_codec)
	{
		//release();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}

	_av_codec_ctx = avcodec_alloc_context3(_av_codec);
	if (!_av_codec_ctx)
	{
		//release();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}

	if (avcodec_open2(_av_codec_ctx, _av_codec, NULL)<0)
	{
		//release();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}

	_av_frame = av_frame_alloc();
	_av_video_frame = av_frame_alloc();
	if (!_av_frame || !_av_video_frame)
	{
		//release();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}

	_av_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	if (!_av_packet)
	{
		//release();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}
	av_init_packet(_av_packet);
	return dk_ff_video_decoder::ERR_CODE_SUCCESS;
}

dk_ff_video_decoder::ERR_CODE ffmpeg_decoder_core::release_decoder(void)
{
	if (_av_packet)
	{
		av_free_packet(_av_packet);
		_av_packet = nullptr;
	}

	if (_av_video_frame)
	{
		av_free(_av_video_frame);
		_av_video_frame = nullptr;
	}

	if (_av_frame)
	{
		av_free(_av_frame);
		_av_frame = nullptr;
	}

	if (_buffer)
	{
		av_free(_buffer);
		_buffer = nullptr;
	}

	if (_av_format_ctx)
	{
		av_free(_av_format_ctx);
		_av_format_ctx = nullptr;
	}

	if (_sws_ctx)
	{
		sws_freeContext(_sws_ctx);
		_sws_ctx = nullptr;
	}

	if (_av_codec_ctx)
	{
		avcodec_close(_av_codec_ctx);
		av_free(_av_codec_ctx);
		_av_codec_ctx = nullptr;
	}
	return dk_ff_video_decoder::ERR_CODE_SUCCESS;
}

dk_ff_video_decoder::ERR_CODE ffmpeg_decoder_core::decode(DK_VIDEO_ENTITY_T * data)
{
	int32_t value = dk_ff_video_decoder::ERR_CODE_FAILED;
	int32_t result = -1;
	int32_t	got_frame = 0;
	_av_packet->data = data->bitstream;
	_av_packet->size = data->bitstream_size;
	while (_av_packet->size>0)
	{
		__try
		{
			result = avcodec_decode_video2(_av_codec_ctx, _av_frame, &got_frame, _av_packet);
			if (result<0)
				break;

			_av_packet->size -= result;
			_av_packet->data += result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			got_frame = -1;
			break;
		}
	}

	if (got_frame && (got_frame!=-1))
	{
		AVFrame	*frame = 0;
		if (_osubmedia_type != VMXNET_SUB_MEDIA_TYPE_RGB32)
		{
			if (iwidth != owidth || iheight != oheight)
			{
				if (_buffer == 0)
				{
					int number_of_bytes = avpicture_get_size(_codec_ctx->pix_fmt, owidth, oheight);
					_buffer = static_cast<unsigned char*>(av_malloc(number_of_bytes));
					avpicture_fill((AVPicture *)_video_frame, _buffer, _codec_ctx->pix_fmt, owidth, oheight);
				}

				if (_codec_ctx)
				{
					if (!_sws_ctx)
					{
						_sws_ctx = sws_getCachedContext(0, _codec_ctx->width, _codec_ctx->height, _codec_ctx->pix_fmt, owidth, oheight, _codec_ctx->pix_fmt,
							/*SWS_SPLINE*/SWS_FAST_BILINEAR, 0, 0, 0);
					}
				}

				if (_frame && _video_frame && _sws_ctx)
				{
					__try
					{
						sws_scale(_sws_ctx, _video_frame->data, _video_frame->linesize, 0, _codec_ctx->height, _video_frame->data, _video_frame->linesize);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						if (_sws_ctx)
						{
							sws_freeContext(_sws_ctx);
							_sws_ctx = 0;
						}
						return VMXNET_STATUS_FAIL;
					}
				}

				frame = _video_frame;
			}
			else //decode as it's resolution is
			{
				frame = _frame;
			}


			int y_width = owidth;
			int y_height = oheight;
			int u_width = owidth >> 1;
			int u_height = oheight >> 1;
			int v_width = u_width;
			int v_height = u_height;

			unsigned char *ptr_y = output;
			unsigned char *ptr_u = 0;
			unsigned char *ptr_v = 0;
			int y_stride = frame->linesize[0];
			int u_stride = frame->linesize[1];
			int v_stride = frame->linesize[2];
			unsigned char *y_buffer = frame->data[0];
			unsigned char *u_buffer = frame->data[1];
			unsigned char *v_buffer = frame->data[2];

#if 0
			int decoded_buffer_size = y_width*y_height*1.5;
			if (_osubmedia_type == VMXNET_SUB_MEDIA_TYPE_IYUV) //4:2:0 Y U V
			{
				ptr_u = ptr_y;
				ptr_u += y_width*y_height;
				ptr_v = ptr_u;
				ptr_v += u_width*u_height;
			}
			else if (_osubmedia_type == VMXNET_SUB_MEDIA_TYPE_YV12) // 4:2:0 Y V U
			{
				ptr_v = ptr_y;
				ptr_v += y_width*y_height;
				ptr_u = ptr_v;
				ptr_u += v_width*v_height;
			}


			for (int h = 0; h<y_height; h++)
			{
				memcpy(ptr_y + h*y_width, y_buffer + h*y_stride, y_width);
				if (h<u_height)
				{
					memcpy(ptr_v + h*v_width, v_buffer + h*v_stride, v_width);
					memcpy(ptr_u + h*u_width, u_buffer + h*u_stride, u_width);
				}
			}
			osize = _decoded_buffer_size;
#else
			int decoded_buffer_size = stride*y_height*1.5;
			if (_osubmedia_type == VMXNET_SUB_MEDIA_TYPE_IYUV) //4:2:0 Y U V
			{
				ptr_u = ptr_y;
				ptr_u += stride*y_height;
				ptr_v = ptr_u;
				ptr_v += (stride >> 1)*u_height;
			}
			else if (_osubmedia_type == VMXNET_SUB_MEDIA_TYPE_YV12) // 4:2:0 Y V U
			{
				ptr_v = ptr_y;
				ptr_v += stride*y_height;
				ptr_u = ptr_v;
				ptr_u += (stride >> 1)*u_height;
			}

			for (int h = 0; h<y_height; h++)
			{
				memcpy(ptr_y + h*stride, y_buffer + h*y_stride, y_width);
				if (h<u_height)
				{
					memcpy(ptr_v + h*(stride >> 1), v_buffer + h*v_stride, v_width);
					memcpy(ptr_u + h*(stride >> 1), u_buffer + h*u_stride, u_width);
				}
			}
			osize = _decoded_buffer_size;
#endif

		}
		else
		{
			//YUV2ARGB32( subpe, _codec_ctx, _frame );
		}

	return dk_ff_video_decoder::ERR_CODE_FAILED;
}