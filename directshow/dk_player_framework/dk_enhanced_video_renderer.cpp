#include "dk_enhanced_video_renderer.h"
#include <atlstr.h>
#include <dk_dshow_helper.h>
#include <uuids.h>
#include <initguid.h>

dk_enhanced_video_renderer::dk_enhanced_video_renderer(void)
{

}

dk_enhanced_video_renderer::~dk_enhanced_video_renderer(void)
{

}


HRESULT dk_enhanced_video_renderer::add_to_graph(CComPtr<IGraphBuilder> graph, HWND hwnd, bool dxva2)
{
	CComPtr<IBaseFilter> renderer;
	HRESULT hr = dk_dshow_helper::add_filter_by_clsid(graph, L"EVR", CLSID_EnhancedVideoRenderer, &renderer);
	if (FAILED(hr))
		return hr;

	CComQIPtr<IMFGetService> mfgs = renderer;
	CComPtr<IMFVideoDisplayControl> display;
	if (mfgs)
		hr = mfgs->GetService(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&display));
	else
		return E_FAIL;

	if (FAILED(hr))
		return hr;

	// Set the clipping window.
	hr = display->SetVideoWindow(hwnd);
	if (FAILED(hr))
		return hr;

	// Preserve aspect ratio by letter-boxing
	hr = display->SetAspectRatioMode(MFVideoARMode_PreservePicture);
	if (FAILED(hr))
		return hr;
	
	_renderer = renderer;
	_display = display;
	return hr;
}

/*HRESULT dk_enhanced_video_renderer::finalize_graph(IGraphBuilder * graph)
{
	if (_evr == NULL)
	{
		return S_OK;
	}

	BOOL bremoved;
	HRESULT hr = eap_dshow_helper::remove_unconnected_renderer(graph, _evr, &bremoved);
	if (bremoved)
	{
		safe_release(&_evr);
		safe_release(&_video_display);
	}
	return hr;
}*/


HRESULT dk_enhanced_video_renderer::update_video_window(HWND hwnd, const LPRECT rect)
{
	if (!_display)
	{
		return S_OK;
	}

	if (rect)
	{
		return _display->SetVideoPosition(NULL, rect);
	}
	else
	{
		RECT rect2;
		GetClientRect(hwnd, &rect2);
		return _display->GetVideoPosition(NULL, &rect2);
	}
}

HRESULT dk_enhanced_video_renderer::repaint(HWND hwnd, HDC hdc)
{
	if (_display)
	{
		return _display->RepaintVideo();
	}
	else
	{
		return S_OK;
	}
}

HRESULT dk_enhanced_video_renderer::on_change_displaymode(void)
{
	return S_OK;
}