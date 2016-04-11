
#include "dk_aac_encoder.h"

extern "C" {
#include <faac.h>
}

class faac_encoder
{
public:
	faac_encoder(dk_aac_encoder * front);
	~faac_encoder(void);

	dk_aac_encoder::err_code initialize_encoder(dk_aac_encoder::configuration_t * config);
	dk_aac_encoder::err_code release_encoder(void);

	dk_aac_encoder::err_code encode(dk_aac_encoder::dk_audio_entity_t * pcm, dk_aac_encoder::dk_audio_entity_t * encoded);
	dk_aac_encoder::err_code encode(dk_aac_encoder::dk_audio_entity_t * pcm);
	dk_aac_encoder::err_code get_queued_data(dk_aac_encoder::dk_audio_entity_t * encoded);

	virtual uint8_t * extradata(void);
	virtual size_t extradata_size(void);

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
