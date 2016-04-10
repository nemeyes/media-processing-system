#ifndef _DK_MMWAVE_RENDERER_H_
#define _DK_MMWAVE_RENDERER_H_

#include <dk_audio_base.h>

class mmwave_renderer;
class EXP_CLASS dk_mmwave_renderer : public dk_audio_renderer
{
public:
	dk_mmwave_renderer(void);
	virtual ~dk_mmwave_renderer(void);

	dk_mmwave_renderer::err_code initialize_renderer(void * config);
	dk_mmwave_renderer::err_code release_renderer(void);
	dk_mmwave_renderer::err_code render(dk_mmwave_renderer::dk_audio_entity_t * decoded);

private:
	mmwave_renderer * _core;

};













#endif