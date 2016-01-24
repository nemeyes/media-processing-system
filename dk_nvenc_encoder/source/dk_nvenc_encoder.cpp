#include <windows.h>
#include <tchar.h>
#include "nvenc_encoder.h"
#include "dk_nvenc_encoder.h"
#include "nvEncodeAPI.h"
#include <cstdio>
#include <cstdlib>
#include <stdio.h>

typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);

dk_nvenc_encoder::dk_nvenc_encoder(void)
{
	_core = new nvenc_encoder();
}

dk_nvenc_encoder::~dk_nvenc_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::initialize_encoder(void * config)
{
	return _core->initialize_encoder(static_cast<dk_nvenc_encoder::configuration_t*>(config));
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::release_encoder(void)
{
	return _core->release_encoder();
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input, dk_nvenc_encoder::dk_video_entity_t * bitstream)
{
	return _core->encode(input, bitstream);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input)
{
	return _core->encode(input);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream)
{
	return _core->get_queued_data(bitstream);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::encode_async(dk_nvenc_encoder::dk_video_entity_t * input)
{
	return _core->encode_async(input);
}

dk_nvenc_encoder::ERR_CODE dk_nvenc_encoder::check_encoding_flnish(void)
{
	return _core->check_encoding_flnish();
}