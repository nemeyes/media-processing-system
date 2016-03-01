#include "dk_media_sdk_decoder.h"
#include "media_sdk_decoder_core.h"

dk_media_sdk_decoder::dk_media_sdk_decoder(void)
{
	_core = new media_sdk_decoder_core();
}

dk_media_sdk_decoder::~dk_media_sdk_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_media_sdk_decoder::ERR_CODE dk_media_sdk_decoder::initialize(unsigned int width, unsigned int height)
{
	return _core->initialize(width, height);
}

dk_media_sdk_decoder::ERR_CODE dk_media_sdk_decoder::release(void)
{
	return _core->release();
}

dk_media_sdk_decoder::ERR_CODE dk_media_sdk_decoder::decode(unsigned char * input, size_t isize, unsigned int stride, unsigned char * output, size_t & osize)
{
	return _core->decode(input, isize, stride, output, osize);
}