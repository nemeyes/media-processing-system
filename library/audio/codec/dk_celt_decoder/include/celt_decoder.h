#ifndef _CELT_DECODER_H_
#define _CELT_DECODER_H_

#include "dk_celt_decoder.h"

#include <opus.h>

namespace debuggerking
{
	class celt_core
	{
	public:
		celt_core(celt_decoder * front);
		~celt_core(void);

		int32_t initialize_decoder(celt_decoder::configuration_t * config);
		int32_t release_decoder(void);
		int32_t decode(celt_decoder::entity_t * encoded, celt_decoder::entity_t * pcm);
		int32_t decode(celt_decoder::entity_t * encoded);
		int32_t get_queued_data(celt_decoder::entity_t * pcm);

	private:
		celt_decoder * _front;
		celt_decoder::configuration_t _config;
		OpusDecoder * _decoder;
		unsigned long _framesize;

		int16_t * _buffer;
		size_t _buffer_pos;

	};
};

#endif