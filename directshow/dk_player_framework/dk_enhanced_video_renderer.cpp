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

void dk_enhanced_video_renderer::list_dxva2_decoder_guids(std::vector<GUID> * guids)
{
	CComPtr<IPin> input_pin;
	HRESULT hr;
	if (_renderer)
	{
		input_pin = get_input_pin();

		UINT cnt_decoder_guids = 0;
		GUID * decoder_guids = NULL;
		//BOOL found_dxva2_configuration = FALSE;
		//GUID decoder_guid = GUID_NULL;

		DXVA2_ConfigPictureDecode dxva2_config;
		ZeroMemory(&dxva2_config, sizeof(dxva2_config));

		CComPtr<IDirect3DDeviceManager9> d3d_device_manager;
		CComPtr<IDirectXVideoDecoderService> d3d_video_decoder_service;
		
		HANDLE d3d_device = INVALID_HANDLE_VALUE;

		CComQIPtr<IMFGetService> mf_get_service(input_pin);
		if (mf_get_service)
		{
			hr = mf_get_service->GetService(MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&d3d_device_manager));

			if (SUCCEEDED(hr))
			{
				hr = d3d_device_manager->OpenDeviceHandle(&d3d_device);
				if (SUCCEEDED(hr))
				{
					hr = d3d_device_manager->GetVideoService(d3d_device, IID_PPV_ARGS(&d3d_video_decoder_service));
					if (SUCCEEDED(hr))
					{
						hr = d3d_video_decoder_service->GetDecoderDeviceGuids(&cnt_decoder_guids, &decoder_guids);
						if (SUCCEEDED(hr))
						{
							for (UINT index = 0; index < cnt_decoder_guids; index++)
							{
								guids->push_back(decoder_guids[index]);
								{
									UINT count_configurations = 0;
									DXVA2_ConfigPictureDecode * configurations = NULL;

									/*D3DFORMAT * formats = NULL;
									UINT count_formats = 0;
									
									hr = d3d_video_decoder_service->GetDecoderRenderTargets(decoder_guids[index], &count_formats, &formats);
									if (SUCCEEDED(hr))
									{
										//for (UINT format_index = 0; format_index < count_formats; format_index++)
										//{
										//	if (formats[])
										//}*/
										DXVA2_VideoDesc video_desc = { 0 };
										//videoDesc.Format = formats[]
										hr = d3d_video_decoder_service->GetDecoderConfigurations(decoder_guids[index], &video_desc, NULL, &count_configurations, &configurations);
										for (UINT config_index = 0; config_index < count_configurations; config_index++)
										{
											configurations[config_index].Config4GroupedCoefs;
										}

									/*}*/



								}
								/*// Do we support this mode?
								if (!IsSupportedDecoderMode(pDecoderGuids[iGuid]))
								{
									continue;
								}

								// Find a configuration that we support. 
								hr = FindDecoderConfiguration(pDecoderService, pDecoderGuids[iGuid],
									&config, &bFoundDXVA2Configuration);
								if (FAILED(hr))
								{
									break;
								}

								if (bFoundDXVA2Configuration)
								{
									// Found a good configuration. Save the GUID and exit the loop.
									guidDecoder = pDecoderGuids[iGuid];
									break;
								}*/
							}
						}
					}
				}
			}

		}
	}
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