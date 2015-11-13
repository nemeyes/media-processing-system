#include "dk_celt_encoder.h"
#include "celt_encoder.h"

dk_celt_encoder::dk_celt_encoder(void)
{
	_core = new celt_encoder();
}

dk_celt_encoder::~dk_celt_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::initialize(dk_celt_encoder::configuration_t * config)
{
	return _core->initialize(config);
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::encode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize)
{
	return _core->encode(input, isize, output, osize);
}

dk_celt_encoder::ERR_CODE dk_celt_encoder::release(void)
{
	return _core->release();
}