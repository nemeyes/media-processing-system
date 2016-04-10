#ifndef _DK_DIRECTDRAW_RENDERER_H_
#define _DK_DIRECTDRAW_RENDERER_H_

#include <dk_video_base.h>

class directdraw_renderer;
class EXP_CLASS dk_directdraw_renderer : public dk_video_renderer
{
public:
	dk_directdraw_renderer(void);
	virtual ~dk_directdraw_renderer(void);

	dk_directdraw_renderer::err_code initialize_renderer(void * config);
	dk_directdraw_renderer::err_code release_renderer(void);
	dk_directdraw_renderer::err_code render(dk_video_entity_t * decoded);
	
private:
	directdraw_renderer * _core;

};


#endif