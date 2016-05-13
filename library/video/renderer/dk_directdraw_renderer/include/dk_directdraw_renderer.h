#ifndef _DK_DIRECTDRAW_RENDERER_H_
#define _DK_DIRECTDRAW_RENDERER_H_

#include <dk_video_base.h>
namespace debuggerking
{
	class directdraw_core;
	class EXP_CLASS directdraw_renderer : public video_renderer
	{
	public:
		directdraw_renderer(void);
		virtual ~directdraw_renderer(void);

		int32_t initialize_renderer(void * config);
		int32_t release_renderer(void);
		int32_t render(directdraw_renderer::entity_t * decoded);
	
	private:
		directdraw_core * _core;
	};
};

#endif