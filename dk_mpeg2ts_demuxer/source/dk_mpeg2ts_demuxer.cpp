#include "dk_mpeg2ts_demuxer.h"
#include "mpeg2ts_demuxer.h"

dk_mpeg2ts_demuxer::dk_mpeg2ts_demuxer(void)
{
	_core = new mpeg2ts_demuxer();
}

dk_mpeg2ts_demuxer::~dk_mpeg2ts_demuxer(void)
{
	delete _core;
	_core = nullptr;
}

dk_mpeg2ts_demuxer::ERR_CODE dk_mpeg2ts_demuxer::initialize(void)
{
	return _core->initialize();
}

dk_mpeg2ts_demuxer::ERR_CODE dk_mpeg2ts_demuxer::release(void)
{
	return _core->release();
}

dk_mpeg2ts_demuxer::ERR_CODE dk_mpeg2ts_demuxer::demultiplexing(uint8_t * buffer, size_t nb)
{
	return _core->demultiplexing(buffer, nb);
}