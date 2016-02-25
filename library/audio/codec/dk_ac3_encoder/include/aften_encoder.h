
#include "dk_ac3_encoder.h"

extern "C" {
	#include <a52.h>
}

class aften_encoder
{
public:


	aften_encoder(dk_ac3_encoder * front);
	~aften_encoder(void);

	dk_ac3_encoder::ERR_CODE initialize_encoder(dk_ac3_encoder::configuration_t * config);
	dk_ac3_encoder::ERR_CODE release_encoder(void);

	dk_ac3_encoder::ERR_CODE encode(dk_audio_entity_t * pcm, dk_audio_entity_t * encoded);
	dk_ac3_encoder::ERR_CODE encode(dk_audio_entity_t * pcm);
	dk_ac3_encoder::ERR_CODE get_queued_data(dk_audio_entity_t * encoded);

	virtual uint8_t * extradata(void);
	virtual size_t extradata_size(void);

private:
	dk_ac3_encoder * _front;
	dk_ac3_encoder::configuration_t _config;


	AftenContext _aften_context;



	/*uint8_t * _extradata;
	unsigned long  _extradata_size;*/

	int * _buffer;
	unsigned long _buffer_index;

	uint8_t * _buffer4queue;
};
