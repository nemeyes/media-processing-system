#ifndef _FAAC_ENCODER_H_
#define _FAAC_ENCODER_H_

#include "dk_aac_encoder.h"

extern "C" {
#include <faac.h>
}

namespace debuggerking
{
	class faac_encoder
	{
	public:
		faac_encoder(aac_encoder * front);
		~faac_encoder(void);

		int32_t initialize_encoder(aac_encoder::configuration_t * config);
		int32_t release_encoder(void);

		int32_t encode(aac_encoder::entity_t * pcm, aac_encoder::entity_t * encoded);
		int32_t encode(aac_encoder::entity_t * pcm);
		int32_t get_queued_data(aac_encoder::entity_t * encoded);

	private:
		aac_encoder * _front;
		aac_encoder::configuration_t _config;
		faacEncHandle _faac_encoder;

		uint8_t * _extradata;
		unsigned long  _extradata_size;

		int * _buffer;
		unsigned long _buffer_index;

		uint8_t * _buffer4queue;
	};
};

#endif