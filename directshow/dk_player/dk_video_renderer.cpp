#include "dk_video_renderer.h"
#include <dk_dshow_helper.h>

dk_evr_video_renderer::dk_evr_video_renderer(void)
	: _evr(NULL)
	, _video_display(NULL)
{

}

dk_evr_video_renderer::~dk_evr_video_renderer(void)
{
	safe_release(&_evr);
	safe_release(&_video_display);
}

bool dk_evr_video_renderer::has_video(void) const
{
	return (_video_display != NULL);
}

HRESULT dk_evr_video_renderer::add_to_graph(IGraphBuilder * graph, HWND hwnd)
{
	IBaseFilter * evr = NULL;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"EVR", CLSID_EnhancedVideoRenderer, &evr);
	if (FAILED(hr))
	{
		goto done;
	}
	hr = dk_dshow_helper::initialize_evr(evr, hwnd, &_video_display);
	if (FAILED(hr))
	{
		goto done;
	}

	_evr = evr;
	_evr->AddRef();
done:
	safe_release(&evr);
	return hr;
}

HRESULT dk_evr_video_renderer::finalize_graph(IGraphBuilder * graph)
{
	if (_evr == NULL)
	{
		return S_OK;
	}

	BOOL bremoved;
	HRESULT hr = dk_dshow_helper::remove_unconnected_renderer(graph, _evr, &bremoved);
	if (bremoved)
	{
		safe_release(&_evr);
		safe_release(&_video_display);
	}
	return hr;
}


HRESULT dk_evr_video_renderer::update_video_window(HWND hwnd, const LPRECT rect)
{
	if (_video_display == NULL)
	{
		return S_OK;
	}

	if (rect)
	{
		return _video_display->SetVideoPosition(NULL, rect);
	}
	else
	{
		RECT rect2;
		GetClientRect(hwnd, &rect2);
		return _video_display->GetVideoPosition(NULL, &rect2);
	}
}

HRESULT dk_evr_video_renderer::repaint(HWND hwnd, HDC hdc)
{
	if (_video_display)
	{
		return _video_display->RepaintVideo();
	}
	else
	{
		return S_OK;
	}
}

HRESULT dk_evr_video_renderer::on_change_displaymode(void)
{
	return S_OK;
}

