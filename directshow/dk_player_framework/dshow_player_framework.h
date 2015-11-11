#pragma once
#include "dk_player_framework.h"
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>
#include <uuids.h>
#include <initguid.h>

#include "dk_haali_media_splitter.h"
#include "dk_microsoft_video_decoder.h"
#include "dk_enhanced_video_renderer.h"

class dshow_player_framework
{
public:
	dshow_player_framework(void);
	~dshow_player_framework(void);

	dk_player_framework::ERR_CODE initialize(HWND hwnd, bool aspect_ratio, bool use_clock);
	dk_player_framework::ERR_CODE release(void);

	dk_player_framework::STATE state(void);

	dk_player_framework::ERR_CODE open_file(wchar_t * file);
	//dk_player_framework::ERR_CODE open_rtsp(wchar_t * url);
	dk_player_framework::ERR_CODE play(void);
	dk_player_framework::ERR_CODE pause(void);
	dk_player_framework::ERR_CODE stop(void);

	void aspect_ratio(bool enable);
	void fullscreen(bool enalbe);

	HRESULT update_video_windows(const LPRECT rect);
	HRESULT repaint(HDC hdc);
	HRESULT on_change_displaymode(void);
	HRESULT handle_graphevent(fn_graph_event func);
private:
	HWND _hwnd;
	bool _aspect_ratio;
	bool _use_clock;

	dk_player_framework::STATE _state;

	CComPtr<IGraphBuilder> _graph;
	CComPtr<IMediaControl> _control;
	CComPtr<IMediaEventEx> _event;

	dk_base_source_filter * _source;
	dk_base_transform_filter * _decoder;
	dk_base_render_filter * _renderer;
};