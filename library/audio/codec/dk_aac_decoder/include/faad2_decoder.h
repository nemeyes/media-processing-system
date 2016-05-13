#ifndef _FAAC_DECODER_H_
#define _FAAC_DECODER_H_

#include <faad.h>
#include "dk_aac_decoder.h"

namespace debuggerking
{
	class faad2_decoder
	{
	public:
		faad2_decoder(aac_decoder * front);
		~faad2_decoder(void);

		int32_t initialize_decoder(aac_decoder::configuration_t * config);
		int32_t release_decoder(void);
		int32_t decode(aac_decoder::entity_t * encoded, aac_decoder::entity_t * pcm);
		int32_t decode(aac_decoder::entity_t * encoded);
		int32_t get_queued_data(aac_decoder::entity_t * pcm);

	private:
		aac_decoder * _front;
		aac_decoder::configuration_t _config;

		unsigned char * _buffer;
		unsigned long _buffer_size;

		unsigned int _calc_frames;
		unsigned int _bytes_consumed;
		unsigned int _decoded_frames;

		NeAACDecHandle _aac_decoder;
		NeAACDecFrameInfo _aac_frame_info;
		NeAACDecConfigurationPtr _aac_config;
		uint8_t _channels;
	};
};


#endif