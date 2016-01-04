#include "dk_media_buffering.h"
#include "dk_video_buffer.h"

dk_media_buffer::dk_media_buffer(void)
	: _width(0)
	, _height(0)
{
	_vbuffer = new dk_video_buffer();
}

dk_media_buffer::~dk_media_buffer(void)
{
	delete _vbuffer;
}

dk_media_buffer & dk_media_buffer::instance(void)
{
	static dk_media_buffer _instance;
	return _instance;
}

dk_media_buffer::ERR_CODE dk_media_buffer::push_video(uint8_t * es, size_t size)
{
	_vbuffer->push_bitstream(es, size);
	return dk_media_buffer::ERR_CODE_SUCCESS;
}

dk_media_buffer::ERR_CODE dk_media_buffer::pop_video(uint8_t * es, size_t & size)
{
	return _vbuffer->pop_bitstream(es, size);
}

dk_media_buffer::ERR_CODE dk_media_buffer::get_video_resolution(int32_t & width, int32_t & height)
{
	width = _width;
	height = _height;
	return dk_media_buffer::ERR_CODE_SUCCESS;
}

dk_media_buffer::ERR_CODE dk_media_buffer::set_vps(uint8_t * vps, size_t size)
{
	return _vbuffer->set_vps(vps, size);
}

dk_media_buffer::ERR_CODE dk_media_buffer::set_sps(uint8_t * sps, size_t size)
{
	return _vbuffer->set_sps(sps, size);
}

dk_media_buffer::ERR_CODE dk_media_buffer::set_pps(uint8_t * pps, size_t size)
{
	return _vbuffer->set_pps(pps, size);
}

dk_media_buffer::ERR_CODE dk_media_buffer::get_vps(uint8_t * vps, size_t & size)
{
	return _vbuffer->get_vps(vps, size);
}

dk_media_buffer::ERR_CODE dk_media_buffer::get_sps(uint8_t * sps, size_t & size)
{
	return _vbuffer->get_sps(sps, size);
}

dk_media_buffer::ERR_CODE dk_media_buffer::get_pps(uint8_t * pps, size_t & size)
{
	return _vbuffer->get_pps(pps, size);
}
