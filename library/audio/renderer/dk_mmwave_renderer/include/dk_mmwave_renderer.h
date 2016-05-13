#ifndef _DK_MMWAVE_RENDERER_H_
#define _DK_MMWAVE_RENDERER_H_

#include <dk_audio_base.h>

namespace debuggerking
{
	class mmwave_core;
	class EXP_CLASS mmwave_renderer : public audio_renderer
	{
	public:
		mmwave_renderer(void);
		virtual ~mmwave_renderer(void);

		int32_t initialize_renderer(void * config);
		int32_t release_renderer(void);
		int32_t render(mmwave_renderer::entity_t * decoded);

	private:
		mmwave_core * _core;

	};
};













#endif