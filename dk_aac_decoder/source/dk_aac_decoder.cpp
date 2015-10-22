#include "dk_aac_decoder.h"
#include "aac_dec_core.h"

dk_aac_decoder::dk_aac_decoder(void)
{
	_core = new aac_dec_core();
}

dk_aac_decoder::~dk_aac_decoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_aac_decoder::ERR_CODE dk_aac_decoder::initialize(dk_aac_decoder::configuration_t config, unsigned char * extra_data, int extra_data_size, int & samplerate, int & channels)
{
	return _core->initialize(config, extra_data, extra_data_size, samplerate, channels);
}

dk_aac_decoder::ERR_CODE dk_aac_decoder::release(void)
{
	return _core->release();
}

dk_aac_decoder::ERR_CODE dk_aac_decoder::decode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int & osize)
{
	return _core->decode(input, isize, output, osize);
}