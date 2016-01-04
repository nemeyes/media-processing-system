#include "dk_media_buffering.h"
#include "dk_video_buffer.h"

dk_media_buffering::dk_media_buffering(void)
{
	_vbuffer = new dk_video_buffer();
}

dk_media_buffering::~dk_media_buffering(void)
{
	delete _vbuffer;
}

dk_media_buffering & dk_media_buffering::instance(void)
{
	static dk_media_buffering _instance;
	return _instance;
}

dk_media_buffering::ERR_CODE dk_media_buffering::push_video(uint8_t * es, size_t size)
{
	_vbuffer->push_bitstream(es, size);
	return dk_media_buffering::ERR_CODE_SUCCESS;
}

dk_media_buffering::ERR_CODE dk_media_buffering::pop_video(uint8_t * es, size_t & size)
{
	return _vbuffer->pop_bitstream(es, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_vps(uint8_t * vps, size_t size)
{
	return _vbuffer->set_vps(vps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_sps(uint8_t * sps, size_t size)
{
	return _vbuffer->set_sps(sps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_pps(uint8_t * pps, size_t size)
{
	return _vbuffer->set_pps(pps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_vps(uint8_t * vps, size_t & size)
{
	return _vbuffer->get_vps(vps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_sps(uint8_t * sps, size_t & size)
{
	return _vbuffer->get_sps(sps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_pps(uint8_t * pps, size_t & size)
{
	return _vbuffer->get_pps(pps, size);
}
