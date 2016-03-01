#include "dk_ff_mpeg2ts_muxer.h"
#include "ff_mpeg2ts_muxer_core.h"

dk_ff_mpeg2ts_muxer::_configuration_t::_video_configuration_t::_video_configuration_t(void)
	: stream_index(0)
	, width(0)
	, height(0)
	, bitrate(0)
	, fps(30)
	, extradata_size(0)
{
	memset(extradata, 0x00, sizeof(extradata));
}

dk_ff_mpeg2ts_muxer::_configuration_t::_video_configuration_t::_video_configuration_t(const dk_ff_mpeg2ts_muxer::_configuration_t::_video_configuration_t & clone)
{
	stream_index = clone.stream_index;
	width = clone.width;
	height = clone.height;
	bitrate = clone.bitrate;
	fps = clone.fps;
	bitrate = clone.bitrate;
	if (clone.extradata_size>0)
	{
		extradata_size = clone.extradata_size;
		memcpy(extradata, clone.extradata, sizeof(extradata_size));
	}
	else
	{
		extradata_size = 0;
		memset(extradata, 0x00, sizeof(extradata));
	}
}

dk_ff_mpeg2ts_muxer::_configuration_t::_video_configuration_t dk_ff_mpeg2ts_muxer::_configuration_t::_video_configuration_t::operator = (const dk_ff_mpeg2ts_muxer::_configuration_t::_video_configuration_t & clone)
{
	stream_index = clone.stream_index;
	width = clone.width;
	height = clone.height;
	bitrate = clone.bitrate;
	fps = clone.fps;
	bitrate = clone.bitrate;
	if (clone.extradata_size>0)
	{
		extradata_size = clone.extradata_size;
		memcpy(extradata, clone.extradata, sizeof(extradata_size));
	}
	else
	{
		extradata_size = 0;
		memset(extradata, 0x00, sizeof(extradata));
	}
	return (*this);
}


dk_ff_mpeg2ts_muxer::_configuration_t::_configuration_t(void)
{
}

dk_ff_mpeg2ts_muxer::_configuration_t::_configuration_t(const dk_ff_mpeg2ts_muxer::_configuration_t & clone)
{
	vconfig = clone.vconfig;
}

dk_ff_mpeg2ts_muxer::_configuration_t dk_ff_mpeg2ts_muxer::_configuration_t::operator = (const dk_ff_mpeg2ts_muxer::_configuration_t & clone)
{
	vconfig = clone.vconfig;
	return (*this);
}

dk_ff_mpeg2ts_muxer::dk_ff_mpeg2ts_muxer(void)
{
	_core = new ff_mpeg2ts_muxer_core(this);
}

dk_ff_mpeg2ts_muxer::~dk_ff_mpeg2ts_muxer(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_ff_mpeg2ts_muxer::ERR_CODE dk_ff_mpeg2ts_muxer::initialize(configuration_t * config)
{
	return _core->initialize(config);
}

dk_ff_mpeg2ts_muxer::ERR_CODE dk_ff_mpeg2ts_muxer::release(void)
{
	return _core->release();
}


dk_ff_mpeg2ts_muxer::ERR_CODE dk_ff_mpeg2ts_muxer::put_video_stream(uint8_t * buffer, size_t nb, int64_t pts, bool keyframe)
{
	return _core->put_video_stream(buffer, nb, pts, keyframe);
}