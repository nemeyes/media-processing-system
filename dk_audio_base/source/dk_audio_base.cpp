#include "dk_audio_base.h"

dk_audio_base::dk_audio_base(void)
{

}

dk_audio_base::~dk_audio_base(void)
{

}

dk_audio_base::ERR_CODE dk_audio_base::put_audio(dk_audio_entity_t * samples)
{
	return ERR_CODE_SUCCESS;
}

dk_audio_base::ERR_CODE dk_audio_base::get_audio(dk_audio_entity_t * samples)
{
	return ERR_CODE_SUCCESS;
}


dk_audio_decoder::dk_audio_decoder(void)
{

}

dk_audio_decoder::~dk_audio_decoder(void)
{

}

/*dk_audio_decoder::ERR_CODE dk_audio_decoder::initialize_decoder(configuration_t * config)
{
	return ERR_CODE_SUCCESS;
}*/

dk_audio_decoder::ERR_CODE dk_audio_decoder::release_decoder(void)
{
	return ERR_CODE_SUCCESS;
}

dk_audio_decoder::ERR_CODE dk_audio_decoder::decode(dk_audio_entity_t * encoded, dk_audio_entity_t * decoded)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_audio_decoder::ERR_CODE dk_audio_decoder::decode(dk_audio_entity_t * encoded)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_audio_encoder::dk_audio_encoder(void)
{

}

dk_audio_encoder::~dk_audio_encoder(void)
{

}

/*dk_audio_encoder::ERR_CODE dk_audio_encoder::initialize_encoder(dk_audio_encoder::configuration_t * config)
{
	return ERR_CODE_SUCCESS;
}

dk_audio_encoder::ERR_CODE dk_audio_encoder::release_encoder(void)
{
	return ERR_CODE_SUCCESS;
}*/
