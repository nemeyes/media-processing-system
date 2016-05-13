#include "dk_mmwave_renderer.h"
#include "mmwave_renderer.h"

debuggerking::mmwave_renderer::mmwave_renderer(void)
{
	_core = new mmwave_core();
}

debuggerking::mmwave_renderer::~mmwave_renderer(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t debuggerking::mmwave_renderer::initialize_renderer(void * config)
{
	return _core->initialize_renderer(static_cast<mmwave_renderer::configuration_t*>(config));
}

int32_t debuggerking::mmwave_renderer::release_renderer(void)
{
	return _core->release_renderer();
}

int32_t debuggerking::mmwave_renderer::render(mmwave_renderer::entity_t * pcm)
{
	return _core->render(pcm);
}