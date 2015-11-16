#include "dk_celt_decoder.h"

#include <opus.h>

class celt_decoder
{
public:
	celt_decoder(void);
	~celt_decoder(void);

	dk_celt_decoder::ERR_CODE initialize(dk_celt_decoder::configuration_t * config);
	dk_celt_decoder::ERR_CODE decode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize);
	dk_celt_decoder::ERR_CODE release(void);


private:
	dk_celt_decoder::configuration_t _config;
	OpusDecoder * _decoder;

};