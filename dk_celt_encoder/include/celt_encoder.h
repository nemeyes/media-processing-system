#include "dk_celt_encoder.h"

#include <opus.h>
#include "speex_resampler.h"

class celt_encoder
{
public:
	typedef struct _resampler_t 
	{
		SpeexResamplerState * resampler;
		int16_t * inbuffers;
		int16_t * outbuffers;
		int32_t channels;
		int32_t inbuffer_pos;
		int32_t inbuffer_size;
		int32_t outbuffer_pos;
		int32_t outbuffer_size;

		int32_t samplerate;
		int32_t codingrate;
		int done;
	} resampler_t;

	celt_encoder(dk_celt_encoder * front);
	~celt_encoder(void);

	dk_celt_encoder::ERR_CODE initialize_encoder(dk_celt_encoder::configuration_t * config);
	dk_celt_encoder::ERR_CODE release_encoder(void);

	//dk_celt_encoder::ERR_CODE encode(int16_t * input, size_t isize, uint8_t * output, size_t & osize);
	dk_celt_encoder::ERR_CODE encode(dk_audio_entity_t * encoded, dk_audio_entity_t * decoded);
	dk_celt_encoder::ERR_CODE encode(dk_audio_entity_t * pcm);
	dk_celt_encoder::ERR_CODE get_queued_data(dk_audio_entity_t * encoded);

private:
	dk_celt_encoder::ERR_CODE setup_resampler(int32_t samplerate, int32_t codingrate, int32_t channels, int32_t complexity);
	dk_celt_encoder::ERR_CODE clear_resampler(void);



private:
	dk_celt_encoder * _front;

	dk_celt_encoder::configuration_t _config;
	OpusEncoder * _encoder;
	unsigned long _framesize;

	resampler_t * _resampler;
	int32_t _skip;

	int16_t * _buffer;
	size_t _buffer_pos;

	uint8_t * _buffer4queue;
	size_t _buffer4queue_size;

};