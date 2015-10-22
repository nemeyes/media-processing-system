#include "dk_x264_encoder.h"
#include "x264_encoder_core.h"

dk_x264_encoder::dk_x264_encoder(void)
	: _core(0)
{
	_core = new x264_encoder_core();
}

dk_x264_encoder::~dk_x264_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_x264_encoder::ERR_CODE dk_x264_encoder::initialize(dk_x264_encoder::configuration_t conf)
{
	return _core->initialize(conf);
}

dk_x264_encoder::ERR_CODE dk_x264_encoder::release(void)
{
	return _core->release();
}

dk_x264_encoder::ERR_CODE dk_x264_encoder::encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, dk_x264_encoder::PIC_TYPE & pic_type, bool flush)
{
	return _core->encode(input, isize, output, osize, pic_type, flush);
}