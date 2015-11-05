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

dk_player_framework::ERR_CODE dk_player_framework::initialize(HWND hwnd)
{
	return _core->initialize(hwnd);
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