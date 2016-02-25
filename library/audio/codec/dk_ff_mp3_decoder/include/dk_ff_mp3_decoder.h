#ifndef _DK_FF_MP3_DECODER_H_
#define _DK_FF_MP3_DECODER_H_

#include <cstdint>

#include <dk_audio_base.h>

class mp3_decoder;
class EXP_CLASS dk_ff_mp3_decoder : public dk_audio_decoder
{
public:
	typedef struct EXP_CLASS _configuration_t
	{
		int32_t samplerate;
		int32_t channels;
		int32_t bitdepth;
		_configuration_t(void);
		_configuration_t(const _configuration_t & clone);
		_configuration_t operator=(const _configuration_t & clone);
	} configuration_t;

	dk_ff_mp3_decoder(void);
	virtual ~dk_ff_mp3_decoder(void);

	dk_ff_mp3_decoder::ERR_CODE initialize_decoder(void* config);
	dk_ff_mp3_decoder::ERR_CODE release_decoder(void);

	dk_ff_mp3_decoder::ERR_CODE decode(dk_ff_mp3_decoder::dk_audio_entity_t * encoded, dk_ff_mp3_decoder::dk_audio_entity_t * pcm);


private:
	mp3_decoder * _core;
};



#endif