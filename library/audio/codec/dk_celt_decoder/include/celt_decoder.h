#include "dk_celt_decoder.h"

#include <opus.h>

class celt_decoder
{
public:
	celt_decoder(dk_celt_decoder * front);
	~celt_decoder(void);

	dk_celt_decoder::err_code initialize_decoder(dk_celt_decoder::configuration_t * config);
	dk_celt_decoder::err_code release_decoder(void);
	dk_celt_decoder::err_code decode(dk_celt_decoder::dk_audio_entity_t * encoded, dk_celt_decoder::dk_audio_entity_t * pcm);

private:
	dk_celt_decoder * _front;
	dk_celt_decoder::configuration_t _config;
	OpusDecoder * _decoder;
	unsigned long _framesize;

	int16_t * _buffer;
	size_t _buffer_pos;

};