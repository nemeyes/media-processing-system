#ifndef _DK_FF_MP3_DECODER_H_
#define _DK_FF_MP3_DECODER_H_

#include <dk_audio_base.h>

namespace debuggerking
{
	class ffmpeg_core;
	class EXP_CLASS ff_mp3_decoder : public audio_decoder
	{
	public:
		ff_mp3_decoder(void);
		virtual ~ff_mp3_decoder(void);

		int32_t initialize_decoder(void* config);
		int32_t release_decoder(void);
		int32_t decode(ff_mp3_decoder::entity_t * encoded, ff_mp3_decoder::entity_t * pcm);
		int32_t decode(audio_decoder::entity_t * encoded);
		int32_t get_queued_data(audio_decoder::entity_t * pcm);
		virtual void after_decoding_callback(uint8_t * pcm, size_t size);

	private:
		ffmpeg_core * _core;
	};
};


#endif