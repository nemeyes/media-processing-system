#include <tchar.h>
#include "dk_directdraw_renderer.h"
#include "directdraw_renderer.h"

debuggerking::directdraw_renderer::directdraw_renderer(void)
{
	_core = new directdraw_core(this);
}

debuggerking::directdraw_renderer::~directdraw_renderer(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t debuggerking::directdraw_renderer::initialize_renderer(void * config)
{
	return _core->initialize_renderer(static_cast<directdraw_renderer::configuration_t*>(config));
}

int32_t debuggerking::directdraw_renderer::release_renderer(void)
{
	return _core->release_renderer();
}

int32_t debuggerking::directdraw_renderer::render(directdraw_renderer::entity_t * p)
{
	return _core->render(p);
}