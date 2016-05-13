#include "dk_ff_mp3_decoder.h"
#include "mp3_decoder.h"

debuggerking::ff_mp3_decoder::ff_mp3_decoder(void)
{
	_core = new ffmpeg_core(this);
}

debuggerking::ff_mp3_decoder::~ff_mp3_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t debuggerking::ff_mp3_decoder::initialize_decoder(void * config)
{
	int32_t status = ff_mp3_decoder::initialize(static_cast<audio_base::configuration_t*>(config));
	if (status != ff_mp3_decoder::err_code_t::success)
		return status;
	return _core->initialize_decoder(static_cast<ff_mp3_decoder::configuration_t*>(config));
}

int32_t debuggerking::ff_mp3_decoder::release_decoder(void)
{
	int32_t status = _core->release_decoder();
	if (status != ff_mp3_decoder::err_code_t::success)
		return status;
	return audio_base::release();
}

int32_t debuggerking::ff_mp3_decoder::decode(ff_mp3_decoder::entity_t * encoded, ff_mp3_decoder::entity_t * pcm)
{
	return _core->decode(encoded, pcm);
}

int32_t debuggerking::ff_mp3_decoder::decode(audio_decoder::entity_t * encoded)
{
	return _core->decode(encoded);
}

int32_t debuggerking::ff_mp3_decoder::get_queued_data(audio_decoder::entity_t * pcm)
{
	return _core->get_queued_data(pcm);
}

void debuggerking::ff_mp3_decoder::after_decoding_callback(uint8_t * pcm, size_t size)
{

}