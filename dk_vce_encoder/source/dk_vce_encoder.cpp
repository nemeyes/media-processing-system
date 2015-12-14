#include "dk_vce_encoder.h"
#include "vce_encoder.h"

dk_vce_encoder::dk_vce_encoder(void)
{
	_core = new vce_encoder(this);
}

dk_vce_encoder::~dk_vce_encoder(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::initialize(void * config)
{
	return _core->initialize(static_cast<dk_vce_encoder::configuration_t*>(config));
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::release(void)
{
	return _core->release();
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::encode(dk_vce_encoder::dk_video_entity_t * rawstream, dk_vce_encoder::dk_video_entity_t * bitstream)
{
	return _core->encode(rawstream, bitstream);
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::encode(dk_vce_encoder::dk_video_entity_t * rawstream)
{
	return _core->encode(rawstream);
}

dk_vce_encoder::ERR_CODE dk_vce_encoder::get_queued_data(dk_vce_encoder::dk_video_entity_t * bitstream)
{
	return _core->get_queued_data(bitstream);
}

/*dk_vce_encoder::ERR_CODE dk_vce_encoder::encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, dk_vce_encoder::PIC_TYPE & pic_type, bool flush)
{
	return _core->encode(input, isize, output, osize, pic_type, flush);
}*/