#ifndef _FAAC_DECODER_H_
#define _FAAC_DECODER_H_

#include <faad.h>
#include "dk_aac_decoder.h"

class faad2_decoder
{
public:
	faad2_decoder(void);
	~faad2_decoder(void);

	dk_aac_decoder::ERR_CODE initialize_decoder(dk_aac_decoder::configuration_t * config);
	dk_aac_decoder::ERR_CODE release_decoder(void);

	dk_aac_decoder::ERR_CODE decode(dk_audio_entity_t * encoded, dk_audio_entity_t * pcm);


	/*dk_aac_decoder::ERR_CODE initialize(dk_aac_decoder::configuration_t config, unsigned char * extra_data, int extra_data_size, int & samplerate, int & channels);
	dk_aac_decoder::ERR_CODE release(void);
	dk_aac_decoder::ERR_CODE decode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int & osize);*/

private:
	dk_aac_decoder::configuration_t _config;
	//unsigned char * _extra_data;
	//int _extra_data_size;

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