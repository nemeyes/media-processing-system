#ifndef _DK_FF_VIDEO_DECODER_H_
#define _DK_FF_VIDEO_DECODER_H_

#include <dk_video_base.h>

class ffmpeg_decoder_core;
class EXP_CLASS dk_ff_video_decoder : public dk_video_decoder
{
	friend class ffmpeg_decoder_core;
public:
	dk_ff_video_decoder(void);
	~dk_ff_video_decoder(void);

	ERR_CODE initialize_decoder(configuration_t * config);
	ERR_CODE release_decoder(void);
	ERR_CODE decode(dk_video_entity_t * encoded, dk_video_entity_t * decoded);

private:
	ffmpeg_decoder_core * _core;
};

#endif