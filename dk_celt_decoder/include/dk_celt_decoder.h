#ifndef _DK_CELT_DECODER_H_
#define _DK_CELT_DECODER_H_

#include <cstdint>

#include <dk_audio_base.h>

class celt_decoder;
class EXP_CLASS dk_celt_decoder : public dk_audio_decoder
{
public:
	dk_celt_decoder(void);
	~dk_celt_decoder(void);

	dk_celt_decoder::ERR_CODE initialize_decoder(dk_celt_decoder::configuration_t * config);
	dk_celt_decoder::ERR_CODE decode(dk_audio_entity_t * encoded, dk_audio_entity_t * pcm);
	//dk_celt_decoder::ERR_CODE decode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize);
	dk_celt_decoder::ERR_CODE release_decoder(void);

private:
	celt_decoder * _core;
};



#endif