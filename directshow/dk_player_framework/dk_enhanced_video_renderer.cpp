#include "dk_enhanced_video_renderer.h"
#include <atlstr.h>
#include <dk_dshow_helper.h>
#include <uuids.h>
#include <initguid.h>

dk_enhanced_video_renderer::dk_enhanced_video_renderer(void)
	: _fullscreen(false)
{

}

dk_enhanced_video_renderer::~dk_enhanced_video_renderer(void)
{

}

CComPtr<IBaseFilter> dk_enhanced_video_renderer::get_filter(void)
{
	return _renderer;
}

CComPtr<IPin> dk_enhanced_video_renderer::get_output_pin(void)
{
	return NULL;
}

CComPtr<IPin> dk_enhanced_video_renderer::get_input_pin(void)
{
	CComPtr<IPin> inpin;
	dk_dshow_helper::get_pin(_renderer, L"EVR Input0", &inpin);
	return inpin;
}

void dk_enhanced_video_renderer::aspect_ratio(bool enable)
{
	if (_display)
	{
		HRESULT hr;
		if (enable)
			hr = _display->SetAspectRatioMode(MFVideoARMode_PreservePicture);
		else
			hr = _display->SetAspectRatioMode(MFVideoARMode_None);
	}
}

void dk_enhanced_video_renderer::fullscreen(bool enable)
{
	if (_fullscreen == enable)
		return;
	if (enable)
	{
		GetWindowRect(_hwnd, &_original_rect);
		HMONITOR monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
		if (!monitor) 
			return;

		MONITORINFO info;
		info.cbSize = sizeof(info);
		if (!GetMonitorInfo(monitor, &info))
			return;

		if (_display)
			_display->SetFullscreen(TRUE);

		RECT rect(info.rcMonitor);
		::SetWindowPos(_hwnd, HWND_TOPMOST, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
		::ShowCursor(FALSE);
		::SetFocus(_hwnd);
	}
	else
	{
		::SetWindowPos(_hwnd, HWND_NOTOPMOST, _original_rect.left, _original_rect.top, _original_rect.right, _original_rect.bottom, SWP_HIDEWINDOW);

		if (_display)
			_display->SetFullscreen(FALSE);

		::ShowCursor(TRUE);
		::ShowWindow(_hwnd, SW_SHOW);
		::SetFocus(_hwnd);
	}

	_fullscreen = enable;
}


HRESULT dk_enhanced_video_renderer::add_to_graph(CComPtr<IGraphBuilder> graph, HWND hwnd, bool aspect_ratio)
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
	if (aspect_ratio)
		hr = display->SetAspectRatioMode(MFVideoARMode_PreservePicture);
	else
		hr = display->SetAspectRatioMode(MFVideoARMode_None);
	if (FAILED(hr))
		return hr;
	
	_renderer = renderer;
	_display = display;
	_hwnd = hwnd;
	return hr;
}

HRESULT dk_enhanced_video_renderer::update_video_window(HWND hwnd, const LPRECT rect)
{
	if (!_display)
		return S_OK;

	if (rect)
		return _display->SetVideoPosition(NULL, rect);
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
		return _display->RepaintVideo();
	else
		return S_OK;
}

HRESULT dk_enhanced_video_renderer::on_change_displaymode(void)
{
	return S_OK;
}