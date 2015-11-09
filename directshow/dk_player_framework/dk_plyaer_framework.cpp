#include <windows.h>
#include "dk_player_framework.h"
#include "dshow_player_framework.h"

dk_player_framework::dk_player_framework(void)
{
	_core = new dshow_player_framework();
}

dk_player_framework::~dk_player_framework(void)
{
	delete _core;
}

dk_player_framework::ERR_CODE dk_player_framework::initialize(HWND hwnd, bool aspect_ratio, bool use_clock)
{
	return _core->initialize(hwnd, aspect_ratio, use_clock);
}

dk_player_framework::ERR_CODE dk_player_framework::release(void)
{
	return _core->release();
}

dk_player_framework::STATE dk_player_framework::state(void)
{
	return _core->state();
}

dk_player_framework::ERR_CODE dk_player_framework::open_file(wchar_t * file)
{
	return _core->open_file(file);
}

//dk_player_framework::ERR_CODE open_rtsp(wchar_t * url);
dk_player_framework::ERR_CODE dk_player_framework::play(void)
{
	return _core->play();
}

dk_player_framework::ERR_CODE dk_player_framework::pause(void)
{
	return _core->pause();
}

dk_player_framework::ERR_CODE dk_player_framework::stop(void)
{
	return _core->stop();
}

void dk_player_framework::aspect_ratio(bool enable)
{
	return _core->aspect_ratio(enable);
}

void dk_player_framework::fullscreen(bool enable)
{
	return _core->fullscreen(enable);
}

HRESULT dk_player_framework::update_video_windows(const LPRECT rect)
{
	return _core->update_video_windows(rect);
}

HRESULT dk_player_framework::repaint(HDC hdc)
{
	return _core->repaint(hdc);
}

HRESULT dk_player_framework::on_change_displaymode(void)
{
	return _core->on_change_displaymode();
}

HRESULT dk_player_framework::handle_graphevent(fn_graph_event func)
{
	return _core->handle_graphevent(func);
}