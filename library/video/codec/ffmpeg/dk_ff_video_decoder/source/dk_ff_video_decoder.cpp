#include "dk_ff_video_decoder.h"
#include "ffmpeg_decoder.h"

debuggerking::ff_video_decoder::ff_video_decoder(void)
{
	_core = new ffmpeg_core(this);
}

debuggerking::ff_video_decoder::~ff_video_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t debuggerking::ff_video_decoder::initialize_decoder(void * config)
{
	int32_t status = debuggerking::ff_video_decoder::err_code_t::fail;
	status = _core->initialize_decoder(static_cast<debuggerking::ff_video_decoder::configuration_t*>(config));
	return status;
}

int32_t debuggerking::ff_video_decoder::release_decoder(void)
{
	return _core->release_decoder();
}

int32_t debuggerking::ff_video_decoder::decode(video_decoder::entity_t * encoded, video_decoder::entity_t * decoded)
{
	return _core->decode(encoded, decoded);
}

int32_t debuggerking::ff_video_decoder::decode(video_decoder::entity_t * encoded)
{
	return _core->decode(encoded);
}

int32_t debuggerking::ff_video_decoder::get_queued_data(video_decoder::entity_t * decoded)
{
	return _core->get_queued_data(decoded);
}

void debuggerking::ff_video_decoder::after_decoding_callback(uint8_t * decoded, size_t size)
{

}