#include "dk_aac_encoder.h"
#include "aac_enc_core.h"

dk_aac_encoder::dk_aac_encoder(void)
{
	_core = new aac_enc_core();
}

dk_aac_encoder::~dk_aac_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, unsigned char * extra_data, unsigned long & extra_data_size)
{
	return _core->initialize(config, input_samples, max_output_bytes, extra_data, extra_data_size);
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::release(void)
{
	return _core->release();
}

dk_aac_encoder::ERR_CODE dk_aac_encoder::encode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int &osize)
{
	return _core->encode(input, isize, output, osize);
}



