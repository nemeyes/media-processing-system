#ifndef _AAC_DEC_CORE_H_
#define _AAC_DEC_CORE_H_

#include <faad.h>
#include "dk_aac_decoder.h"

class aac_dec_core
{
public:
	aac_dec_core(void);
	~aac_dec_core(void);

	dk_aac_decoder::ERR_CODE initialize(dk_aac_decoder::configuration_t config, unsigned char * extra_data, int extra_data_size, int & samplerate, int & channels);
	dk_aac_decoder::ERR_CODE release(void);
	dk_aac_decoder::ERR_CODE decode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int & osize);

private:
	dk_aac_decoder::configuration_t _config;
	unsigned char * _extra_data;
	int _extra_data_size;

	unsigned char * _buffer;
	unsigned long _buffer_size;

	unsigned int _calc_frames;
	unsigned int _bytes_consumed;
	unsigned int _decoded_frames;

	NeAACDecHandle _aac_decoder;
	NeAACDecFrameInfo _aac_frame_info;
	NeAACDecConfigurationPtr _aac_config;
};














#endif