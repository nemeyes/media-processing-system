#include "celt_decoder.h"


celt_decoder::celt_decoder(void)
	: _decoder(nullptr)
{

}

celt_decoder::~celt_decoder(void)
{

}

dk_celt_decoder::ERR_CODE celt_decoder::initialize(dk_celt_decoder::configuration_t * config)
{
	_config = *config;

	int32_t err;
	_decoder = opus_decoder_create(config->samplerate, config->channels, &err);
	if (err != OPUS_OK || _decoder == NULL)
		return dk_celt_decoder::ERR_CODE_FAILED;
	
	return dk_celt_decoder::ERR_CODE_SUCCESS;
}

dk_celt_decoder::ERR_CODE celt_decoder::decode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize)
{
	int32_t bytes_written = 0;
	bytes_written = opus_decode(_decoder, input, (int)isize, (opus_int16*)output, osize, 0);
	osize = 0;
	if (bytes_written > 0)
	{
		osize = bytes_written;
	}
	return dk_celt_decoder::ERR_CODE_SUCCESS;
}

dk_celt_decoder::ERR_CODE celt_decoder::release(void)
{
	if (_decoder)
	{
		opus_decoder_destroy(_decoder);
		_decoder = nullptr;
	}
	return dk_celt_decoder::ERR_CODE_SUCCESS;
}