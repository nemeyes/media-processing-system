#ifndef _DK_FF_VIDEO_DECODER_H_
#define _DK_FF_VIDEO_DECODER_H_

#include <dk_base_video.h>

class ffmpeg_decoder_core;
class EXP_CLASS dk_ff_video_decoder : public dk_base_video_decoder
{
public:
	dk_ff_video_decoder(void);
	~dk_ff_video_decoder(void);

	ERR_CODE initialize_decoder(CONFIGURATION_T * config);
	ERR_CODE release_decoder(void);
	ERR_CODE decode(DK_VIDEO_ENTITY_T * bitstream);

private:
	ffmpeg_decoder_core * _core;
};

#endif