#include "dk_base_video.h"

dk_base_video::dk_base_video(void)
{

}

dk_base_video::~dk_base_video(void)
{

}

dk_base_video::ERR_CODE dk_base_video::put_video(DK_VIDEO_ENTITY_T * bitstream)
{
	return ERR_CODE_SUCCESS;
}

dk_base_video::ERR_CODE dk_base_video::get_video(DK_VIDEO_ENTITY_T * bitstream)
{
	return ERR_CODE_SUCCESS;
}


dk_base_video_decoder::dk_base_video_decoder(void)
{

}

dk_base_video_decoder::~dk_base_video_decoder(void)
{

}

dk_base_video_decoder::ERR_CODE dk_base_video_decoder::initialize_decoder(CONFIGURATION_T * config)
{
	return ERR_CODE_SUCCESS;
}

dk_base_video_decoder::ERR_CODE dk_base_video_decoder::release_decoder(void)
{
	return ERR_CODE_SUCCESS;
}

dk_base_video_encoder::dk_base_video_encoder(void)
{

}

dk_base_video_encoder::~dk_base_video_encoder(void)
{

}

dk_base_video_encoder::ERR_CODE dk_base_video_encoder::initialize_encoder(void)
{
	return ERR_CODE_SUCCESS;
}

dk_base_video_encoder::ERR_CODE dk_base_video_encoder::release_encoder(void)
{
	return ERR_CODE_SUCCESS;
}
