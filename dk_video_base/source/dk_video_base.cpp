#include "dk_video_base.h"

dk_video_base::dk_video_base(void)
{

}

dk_video_base::~dk_video_base(void)
{

}

dk_video_base::ERR_CODE dk_video_base::put_video(dk_video_entity_t * bitstream)
{
	return ERR_CODE_SUCCESS;
}

dk_video_base::ERR_CODE dk_video_base::get_video(dk_video_entity_t * bitstream)
{
	return ERR_CODE_SUCCESS;
}


dk_video_decoder::dk_video_decoder(void)
{

}

dk_video_decoder::~dk_video_decoder(void)
{

}

dk_video_decoder::ERR_CODE dk_video_decoder::initialize_decoder(void * config)
{
	return ERR_CODE_SUCCESS;
}

dk_video_decoder::ERR_CODE dk_video_decoder::release_decoder(void)
{
	return ERR_CODE_SUCCESS;
}

dk_video_decoder::ERR_CODE dk_video_decoder::decode(dk_video_entity_t * bitstream, dk_video_entity_t * decoded)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_decoder::ERR_CODE dk_video_decoder::decode(dk_video_entity_t * bitstream)
{
	return ERR_CODE_NOT_IMPLEMENTED;
}

dk_video_encoder::dk_video_encoder(void)
{

}

dk_video_encoder::~dk_video_encoder(void)
{

}

dk_video_encoder::ERR_CODE dk_video_encoder::initialize_encoder(void)
{
	return ERR_CODE_SUCCESS;
}

dk_video_encoder::ERR_CODE dk_video_encoder::release_encoder(void)
{
	return ERR_CODE_SUCCESS;
}
