#ifndef _DK_CELT_DECODER_H_
#define _DK_CELT_DECODER_H_

#include <cstdint>

#include <dk_audio_base.h>

class celt_decoder;
class EXP_CLASS dk_celt_decoder : public dk_audio_decoder
{
public:
	typedef struct EXP_CLASS _configuration_t : public dk_audio_decoder::configuration_t
	{
		int32_t framesize;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;

	dk_celt_decoder(void);
	virtual ~dk_celt_decoder(void);

	dk_celt_decoder::err_code initialize_decoder(void* config);
	dk_celt_decoder::err_code release_decoder(void);
	dk_celt_decoder::err_code decode(dk_celt_decoder::dk_audio_entity_t * encoded, dk_celt_decoder::dk_audio_entity_t * pcm);

private:
	celt_decoder * _core;
};



#endif