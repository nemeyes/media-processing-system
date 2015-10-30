#include <atlbase.h>
#include <atlconv.h>
#include <dshow.h>
#include <streams.h>
#include <uuids.h>
#include <dk_dshow_helper.h>

#include "dk_video_renderer.h"
#include "dk_dshow_player.h"

dk_dshow_player::dk_dshow_player(HWND hwnd)
	: _state(dk_dshow_player::STATE_NO_GRAPH)
	, _hwnd(hwnd)
	, _graph(nullptr)
	, _control(nullptr)
	, _event(nullptr)
	, _video_renderer(nullptr)
{

}

dk_dshow_player::~dk_dshow_player(void)
{
	release_graph();
}

HRESULT dk_dshow_player::open_file(wchar_t * filename)
{
	IBaseFilter * source = nullptr;
	HRESULT hr = initialize_graph();
	if (FAILED(hr))
	{
		goto done;
	}

	hr = _graph->AddSourceFilter(filename, NULL, &source);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = render_video(source);

done:
	if (FAILED(hr))
	{
		release_graph();
	}
	safe_release(&source);
	return hr;
}

HRESULT dk_dshow_player::handle_graphevent(fn_graph_event func)
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

HRESULT dk_dshow_player::play(void)
{
	if (_state != STATE_PAUSED && _state != STATE_STOPPED)
	{
		return VFW_E_WRONG_STATE;
	}
	HRESULT hr = _control->Run();
	if (SUCCEEDED(hr))
	{
		_state = STATE_RUNNING;
	}
	return hr;
}

HRESULT dk_dshow_player::pause(void)
{
	if (_state != STATE_RUNNING)
	{
		return VFW_E_WRONG_STATE;
	}

	HRESULT hr = _control->Pause();
	if (SUCCEEDED(hr))
	{
		_state = STATE_PAUSED;
	}
	return hr;
}

HRESULT dk_dshow_player::stop(void)
{
	if ((_state != STATE_RUNNING) && (_state != STATE_PAUSED))
	{
		return VFW_E_WRONG_STATE;
	}

	HRESULT hr = _control->Stop();
	if (SUCCEEDED(hr))
	{
		_state = STATE_STOPPED;
	}
	return hr;
}

bool dk_dshow_player::has_video(void) const
{
	return (_video_renderer && _video_renderer->has_video());
}

HRESULT dk_dshow_player::update_video_windows(const LPRECT rect)
{
	if (_video_renderer)
	{
		return _video_renderer->update_video_window(_hwnd, rect);
	}
	else
	{
		return S_OK;
	}
}

HRESULT dk_dshow_player::repaint(HDC hdc)
{
	if (_video_renderer)
	{
		return _video_renderer->repaint(_hwnd, hdc);
	}
	else
	{
		return S_OK;
	}
}

HRESULT dk_dshow_player::on_change_displaymode(void)
{
	if (_video_renderer)
	{
		return _video_renderer->on_change_displaymode();
	}
	else
	{
		return S_OK;
	}
}

HRESULT dk_dshow_player::initialize_graph(void)
{
	release_graph();

	//HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, __uuidof(IGraphBuilder), (void**)&_graph);

	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_graph));
	if (FAILED(hr))
	{
		goto done;
	}
	hr = _graph->QueryInterface(IID_PPV_ARGS(&_control));
	if (FAILED(hr))
	{
		goto done;
	}
	hr = _graph->QueryInterface(IID_PPV_ARGS(&_event));
	if (FAILED(hr))
	{
		goto done;
	}

	hr = _event->SetNotifyWindow((OAHWND)_hwnd, WM_GRAPH_EVENT, NULL);
	if (FAILED(hr))
	{
		goto done;
	}

	_state = STATE_STOPPED;
done:
	return hr;
}

void dk_dshow_player::release_graph(void)
{
	if (_event)
	{
		_event->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
	}

	safe_release(&_graph);
	safe_release(&_control);
	safe_release(&_event);

	delete _video_renderer;
	_video_renderer = NULL;

	_state = STATE_NO_GRAPH;
}

HRESULT dk_dshow_player::create_video_renderer(void)
{
	HRESULT hr = E_FAIL;
	enum { EVR, VMR9, VMR7 };

	for (DWORD i = EVR; i < VMR7; i++)
	{
		switch (i)
		{
		case EVR:
			_video_renderer = new (std::nothrow) dk_evr_video_renderer();
			break;
		case VMR9:
			//_video_renderer = new (Std::nothrow) dk_vmr9_renderer();
			break;
		case VMR7:
			//_video_renderer = new (std::nothrow) dk_vmr7_renderer();
			break;
		}

		if (!_video_renderer)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		hr = _video_renderer->add_to_graph(_graph, _hwnd);
		if (SUCCEEDED(hr))
		{
			break;
		}

		delete _video_renderer;
		_video_renderer = NULL;
	}
	return hr;
}

HRESULT dk_dshow_player::render_video(IBaseFilter * source)
{
	BOOL render_anypin = FALSE;

	IFilterGraph2 * graph = NULL;
	IEnumPins * penum = NULL;
	IBaseFilter * audio_renderer = NULL;
	
	HRESULT hr = _graph->QueryInterface(__uuidof(IFilterGraph2), (void**)&graph);
	if (FAILED(hr))
	{
		goto done;
	}
	
	hr = create_video_renderer();
	if (FAILED(hr))
	{
		goto done;
	}


	hr = dk_dshow_helper::add_filter_by_clsid(graph, L"Audio Renderer", CLSID_DSoundRender, &audio_renderer);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = source->EnumPins(&penum);
	if (FAILED(hr))
	{
		goto done;
	}

	IPin * ppin;
	while (S_OK == penum->Next(1, &ppin, NULL))
	{
		HRESULT hr2 = graph->RenderEx(ppin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL);

		ppin->Release();
		if (SUCCEEDED(hr2))
			render_anypin = TRUE;
	}

	hr = _video_renderer->finalize_graph(_graph);
	if (FAILED(hr))
	{
		goto done;
	}

	BOOL removed = FALSE;
	hr = dk_dshow_helper::remove_unconnected_renderer(_graph, audio_renderer, &removed);
done:
	safe_release(&penum);
	safe_release(&audio_renderer);
	safe_release(&graph);

	if (SUCCEEDED(hr))
	{
		if (!render_anypin)
		{
			hr = VFW_E_CANNOT_RENDER;
		}
	}
	return hr;
}