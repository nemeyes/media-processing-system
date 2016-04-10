#ifndef _FAAC_DECODER_H_
#define _FAAC_DECODER_H_

#include <faad.h>
#include "dk_aac_decoder.h"

class faad2_decoder
{
public:
	faad2_decoder(dk_aac_decoder * front);
	~faad2_decoder(void);

	dk_aac_decoder::err_code initialize_decoder(dk_aac_decoder::configuration_t * config);
	dk_aac_decoder::err_code release_decoder(void);
	dk_aac_decoder::err_code decode(dk_aac_decoder::dk_audio_entity_t * encoded, dk_aac_decoder::dk_audio_entity_t * pcm);

private:
	dk_aac_decoder * _front;
	dk_aac_decoder::configuration_t _config;

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

#endif