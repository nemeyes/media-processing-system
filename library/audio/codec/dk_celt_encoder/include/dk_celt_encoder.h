#ifndef _DK_CELT_ENCODER_H_
#define _DK_CELT_ENCODER_H_

#include <cstdint>

#include <dk_audio_base.h>

class celt_encoder;
class EXP_CLASS dk_celt_encoder : public dk_audio_encoder
{
	friend class celt_encoder;
public:
	typedef struct EXP_CLASS _configuration_t : public dk_audio_encoder::configuration_t
	{
		int32_t codingrate;
		int32_t framesize;
		int32_t complexity;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;

	dk_celt_encoder(void);
	virtual ~dk_celt_encoder(void);

	dk_celt_encoder::err_code initialize_encoder(void * config);
	dk_celt_encoder::err_code release_encoder(void);

	dk_celt_encoder::err_code encode(dk_celt_encoder::dk_audio_entity_t * pcm, dk_celt_encoder::dk_audio_entity_t * encoded);
	dk_celt_encoder::err_code encode(dk_celt_encoder::dk_audio_entity_t * pcm);
	dk_celt_encoder::err_code get_queued_data(dk_celt_encoder::dk_audio_entity_t * encoded);
private:
	celt_encoder * _core;
};



#endif