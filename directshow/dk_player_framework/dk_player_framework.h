#pragma once

const UINT WM_GRAPH_EVENT = WM_APP + 1;

#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif

class dshow_player_framework;
class EXP_CLASS dk_player_framework
{
public:
	typedef enum _ERR_CODE
	{
		ERR_CODE_SUCCESS,
		ERR_CODE_FAILED
	} ERR_CODE;

	typedef enum _STATE
	{
		STATE_NO_GRAPH,
		STATE_RUNNING,
		STATE_PAUSED,
		STATE_STOPPED,
	} STATE;

	dk_player_framework(void);
	~dk_player_framework(void);

	dk_player_framework::ERR_CODE initialize(HWND hwnd);
	dk_player_framework::ERR_CODE release(void);

	dk_player_framework::STATE state(void);

	dk_player_framework::ERR_CODE open_file(wchar_t * file);
	//dk_player_framework::ERR_CODE open_rtsp(wchar_t * url);
	dk_player_framework::ERR_CODE play(void);
	dk_player_framework::ERR_CODE pause(void);
	dk_player_framework::ERR_CODE stop(void);

private:
	dshow_player_framework * _core;


};