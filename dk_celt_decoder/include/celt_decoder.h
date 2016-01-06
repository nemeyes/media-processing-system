#include "dk_celt_decoder.h"

#include <opus.h>

class celt_decoder
{
public:
	celt_decoder(void);
	~celt_decoder(void);

	dk_celt_decoder::ERR_CODE initialize_decoder(dk_celt_decoder::configuration_t * config);
	dk_celt_decoder::ERR_CODE release_decoder(void);

	dk_celt_decoder::ERR_CODE decode(dk_celt_decoder::dk_audio_entity_t * encoded, dk_celt_decoder::dk_audio_entity_t * pcm);
private:
	dk_celt_decoder::configuration_t _config;
	OpusDecoder * _decoder;
	unsigned long _framesize;

	int16_t * _buffer;
	size_t _buffer_pos;

};