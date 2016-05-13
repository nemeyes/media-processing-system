#ifndef _DK_CELT_ENCODER_H_
#define _DK_CELT_ENCODER_H_

#include <dk_audio_base.h>

namespace debuggerking
{
	class celt_core;
	class EXP_CLASS celt_encoder : public audio_encoder
	{
		friend class celt_core;
	public:
		typedef struct EXP_CLASS _configuration_t : public audio_encoder::configuration_t
		{
			int32_t codingrate;
			int32_t framesize;
			int32_t complexity;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t operator=(const _configuration_t & clone);
		} configuration_t;

		celt_encoder(void);
		virtual ~celt_encoder(void);

		int32_t initialize_encoder(void * config);
		int32_t release_encoder(void);

		int32_t encode(celt_encoder::entity_t * pcm, celt_encoder::entity_t * encoded);
		int32_t encode(celt_encoder::entity_t * pcm);
		int32_t get_queued_data(celt_encoder::entity_t * encoded);
		virtual void after_encoding_callback(uint8_t * bistream, size_t size);

	private:
		celt_core * _core;
	};
};


#endif