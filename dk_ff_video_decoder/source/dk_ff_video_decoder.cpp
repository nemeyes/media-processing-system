#include "dk_ff_video_decoder.h"
#include "ffmpeg_decoder_core.h"

dk_ff_video_decoder::dk_ff_video_decoder(void)
{
	_core = new ffmpeg_decoder_core();
}

dk_ff_video_decoder::~dk_ff_video_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_ff_video_decoder::ERR_CODE dk_ff_video_decoder::initialize_decoder(CONFIGURATION_T * config)
{
	return _core->initialize_decoder(config);
}

dk_ff_video_decoder::ERR_CODE dk_ff_video_decoder::release_decoder(void)
{
	return _core->release_decoder();
}

dk_ff_video_decoder::ERR_CODE dk_ff_video_decoder::decode(DK_VIDEO_ENTITY_T * bitstream)
{
	return _core->decode(bitstream);
}