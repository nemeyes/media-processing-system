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

dk_player_framework::ERR_CODE dk_player_framework::initialize(HWND hwnd, bool aspect_ratio, bool use_clock, bool enable_audio)
{
	return _core->initialize(hwnd, aspect_ratio, use_clock, enable_audio);
}

dk_player_framework::ERR_CODE dk_player_framework::release(void)
{
	return _core->release();
}

dk_player_framework::STATE dk_player_framework::state(void)
{
	return _core->state();
}

bool dk_player_framework::seekable(void)
{
	return _core->seekable();
}

int dk_player_framework::seek_resolution(void)
{
	return _core->seek_resolution();
}

int dk_player_framework::current_seek_position(void)
{
	return _core->current_seek_position();
}

long long dk_player_framework::current_media_time(void)
{
	return _core->current_media_time();
}

long long dk_player_framework::get_total_duration(void)
{
	return _core->get_total_duration();
}

float dk_player_framework::get_step_duration(void)
{
	return _core->get_step_duration();
}

dk_player_framework::ERR_CODE dk_player_framework::seek(int position)
{
	return _core->seek(position);
}

dk_player_framework::ERR_CODE dk_player_framework::slowfoward_rate(double rate)
{
	return _core->slowfoward_rate(rate);
}

dk_player_framework::ERR_CODE dk_player_framework::fastforward_rate(double rate)
{
	return _core->fastforward_rate(rate);
}

dk_player_framework::ERR_CODE dk_player_framework::open_file(wchar_t * file)
{
	return _core->open_file(file);
}

dk_player_framework::ERR_CODE dk_player_framework::open_rtmp(wchar_t * url, wchar_t * username, wchar_t * password)
{
	return _core->open_rtmp(url, username, password);
}

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

void dk_player_framework::list_dxva2_decoder_guids(std::vector<GUID> * guids)
{
	return _core->list_dxva2_decoder_guids(guids);
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
