#pragma once
#include "dk_player_framework.h"
#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>
#include <uuids.h>
#include <initguid.h>

#include "dk_haali_media_splitter.h"
#include "dk_ms_video_decoder.h"
#include "dk_enhanced_video_renderer.h"

class dshow_player_framework
{
public:
	dshow_player_framework(void);
	~dshow_player_framework(void);

	dk_player_framework::ERR_CODE initialize(HWND hwnd);
	dk_player_framework::ERR_CODE release(void);

	dk_player_framework::STATE state(void);

	dk_player_framework::ERR_CODE open_file(wchar_t * file);
	//dk_player_framework::ERR_CODE open_rtsp(wchar_t * url);
	dk_player_framework::ERR_CODE play(void);
	dk_player_framework::ERR_CODE pause(void);
	dk_player_framework::ERR_CODE stop(void);

private:
	HWND _hwnd;

	dk_player_framework::STATE _state;

	CComPtr<IGraphBuilder> _graph;
	CComPtr<IMediaControl> _control;
	CComPtr<IMediaEventEx> _event;

	dk_haali_media_splitter * _source;
	dk_ms_video_decoder * _decoder;
	dk_enhanced_video_renderer * _renderer;
};