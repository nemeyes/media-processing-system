#ifndef _DK_FF_MP3_DECODER_H_
#define _DK_FF_MP3_DECODER_H_

#include <cstdint>

#include <dk_audio_base.h>

class mp3_decoder;
class EXP_CLASS dk_ff_mp3_decoder : public dk_audio_decoder
{
public:
	dk_ff_mp3_decoder(void);
	virtual ~dk_ff_mp3_decoder(void);

	dk_ff_mp3_decoder::err_code initialize_decoder(void* config);
	dk_ff_mp3_decoder::err_code release_decoder(void);
	dk_ff_mp3_decoder::err_code decode(dk_ff_mp3_decoder::dk_audio_entity_t * encoded, dk_ff_mp3_decoder::dk_audio_entity_t * pcm);

private:
	mp3_decoder * _core;
};



#endif