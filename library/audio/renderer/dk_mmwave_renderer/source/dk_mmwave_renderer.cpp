#include "dk_mmwave_renderer.h"
#include "mmwave_renderer.h"

dk_mmwave_renderer::dk_mmwave_renderer(void)
{
	_core = new mmwave_renderer();
}

dk_mmwave_renderer::~dk_mmwave_renderer(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_mmwave_renderer::err_code dk_mmwave_renderer::initialize_renderer(void * config)
{
	return _core->initialize_renderer(static_cast<dk_mmwave_renderer::configuration_t*>(config));
}

dk_mmwave_renderer::err_code dk_mmwave_renderer::release_renderer(void)
{
	return _core->release_renderer();
}

dk_mmwave_renderer::err_code dk_mmwave_renderer::render(dk_mmwave_renderer::dk_audio_entity_t * pcm)
{
	return _core->render(pcm);
}