#ifndef _DK_CELT_ENCODER_H_
#define _DK_CELT_ENCODER_H_

#include <cstdint>

#include <dk_audio_base.h>

class celt_encoder;
class EXP_CLASS dk_celt_encoder : public dk_audio_encoder
{
public:
	dk_celt_encoder(void);
	~dk_celt_encoder(void);

	dk_celt_encoder::ERR_CODE initialize_encoder(dk_celt_encoder::configuration_t * config);
	dk_celt_encoder::ERR_CODE release_encoder(void);

	dk_celt_encoder::ERR_CODE encode(dk_audio_entity_t * encoded, dk_audio_entity_t * decoded);
	//dk_celt_encoder::ERR_CODE encode(int16_t * input, size_t isize, uint8_t * output, size_t & osize);
	

private:
	celt_encoder * _core;
};



#endif