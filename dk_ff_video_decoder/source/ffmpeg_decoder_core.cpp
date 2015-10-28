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

#include <dk_simd_colorspace_converter.h>

ffmpeg_decoder_core::ffmpeg_decoder_core(dk_ff_video_decoder * front)
	: _front(front)
{

}

ffmpeg_decoder_core::~ffmpeg_decoder_core(void)
{

}

dk_ff_video_decoder::ERR_CODE ffmpeg_decoder_core::initialize_decoder(dk_ff_video_decoder::configuration_t * config)
{
	switch (config->ismt)
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
		release_decoder();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}

	_av_codec_ctx = avcodec_alloc_context3(_av_codec);
	if (!_av_codec_ctx)
	{
		release_decoder();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}
	_av_codec_ctx->delay = 0;
	if (config->extradata && (config->extradata_size>0))
	{
		_av_codec_ctx->extradata = static_cast<uint8_t*>(av_malloc(config->extradata_size));
		_av_codec_ctx->extradata_size = config->extradata_size;
		memcpy(_av_codec_ctx->extradata, config->extradata, _av_codec_ctx->extradata_size);
		//memset(_av_codec_ctx->extradata, 0x00, config->extra_data_size);
	}
	if (avcodec_open2(_av_codec_ctx, _av_codec, NULL)<0)
	{
		release_decoder();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}

	_av_frame = av_frame_alloc();
	_av_video_frame = av_frame_alloc();
	if (!_av_frame || !_av_video_frame)
	{
		release_decoder();
		return dk_ff_video_decoder::ERR_CODE_FAILED;
	}

	_av_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	if (!_av_packet)
	{
		release_decoder();
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

dk_ff_video_decoder::ERR_CODE ffmpeg_decoder_core::decode(dk_video_entity_t * encoded, dk_video_entity_t * decoded)
{
	int32_t value = dk_ff_video_decoder::ERR_CODE_FAILED;
	int32_t result = -1;
	int32_t	got_frame = 0;
	_av_packet->data = encoded->data;
	_av_packet->size = encoded->data_size;
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

	if (got_frame && (got_frame != -1))
	{
		AVFrame	* frame = 0;
		if ((_front->_config->iwidth != _front->_config->owidth) || (_front->_config->iheight != _front->_config->oheight))
		{
			if (_buffer == 0)
			{
				int number_of_bytes = avpicture_get_size(_av_codec_ctx->pix_fmt, _front->_config->owidth, _front->_config->oheight);
				_buffer = static_cast<unsigned char*>(av_malloc(number_of_bytes));
				avpicture_fill((AVPicture *)_av_video_frame, _buffer, _av_codec_ctx->pix_fmt, _front->_config->owidth, _front->_config->oheight);
			}

			if (_av_codec_ctx)
			{
				if (!_sws_ctx)
				{
					_sws_ctx = sws_getCachedContext(0, _av_codec_ctx->width, _av_codec_ctx->height,
						_av_codec_ctx->pix_fmt, _front->_config->owidth, _front->_config->oheight,
						_av_codec_ctx->pix_fmt, /*SWS_SPLINE*/SWS_FAST_BILINEAR, 0, 0, 0);
				}
			}

			if (_av_frame && _av_video_frame && _sws_ctx)
			{
				__try
				{
					sws_scale(_sws_ctx, _av_video_frame->data, _av_video_frame->linesize, 0, _av_codec_ctx->height, _av_video_frame->data, _av_video_frame->linesize);
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					if (_sws_ctx)
					{
						sws_freeContext(_sws_ctx);
						_sws_ctx = 0;
					}
					return dk_ff_video_decoder::ERR_CODE_FAILED;
				}
			}

			frame = _av_video_frame;
		}
		else //decode as it's resolution is
		{
			frame = _av_frame;
		}

		int32_t y_width = _front->_config->owidth;
		int32_t y_height = _front->_config->oheight;
		int32_t uv_width = _front->_config->owidth >> 1;
		int32_t uv_height = _front->_config->oheight >> 1;

		int src_y_stride = frame->linesize[0];
		int src_u_stride = frame->linesize[1];
		int src_v_stride = frame->linesize[2];
		unsigned char * src_y_plane = frame->data[0];
		unsigned char * src_u_plane = frame->data[1];
		unsigned char * src_v_plane = frame->data[2];

		if ((_front->_config->osmt == dk_ff_video_decoder::SUBMEDIA_TYPE_I420) || (_front->_config->osmt == dk_ff_video_decoder::SUBMEDIA_TYPE_YV12))
		{
			unsigned char * dst_y_plane = decoded->data;
			unsigned char * dst_u_plane = 0;
			unsigned char * dst_v_plane = 0;
			int32_t dst_y_stride = _front->_config->ostride;
			int32_t dst_uv_stride = dst_y_stride >> 1;

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
			decoded->data_size = _front->_config->ostride*y_height*1.5;
			if (_front->_config->osmt == dk_ff_video_decoder::SUBMEDIA_TYPE_I420) //4:2:0 Y U V
			{
				dst_u_plane = dst_y_plane + dst_y_stride*y_height;
				dst_v_plane = dst_u_plane + dst_uv_stride*uv_height;
			}
			else if (_front->_config->osmt == dk_ff_video_decoder::SUBMEDIA_TYPE_YV12) // 4:2:0 Y V U
			{
				dst_v_plane = dst_y_plane + dst_y_stride*y_height;
				dst_u_plane = dst_v_plane + dst_uv_stride*uv_height;
			}

			for (int h = 0; h < y_height; h++)
			{
				memcpy(dst_y_plane + h*dst_y_stride, src_y_plane + h*src_y_stride, y_width);
				if (h < uv_height)
				{
					memcpy(dst_v_plane + h*dst_uv_stride, src_v_plane + h*src_v_stride, uv_width);
					memcpy(dst_u_plane + h*dst_uv_stride, src_u_plane + h*src_v_stride, uv_width);
				}
			}
#endif
		}
		else if (_front->_config->osmt == dk_ff_video_decoder::SUBMEDIA_TYPE_RGB32)
		{
			uint8_t * dst = decoded->data;
			int32_t dst_stride = _front->_config->ostride;
			dk_simd_colorspace_converter::convert_i420_to_rgba(y_width, y_height, src_y_plane, src_y_stride, src_u_plane, src_u_stride, src_v_plane, src_v_stride, dst, dst_stride, 0, false);
		}
		else if (_front->_config->osmt == dk_ff_video_decoder::SUBMEDIA_TYPE_RGB24)
		{
			uint8_t * dst = decoded->data;
			int32_t dst_stride = _front->_config->ostride;
			dk_simd_colorspace_converter::convert_i420_to_rgba(y_width, y_height, src_y_plane, src_y_stride, src_u_plane, src_u_stride, src_v_plane, src_v_stride, dst, dst_stride, 0, false);
		}
		return dk_ff_video_decoder::ERR_CODE_SUCCESS;
	}
	return dk_ff_video_decoder::ERR_CODE_FAILED;
}