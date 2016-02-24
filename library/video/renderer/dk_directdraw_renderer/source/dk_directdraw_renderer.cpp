#include <tchar.h>
#include "dk_directdraw_renderer.h"
#include "directdraw_renderer.h"

dk_directdraw_renderer::dk_directdraw_renderer(void)
{
	_core = new directdraw_renderer();
}

dk_directdraw_renderer::~dk_directdraw_renderer(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_directdraw_renderer::ERR_CODE dk_directdraw_renderer::initialize_renderer(void * config)
{
	return _core->initialize_renderer(static_cast<dk_directdraw_renderer::configuration_t*>(config));
}

dk_directdraw_renderer::ERR_CODE dk_directdraw_renderer::release_renderer(void)
{
	return _core->release_renderer();
}

dk_directdraw_renderer::ERR_CODE dk_directdraw_renderer::render(dk_directdraw_renderer::dk_video_entity_t * p)
{
	return _core->render(p);
}