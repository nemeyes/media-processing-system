
#include "dk_aac_encoder.h"

extern "C" {
#include <faac.h>
}

class faac_encoder
{
public:
	faac_encoder(dk_aac_encoder * front);
	~faac_encoder(void);

	dk_aac_encoder::ERR_CODE initialize_encoder(dk_aac_encoder::configuration_t * config);
	dk_aac_encoder::ERR_CODE release_encoder(void);

	dk_aac_encoder::ERR_CODE encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded);
	dk_aac_encoder::ERR_CODE encode(dk_audio_entity_t * pcm);
	dk_aac_encoder::ERR_CODE get_queued_data(dk_audio_entity_t * encoded);

	virtual uint8_t * extradata(void);
	virtual size_t extradata_size(void);
	
	/*dk_aac_encoder::ERR_CODE initialize(dk_aac_encoder::configuration_t config, unsigned long & input_samples, unsigned long & max_output_bytes, uint8_t * extra_data, size_t & extra_data_size);
	dk_aac_encoder::ERR_CODE release(void);

	dk_aac_encoder::ERR_CODE encode(int32_t * input, size_t isize, uint8_t * output, size_t osize, size_t & bytes_written);
	dk_aac_encoder::ERR_CODE encode(uint8_t * input, size_t isize, uint8_t * output, size_t & osize);*/

private:
	dk_aac_encoder * _front;
	dk_aac_encoder::configuration_t _config;
	faacEncHandle _faac_encoder;

	uint8_t * _extradata;
	unsigned long  _extradata_size;

	int * _buffer;
	unsigned long _buffer_index;

	uint8_t * _buffer4queue;
};
