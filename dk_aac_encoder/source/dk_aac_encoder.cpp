#include "dk_aac_encoder.h"
#include "faac_encoder.h"

dk_aac_encoder::dk_aac_encoder(void)
{
	_core = new faac_encoder();
}

dk_aac_encoder::~dk_aac_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, uint8_t * extra_data, size_t & extra_data_size)
{
	return _core->initialize(config, input_samples, max_output_bytes, extra_data, extra_data_size);
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::release(void)
{
	return _core->release();
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::encode(uint8_t * input, size_t isize, uint8_t * output, size_t osize)
{
	return _core->encode(input, isize, output, osize);
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::encode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize, int64_t & encode_done)
{
	return _core->encode(input, isize, output, osize, encode_done);
}



