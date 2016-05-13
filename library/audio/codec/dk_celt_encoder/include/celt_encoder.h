#ifndef _CELT_DECODER_H_
#define _CELT_DECODER_H_

#include "dk_celt_encoder.h"

#include <opus.h>
#include "speex_resampler.h"

namespace debuggerking
{
	class celt_core
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

		celt_core(celt_encoder * front);
		~celt_core(void);

		int32_t initialize_encoder(celt_encoder::configuration_t * config);
		int32_t release_encoder(void);

		int32_t encode(celt_encoder::entity_t * encoded, celt_encoder::entity_t * decoded);
		int32_t encode(celt_encoder::entity_t * pcm);
		int32_t get_queued_data(celt_encoder::entity_t * encoded);

	private:
		int32_t setup_resampler(int32_t samplerate, int32_t codingrate, int32_t channels, int32_t complexity);
		int32_t clear_resampler(void);



	private:
		celt_encoder * _front;

		celt_encoder::configuration_t _config;
		OpusEncoder * _encoder;
		unsigned long _framesize;

		resampler_t * _resampler;
		int32_t _skip;

		int16_t * _buffer;
		size_t _buffer_pos;

		uint8_t * _buffer4queue;
		size_t _buffer4queue_size;

	};
};

#endif