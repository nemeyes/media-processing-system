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

dk_player_framework::ERR_CODE dshow_player_framework::initialize(HWND hwnd, bool aspect_ratio, bool use_clock)
{
	HRESULT hr;
	hr = release();
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	CComPtr<IGraphBuilder> graph;
	CComPtr<IMediaControl> control;
	CComPtr<IMediaEventEx> event;
	hr = graph.CoCreateInstance(CLSID_FilterGraph);
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	hr = graph->QueryInterface(IID_PPV_ARGS(&control));
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	hr = graph->QueryInterface(IID_PPV_ARGS(&event));
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	hr = event->SetNotifyWindow((OAHWND)_hwnd, WM_GRAPH_EVENT, NULL);
	if (FAILED(hr))
		return dk_player_framework::ERR_CODE_FAILED;

	if (!use_clock)
	{
		CComQIPtr<IMediaFilter> sync_interface(graph);
		if (sync_interface)
			hr = sync_interface->SetSyncSource(NULL);
		if (FAILED(hr))
			return dk_player_framework::ERR_CODE_FAILED;
	}

	_hwnd = hwnd;
	_aspect_ratio = aspect_ratio;
	_use_clock = use_clock;
	_graph = graph;
	_control = control;
	_event = event;
	_state = dk_player_framework::STATE_STOPPED;


	return dk_player_framework::ERR_CODE_SUCCESS;
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

dk_player_framework::ERR_CODE dshow_player_framework::open_file(wchar_t * path)
{
	HRESULT hr;
	if (!path || wcslen(path) < 1)
		return dk_player_framework::ERR_CODE_FAILED;

	hr = _source->add_to_graph(_graph, path);
	hr = _decoder->add_to_graph(_graph, false);
	hr = _renderer->add_to_graph(_graph, _hwnd, _aspect_ratio, false);
	hr = _graph->Render(_source->get_outpin());
	if (SUCCEEDED(hr))
		return dk_player_framework::ERR_CODE_SUCCESS;
	else
		return dk_player_framework::ERR_CODE_FAILED;
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
		RECT rect;
		::GetClientRect(_hwnd, &rect);
		update_video_windows(&rect);
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
		RECT rect;
		::GetClientRect(_hwnd, &rect);
		update_video_windows(&rect);
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

void dshow_player_framework::aspect_ratio(bool enable)
{
	if (_renderer)
		_renderer->aspect_ratio(enable);
}

void dshow_player_framework::fullscreen(bool enable)
{
	if (_renderer)
		_renderer->fullscreen(enable);
}

HRESULT dshow_player_framework::update_video_windows(const LPRECT rect)
{
	if (_renderer)
	{
		return _renderer->update_video_window(_hwnd, rect);
	}
	else
	{
		return S_OK;
	}
}

HRESULT dshow_player_framework::repaint(HDC hdc)
{
	if (_renderer)
	{
		return _renderer->repaint(_hwnd, hdc);
	}
	else
	{
		return S_OK;
	}
}

HRESULT dshow_player_framework::on_change_displaymode(void)
{
	if (_renderer)
	{
		return _renderer->on_change_displaymode();
	}
	else
	{
		return S_OK;
	}
}

HRESULT dshow_player_framework::handle_graphevent(fn_graph_event func)
{
	if (!_event)
		return E_UNEXPECTED;

	long code = 0;
	LONG_PTR param1 = 0, param2 = 0;

	HRESULT hr = S_OK;
	while (SUCCEEDED(_event->GetEvent(&code, &param1, &param2, 0)))
	{
		func(_hwnd, code, param1, param2);

		hr = _event->FreeEventParams(code, param1, param2);
		if (FAILED(hr))
		{
			break;
		}
	}
	return hr;
}