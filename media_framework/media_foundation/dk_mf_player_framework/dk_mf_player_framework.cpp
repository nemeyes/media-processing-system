#include <windows.h>
#include "dk_mf_player_framework.h"
#include "mf_player_framework.h"

dk_mf_player_framework::dk_mf_player_framework(void)
{
	_core = new (std::nothrow) mf_player_framework();
}

dk_mf_player_framework::~dk_mf_player_framework(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::initialize(HWND hwnd, bool aspect_ratio, bool use_clock, bool enable_audio)
{
	return _core->initialize(hwnd, aspect_ratio, use_clock, enable_audio);
}

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::release(void)
{
	return _core->release();
}

// Playback control
dk_mf_player_framework::ERR_CODE dk_mf_player_framework::seek(int position)
{
	return _core->seek(position);
}

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::slowfoward_rate(float rate)
{
	return _core->slowfoward_rate(rate);
}

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::fastforward_rate(float rate)
{
	return _core->fastforward_rate(rate);
}

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::open_file(const wchar_t * file)
{
	return _core->open_file(file);
}

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::play(void)
{
	return _core->play();
}

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::pause(void)
{
	return _core->pause();
}

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::stop(void)
{
	return _core->stop();
}

dk_mf_player_framework::STATE dk_mf_player_framework::state(void) const
{
	return _core->state();
}