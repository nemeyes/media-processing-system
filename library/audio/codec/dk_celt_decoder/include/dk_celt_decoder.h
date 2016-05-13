#ifndef _DK_CELT_DECODER_H_
#define _DK_CELT_DECODER_H_

#include <dk_audio_base.h>

namespace debuggerking
{
	class celt_core;
	class EXP_CLASS celt_decoder : public audio_decoder
	{
	public:
		typedef struct EXP_CLASS _configuration_t : public audio_decoder::configuration_t
		{
			int32_t framesize;
			_configuration_t(void);
			_configuration_t(const _configuration_t & clone);
			_configuration_t operator=(const _configuration_t & clone);
		} configuration_t;

		celt_decoder(void);
		virtual ~celt_decoder(void);

		int32_t initialize_decoder(void* config);
		int32_t release_decoder(void);
		int32_t decode(celt_decoder::entity_t * encoded, celt_decoder::entity_t * pcm);
		int32_t decode(celt_decoder::entity_t * encoded);
		int32_t get_queued_data(celt_decoder::entity_t * pcm);
		virtual void after_decoding_callback(uint8_t * decoded, size_t size);

	private:
		celt_core * _core;
	};
};


#endif