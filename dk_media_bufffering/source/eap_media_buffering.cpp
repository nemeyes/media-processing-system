#include "eap_media_buffering.h"
#include "h264_stream.h"

#include "eap_video_buffer.h"

eap_media_buffering::eap_media_buffering(void)
	: _width(0)
	, _height(0)
{
	_vbuffer = new eap_video_buffer();
}

eap_media_buffering::~eap_media_buffering(void)
{
	delete _vbuffer;
}

eap_media_buffering & eap_media_buffering::instance(void)
{
	static eap_media_buffering _instance;
	return _instance;
}

eap_media_buffering::ERR_CODE  eap_media_buffering::push_video(uint8_t * es, size_t size)
{
	int32_t sz_start_code = 4;
	//int opt_verbose = 1;
	//int opt_probe = 0;
	uint8_t * p = es; 
	size_t sz = size;
	int64_t off = 0;
	int nal_start, nal_end;
	while (find_nal_unit(p, sz, &nal_start, &nal_end) > 0)
	{
		p += nal_start;
		if ((p[0] & 0x1F) == NAL_UNIT_TYPE_SPS)
		{
			h264_stream_t* h = h264_new();
			read_nal_unit(h, p, nal_end - nal_start);
			set_sps(p - sz_start_code, nal_end - nal_start + sz_start_code);
			_width = (h->sps->pic_width_in_mbs_minus1 + 1) << 4;
			_height = (h->sps->pic_height_in_map_units_minus1 + 1) << 4;

			h264_free(h);
		}
		else if((p[0] & 0x1F) ==NAL_UNIT_TYPE_PPS)
		{
			set_pps(p - sz_start_code, nal_end - nal_start + sz_start_code);
		}
		else if ((p[0] & 0x1F) == NAL_UNIT_TYPE_CODED_SLICE_IDR)
		{
			_vbuffer->push_bitstream(p - sz_start_code, nal_end - nal_start + sz_start_code);
		}
		else
		{
			_vbuffer->push_bitstream(p - sz_start_code, nal_end - nal_start + sz_start_code);
		}

		p += (nal_end - nal_start);
		sz -= nal_end;
	}

	if ((p + sz_start_code) < (es + size))
	{
		nal_end = es + size - p;
		p += nal_start;
		if ((p[0] & 0x1F) == NAL_UNIT_TYPE_SPS)
		{
			h264_stream_t* h = h264_new();
			read_nal_unit(h, p, nal_end - nal_start);
			set_sps(p - sz_start_code, nal_end - nal_start + sz_start_code);
			_width = (h->sps->pic_width_in_mbs_minus1 + 1) << 4;
			_height = (h->sps->pic_height_in_map_units_minus1 + 1) << 4;

			h264_free(h);
		}
		else if ((p[0] & 0x1F) == NAL_UNIT_TYPE_PPS)
		{
			set_pps(p - sz_start_code, nal_end - nal_start + sz_start_code);
		}
		else if ((p[0] & 0x1F) == NAL_UNIT_TYPE_CODED_SLICE_IDR)
		{
			_vbuffer->push_bitstream(p - sz_start_code, nal_end - nal_start + sz_start_code);
		}
		else
		{
			_vbuffer->push_bitstream(p - sz_start_code, nal_end - nal_start + sz_start_code);
		}
	}

	return eap_media_buffering::ERR_CODE_SUCCESS;
}

eap_media_buffering::ERR_CODE eap_media_buffering::pop_video(uint8_t * es, size_t & size)
{
	return _vbuffer->pop_bitstream(es, size);
}

eap_media_buffering::ERR_CODE eap_media_buffering::get_video_resolution(int32_t & width, int32_t & height)
{
	width = _width;
	height = _height;
	return eap_media_buffering::ERR_CODE_SUCCESS;
}

eap_media_buffering::ERR_CODE eap_media_buffering::set_vps(uint8_t * vps, size_t size)
{
	return _vbuffer->set_vps(vps, size);
}

eap_media_buffering::ERR_CODE eap_media_buffering::set_sps(uint8_t * sps, size_t size)
{
	return _vbuffer->set_sps(sps, size);
}

eap_media_buffering::ERR_CODE eap_media_buffering::set_pps(uint8_t * pps, size_t size)
{
	return _vbuffer->set_pps(pps, size);
}

eap_media_buffering::ERR_CODE eap_media_buffering::get_vps(uint8_t * vps, size_t & size)
{
	return _vbuffer->get_vps(vps, size);
}

eap_media_buffering::ERR_CODE eap_media_buffering::get_sps(uint8_t * sps, size_t & size)
{
	return _vbuffer->get_sps(sps, size);
}

eap_media_buffering::ERR_CODE eap_media_buffering::get_pps(uint8_t * pps, size_t & size)
{
	return _vbuffer->get_pps(pps, size);
}
