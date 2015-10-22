
#include "dk_aac_encoder.h"

extern "C" {
#include <faac.h>
}

class aac_enc_core
{
public:
	aac_enc_core(void);
	~aac_enc_core(void);

	dk_aac_encoder::ERR_CODE initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, unsigned char * extra_data, unsigned long & extra_data_size);
	dk_aac_encoder::ERR_CODE release(void);
	dk_aac_encoder::ERR_CODE encode(unsigned char * input, unsigned int isize, unsigned char * output, unsigned int &osize);

private:
	dk_aac_encoder::configuration_t _config;
	faacEncHandle _faac_encoder;

	int * _pcm_buffer;
	unsigned long _pcm_buffer_index;
};
