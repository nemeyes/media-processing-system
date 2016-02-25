#ifndef _DK_AC3_ENCODER_H_
#define _DK_AC3_ENCODER_H_

#include <cstdint>

#include <dk_audio_base.h>

class aften_encoder;
class EXP_CLASS dk_ac3_encoder : public dk_audio_encoder
{
public:
	/*
	A52_SAMPLE_FMT_U8 = 0,
	A52_SAMPLE_FMT_S16,
	A52_SAMPLE_FMT_S20,
	A52_SAMPLE_FMT_S24,
	A52_SAMPLE_FMT_S32,
	A52_SAMPLE_FMT_FLT,
	A52_SAMPLE_FMT_DBL
	*/
	typedef struct EXP_CLASS _configuration_t
	{

		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;


	dk_ac3_encoder(void);
	virtual ~dk_ac3_encoder(void);

	dk_ac3_encoder::ERR_CODE initialize_encoder(void * config);
	dk_ac3_encoder::ERR_CODE release_encoder(void);

	dk_ac3_encoder::ERR_CODE encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded);
	dk_ac3_encoder::ERR_CODE encode(dk_audio_entity_t * pcm);
	dk_ac3_encoder::ERR_CODE get_queued_data(dk_audio_entity_t * encoded);

	virtual uint8_t * extradata(void);
	virtual size_t extradata_size(void);

private:
	aften_encoder * _core;
};




#endif