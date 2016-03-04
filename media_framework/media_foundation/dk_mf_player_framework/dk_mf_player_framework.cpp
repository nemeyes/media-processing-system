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

dk_mf_player_framework::ERR_CODE dk_mf_player_framework::open_file(const wchar_t * file, HWND hwnd)
{
	return _core->open_file(file, hwnd);
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