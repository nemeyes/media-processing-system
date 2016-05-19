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

int32_t debuggerking::directdraw_renderer::enable_osd_text(bool enable)
{
	return _core->enable_osd_text(enable);
}

int32_t debuggerking::directdraw_renderer::set_osd_text(wchar_t * osd)
{
	return _core->set_osd_text(osd);
}

int32_t debuggerking::directdraw_renderer::set_osd_text_position(unsigned short x, unsigned short y)
{
	return _core->set_osd_text_position(x, y);
}

int32_t debuggerking::directdraw_renderer::set_osd_text_font_size(unsigned char size)
{
	return _core->set_osd_text_font_size(size);
}

int32_t debuggerking::directdraw_renderer::set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue)
{
	return _core->set_osd_text_color(red, green, blue);
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