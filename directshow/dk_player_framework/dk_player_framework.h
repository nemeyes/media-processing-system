#pragma once

const UINT WM_GRAPH_EVENT = WM_APP + 1;
typedef void (CALLBACK * fn_graph_event)(HWND hwnd, long eventCode, LONG_PTR param1, LONG_PTR param2);

#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif

#include <vector>

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

	dk_player_framework::ERR_CODE initialize(HWND hwnd, bool aspect_ratio, bool use_clock);
	dk_player_framework::ERR_CODE release(void);

	dk_player_framework::STATE state(void);

	dk_player_framework::ERR_CODE open_file(wchar_t * file);
	//dk_player_framework::ERR_CODE open_rtsp(wchar_t * url);
	dk_player_framework::ERR_CODE play(void);
	dk_player_framework::ERR_CODE pause(void);
	dk_player_framework::ERR_CODE stop(void);

	void aspect_ratio(bool enable);
	void fullscreen(bool enable);
	void list_dxva2_decoder_guids(std::vector<GUID> * guids);

	HRESULT update_video_windows(const LPRECT rect);
	HRESULT repaint(HDC hdc);
	HRESULT on_change_displaymode(void);
	HRESULT handle_graphevent(fn_graph_event func);

private:
	dshow_player_framework * _core;


};