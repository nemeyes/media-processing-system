#include "dk_ff_decoder.h"
#include <mmsystem.h>

dk_ff_decoder::dk_ff_decoder(void)
	: _av_packet(0)
	, _format_context(0)
	, _codec_context(0)
	, _codec(0)
	, _video_frame(0)
	, _buffer(0)
	, _number_of_bytes(0)
	, _yuv420p_frame(0)
	, _yuv420p_sws_context(0)
	, _dst_width(640)
	, _dst_height(360)
	//	, _dst_width(214)
	//	, _dst_height(120)
{
#if defined(DEBUG_DECODE_YUV)
	_file = CreateFile(_T("decoder.yuv"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	_sbuffer = 0;
#endif
}

dk_ff_decoder::~dk_ff_decoder(void)
{
#if defined(DEBUG_DECODE_YUV)
	CloseHandle(_file);
#endif
	release(0);
}

uint16_t dk_ff_decoder::initialize(LPSUBMEDIA_PROCESS_ELEMENT_T subpe)
{
	if (subpe->media_info.submedia_type == VMS_SUB_MEDIA_TYPE_UNKNOWN)
		return VMS_STATUS_MEDIA_FILTER_INSTANCE_IS_NOT_INITIALIZED;
	_codec = avcodec_find_decoder(CODEC_ID_H264);

	if (_codec == 0)
	{
		release(subpe);
		return VMS_STATUS_MEDIA_FILTER_INSTANCE_IS_NOT_INITIALIZED;
	}

	_codec_context = avcodec_alloc_context();
	if (_codec_context == 0)
	{
		release(subpe);
		return VMS_STATUS_MEDIA_FILTER_INSTANCE_IS_NOT_INITIALIZED;
	}

	if (avcodec_open(_codec_context, _codec)<0)
	{
		release(subpe);
		return VMS_STATUS_MEDIA_FILTER_INSTANCE_IS_NOT_INITIALIZED;
	}

	if (subpe->media_info.media_type == VMS_MEDIA_TYPE_VIDEO)
	{
		_video_frame = avcodec_alloc_frame();
		_yuv420p_frame = avcodec_alloc_frame();
		if ((_video_frame == 0) || (_yuv420p_frame == 0))
		{
			release(subpe);
			return VMS_STATUS_MEDIA_FILTER_INSTANCE_IS_NOT_INITIALIZED;
		}
	}
	else
		return VMS_STATUS_MEDIA_FILTER_INSTANCE_IS_NOT_INITIALIZED;


	_av_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	if (_av_packet == 0)
	{
		release(subpe);
		return VMS_STATUS_MEDIA_FILTER_INSTANCE_IS_NOT_INITIALIZED;
	}
	av_init_packet(_av_packet);

	int ms_count = subpe->pe->ms_count;
	int square_root = 0;
	for (int i = 0; i<8; i++)
	{
		int prev_count = pow((float)i, 2);
		int next_count = pow((float)(i + 1), 2);
		if (prev_count<ms_count && ms_count <= next_count)
		{
			square_root = i + 1;
			break;
		}
	}

	_dst_width = (int)(1280 / square_root);
	_dst_height = (int)(720 / square_root);
	_dst_width = _dst_width >> 2;
	_dst_width = _dst_width << 2;
	_dst_height = _dst_height >> 2;
	_dst_height = _dst_height << 2;

	_enable = false;
	return VMS_STATUS_SUCCESS;
}

uint16_t dk_ff_decoder::release(LPSUBMEDIA_PROCESS_ELEMENT_T subpe)
{
	_enable = false;
	if (_av_packet != 0)
	{
		av_free_packet(_av_packet);
		_av_packet = 0;
	}

	if (_yuv420p_frame)
	{
		av_free(_yuv420p_frame);
		_yuv420p_frame = 0;
	}

	if (_video_frame)
	{
		av_free(_video_frame);
		_video_frame = 0;
	}

	if (_buffer)
	{
		av_free(_buffer);
		_buffer = 0;
	}

	if (_format_context)
	{
		av_free(_format_context);
		_format_context = 0;
	}

	if (_yuv420p_sws_context)
	{
		sws_freeContext(_yuv420p_sws_context);
		_yuv420p_sws_context = 0;
	}

	if (_codec_context)
	{
		avcodec_close(_codec_context);
		av_free(_codec_context);
		_codec_context = 0;
	}

	if (subpe && (subpe->y_buffer_of_filter != 0))
		subpe->y_buffer_of_filter = 0;
	if (subpe && (subpe->u_buffer_of_filter != 0))
		subpe->u_buffer_of_filter = 0;
	if (subpe && (subpe->v_buffer_of_filter != 0))
		subpe->v_buffer_of_filter = 0;
	return VMS_STATUS_SUCCESS;
}

uint16_t dk_ff_decoder::process(LPSUBMEDIA_PROCESS_ELEMENT_T subpe)
{
	int32_t result = -1;
	if (!_enable)
		return VMS_STATUS_MEDIA_FILTER__NOT_ALLOWED;
	if (subpe->media_info.media_type == VMS_MEDIA_TYPE_VIDEO)
	{
		int32_t	got_frame = 0;
		_av_packet->data = subpe->buffer_of_source;
		_av_packet->size = subpe->data_size_of_source;
		while (_av_packet->size>0)
		{
			__try
			{
				result = avcodec_decode_video2(_codec_context, _video_frame, &got_frame, _av_packet);
				if (result<0)
					return VMS_STATUS_MEDIA_FILTER__NOT_ALLOWED;

				if (got_frame != 0)
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

		if (got_frame == 0)
		{
			return VMS_STATUS_MEDIA_FILTER__NOT_ALLOWED;
		}

		if (got_frame == -1)
		{
			return VMS_STATUS_MEDIA_FILTER__NOT_ALLOWED;
		}
		if (_buffer == 0)
		{
			_number_of_bytes = avpicture_get_size(_codec_context->pix_fmt, _dst_width, _dst_height);
			_buffer = static_cast<unsigned char*>(av_malloc(_number_of_bytes));
#if defined(DEBUG_DECODE_YUV)
			_sbuffer = static_cast<unsigned char*>(av_malloc(_number_of_bytes));
#endif
			avpicture_fill((AVPicture *)_yuv420p_frame, _buffer, _codec_context->pix_fmt, _dst_width, _dst_height);
		}

		if (_codec_context)
		{
			if (!_yuv420p_sws_context)
			{
				_yuv420p_sws_context = sws_getCachedContext(0, _codec_context->width, _codec_context->height, _codec_context->pix_fmt,
					_dst_width, _dst_height, _codec_context->pix_fmt, SWS_SPLINE/*SWS_FAST_BILINEAR*/, 0, 0, 0);
			}
		}

		if (_video_frame && _yuv420p_frame && _yuv420p_sws_context)
		{
			__try
			{
				sws_scale(_yuv420p_sws_context, _video_frame->data, _video_frame->linesize, 0, _codec_context->height, _yuv420p_frame->data, _yuv420p_frame->linesize);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				if (_yuv420p_sws_context)
				{
					sws_freeContext(_yuv420p_sws_context);
					_yuv420p_sws_context = 0;
				}
				return VMS_STATUS_MEDIA_FILTER_PROCESS_FAIL;
			}
		}

		subpe->media_info.dst_video_fmt.width = _dst_width;
		subpe->media_info.dst_video_fmt.height = _dst_height;

		subpe->y_width = _dst_width;
		subpe->y_height = _dst_height;
		subpe->u_width = _dst_width >> 1;
		subpe->u_height = _dst_height >> 1;
		subpe->v_width = subpe->u_width;
		subpe->v_height = subpe->u_height;

		subpe->y_stride = _yuv420p_frame->linesize[0];
		subpe->u_stride = _yuv420p_frame->linesize[1];
		subpe->v_stride = _yuv420p_frame->linesize[2];

		subpe->y_buffer_of_filter = _yuv420p_frame->data[0];
		subpe->u_buffer_of_filter = _yuv420p_frame->data[1];
		subpe->v_buffer_of_filter = _yuv420p_frame->data[2];

#if defined(DEBUG_DECODE_YUV)
		unsigned char *ptr_y = _sbuffer;
		int stride_y = subpe->y_stride;
		int stride_u = subpe->u_stride;
		int stride_v = subpe->v_stride;

		unsigned char *data_y = (unsigned char*)subpe->y_buffer_of_filter;
		unsigned char *data_u = (unsigned char*)subpe->u_buffer_of_filter;
		unsigned char *data_v = (unsigned char*)subpe->v_buffer_of_filter;

		int width_y = subpe->y_width;
		int width_u = width_y >> 1;
		int height_y = subpe->y_height;
		int height_u = height_y >> 1;

		BYTE * ptr_u = ptr_y;
		ptr_u += width_y*height_y;
		BYTE * ptr_v = ptr_u;
		ptr_v += width_y*height_y / 4;

		for (int h = 0; h<height_y; h++)
		{
			memcpy(ptr_y + h*width_y, data_y + h*stride_y, width_y);
			if (h<height_u)
			{
				memcpy(ptr_u + h*width_u, data_u + h*stride_u, width_u);
				memcpy(ptr_v + h*width_u, data_v + h*stride_u, width_u);
			}
		}

		char *file_src;
		file_src = (char*)_sbuffer;
		int tmp_input_size = _number_of_bytes;
		while (true)
		{
			DWORD written = 0;
			WriteFile(_file, (char*)file_src, (DWORD)tmp_input_size, &written, NULL);
			tmp_input_size -= written;
			file_src += written;
			if (tmp_input_size<1)
				break;
		}
#endif
		return VMS_STATUS_SUCCESS;
	}
	else
		return VMS_STATUS_FAIL;
}

void dk_ff_decoder::yuv420_to_argb8888(uint8_t *yp, uint8_t *up, uint8_t *vp,
	uint32_t sy, uint32_t suv,
	int width, int height, uint32_t *rgb, uint32_t srgb)
{
	__m128i y0r0, y0r1, u0, v0;
	__m128i y00r0, y01r0, y00r1, y01r1;
	__m128i u00, u01, v00, v01;
	__m128i rv00, rv01, gu00, gu01, gv00, gv01, bu00, bu01;
	__m128i r00, r01, g00, g01, b00, b01;
	__m128i rgb0123, rgb4567, rgb89ab, rgbcdef;
	__m128i gbgb;
	__m128i ysub, uvsub;
	__m128i zero, facy, facrv, facgu, facgv, facbu;
	__m128i *srcy128r0, *srcy128r1;
	__m128i *dstrgb128r0, *dstrgb128r1;
	__m64   *srcu64, *srcv64;
	int x, y;

	ysub = _mm_set1_epi32(0x00100010);
	uvsub = _mm_set1_epi32(0x00800080);

	facy = _mm_set1_epi32(0x004a004a);
	facrv = _mm_set1_epi32(0x00660066);
	facgu = _mm_set1_epi32(0x00190019);
	facgv = _mm_set1_epi32(0x00340034);
	facbu = _mm_set1_epi32(0x00810081);

	zero = _mm_set1_epi32(0x00000000);

	for (y = 0; y < height; y += 2) {

		srcy128r0 = (__m128i *)(yp + sy*y);
		srcy128r1 = (__m128i *)(yp + sy*y + sy);
		srcu64 = (__m64 *)(up + suv*(y / 2));
		srcv64 = (__m64 *)(vp + suv*(y / 2));

		dstrgb128r0 = (__m128i *)(rgb + srgb*y);
		dstrgb128r1 = (__m128i *)(rgb + srgb*y + srgb);

		for (x = 0; x < width; x += 16) {

			u0 = _mm_loadl_epi64((__m128i *)srcu64); srcu64++;
			v0 = _mm_loadl_epi64((__m128i *)srcv64); srcv64++;

			y0r0 = _mm_load_si128(srcy128r0++);
			y0r1 = _mm_load_si128(srcy128r1++);

			// constant y factors
			y00r0 = _mm_mullo_epi16(_mm_sub_epi16(_mm_unpacklo_epi8(y0r0, zero), ysub), facy);
			y01r0 = _mm_mullo_epi16(_mm_sub_epi16(_mm_unpackhi_epi8(y0r0, zero), ysub), facy);
			y00r1 = _mm_mullo_epi16(_mm_sub_epi16(_mm_unpacklo_epi8(y0r1, zero), ysub), facy);
			y01r1 = _mm_mullo_epi16(_mm_sub_epi16(_mm_unpackhi_epi8(y0r1, zero), ysub), facy);

			// expand u and v so they're aligned with y values
			u0 = _mm_unpacklo_epi8(u0, zero);
			u00 = _mm_sub_epi16(_mm_unpacklo_epi16(u0, u0), uvsub);
			u01 = _mm_sub_epi16(_mm_unpackhi_epi16(u0, u0), uvsub);

			v0 = _mm_unpacklo_epi8(v0, zero);
			v00 = _mm_sub_epi16(_mm_unpacklo_epi16(v0, v0), uvsub);
			v01 = _mm_sub_epi16(_mm_unpackhi_epi16(v0, v0), uvsub);

			// common factors on both rows.
			rv00 = _mm_mullo_epi16(facrv, v00);
			rv01 = _mm_mullo_epi16(facrv, v01);
			gu00 = _mm_mullo_epi16(facgu, u00);
			gu01 = _mm_mullo_epi16(facgu, u01);
			gv00 = _mm_mullo_epi16(facgv, v00);
			gv01 = _mm_mullo_epi16(facgv, v01);
			bu00 = _mm_mullo_epi16(facbu, u00);
			bu01 = _mm_mullo_epi16(facbu, u01);

			// row 0
			r00 = _mm_srai_epi16(_mm_add_epi16(y00r0, rv00), 6);
			r01 = _mm_srai_epi16(_mm_add_epi16(y01r0, rv01), 6);
			g00 = _mm_srai_epi16(_mm_sub_epi16(_mm_sub_epi16(y00r0, gu00), gv00), 6);
			g01 = _mm_srai_epi16(_mm_sub_epi16(_mm_sub_epi16(y01r0, gu01), gv01), 6);
			b00 = _mm_srai_epi16(_mm_add_epi16(y00r0, bu00), 6);
			b01 = _mm_srai_epi16(_mm_add_epi16(y01r0, bu01), 6);

			r00 = _mm_packus_epi16(r00, r01);         // rrrr.. saturated
			g00 = _mm_packus_epi16(g00, g01);         // gggg.. saturated
			b00 = _mm_packus_epi16(b00, b01);         // bbbb.. saturated

			r01 = _mm_unpacklo_epi8(r00, zero); // 0r0r..
			gbgb = _mm_unpacklo_epi8(b00, g00);  // gbgb..
			rgb0123 = _mm_unpacklo_epi16(gbgb, r01);  // 0rgb0rgb..
			rgb4567 = _mm_unpackhi_epi16(gbgb, r01);  // 0rgb0rgb..

			r01 = _mm_unpackhi_epi8(r00, zero);
			gbgb = _mm_unpackhi_epi8(b00, g00);
			rgb89ab = _mm_unpacklo_epi16(gbgb, r01);
			rgbcdef = _mm_unpackhi_epi16(gbgb, r01);

			_mm_store_si128(dstrgb128r0++, rgb0123);
			_mm_store_si128(dstrgb128r0++, rgb4567);
			_mm_store_si128(dstrgb128r0++, rgb89ab);
			_mm_store_si128(dstrgb128r0++, rgbcdef);

			// row 1
			r00 = _mm_srai_epi16(_mm_add_epi16(y00r1, rv00), 6);
			r01 = _mm_srai_epi16(_mm_add_epi16(y01r1, rv01), 6);
			g00 = _mm_srai_epi16(_mm_sub_epi16(_mm_sub_epi16(y00r1, gu00), gv00), 6);
			g01 = _mm_srai_epi16(_mm_sub_epi16(_mm_sub_epi16(y01r1, gu01), gv01), 6);
			b00 = _mm_srai_epi16(_mm_add_epi16(y00r1, bu00), 6);
			b01 = _mm_srai_epi16(_mm_add_epi16(y01r1, bu01), 6);

			r00 = _mm_packus_epi16(r00, r01);         // rrrr.. saturated
			g00 = _mm_packus_epi16(g00, g01);         // gggg.. saturated
			b00 = _mm_packus_epi16(b00, b01);         // bbbb.. saturated

			r01 = _mm_unpacklo_epi8(r00, zero); // 0r0r..
			gbgb = _mm_unpacklo_epi8(b00, g00);  // gbgb..
			rgb0123 = _mm_unpacklo_epi16(gbgb, r01);  // 0rgb0rgb..
			rgb4567 = _mm_unpackhi_epi16(gbgb, r01);  // 0rgb0rgb..

			r01 = _mm_unpackhi_epi8(r00, zero);
			gbgb = _mm_unpackhi_epi8(b00, g00);
			rgb89ab = _mm_unpacklo_epi16(gbgb, r01);
			rgbcdef = _mm_unpackhi_epi16(gbgb, r01);

			_mm_store_si128(dstrgb128r1++, rgb0123);
			_mm_store_si128(dstrgb128r1++, rgb4567);
			_mm_store_si128(dstrgb128r1++, rgb89ab);
			_mm_store_si128(dstrgb128r1++, rgbcdef);

		}
	}
}
