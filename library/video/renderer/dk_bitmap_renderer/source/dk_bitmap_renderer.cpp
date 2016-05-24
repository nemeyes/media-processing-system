#include <tchar.h>
#include "dk_bitmap_renderer.h"
#include "bitmap_renderer.h"

debuggerking::bitmap_renderer::bitmap_renderer(void)
{
	_core = new bitmap_core(this);
}

debuggerking::bitmap_renderer::~bitmap_renderer(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

int32_t debuggerking::bitmap_renderer::enable_osd_text(bool enable)
{
	return _core->enable_osd_text(enable);
}

int32_t debuggerking::bitmap_renderer::set_osd_text(wchar_t * osd)
{
	return _core->set_osd_text(osd);
}

int32_t debuggerking::bitmap_renderer::set_osd_text_position(unsigned short x, unsigned short y)
{
	return _core->set_osd_text_position(x, y);
}

int32_t debuggerking::bitmap_renderer::set_osd_text_font_size(unsigned char size)
{
	return _core->set_osd_text_font_size(size);
}

int32_t debuggerking::bitmap_renderer::set_osd_text_color(unsigned char red, unsigned char green, unsigned char blue)
{
	return _core->set_osd_text_color(red, green, blue);
}

int32_t debuggerking::bitmap_renderer::initialize_renderer(void * config)
{
	return _core->initialize_renderer(static_cast<bitmap_renderer::configuration_t*>(config));
}

int32_t debuggerking::bitmap_renderer::release_renderer(void)
{
	return _core->release_renderer();
}

int32_t debuggerking::bitmap_renderer::render(bitmap_renderer::entity_t * p)
{
	return _core->render(p);
}