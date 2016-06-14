#include "Presenter.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CPresenter class. - Presents samples using DX11.
//
// Notes:
// - Most public methods calls CheckShutdown. This method fails if the presenter was shut down.
//
/////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------
// CPresenter constructor.
//-------------------------------------------------------------------

debuggerking::presenter::presenter(void)
	: _ref_count(1)
	, _cs()
	, _is_shutdown(FALSE)
	, _dxgi_factory2(NULL)
	, _d3d11_device(NULL)
	, _d3d_immediate_context(NULL)
	, _dxgi_manager(NULL)
	, _dxgi_output1(NULL)
	, _sample_allocator_ex(NULL)
	, _dcomp_device(NULL)
	, _hwnd_target(NULL)
	, _root_visual(NULL)
	, _software_dxva_device_in_use(FALSE)
	, _hwnd_video(NULL)
	, _monitors(NULL)
	, _current_monitor(NULL)
	, _device_reset_token(0)
	, _dx_sw_switch(0)
	, _use_xvp(1)
	, _use_dcomp_visual(0)
	, _use_debug_layer(D3D11_CREATE_DEVICE_VIDEO_SUPPORT)
	, _dx11_video_device(NULL)
	, _video_processor_enum(NULL)
	, _video_processor(NULL)
	, _swap_chain1(NULL)
	, _device_changed(FALSE)
	, _resize(TRUE)
	, _3d_video(FALSE)
	, _stereo_enabled(FALSE)
	, _vp_3d_output(MFVideo3DSampleFormat_BaseView)
	, _fullscreen_state(FALSE)
	, _can_process_next_sample(TRUE)
	, _display_rect()
	, _image_width_in_pixels(0)
	, _image_height_in_pixels(0)
	, _real_display_width(0)
	, _real_display_height(0)
	, _rc_src_app()
	, _rc_dst_app()
	, _xvp(NULL)
	, _xvp_control(NULL)
{
	ZeroMemory(&_rc_src_app, sizeof(_rc_src_app));
	ZeroMemory(&_rc_dst_app, sizeof(_rc_dst_app));
}

//-------------------------------------------------------------------
// CPresenter destructor.
//-------------------------------------------------------------------

debuggerking::presenter::~presenter(void)
{
	safe_delete(_monitors);
}

// IUnknown
ULONG debuggerking::presenter::AddRef(void)
{
	return InterlockedIncrement(&_ref_count);
}

// IUnknown
HRESULT debuggerking::presenter::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void ** ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}
	if (iid == IID_IUnknown)
	{
		*ppv = static_cast<IUnknown*>(static_cast<IMFVideoDisplayControl*>(this));
	}
	else if (iid == __uuidof(IMFVideoDisplayControl))
	{
		*ppv = static_cast<IMFVideoDisplayControl*>(this);
	}
	else if (iid == __uuidof(IMFGetService))
	{
		*ppv = static_cast<IMFGetService*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

// IUnknown
ULONG  debuggerking::presenter::Release(void)
{
	ULONG count = InterlockedDecrement(&_ref_count);
	if (count == 0)
	{
		delete this;
	}
	// For thread safety, return a temporary variable.
	return count;
}

// IMFVideoDisplayControl
HRESULT debuggerking::presenter::GetFullscreen(__RPC__out BOOL* pfFullscreen)
{
	autolock lock(&_cs);
	HRESULT hr = CheckShutdown();
	if (FAILED(hr))
	{
		return hr;
	}

	if (pfFullscreen == NULL)
	{
		return E_POINTER;
	}

	*pfFullscreen = _fullscreen_state;

	return S_OK;
}

// IMFVideoDisplayControl
HRESULT debuggerking::presenter::SetFullscreen(BOOL fFullscreen)
{
	autolock lock(&_cs);

	HRESULT hr = CheckShutdown();

	if (SUCCEEDED(hr))
	{
		_fullscreen_state = fFullscreen;

		safe_release(_dx11_video_device);
		safe_release(_video_processor_enum);
		safe_release(_video_processor);
	}

	return hr;
}

// IMFVideoDisplayControl
HRESULT debuggerking::presenter::SetVideoWindow(__RPC__in HWND hwndVideo)
{
	HRESULT hr = S_OK;

	autolock lock(&_cs);

	do
	{
		hr = CheckShutdown();
		if (FAILED(hr))
		{
			break;
		}

		if (!IsWindow(hwndVideo))
		{
			hr = E_INVALIDARG;
			break;
		}

		_monitors = new monitor_array();
		if (!_monitors)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		hr = SetVideoMonitor(hwndVideo);
		if (FAILED(hr))
		{
			break;
		}

		CheckDecodeSwitchRegKey();

		_hwnd_video = hwndVideo;

		hr = CreateDXGIManagerAndDevice();
		if (FAILED(hr))
		{
			break;
		}

		if (_use_xvp)
		{
			hr = CreateXVP();
			if (FAILED(hr))
			{
				break;
			}
		}
	} while (FALSE);

	return hr;
}

//-------------------------------------------------------------------------
// Name: GetService
// Description: IMFGetService
//-------------------------------------------------------------------------

HRESULT debuggerking::presenter::GetService(__RPC__in REFGUID guidService, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppvObject)
{
	HRESULT hr = S_OK;

	if (guidService == MR_VIDEO_ACCELERATION_SERVICE)
	{
		if (riid == __uuidof(IMFDXGIDeviceManager))
		{
			if (NULL != _dxgi_manager)
			{
				*ppvObject = (void*) static_cast<IUnknown*>(_dxgi_manager);
				((IUnknown*)*ppvObject)->AddRef();
			}
			else
			{
				hr = E_NOINTERFACE;
			}
		}
		else if (riid == __uuidof(IMFVideoSampleAllocatorEx))
		{
			if (NULL == _sample_allocator_ex)
			{
				hr = MFCreateVideoSampleAllocatorEx(IID_IMFVideoSampleAllocatorEx, (LPVOID*)&_sample_allocator_ex);
				if (SUCCEEDED(hr) && NULL != _dxgi_manager)
				{
					hr = _sample_allocator_ex->SetDirectXManager(_dxgi_manager);
				}
			}
			if (SUCCEEDED(hr))
			{
				hr = _sample_allocator_ex->QueryInterface(riid, ppvObject);
			}
		}
		else
		{
			hr = E_NOINTERFACE;
		}
	}
	else if (guidService == MR_VIDEO_RENDER_SERVICE)
	{
		hr = QueryInterface(riid, ppvObject);
	}
	else
	{
		hr = MF_E_UNSUPPORTED_SERVICE;
	}

	return hr;
}

BOOL debuggerking::presenter::can_process_next_sample(void)
{
	return _can_process_next_sample;
}

HRESULT debuggerking::presenter::flush(void)
{
	autolock lock(&_cs);

	HRESULT hr = CheckShutdown();

	if (SUCCEEDED(hr) && _use_xvp)
	{
		hr = _xvp->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
	}

	_can_process_next_sample = TRUE;

	return hr;
}

HRESULT debuggerking::presenter::get_monitor_refresh_rate(DWORD* pdwRefreshRate)
{
	if (pdwRefreshRate == NULL)
	{
		return E_POINTER;
	}

	if (_current_monitor == NULL)
	{
		return MF_E_INVALIDREQUEST;
	}

	*pdwRefreshRate = _current_monitor->refresh_rate;

	return S_OK;
}

HRESULT debuggerking::presenter::is_media_type_supported(IMFMediaType * pmedia_type, DXGI_FORMAT dxgi_format)
{
	HRESULT hr = S_OK;
	UINT32 uiNumerator = 30000, uiDenominator = 1001;
	UINT32 uimageWidthInPixels, uimageHeightInPixels = 0;

	do
	{
		hr = CheckShutdown();
		if (FAILED(hr))
		{
			break;
		}

		if (pmedia_type == NULL)
		{
			hr = E_POINTER;
			break;
		}

		if (!_dx11_video_device)
		{
			hr = _d3d11_device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&_dx11_video_device);
			if (FAILED(hr))
			{
				break;
			}
		}

		hr = MFGetAttributeSize(pmedia_type, MF_MT_FRAME_SIZE, &uimageWidthInPixels, &uimageHeightInPixels);

		if (FAILED(hr))
		{
			break;
		}

		MFGetAttributeRatio(pmedia_type, MF_MT_FRAME_RATE, &uiNumerator, &uiDenominator);

		//Check if the format is supported

		D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
		ZeroMemory(&ContentDesc, sizeof(ContentDesc));
		ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
		ContentDesc.InputWidth = (DWORD)uimageWidthInPixels;
		ContentDesc.InputHeight = (DWORD)uimageHeightInPixels;
		ContentDesc.OutputWidth = (DWORD)uimageWidthInPixels;
		ContentDesc.OutputHeight = (DWORD)uimageHeightInPixels;
		ContentDesc.InputFrameRate.Numerator = uiNumerator;
		ContentDesc.InputFrameRate.Denominator = uiDenominator;
		ContentDesc.OutputFrameRate.Numerator = uiNumerator;
		ContentDesc.OutputFrameRate.Denominator = uiDenominator;
		ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

		safe_release(_video_processor_enum);
		hr = _dx11_video_device->CreateVideoProcessorEnumerator(&ContentDesc, &_video_processor_enum);
		if (FAILED(hr))
		{
			break;
		}

		UINT uiFlags;
		hr = _video_processor_enum->CheckVideoProcessorFormat(dxgi_format, &uiFlags);
		if (FAILED(hr) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT))
		{
			hr = MF_E_UNSUPPORTED_D3D_TYPE;
			break;
		}

		if (_use_xvp)
		{
			hr = _xvp->SetInputType(0, pmedia_type, MFT_SET_TYPE_TEST_ONLY);
			if (FAILED(hr))
			{
				break;
			}
		}
	} while (FALSE);

	return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     PresentFrame
//
//  Synopsis:   Present the current outstanding frame in the DX queue
//
//--------------------------------------------------------------------------

HRESULT debuggerking::presenter::present_frame(void)
{
	HRESULT hr = S_OK;

	autolock lock(&_cs);

	do
	{
		hr = CheckShutdown();
		if (FAILED(hr))
		{
			break;
		}

		if (NULL == _swap_chain1)
		{
			break;
		}

		RECT rcDest;
		ZeroMemory(&rcDest, sizeof(rcDest));
		if (CheckEmptyRect(&rcDest))
		{
			hr = S_OK;
			break;
		}

		hr = _swap_chain1->Present(0, 0);
		if (FAILED(hr))
		{
			break;
		}

		_can_process_next_sample = TRUE;
	} while (FALSE);

	return hr;
}

//-------------------------------------------------------------------
// Name: ProcessFrame
// Description: Present one media sample.
//-------------------------------------------------------------------

HRESULT debuggerking::presenter::process_frame(IMFMediaType* pCurrentType, IMFSample* pSample, UINT32* punInterlaceMode, BOOL* pbDeviceChanged, BOOL* pbProcessAgain, IMFSample** ppOutputSample)
{
	HRESULT hr = S_OK;
	BYTE* pData = NULL;
	DWORD dwSampleSize = 0;
	IMFMediaBuffer* pBuffer = NULL;
	IMFMediaBuffer* pEVBuffer = NULL;
	DWORD cBuffers = 0;
	ID3D11Texture2D* pTexture2D = NULL;
	IMFDXGIBuffer* pDXGIBuffer = NULL;
	ID3D11Texture2D* pEVTexture2D = NULL;
	IMFDXGIBuffer* pEVDXGIBuffer = NULL;
	ID3D11Device* pDeviceInput = NULL;
	UINT dwViewIndex = 0;
	UINT dwEVViewIndex = 0;

	autolock lock(&_cs);

	do
	{
		hr = CheckShutdown();
		if (FAILED(hr))
		{
			break;
		}

		if (punInterlaceMode == NULL || pCurrentType == NULL || pSample == NULL || pbDeviceChanged == NULL || pbProcessAgain == NULL)
		{
			hr = E_POINTER;
			break;
		}

		*pbProcessAgain = FALSE;
		*pbDeviceChanged = FALSE;

		hr = pSample->GetBufferCount(&cBuffers);
		if (FAILED(hr))
		{
			break;
		}

		if (1 == cBuffers)
		{
			hr = pSample->GetBufferByIndex(0, &pBuffer);
		}
		else if (2 == cBuffers && m_b3DVideo && 0 != m_vp3DOutput)
		{
			hr = pSample->GetBufferByIndex(0, &pBuffer);
			if (FAILED(hr))
			{
				break;
			}

			hr = pSample->GetBufferByIndex(1, &pEVBuffer);
		}
		else
		{
			hr = pSample->ConvertToContiguousBuffer(&pBuffer);
		}

		if (FAILED(hr))
		{
			break;
		}

		hr = CheckDeviceState(pbDeviceChanged);
		if (FAILED(hr))
		{
			break;
		}

		RECT rcDest;
		ZeroMemory(&rcDest, sizeof(rcDest));
		if (CheckEmptyRect(&rcDest))
		{
			hr = S_OK;
			break;
		}

		MFVideoInterlaceMode unInterlaceMode = (MFVideoInterlaceMode)MFGetAttributeUINT32(pCurrentType, MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

		//
		// Check the per-sample attributes
		//
		if (MFVideoInterlace_MixedInterlaceOrProgressive == unInterlaceMode)
		{
			BOOL fInterlaced = MFGetAttributeUINT32(pSample, MFSampleExtension_Interlaced, FALSE);
			if (!fInterlaced)
			{
				// Progressive sample
				*punInterlaceMode = MFVideoInterlace_Progressive;
			}
			else
			{
				BOOL fBottomFirst = MFGetAttributeUINT32(pSample, MFSampleExtension_BottomFieldFirst, FALSE);
				if (fBottomFirst)
				{
					*punInterlaceMode = MFVideoInterlace_FieldInterleavedLowerFirst;
				}
				else
				{
					*punInterlaceMode = MFVideoInterlace_FieldInterleavedUpperFirst;
				}
			}
		}

		hr = pBuffer->QueryInterface(__uuidof(IMFDXGIBuffer), (LPVOID*)&pDXGIBuffer);
		if (FAILED(hr))
		{
			break;
		}

		hr = pDXGIBuffer->GetResource(__uuidof(ID3D11Texture2D), (LPVOID*)&pTexture2D);
		if (FAILED(hr))
		{
			break;
		}

		hr = pDXGIBuffer->GetSubresourceIndex(&dwViewIndex);
		if (FAILED(hr))
		{
			break;
		}

		if (m_b3DVideo && 0 != m_vp3DOutput)
		{
			if (pEVBuffer && MFVideo3DSampleFormat_MultiView == m_vp3DOutput)
			{
				hr = pEVBuffer->QueryInterface(__uuidof(IMFDXGIBuffer), (LPVOID*)&pEVDXGIBuffer);
				if (FAILED(hr))
				{
					break;
				}

				hr = pEVDXGIBuffer->GetResource(__uuidof(ID3D11Texture2D), (LPVOID*)&pEVTexture2D);
				if (FAILED(hr))
				{
					break;
				}

				hr = pEVDXGIBuffer->GetSubresourceIndex(&dwEVViewIndex);
				if (FAILED(hr))
				{
					break;
				}
			}
		}

		pTexture2D->GetDevice(&pDeviceInput);
		if ((NULL == pDeviceInput) || (pDeviceInput != m_pD3D11Device))
		{
			break;
		}

		if (_use_xvp)
		{
			BOOL bInputFrameUsed = FALSE;

			hr = ProcessFrameUsingXVP(pCurrentType, pSample, pTexture2D, rcDest, ppOutputSample, &bInputFrameUsed);

			if (SUCCEEDED(hr) && !bInputFrameUsed)
			{
				*pbProcessAgain = TRUE;
			}
		}
		else
		{
			hr = ProcessFrameUsingD3D11(pTexture2D, pEVTexture2D, dwViewIndex, dwEVViewIndex, rcDest, *punInterlaceMode, ppOutputSample);

			LONGLONG hnsDuration = 0;
			LONGLONG hnsTime = 0;
			DWORD dwSampleFlags = 0;

			if (ppOutputSample != NULL && *ppOutputSample != NULL)
			{
				if (SUCCEEDED(pSample->GetSampleDuration(&hnsDuration)))
				{
					(*ppOutputSample)->SetSampleDuration(hnsDuration);
				}

				if (SUCCEEDED(pSample->GetSampleTime(&hnsTime)))
				{
					(*ppOutputSample)->SetSampleTime(hnsTime);
				}

				if (SUCCEEDED(pSample->GetSampleFlags(&dwSampleFlags)))
				{
					(*ppOutputSample)->SetSampleFlags(dwSampleFlags);
				}
			}
		}
	} while (FALSE);

	safe_release(pTexture2D);
	safe_release(pDXGIBuffer);
	safe_release(pEVTexture2D);
	safe_release(pEVDXGIBuffer);
	safe_release(pDeviceInput);
	safe_release(pBuffer);
	safe_release(pEVBuffer);

	return hr;
}

HRESULT debuggerking::presenter::SetCurrentMediaType(IMFMediaType* pMediaType)
{
	HRESULT hr = S_OK;
	IMFAttributes* pAttributes = NULL;

	autolock lock(&_cs);

	do
	{
		hr = CheckShutdown();
		if (FAILED(hr))
		{
			break;
		}

		hr = pMediaType->QueryInterface(IID_IMFAttributes, reinterpret_cast<void**>(&pAttributes));
		if (FAILED(hr))
		{
			break;
		}

		HRESULT hr1 = pAttributes->GetUINT32(MF_MT_VIDEO_3D, (UINT32*)&m_b3DVideo);
		if (SUCCEEDED(hr1))
		{
			hr = pAttributes->GetUINT32(MF_MT_VIDEO_3D_FORMAT, (UINT32*)&m_vp3DOutput);
			if (FAILED(hr))
			{
				break;
			}
		}

		//Now Determine Correct Display Resolution
		if (SUCCEEDED(hr))
		{
			UINT32 parX = 0, parY = 0;
			int PARWidth = 0, PARHeight = 0;
			MFVideoArea videoArea = { 0 };
			ZeroMemory(&m_displayRect, sizeof(RECT));

			if (FAILED(MFGetAttributeSize(pMediaType, MF_MT_PIXEL_ASPECT_RATIO, &parX, &parY)))
			{
				parX = 1;
				parY = 1;
			}

			hr = GetVideoDisplayArea(pMediaType, &videoArea);
			if (FAILED(hr))
			{
				break;
			}

			m_displayRect = MFVideoAreaToRect(videoArea);

			PixelAspectToPictureAspect(
				videoArea.Area.cx,
				videoArea.Area.cy,
				parX,
				parY,
				&PARWidth,
				&PARHeight);

			SIZE szVideo = videoArea.Area;
			SIZE szPARVideo = { PARWidth, PARHeight };
			AspectRatioCorrectSize(&szVideo, szPARVideo, videoArea.Area, FALSE);
			m_uiRealDisplayWidth = szVideo.cx;
			m_uiRealDisplayHeight = szVideo.cy;
		}

		if (SUCCEEDED(hr) && _use_xvp)
		{
			// set the input type on the XVP
			hr = m_pXVP->SetInputType(0, pMediaType, 0);
			if (FAILED(hr))
			{
				break;
			}
		}
	} while (FALSE);

	safe_release(pAttributes);

	return hr;
}

//-------------------------------------------------------------------
// Name: Shutdown
// Description: Releases resources held by the presenter.
//-------------------------------------------------------------------

HRESULT debuggerking::presenter::Shutdown(void)
{
	autolock lock(&_cs);

	HRESULT hr = MF_E_SHUTDOWN;

	m_IsShutdown = TRUE;

	safe_release(_dxgi_manager);
	safe_release(m_pDXGIFactory2);
	safe_release(m_pD3D11Device);
	safe_release(m_pD3DImmediateContext);
	safe_release(m_pDXGIOutput1);
	safe_release(_sample_allocator_ex);
	safe_release(m_pDCompDevice);
	safe_release(m_pHwndTarget);
	safe_release(m_pRootVisual);
	safe_release(m_pXVPControl);
	safe_release(m_pXVP);
	safe_release(_dx11_video_device);
	safe_release(_video_processor);
	safe_release(_video_processor_enum);
	safe_release(m_pSwapChain1);

	return hr;
}

/// Private methods

//+-------------------------------------------------------------------------
//
//  Function:   AspectRatioCorrectSize
//
//  Synopsis:   Corrects the supplied size structure so that it becomes the same shape
//              as the specified aspect ratio, the correction is always applied in the
//              horizontal axis
//
//--------------------------------------------------------------------------

void debuggerking::presenter::AspectRatioCorrectSize(
	LPSIZE lpSizeImage,     // size to be aspect ratio corrected
	const SIZE& sizeAr,     // aspect ratio of image
	const SIZE& sizeOrig,   // original image size
	BOOL ScaleXorY          // axis to correct in
	)
{
	int cxAR = sizeAr.cx;
	int cyAR = sizeAr.cy;
	int cxOr = sizeOrig.cx;
	int cyOr = sizeOrig.cy;
	int sx = lpSizeImage->cx;
	int sy = lpSizeImage->cy;

	// MulDiv rounds correctly.
	lpSizeImage->cx = MulDiv((sx * cyOr), cxAR, (cyAR * cxOr));

	if (ScaleXorY && lpSizeImage->cx < cxOr)
	{
		lpSizeImage->cx = cxOr;
		lpSizeImage->cy = MulDiv((sy * cxOr), cyAR, (cxAR * cyOr));
	}
}

void debuggerking::presenter::CheckDecodeSwitchRegKey(void)
{
	const TCHAR* lpcszDXSW = TEXT("DXSWSwitch");
	const TCHAR* lpcszInVP = TEXT("XVP");
	const TCHAR* lpcszDComp = TEXT("DComp");
	const TCHAR* lpcszDebugLayer = TEXT("Dbglayer");
	const TCHAR* lpcszREGKEY = TEXT("SOFTWARE\\Microsoft\\Scrunch\\CodecPack\\MSDVD");
	HKEY hk = NULL;
	DWORD dwData;
	DWORD cbData = sizeof(DWORD);
	DWORD cbType;

	if (0 == RegOpenKeyEx(HKEY_CURRENT_USER, lpcszREGKEY, 0, KEY_READ, &hk))
	{
		if (0 == RegQueryValueEx(hk, lpcszDXSW, 0, &cbType, (LPBYTE)&dwData, &cbData))
		{
			m_DXSWSwitch = dwData;
		}

		dwData = 0;
		cbData = sizeof(DWORD);
		if (0 == RegQueryValueEx(hk, lpcszInVP, 0, &cbType, (LPBYTE)&dwData, &cbData))
		{
			_use_xvp = dwData;
		}

		dwData = 0;
		cbData = sizeof(DWORD);
		if (0 == RegQueryValueEx(hk, lpcszDComp, 0, &cbType, (LPBYTE)&dwData, &cbData))
		{
			m_useDCompVisual = dwData;
		}

		dwData = 0;
		cbData = sizeof(DWORD);
		if (0 == RegQueryValueEx(hk, lpcszDebugLayer, 0, &cbType, (LPBYTE)&dwData, &cbData))
		{
			m_useDebugLayer = dwData;
		}
	}

	if (NULL != hk)
	{
		RegCloseKey(hk);
	}

	return;
}

HRESULT debuggerking::presenter::CheckDeviceState(BOOL* pbDeviceChanged)
{
	if (pbDeviceChanged == NULL)
	{
		return E_POINTER;
	}

	static int deviceStateChecks = 0;
	static D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

	HRESULT hr = SetVideoMonitor(_hwnd_video);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pD3D11Device != NULL)
	{
		// Lost/hung device. Destroy the device and create a new one.
		if (S_FALSE == hr || (m_DXSWSwitch > 0 && deviceStateChecks == m_DXSWSwitch))
		{
			if (m_DXSWSwitch > 0 && deviceStateChecks == m_DXSWSwitch)
			{
				(driverType == D3D_DRIVER_TYPE_HARDWARE) ? driverType = D3D_DRIVER_TYPE_WARP : driverType = D3D_DRIVER_TYPE_HARDWARE;
			}

			hr = CreateDXGIManagerAndDevice(driverType);
			if (FAILED(hr))
			{
				return hr;
			}

			*pbDeviceChanged = TRUE;

			safe_release(_dx11_video_device);
			safe_release(_video_processor_enum);
			safe_release(_video_processor);
			safe_release(m_pSwapChain1);

			deviceStateChecks = 0;
		}
		deviceStateChecks++;
	}

	return hr;
}

BOOL debuggerking::presenter::CheckEmptyRect(RECT* pDst)
{
	GetClientRect(_hwnd_video, pDst);

	return IsRectEmpty(pDst);
}

HRESULT debuggerking::presenter::CheckShutdown(void) const
{
	if (m_IsShutdown)
	{
		return MF_E_SHUTDOWN;
	}
	else
	{
		return S_OK;
	}
}

HRESULT debuggerking::presenter::CreateDCompDeviceAndVisual(void)
{
	HRESULT hr = S_OK;
	IDXGIDevice* pDXGIDevice = NULL;

	do
	{
		hr = m_pD3D11Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDevice));
		if (FAILED(hr))
		{
			break;
		}

		hr = DCompositionCreateDevice(pDXGIDevice, __uuidof(IDCompositionDevice), reinterpret_cast<void**>(&m_pDCompDevice));
		if (FAILED(hr))
		{
			break;
		}

		hr = m_pDCompDevice->CreateTargetForHwnd(_hwnd_video, TRUE, &m_pHwndTarget);
		if (FAILED(hr))
		{
			break;
		}

		hr = m_pDCompDevice->CreateVisual(reinterpret_cast<IDCompositionVisual**>(&m_pRootVisual));
		if (FAILED(hr))
		{
			break;
		}

		hr = m_pHwndTarget->SetRoot(m_pRootVisual);
		if (FAILED(hr))
		{
			break;
		}
	} while (FALSE);

	safe_release(pDXGIDevice);

	return hr;
}

//-------------------------------------------------------------------
// Name: CreateDXGIManagerAndDevice
// Description: Creates D3D11 device and manager.
//
// Note: This method is called once when SetVideoWindow is called using
//       IDX11VideoRenderer.
//-------------------------------------------------------------------

HRESULT debuggerking::presenter::CreateDXGIManagerAndDevice(D3D_DRIVER_TYPE DriverType)
{
	HRESULT hr = S_OK;

	IDXGIAdapter* pTempAdapter = NULL;
	ID3D10Multithread* pMultiThread = NULL;
	IDXGIDevice1* pDXGIDev = NULL;
	IDXGIAdapter1* pAdapter = NULL;
	IDXGIOutput* pDXGIOutput = NULL;

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
	D3D_FEATURE_LEVEL featureLevel;
	UINT resetToken;

	do
	{
		safe_release(m_pD3D11Device);
		if (D3D_DRIVER_TYPE_WARP == DriverType)
		{
			ID3D11Device* pD3D11Device = NULL;

			hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, m_useDebugLayer, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &pD3D11Device, &featureLevel, NULL);

			if (SUCCEEDED(hr))
			{
				m_pD3D11Device = new CPrivate_ID3D11Device(pD3D11Device);
				if (NULL == m_pD3D11Device)
				{
					E_OUTOFMEMORY;
				}
			}
			// safe_release(pD3D11Device);
		}
		else
		{
			for (DWORD dwCount = 0; dwCount < ARRAYSIZE(featureLevels); dwCount++)
			{
				hr = D3D11CreateDevice(NULL, DriverType, NULL, m_useDebugLayer, &featureLevels[dwCount], 1, D3D11_SDK_VERSION, &m_pD3D11Device, &featureLevel, NULL);
				if (SUCCEEDED(hr))
				{
					ID3D11VideoDevice* pDX11VideoDevice = NULL;
					hr = m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&pDX11VideoDevice);
					safe_release(pDX11VideoDevice);

					if (SUCCEEDED(hr))
					{
						break;
					}
					safe_release(m_pD3D11Device);
				}
			}
		}

		if (FAILED(hr))
		{
			break;
		}

		if (NULL == _dxgi_manager)
		{
			hr = MFCreateDXGIDeviceManager(&resetToken, &_dxgi_manager);
			if (FAILED(hr))
			{
				break;
			}
			m_DeviceResetToken = resetToken;
		}

		hr = _dxgi_manager->ResetDevice(m_pD3D11Device, m_DeviceResetToken);
		if (FAILED(hr))
		{
			break;
		}

		safe_release(m_pD3DImmediateContext);
		m_pD3D11Device->GetImmediateContext(&m_pD3DImmediateContext);

		// Need to explitly set the multithreaded mode for this device
		hr = m_pD3DImmediateContext->QueryInterface(__uuidof(ID3D10Multithread), (void**)&pMultiThread);
		if (FAILED(hr))
		{
			break;
		}

		pMultiThread->SetMultithreadProtected(TRUE);

		hr = m_pD3D11Device->QueryInterface(__uuidof(IDXGIDevice1), (LPVOID*)&pDXGIDev);
		if (FAILED(hr))
		{
			break;
		}

		hr = pDXGIDev->GetAdapter(&pTempAdapter);
		if (FAILED(hr))
		{
			break;
		}

		hr = pTempAdapter->QueryInterface(__uuidof(IDXGIAdapter1), (LPVOID*)&pAdapter);
		if (FAILED(hr))
		{
			break;
		}

		safe_release(m_pDXGIFactory2);
		hr = pAdapter->GetParent(__uuidof(IDXGIFactory2), (LPVOID*)&m_pDXGIFactory2);
		if (FAILED(hr))
		{
			break;
		}

		hr = pAdapter->EnumOutputs(0, &pDXGIOutput);
		if (FAILED(hr))
		{
			break;
		}

		safe_release(m_pDXGIOutput1);
		hr = pDXGIOutput->QueryInterface(__uuidof(IDXGIOutput1), (LPVOID*)&m_pDXGIOutput1);
		if (FAILED(hr))
		{
			break;
		}

		if (m_useDCompVisual)
		{
			hr = CreateDCompDeviceAndVisual();
			if (FAILED(hr))
			{
				break;
			}
		}
	} while (FALSE);

	safe_release(pTempAdapter);
	safe_release(pMultiThread);
	safe_release(pDXGIDev);
	safe_release(pAdapter);
	safe_release(pDXGIOutput);

	return hr;
}

//-------------------------------------------------------------------
// Name: CreateXVP
// Description: Creates a new instance of the XVP MFT.
//-------------------------------------------------------------------

HRESULT debuggerking::presenter::CreateXVP(void)
{
	HRESULT hr = S_OK;
	IMFAttributes* pAttributes = NULL;

	do
	{
		hr = CoCreateInstance(CLSID_VideoProcessorMFT, nullptr, CLSCTX_INPROC_SERVER, IID_IMFTransform, (void**)&m_pXVP);
		if (FAILED(hr))
		{
			break;
		}

		hr = m_pXVP->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, ULONG_PTR(_dxgi_manager));
		if (FAILED(hr))
		{
			break;
		}

		// Tell the XVP that we are the swapchain allocator
		hr = m_pXVP->GetAttributes(&pAttributes);
		if (FAILED(hr))
		{
			break;
		}

		hr = pAttributes->SetUINT32(MF_XVP_PLAYBACK_MODE, TRUE);
		if (FAILED(hr))
		{
			break;
		}

		hr = m_pXVP->QueryInterface(IID_PPV_ARGS(&m_pXVPControl));
		if (FAILED(hr))
		{
			break;
		}
	} while (FALSE);

	safe_release(pAttributes);

	return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     FindBOBProcessorIndex
//
//  Synopsis:   Find the BOB video processor. BOB does not require any
//              reference frames and can be used with both Progressive
//              and interlaced video
//
//--------------------------------------------------------------------------

HRESULT debuggerking::presenter::FindBOBProcessorIndex(DWORD* pIndex)
{
	HRESULT hr = S_OK;
	D3D11_VIDEO_PROCESSOR_CAPS caps = {};
	D3D11_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS convCaps = {};

	*pIndex = 0;
	hr = _video_processor_enum->GetVideoProcessorCaps(&caps);
	if (FAILED(hr))
	{
		return hr;
	}
	for (DWORD i = 0; i < caps.RateConversionCapsCount; i++)
	{
		hr = _video_processor_enum->GetVideoProcessorRateConversionCaps(i, &convCaps);
		if (FAILED(hr))
		{
			return hr;
		}

		// Check the caps to see which deinterlacer is supported
		if ((convCaps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BOB) != 0)
		{
			*pIndex = i;
			return hr;
		}
	}

	return E_FAIL;
}

//-------------------------------------------------------------------
// Name: GetVideoDisplayArea
// Description: get the display area from the media type.
//-------------------------------------------------------------------

HRESULT debuggerking::presenter::GetVideoDisplayArea(IMFMediaType* pType, MFVideoArea* pArea)
{
	HRESULT hr = S_OK;
	BOOL bPanScan = FALSE;
	UINT32 uimageWidthInPixels = 0, uimageHeightInPixels = 0;

	hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &uimageWidthInPixels, &uimageHeightInPixels);
	if (FAILED(hr))
	{
		return hr;
	}

	if (uimageWidthInPixels != m_imageWidthInPixels || uimageHeightInPixels != m_imageHeightInPixels)
	{
		safe_release(_video_processor_enum);
		safe_release(_video_processor);
		safe_release(m_pSwapChain1);
	}

	m_imageWidthInPixels = uimageWidthInPixels;
	m_imageHeightInPixels = uimageHeightInPixels;

	bPanScan = MFGetAttributeUINT32(pType, MF_MT_PAN_SCAN_ENABLED, FALSE);

	// In pan/scan mode, try to get the pan/scan region.
	if (bPanScan)
	{
		hr = pType->GetBlob(
			MF_MT_PAN_SCAN_APERTURE,
			(UINT8*)pArea,
			sizeof(MFVideoArea),
			NULL
			);
	}

	// If not in pan/scan mode, or the pan/scan region is not set,
	// get the minimimum display aperture.

	if (!bPanScan || hr == MF_E_ATTRIBUTENOTFOUND)
	{
		hr = pType->GetBlob(
			MF_MT_MINIMUM_DISPLAY_APERTURE,
			(UINT8*)pArea,
			sizeof(MFVideoArea),
			NULL
			);

		if (hr == MF_E_ATTRIBUTENOTFOUND)
		{
			// Minimum display aperture is not set.

			// For backward compatibility with some components,
			// check for a geometric aperture.

			hr = pType->GetBlob(
				MF_MT_GEOMETRIC_APERTURE,
				(UINT8*)pArea,
				sizeof(MFVideoArea),
				NULL
				);
		}

		// Default: Use the entire video area.

		if (hr == MF_E_ATTRIBUTENOTFOUND)
		{
			*pArea = MakeArea(0.0, 0.0, m_imageWidthInPixels, m_imageHeightInPixels);
			hr = S_OK;
		}
	}

	return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   LetterBoxDstRectPixelAspectToPictureAspect
//
//  Synopsis:
//
// Takes a src rectangle and constructs the largest possible destination
// rectangle within the specifed destination rectangle such that
// the video maintains its current shape.
//
// This function assumes that pels are the same shape within both the src
// and dst rectangles.
//
//--------------------------------------------------------------------------

void debuggerking::presenter::LetterBoxDstRect(
	LPRECT lprcLBDst,     // output letterboxed rectangle
	const RECT& rcSrc,    // input source rectangle
	const RECT& rcDst     // input destination rectangle
	)
{
	// figure out src/dest scale ratios
	int iSrcWidth = rcSrc.right - rcSrc.left;
	int iSrcHeight = rcSrc.bottom - rcSrc.top;

	int iDstWidth = rcDst.right - rcDst.left;
	int iDstHeight = rcDst.bottom - rcDst.top;

	int iDstLBWidth = 0;
	int iDstLBHeight = 0;

	//
	// work out if we are Column or Row letter boxing
	//

	if (MulDiv(iSrcWidth, iDstHeight, iSrcHeight) <= iDstWidth)
	{
		//
		// column letter boxing - we add border color bars to the
		// left and right of the video image to fill the destination
		// rectangle.
		//
		iDstLBWidth = MulDiv(iDstHeight, iSrcWidth, iSrcHeight);
		iDstLBHeight = iDstHeight;
	}
	else
	{
		//
		// row letter boxing - we add border color bars to the top
		// and bottom of the video image to fill the destination
		// rectangle
		//
		iDstLBWidth = iDstWidth;
		iDstLBHeight = MulDiv(iDstWidth, iSrcHeight, iSrcWidth);
	}

	//
	// now create a centered LB rectangle within the current destination rect
	//
	lprcLBDst->left = rcDst.left + ((iDstWidth - iDstLBWidth) / 2);
	lprcLBDst->right = lprcLBDst->left + iDstLBWidth;

	lprcLBDst->top = rcDst.top + ((iDstHeight - iDstLBHeight) / 2);
	lprcLBDst->bottom = lprcLBDst->top + iDstLBHeight;
}

//+-------------------------------------------------------------------------
//
//  Function:   PixelAspectToPictureAspect
//
//  Synopsis:   Converts a pixel aspect ratio to a picture aspect ratio
//
//--------------------------------------------------------------------------

void debuggerking::presenter::PixelAspectToPictureAspect(
	int Width,
	int Height,
	int PixelAspectX,
	int PixelAspectY,
	int* pPictureAspectX,
	int* pPictureAspectY
	)
{
	//
	// sanity check - if any inputs are 0, return 0
	//
	if (PixelAspectX == 0 || PixelAspectY == 0 || Width == 0 || Height == 0)
	{
		*pPictureAspectX = 0;
		*pPictureAspectY = 0;
		return;
	}

	//
	// start by reducing both ratios to lowest terms
	//
	ReduceToLowestTerms(Width, Height, &Width, &Height);
	ReduceToLowestTerms(PixelAspectX, PixelAspectY, &PixelAspectX, &PixelAspectY);

	//
	// Make sure that none of the values are larger than 2^16, so we don't
	// overflow on the last operation.   This reduces the accuracy somewhat,
	// but it's a "hail mary" for incredibly strange aspect ratios that don't
	// exist in practical usage.
	//
	while (Width > 0xFFFF || Height > 0xFFFF)
	{
		Width >>= 1;
		Height >>= 1;
	}

	while (PixelAspectX > 0xFFFF || PixelAspectY > 0xFFFF)
	{
		PixelAspectX >>= 1;
		PixelAspectY >>= 1;
	}

	ReduceToLowestTerms(
		PixelAspectX * Width,
		PixelAspectY * Height,
		pPictureAspectX,
		pPictureAspectY
		);
}

HRESULT debuggerking::presenter::ProcessFrameUsingD3D11(ID3D11Texture2D* pLeftTexture2D, ID3D11Texture2D* pRightTexture2D, UINT dwLeftViewIndex, UINT dwRightViewIndex, RECT rcDest, UINT32 unInterlaceMode, IMFSample** ppVideoOutFrame)
{
	HRESULT hr = S_OK;
	ID3D11VideoContext* pVideoContext = NULL;
	ID3D11VideoProcessorInputView* pLeftInputView = NULL;
	ID3D11VideoProcessorInputView* pRightInputView = NULL;
	ID3D11VideoProcessorOutputView* pOutputView = NULL;
	ID3D11Texture2D* pDXGIBackBuffer = NULL;
	ID3D11RenderTargetView* pRTView = NULL;
	IMFSample* pRTSample = NULL;
	IMFMediaBuffer* pBuffer = NULL;
	D3D11_VIDEO_PROCESSOR_CAPS vpCaps = { 0 };
	LARGE_INTEGER lpcStart, lpcEnd;

	do
	{
		if (!_dx11_video_device)
		{
			hr = m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&_dx11_video_device);
			if (FAILED(hr))
			{
				break;
			}
		}

		hr = m_pD3DImmediateContext->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&pVideoContext);
		if (FAILED(hr))
		{
			break;
		}

		// remember the original rectangles
		RECT TRectOld = m_rcDstApp;
		RECT SRectOld = m_rcSrcApp;
		UpdateRectangles(&TRectOld, &SRectOld);

		//Update destination rect with current client rect
		m_rcDstApp = rcDest;

		D3D11_TEXTURE2D_DESC surfaceDesc;
		pLeftTexture2D->GetDesc(&surfaceDesc);

		if (!_video_processor_enum || !_video_processor || m_imageWidthInPixels != surfaceDesc.Width || m_imageHeightInPixels != surfaceDesc.Height)
		{
			safe_release(_video_processor_enum);
			safe_release(_video_processor);

			m_imageWidthInPixels = surfaceDesc.Width;
			m_imageHeightInPixels = surfaceDesc.Height;

			D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
			ZeroMemory(&ContentDesc, sizeof(ContentDesc));
			ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
			ContentDesc.InputWidth = surfaceDesc.Width;
			ContentDesc.InputHeight = surfaceDesc.Height;
			ContentDesc.OutputWidth = surfaceDesc.Width;
			ContentDesc.OutputHeight = surfaceDesc.Height;
			ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

			hr = _dx11_video_device->CreateVideoProcessorEnumerator(&ContentDesc, &_video_processor_enum);
			if (FAILED(hr))
			{
				break;
			}

			UINT uiFlags;
			DXGI_FORMAT VP_Output_Format = DXGI_FORMAT_B8G8R8A8_UNORM;

			hr = _video_processor_enum->CheckVideoProcessorFormat(VP_Output_Format, &uiFlags);
			if (FAILED(hr) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT))
			{
				hr = MF_E_UNSUPPORTED_D3D_TYPE;
				break;
			}

			m_rcSrcApp.left = 0;
			m_rcSrcApp.top = 0;
			m_rcSrcApp.right = m_uiRealDisplayWidth;
			m_rcSrcApp.bottom = m_uiRealDisplayHeight;

			DWORD index;
			hr = FindBOBProcessorIndex(&index);
			if (FAILED(hr))
			{
				break;
			}

			hr = _dx11_video_device->CreateVideoProcessor(_video_processor_enum, index, &_video_processor);
			if (FAILED(hr))
			{
				break;
			}

			if (m_b3DVideo)
			{
				hr = _video_processor_enum->GetVideoProcessorCaps(&vpCaps);
				if (FAILED(hr))
				{
					break;
				}

				if (vpCaps.FeatureCaps & D3D11_VIDEO_PROCESSOR_FEATURE_CAPS_STEREO)
				{
					m_bStereoEnabled = TRUE;
				}

				DXGI_MODE_DESC1 modeFilter = { 0 };
				modeFilter.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				modeFilter.Width = surfaceDesc.Width;
				modeFilter.Height = surfaceDesc.Height;
				modeFilter.Stereo = m_bStereoEnabled;

				DXGI_MODE_DESC1 matchedMode;
				if (_fullscreen_state)
				{
					hr = m_pDXGIOutput1->FindClosestMatchingMode1(&modeFilter, &matchedMode, m_pD3D11Device);
					if (FAILED(hr))
					{
						break;
					}
				}
			}
		}

		// now create the input and output media types - these need to reflect
		// the src and destination rectangles that we have been given.
		RECT TRect = m_rcDstApp;
		RECT SRect = m_rcSrcApp;
		UpdateRectangles(&TRect, &SRect);

		const BOOL fDestRectChanged = !EqualRect(&TRect, &TRectOld);

		if (!m_pSwapChain1 || fDestRectChanged)
		{
			hr = UpdateDXGISwapChain();
			if (FAILED(hr))
			{
				break;
			}
		}

		_can_process_next_sample = FALSE;

		// Get Backbuffer
		hr = m_pSwapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pDXGIBackBuffer);
		if (FAILED(hr))
		{
			break;
		}

		// create the output media sample
		hr = MFCreateSample(&pRTSample);
		if (FAILED(hr))
		{
			break;
		}

		hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pDXGIBackBuffer, 0, FALSE, &pBuffer);
		if (FAILED(hr))
		{
			break;
		}

		hr = pRTSample->AddBuffer(pBuffer);
		if (FAILED(hr))
		{
			break;
		}

		if (m_b3DVideo && 0 != m_vp3DOutput)
		{
			safe_release(pBuffer);

			hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pDXGIBackBuffer, 1, FALSE, &pBuffer);
			if (FAILED(hr))
			{
				break;
			}

			hr = pRTSample->AddBuffer(pBuffer);
			if (FAILED(hr))
			{
				break;
			}
		}

		QueryPerformanceCounter(&lpcStart);

		QueryPerformanceCounter(&lpcEnd);

		//
		// Create Output View of Output Surfaces.
		//
		D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc;
		ZeroMemory(&OutputViewDesc, sizeof(OutputViewDesc));
		if (m_b3DVideo && m_bStereoEnabled)
		{
			OutputViewDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2DARRAY;
		}
		else
		{
			OutputViewDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
		}
		OutputViewDesc.Texture2D.MipSlice = 0;
		OutputViewDesc.Texture2DArray.MipSlice = 0;
		OutputViewDesc.Texture2DArray.FirstArraySlice = 0;
		if (m_b3DVideo && 0 != m_vp3DOutput)
		{
			OutputViewDesc.Texture2DArray.ArraySize = 2; // STEREO
		}

		QueryPerformanceCounter(&lpcStart);

		hr = _dx11_video_device->CreateVideoProcessorOutputView(pDXGIBackBuffer, _video_processor_enum, &OutputViewDesc, &pOutputView);
		if (FAILED(hr))
		{
			break;
		}

		D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputLeftViewDesc;
		ZeroMemory(&InputLeftViewDesc, sizeof(InputLeftViewDesc));
		InputLeftViewDesc.FourCC = 0;
		InputLeftViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
		InputLeftViewDesc.Texture2D.MipSlice = 0;
		InputLeftViewDesc.Texture2D.ArraySlice = dwLeftViewIndex;

		hr = _dx11_video_device->CreateVideoProcessorInputView(pLeftTexture2D, _video_processor_enum, &InputLeftViewDesc, &pLeftInputView);
		if (FAILED(hr))
		{
			break;
		}

		if (m_b3DVideo && MFVideo3DSampleFormat_MultiView == m_vp3DOutput && pRightTexture2D)
		{
			D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputRightViewDesc;
			ZeroMemory(&InputRightViewDesc, sizeof(InputRightViewDesc));
			InputRightViewDesc.FourCC = 0;
			InputRightViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
			InputRightViewDesc.Texture2D.MipSlice = 0;
			InputRightViewDesc.Texture2D.ArraySlice = dwRightViewIndex;

			hr = _dx11_video_device->CreateVideoProcessorInputView(pRightTexture2D, _video_processor_enum, &InputRightViewDesc, &pRightInputView);
			if (FAILED(hr))
			{
				break;
			}
		}
		QueryPerformanceCounter(&lpcEnd);

		QueryPerformanceCounter(&lpcStart);

		SetVideoContextParameters(pVideoContext, &SRect, &TRect, unInterlaceMode);

		// Enable/Disable Stereo
		if (m_b3DVideo)
		{
			pVideoContext->VideoProcessorSetOutputStereoMode(_video_processor, m_bStereoEnabled);

			D3D11_VIDEO_PROCESSOR_STEREO_FORMAT vpStereoFormat = D3D11_VIDEO_PROCESSOR_STEREO_FORMAT_SEPARATE;
			if (MFVideo3DSampleFormat_Packed_LeftRight == m_vp3DOutput)
			{
				vpStereoFormat = D3D11_VIDEO_PROCESSOR_STEREO_FORMAT_HORIZONTAL;
			}
			else if (MFVideo3DSampleFormat_Packed_TopBottom == m_vp3DOutput)
			{
				vpStereoFormat = D3D11_VIDEO_PROCESSOR_STEREO_FORMAT_VERTICAL;
			}

			pVideoContext->VideoProcessorSetStreamStereoFormat(_video_processor,
				0, m_bStereoEnabled, vpStereoFormat, TRUE, TRUE, D3D11_VIDEO_PROCESSOR_STEREO_FLIP_NONE, 0);
		}

		QueryPerformanceCounter(&lpcEnd);

		QueryPerformanceCounter(&lpcStart);

		D3D11_VIDEO_PROCESSOR_STREAM StreamData;
		ZeroMemory(&StreamData, sizeof(StreamData));
		StreamData.Enable = TRUE;
		StreamData.OutputIndex = 0;
		StreamData.InputFrameOrField = 0;
		StreamData.PastFrames = 0;
		StreamData.FutureFrames = 0;
		StreamData.ppPastSurfaces = NULL;
		StreamData.ppFutureSurfaces = NULL;
		StreamData.pInputSurface = pLeftInputView;
		StreamData.ppPastSurfacesRight = NULL;
		StreamData.ppFutureSurfacesRight = NULL;

		if (m_b3DVideo && MFVideo3DSampleFormat_MultiView == m_vp3DOutput && pRightTexture2D)
		{
			StreamData.pInputSurfaceRight = pRightInputView;
		}

		hr = pVideoContext->VideoProcessorBlt(_video_processor, pOutputView, 0, 1, &StreamData);
		if (FAILED(hr))
		{
			break;
		}
		QueryPerformanceCounter(&lpcEnd);

		if (ppVideoOutFrame != NULL)
		{
			*ppVideoOutFrame = pRTSample;
			(*ppVideoOutFrame)->AddRef();
		}
	} while (FALSE);

	safe_release(pBuffer);
	safe_release(pRTSample);
	safe_release(pDXGIBackBuffer);
	safe_release(pOutputView);
	safe_release(pLeftInputView);
	safe_release(pRightInputView);
	safe_release(pVideoContext);

	return hr;
}

HRESULT debuggerking::presenter::ProcessFrameUsingXVP(IMFMediaType* pCurrentType, IMFSample* pVideoFrame, ID3D11Texture2D* pTexture2D, RECT rcDest, IMFSample** ppVideoOutFrame, BOOL* pbInputFrameUsed)
{
	HRESULT hr = S_OK;
	ID3D11VideoContext* pVideoContext = NULL;
	ID3D11Texture2D* pDXGIBackBuffer = NULL;
	IMFSample* pRTSample = NULL;
	IMFMediaBuffer* pBuffer = NULL;
	IMFAttributes*  pAttributes = NULL;
	D3D11_VIDEO_PROCESSOR_CAPS vpCaps = { 0 };

	do
	{
		if (!_dx11_video_device)
		{
			hr = m_pD3D11Device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&_dx11_video_device);
			if (FAILED(hr))
			{
				break;
			}
		}

		hr = m_pD3DImmediateContext->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&pVideoContext);
		if (FAILED(hr))
		{
			break;
		}

		// remember the original rectangles
		RECT TRectOld = m_rcDstApp;
		RECT SRectOld = m_rcSrcApp;
		UpdateRectangles(&TRectOld, &SRectOld);

		//Update destination rect with current client rect
		m_rcDstApp = rcDest;

		D3D11_TEXTURE2D_DESC surfaceDesc;
		pTexture2D->GetDesc(&surfaceDesc);

		BOOL fTypeChanged = FALSE;
		if (!_video_processor_enum || !m_pSwapChain1 || m_imageWidthInPixels != surfaceDesc.Width || m_imageHeightInPixels != surfaceDesc.Height)
		{
			safe_release(_video_processor_enum);
			safe_release(m_pSwapChain1);

			m_imageWidthInPixels = surfaceDesc.Width;
			m_imageHeightInPixels = surfaceDesc.Height;
			fTypeChanged = TRUE;

			D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
			ZeroMemory(&ContentDesc, sizeof(ContentDesc));
			ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
			ContentDesc.InputWidth = surfaceDesc.Width;
			ContentDesc.InputHeight = surfaceDesc.Height;
			ContentDesc.OutputWidth = surfaceDesc.Width;
			ContentDesc.OutputHeight = surfaceDesc.Height;
			ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

			hr = _dx11_video_device->CreateVideoProcessorEnumerator(&ContentDesc, &_video_processor_enum);
			if (FAILED(hr))
			{
				break;
			}

			m_rcSrcApp.left = 0;
			m_rcSrcApp.top = 0;
			m_rcSrcApp.right = m_uiRealDisplayWidth;
			m_rcSrcApp.bottom = m_uiRealDisplayHeight;

			if (m_b3DVideo)
			{
				hr = _video_processor_enum->GetVideoProcessorCaps(&vpCaps);
				if (FAILED(hr))
				{
					break;
				}

				if (vpCaps.FeatureCaps & D3D11_VIDEO_PROCESSOR_FEATURE_CAPS_STEREO)
				{
					m_bStereoEnabled = TRUE;
				}

				DXGI_MODE_DESC1 modeFilter = { 0 };
				modeFilter.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				modeFilter.Width = surfaceDesc.Width;
				modeFilter.Height = surfaceDesc.Height;
				modeFilter.Stereo = m_bStereoEnabled;

				DXGI_MODE_DESC1 matchedMode;
				if (_fullscreen_state)
				{
					hr = m_pDXGIOutput1->FindClosestMatchingMode1(&modeFilter, &matchedMode, m_pD3D11Device);
					if (FAILED(hr))
					{
						break;
					}
				}

				hr = m_pXVP->GetAttributes(&pAttributes);
				if (FAILED(hr))
				{
					break;
				}

				hr = pAttributes->SetUINT32(MF_ENABLE_3DVIDEO_OUTPUT, (0 != m_vp3DOutput) ? MF3DVideoOutputType_Stereo : MF3DVideoOutputType_BaseView);
				if (FAILED(hr))
				{
					break;
				}
			}
		}

		// now create the input and output media types - these need to reflect
		// the src and destination rectangles that we have been given.
		RECT TRect = m_rcDstApp;
		RECT SRect = m_rcSrcApp;
		UpdateRectangles(&TRect, &SRect);

		const BOOL fDestRectChanged = !EqualRect(&TRect, &TRectOld);
		const BOOL fSrcRectChanged = !EqualRect(&SRect, &SRectOld);

		if (!m_pSwapChain1 || fDestRectChanged)
		{
			hr = UpdateDXGISwapChain();
			if (FAILED(hr))
			{
				break;
			}
		}

		if (fTypeChanged || fSrcRectChanged || fDestRectChanged)
		{
			// stop streaming to avoid multiple start\stop calls internally in XVP
			hr = m_pXVP->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
			if (FAILED(hr))
			{
				break;
			}

			if (fTypeChanged)
			{
				hr = SetXVPOutputMediaType(pCurrentType, DXGI_FORMAT_B8G8R8A8_UNORM);
				if (FAILED(hr))
				{
					break;
				}
			}

			if (fDestRectChanged)
			{
				hr = m_pXVPControl->SetDestinationRectangle(&m_rcDstApp);
				if (FAILED(hr))
				{
					break;
				}
			}

			if (fSrcRectChanged)
			{
				hr = m_pXVPControl->SetSourceRectangle(&SRect);
				if (FAILED(hr))
				{
					break;
				}
			}

			hr = m_pXVP->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
			if (FAILED(hr))
			{
				break;
			}
		}

		_can_process_next_sample = FALSE;

		// Get Backbuffer
		hr = m_pSwapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pDXGIBackBuffer);
		if (FAILED(hr))
		{
			break;
		}

		// create the output media sample
		hr = MFCreateSample(&pRTSample);
		if (FAILED(hr))
		{
			break;
		}

		hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pDXGIBackBuffer, 0, FALSE, &pBuffer);
		if (FAILED(hr))
		{
			break;
		}

		hr = pRTSample->AddBuffer(pBuffer);
		if (FAILED(hr))
		{
			break;
		}

		if (m_b3DVideo && 0 != m_vp3DOutput)
		{
			safe_release(pBuffer);

			hr = MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pDXGIBackBuffer, 1, FALSE, &pBuffer);
			if (FAILED(hr))
			{
				break;
			}

			hr = pRTSample->AddBuffer(pBuffer);
			if (FAILED(hr))
			{
				break;
			}
		}

		DWORD dwStatus = 0;
		MFT_OUTPUT_DATA_BUFFER outputDataBuffer = {};
		outputDataBuffer.pSample = pRTSample;
		hr = m_pXVP->ProcessOutput(0, 1, &outputDataBuffer, &dwStatus);
		if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
		{
			//call process input on the MFT to deliver the YUV video sample
			// and the call process output to extract of newly processed frame
			hr = m_pXVP->ProcessInput(0, pVideoFrame, 0);
			if (FAILED(hr))
			{
				break;
			}

			*pbInputFrameUsed = TRUE;

			hr = m_pXVP->ProcessOutput(0, 1, &outputDataBuffer, &dwStatus);
			if (FAILED(hr))
			{
				break;
			}
		}
		else
		{
			*pbInputFrameUsed = FALSE;
		}

		if (ppVideoOutFrame != NULL)
		{
			*ppVideoOutFrame = pRTSample;
			(*ppVideoOutFrame)->AddRef();
		}
	} while (FALSE);

	safe_release(pAttributes);
	safe_release(pBuffer);
	safe_release(pRTSample);
	safe_release(pDXGIBackBuffer);
	safe_release(pVideoContext);

	return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   ReduceToLowestTerms
//
//  Synopsis:   reduces a numerator and denominator pair to their lowest terms
//
//--------------------------------------------------------------------------

void debuggerking::presenter::ReduceToLowestTerms(
	int NumeratorIn,
	int DenominatorIn,
	int* pNumeratorOut,
	int* pDenominatorOut
	)
{
	int GCD = gcd(NumeratorIn, DenominatorIn);

	*pNumeratorOut = NumeratorIn / GCD;
	*pDenominatorOut = DenominatorIn / GCD;
}

HRESULT debuggerking::presenter::SetMonitor(UINT adapterID)
{
	HRESULT hr = S_OK;
	DWORD dwMatchID = 0;

	autolock lock(&_cs);

	do
	{
		hr = _monitors->MatchGUID(adapterID, &dwMatchID);
		if (FAILED(hr))
		{
			break;
		}

		if (hr == S_FALSE)
		{
			hr = E_INVALIDARG;
			break;
		}

		m_lpCurrMon = &(*_monitors)[dwMatchID];
		m_ConnectionGUID = adapterID;
	} while (FALSE);

	return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     SetVideoContextParameters
//
//  Synopsis:   Updates the various parameters used for VpBlt call
//
//--------------------------------------------------------------------------

void debuggerking::presenter::SetVideoContextParameters(ID3D11VideoContext* pVideoContext, const RECT* pSRect, const RECT* pTRect, UINT32 unInterlaceMode)
{
	D3D11_VIDEO_FRAME_FORMAT FrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
	if (MFVideoInterlace_FieldInterleavedUpperFirst == unInterlaceMode || MFVideoInterlace_FieldSingleUpper == unInterlaceMode || MFVideoInterlace_MixedInterlaceOrProgressive == unInterlaceMode)
	{
		FrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
	}
	else if (MFVideoInterlace_FieldInterleavedLowerFirst == unInterlaceMode || MFVideoInterlace_FieldSingleLower == unInterlaceMode)
	{
		FrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST;
	}

	// input format
	pVideoContext->VideoProcessorSetStreamFrameFormat(_video_processor, 0, FrameFormat);

	// Output rate (repeat frames)
	pVideoContext->VideoProcessorSetStreamOutputRate(_video_processor, 0, D3D11_VIDEO_PROCESSOR_OUTPUT_RATE_NORMAL, TRUE, NULL);

	// Source rect
	pVideoContext->VideoProcessorSetStreamSourceRect(_video_processor, 0, TRUE, pSRect);

	// Stream dest rect
	pVideoContext->VideoProcessorSetStreamDestRect(_video_processor, 0, TRUE, pTRect);

	pVideoContext->VideoProcessorSetOutputTargetRect(_video_processor, TRUE, &m_rcDstApp);

	// Stream color space
	D3D11_VIDEO_PROCESSOR_COLOR_SPACE colorSpace = {};
	colorSpace.YCbCr_xvYCC = 1;
	pVideoContext->VideoProcessorSetStreamColorSpace(_video_processor, 0, &colorSpace);

	// Output color space
	pVideoContext->VideoProcessorSetOutputColorSpace(_video_processor, &colorSpace);

	// Output background color (black)
	D3D11_VIDEO_COLOR backgroundColor = {};
	backgroundColor.RGBA.A = 1.0F;
	backgroundColor.RGBA.R = 1.0F * static_cast<float>(GetRValue(0)) / 255.0F;
	backgroundColor.RGBA.G = 1.0F * static_cast<float>(GetGValue(0)) / 255.0F;
	backgroundColor.RGBA.B = 1.0F * static_cast<float>(GetBValue(0)) / 255.0F;

	pVideoContext->VideoProcessorSetOutputBackgroundColor(_video_processor, FALSE, &backgroundColor);
}

HRESULT debuggerking::presenter::SetVideoMonitor(HWND hwndVideo)
{
	HRESULT hr = S_OK;
	CAMDDrawMonitorInfo* pMonInfo = NULL;
	HMONITOR hMon = NULL;

	if (!_monitors)
	{
		return E_UNEXPECTED;
	}

	hMon = MonitorFromWindow(hwndVideo, MONITOR_DEFAULTTONULL);

	do
	{
		if (NULL != hMon)
		{
			_monitors->TerminateDisplaySystem();
			m_lpCurrMon = NULL;

			hr = _monitors->InitializeDisplaySystem(hwndVideo);
			if (FAILED(hr))
			{
				break;
			}

			pMonInfo = _monitors->FindMonitor(hMon);
			if (NULL != pMonInfo && pMonInfo->uDevID != m_ConnectionGUID)
			{
				hr = SetMonitor(pMonInfo->uDevID);
				if (FAILED(hr))
				{
					break;
				}
				hr = S_FALSE;
			}
		}
		else
		{
			hr = E_POINTER;
			break;
		}
	} while (FALSE);

	return hr;
}

//-------------------------------------------------------------------
// Name: SetXVPOutputMediaType
// Description: Tells the XVP about the size of the destination surface
// and where within the surface we should be writing.
//-------------------------------------------------------------------

HRESULT debuggerking::presenter::SetXVPOutputMediaType(IMFMediaType* pType, DXGI_FORMAT vpOutputFormat)
{
	HRESULT hr = S_OK;
	IMFVideoMediaType* pMTOutput = NULL;
	MFVIDEOFORMAT mfvf = {};

	if (SUCCEEDED(hr))
	{
		hr = MFInitVideoFormat_RGB(
			&mfvf,
			m_rcDstApp.right,
			m_rcDstApp.bottom,
			MFMapDXGIFormatToDX9Format(vpOutputFormat));
	}

	if (SUCCEEDED(hr))
	{
		hr = MFCreateVideoMediaType(&mfvf, &pMTOutput);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pXVP->SetOutputType(0, pMTOutput, 0);
	}

	safe_release(pMTOutput);

	return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     UpdateDXGISwapChain
//
//  Synopsis:   Creates SwapChain for HWND or DComp or Resizes buffers in case of resolution change
//
//--------------------------------------------------------------------------

_Post_satisfies_(this->m_pSwapChain1 != NULL)
HRESULT debuggerking::presenter::UpdateDXGISwapChain(void)
{
	HRESULT hr = S_OK;

	// Get the DXGISwapChain1
	DXGI_SWAP_CHAIN_DESC1 scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	scd.Scaling = DXGI_SCALING_STRETCH;
	scd.Width = m_rcDstApp.right;
	scd.Height = m_rcDstApp.bottom;
	scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.Stereo = m_bStereoEnabled;
	scd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.Flags = m_bStereoEnabled ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0; //opt in to do direct flip;
	scd.BufferCount = 4;

	do
	{
		if (m_pSwapChain1)
		{
			// Resize our back buffers for the desired format.
			hr = m_pSwapChain1->ResizeBuffers
				(
				4,
				m_rcDstApp.right,
				m_rcDstApp.bottom,
				scd.Format,
				scd.Flags
				);

			break;
		}

		if (!m_useDCompVisual)
		{
			hr = m_pDXGIFactory2->CreateSwapChainForHwnd(m_pD3D11Device, _hwnd_video, &scd, NULL, NULL, &m_pSwapChain1);
			if (FAILED(hr))
			{
				break;
			}

			if (_fullscreen_state)
			{
				hr = m_pSwapChain1->SetFullscreenState(TRUE, NULL);
				if (FAILED(hr))
				{
					break;
				}
			}
			else
			{
				hr = m_pSwapChain1->SetFullscreenState(FALSE, NULL);
				if (FAILED(hr))
				{
					break;
				}
			}
		}
		else
		{
			// Create a swap chain for composition
			hr = m_pDXGIFactory2->CreateSwapChainForComposition(m_pD3D11Device, &scd, NULL, &m_pSwapChain1);
			if (FAILED(hr))
			{
				break;
			}


			hr = m_pRootVisual->SetContent(m_pSwapChain1);
			if (FAILED(hr))
			{
				break;
			}

			hr = m_pDCompDevice->Commit();
			if (FAILED(hr))
			{
				break;
			}
		}
	} while (FALSE);

	return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     UpodateRectangles
//
//  Synopsis:   Figures out the real source and destination rectangles
//              to use when drawing the video frame into the clients
//              destination location.  Takes into account pixel aspect
//              ration correction, letterboxing and source rectangles.
//
//--------------------------------------------------------------------------

void debuggerking::presenter::UpdateRectangles(RECT* pDst, RECT* pSrc)
{
	// take the given src rect and reverse map it into the native video
	// image rectange.  For example, consider a video with a buffer size of
	// 720x480 and an active area of 704x480 - 8,0 with a picture aspect
	// ratio of 4:3.  The user sees the native video size as 640x480.
	//
	// If the user gave us a src rectangle of (180, 135, 540, 405)
	// then this gets reversed mapped to
	//
	// 8 + (180 * 704 / 640) = 206
	// 0 + (135 * 480 / 480) = 135
	// 8 + (540 * 704 / 640) = 602
	// 0 + (405 * 480 / 480) = 405

	RECT Src = *pSrc;

	pSrc->left = m_displayRect.left + MulDiv(pSrc->left, (m_displayRect.right - m_displayRect.left), m_uiRealDisplayWidth);
	pSrc->right = m_displayRect.left + MulDiv(pSrc->right, (m_displayRect.right - m_displayRect.left), m_uiRealDisplayWidth);

	pSrc->top = m_displayRect.top + MulDiv(pSrc->top, (m_displayRect.bottom - m_displayRect.top), m_uiRealDisplayHeight);
	pSrc->bottom = m_displayRect.top + MulDiv(pSrc->bottom, (m_displayRect.bottom - m_displayRect.top), m_uiRealDisplayHeight);

	LetterBoxDstRect(pDst, Src, m_rcDstApp);
}
