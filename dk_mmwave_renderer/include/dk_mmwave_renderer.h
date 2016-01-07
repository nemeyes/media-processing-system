#ifndef _DK_MMWAVE_RENDERER_H_
#define _DK_MMWAVE_RENDERER_H_

#include <dk_audio_base.h>

class mmwave_renderer;
class EXP_CLASS dk_mmwave_renderer : public dk_audio_renderer
{
public:
	typedef struct EXP_CLASS _configuration_t
	{
		int32_t samplerate;
		int32_t bitdepth;
		int32_t channels;
		_configuration_t(void)
			: samplerate(0)
			, bitdepth(0)
			, channels(0)
		{
		
		}

		_configuration_t(const _configuration_t & clone)
		{
			samplerate = clone.samplerate;
			bitdepth = clone.bitdepth;
			channels = clone.channels;
		}

		_configuration_t & operator=(const _configuration_t & clone)
		{
			samplerate = clone.samplerate;
			bitdepth = clone.bitdepth;
			channels = clone.channels;
			return (*this);
		}
	} configuration_t;

	dk_mmwave_renderer(void);
	virtual ~dk_mmwave_renderer(void);

	dk_mmwave_renderer::ERR_CODE initialize_renderer(void * config);
	dk_mmwave_renderer::ERR_CODE release_renderer(void);
	dk_mmwave_renderer::ERR_CODE render(dk_mmwave_renderer::dk_audio_entity_t * decoded);

private:
	mmwave_renderer * _core;

};













#endif