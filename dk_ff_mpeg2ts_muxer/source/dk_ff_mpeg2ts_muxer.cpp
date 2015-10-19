#include "dk_ff_mpeg2ts_muxer.h"
#include "ff_mpeg2ts_muxer_core.h"

dk_ff_mpeg2ts_muxer::dk_ff_mpeg2ts_muxer(void)
{
	_core = new ff_mpeg2ts_muxer_core();
}

dk_ff_mpeg2ts_muxer::~dk_ff_mpeg2ts_muxer(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_ff_mpeg2ts_muxer::ERR_CODE dk_ff_mpeg2ts_muxer::initialize(configuration_t & config)
{
	return _core->initialize(config);
}

dk_ff_mpeg2ts_muxer::ERR_CODE dk_ff_mpeg2ts_muxer::release(void)
{
	return _core->release();
}


dk_ff_mpeg2ts_muxer::ERR_CODE dk_ff_mpeg2ts_muxer::put_video_stream(unsigned char * buffer, size_t nb, long long pts, bool keyframe)
{
	return _core->put_video_stream(buffer, nb, pts, keyframe);
}