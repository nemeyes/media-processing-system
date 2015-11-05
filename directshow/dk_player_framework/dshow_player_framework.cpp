#include <windows.h>
#include "dshow_player_framework.h"

dshow_player_framework::dshow_player_framework(void)
{
	_source = new dk_haali_media_splitter();
	_decoder = new dk_ms_video_decoder();
	_renderer = new dk_enhanced_video_renderer();
}

dshow_player_framework::~dshow_player_framework(void)
{
	if (_decoder)
		delete _decoder;
	if (_renderer)
		delete _renderer;
}

dk_player_framework::ERR_CODE dshow_player_framework::initialize(HWND hwnd)
{
	HRESULT hr;
	hr = release();

	CComPtr<IGraphBuilder> graph;
	CComPtr<IMediaControl> control;
	CComPtr<IMediaEventEx> event;
	hr = graph.CoCreateInstance(CLSID_FilterGraph);
	if (FAILED(hr))
		goto FAILED;

	hr = graph->QueryInterface(IID_PPV_ARGS(&control));
	if (FAILED(hr))
		goto FAILED;

	hr = graph->QueryInterface(IID_PPV_ARGS(&event));
	if (FAILED(hr))
		goto FAILED;

	hr = event->SetNotifyWindow((OAHWND)_hwnd, WM_GRAPH_EVENT, NULL);
	_hwnd = hwnd;
	_graph = graph;
	_control = control;
	_event = event;
	_state = dk_player_framework::STATE_STOPPED;

	return dk_player_framework::ERR_CODE_SUCCESS;
FAILED:
	return dk_player_framework::ERR_CODE_FAILED;
}

dk_player_framework::ERR_CODE dshow_player_framework::release(void)
{
	if (_event)
		_event->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
	_state = dk_player_framework::STATE_NO_GRAPH;
	return dk_player_framework::ERR_CODE_SUCCESS;
}

dk_player_framework::STATE dshow_player_framework::state(void) 
{
	return _state;
}

dk_player_framework::ERR_CODE dshow_player_framework::open_file(wchar_t * file)
{
	HRESULT hr;
	hr = _source->add_to_graph(_graph, L"C:\\workspace\\03.movie\\Heroes.Reborn.S01E05.720p.HDTV.x264-KILLERS.mkv");
	hr = _decoder->add_to_graph(_graph, false);
	hr = _renderer->add_to_graph(_graph, _hwnd, false);
	//_graph->RenderFile(L"C:\\workspace\\03.movie\\Heroes.Reborn.S01E05.720p.HDTV.x264-KILLERS.mkv", NULL);
	//_graph->
	hr = _graph->Render(_source->get_outpin());
	play();
	return dk_player_framework::ERR_CODE_SUCCESS;
}

//dk_player_framework::ERR_CODE open_rtsp(wchar_t * url);
dk_player_framework::ERR_CODE dshow_player_framework::play(void)
{
	if (_state != dk_player_framework::STATE_PAUSED && _state != dk_player_framework::STATE_STOPPED)
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}
	HRESULT hr = _control->Run();
	if (SUCCEEDED(hr))
	{
		_state = dk_player_framework::STATE_RUNNING;
		return dk_player_framework::ERR_CODE_SUCCESS;
	}
	else
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}
}

dk_player_framework::ERR_CODE dshow_player_framework::pause(void)
{
	if (_state != dk_player_framework::STATE_RUNNING)
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}

	HRESULT hr = _control->Pause();
	if (SUCCEEDED(hr))
	{
		_state = dk_player_framework::STATE_PAUSED;
		return dk_player_framework::ERR_CODE_SUCCESS;
	}
	else
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}
}

dk_player_framework::ERR_CODE dshow_player_framework::stop(void)
{
	if ((_state != dk_player_framework::STATE_RUNNING) && (_state != dk_player_framework::STATE_PAUSED))
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}

	HRESULT hr = _control->Stop();
	if (SUCCEEDED(hr))
	{
		_state = dk_player_framework::STATE_STOPPED;
		return dk_player_framework::ERR_CODE_SUCCESS;
	}
	else
	{
		return dk_player_framework::ERR_CODE_FAILED;
	}
}