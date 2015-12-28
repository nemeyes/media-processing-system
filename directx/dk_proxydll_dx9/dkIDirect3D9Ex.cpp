#include "stdafx.h"
#include "dkIDirect3D9Ex.h"
#include <sstream>


dkIDirect3D9Ex::dkIDirect3D9Ex(IDirect3D9Ex *pOriginal)
{
	m_pIDirect3D9Ex = pOriginal;
}


dkIDirect3D9Ex::~dkIDirect3D9Ex(void)
{
}

/**
* Base QueryInterface functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::QueryInterface(REFIID riid, LPVOID* ppv)
{
#if 0

	return m_pIDirect3D9Ex->QueryInterface(riid, ppv);
#else

	HRESULT hr = S_OK;
	hr = m_pIDirect3D9Ex->QueryInterface(riid, ppv);


	std::stringstream ss;
	ss << "[jack] hr = " << hr << std::endl;
	char str[128] = { 0, };
	//sprintf_s(str, "%x", riid);
	if (strcmp(str, "81bdcbca") == 0)
		ss << "[jack] QueryInterface :IID_IDirect3D (" << str << ")" << std::endl;

	OutputDebugString(ss.str().c_str());

	if (SUCCEEDED(hr))
	{

		{
			//((IDirect3D9*)*ppv)->Release();
			//extern HINSTANCE gl_hOriginalDll;
			//extern UINT gl_SDKVersion;
			//// Hooking IDirect3D Object from Original Library
			//typedef IDirect3D9 *(WINAPI* D3D9_Type)(UINT SDKVersion);
			//D3D9_Type D3DCreate9_fn = (D3D9_Type)GetProcAddress(gl_hOriginalDll, "Direct3DCreate9");



			//extern EAPDirect3D9* gl_pEAPDirect3D9;
			//IDirect3D9 *pIDirect3D9_orig = D3DCreate9_fn(gl_SDKVersion);

			//
			//// Create my IDirect3D9 object and store pointer to original object there.
			//// note: the object will delete itself once Ref count is zero (similar to COM objects)
			//gl_pEAPDirect3D9 = new EAPDirect3D9(pIDirect3D9_orig);
			//*ppv = gl_pEAPDirect3D9;
		}

		{


			extern dkIDirect3D9* gl_dkIDirect3D9;
			IDirect3D9 *pIDirect3D9_orig = (IDirect3D9*)(*ppv);


			// Create my IDirect3D9 object and store pointer to original object there.
			// note: the object will delete itself once Ref count is zero (similar to COM objects)
			gl_dkIDirect3D9 = new dkIDirect3D9(pIDirect3D9_orig);
			*ppv = gl_dkIDirect3D9;
		}
	}

	return hr;
#endif
}

/**
* Base AddRef functionality.
***/
ULONG WINAPI dkIDirect3D9Ex::AddRef()
{
	return ++m_nRefCount;
}

/**
* Base Release functionality.
***/
ULONG WINAPI dkIDirect3D9Ex::Release()
{
	if (--m_nRefCount == 0)
	{
		delete this;
		return 0;
	}
	return m_nRefCount;
}

/**
* Base GetAdapterCount functionality.
***/
UINT dkIDirect3D9Ex::GetAdapterCount()
{
	return m_pIDirect3D9Ex->GetAdapterCount();
}

/**
* Base GetAdapterIdentifier functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	return m_pIDirect3D9Ex->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

/**
* Base GetAdapterModeCount functionality.
***/
UINT WINAPI dkIDirect3D9Ex::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
	return m_pIDirect3D9Ex->GetAdapterModeCount(Adapter, Format);
}

HRESULT	WINAPI dkIDirect3D9Ex::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	return m_pIDirect3D9Ex->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

/**
* Base GetAdapterDisplayMode functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode)
{
	return m_pIDirect3D9Ex->GetAdapterDisplayMode(Adapter, pMode);
}

/**
* Base CheckDeviceType functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	return m_pIDirect3D9Ex->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

/**
* Base CheckDeviceFormat functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return m_pIDirect3D9Ex->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType,
		CheckFormat);
}

/**
* Base CheckDeviceMultiSampleType functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return m_pIDirect3D9Ex->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed,
		MultiSampleType, pQualityLevels);
}

/**
* Base CheckDepthStencilMatch functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return m_pIDirect3D9Ex->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat,
		DepthStencilFormat);
}

/**
* Base CheckDeviceFormatConversion functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return m_pIDirect3D9Ex->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

/**
* Base GetDeviceCaps functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps)
{
	return m_pIDirect3D9Ex->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

/**
* Base GetAdapterMonitor functionality.
***/
HMONITOR WINAPI dkIDirect3D9Ex::GetAdapterMonitor(UINT Adapter)
{
	return m_pIDirect3D9Ex->GetAdapterMonitor(Adapter);
}

/**
* Create D3D device ex proxy.
* First it creates the device, then it loads the game configuration
* calling the ProxyHelper class. Last it creates and returns the
* device proxy calling D3DProxyDeviceFactory::Get().
***/
HRESULT WINAPI dkIDirect3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
	// Create real interface
	HRESULT hResult = m_pIDirect3D9Ex->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags,
		pPresentationParameters, ppReturnedDeviceInterface);

#if 1
	extern dkIDirect3DDevice9* gl_dkIDirect3DDevice9;
	gl_dkIDirect3DDevice9 = new dkIDirect3DDevice9(*ppReturnedDeviceInterface);
	*ppReturnedDeviceInterface = gl_dkIDirect3DDevice9;
#endif

	if (FAILED(hResult))
		return hResult;
	OutputDebugString("[OK] Normal D3D device created\n");
	/*
	debugf("Number of back buffers = %d\n", pPresentationParameters->BackBufferCount);

	// load configuration file
	ProxyHelper helper = ProxyHelper();
	ProxyConfig cfg;
	ProxyHelper::OculusProfile oculusProfile;
	if(!helper.LoadConfig(cfg, oculusProfile)) {
	OutputDebugString("[ERR] Config loading failed, config could not be loaded. Returning normal D3DDevice. Vireio will not be active.\n");
	return hResult;
	}

	OutputDebugString("[OK] Config loading - OK\n");

	if(cfg.stereo_mode == StereoView::DISABLED) {
	OutputDebugString("[WARN] stereo_mode == disabled. Returning normal D3DDevice. Vireio will not be active.\n");
	return hResult;
	}

	OutputDebugString("[OK] Stereo mode is enabled.\n");

	debugf("Config type: %s", cfg.game_type.c_str());
	OutputDebugString("\n");

	// Create and return proxy TODO !!
	//*ppReturnedDeviceInterface = D3DProxyDeviceFactory::Get(cfg, *ppReturnedDeviceInterface, this);

	OutputDebugString("[OK] Vireio D3D device created.\n");
	*/
	return hResult;
}

/**
* Base GetAdapterModeCountEx functionality.
***/
UINT WINAPI dkIDirect3D9Ex::GetAdapterModeCountEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter)
{
	return m_pIDirect3D9Ex->GetAdapterModeCountEx(Adapter, pFilter);
}

/**
* Base EnumAdapterModesEx functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::EnumAdapterModesEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode)
{
	return m_pIDirect3D9Ex->EnumAdapterModesEx(Adapter, pFilter, Mode, pMode);
}

/**
* Base GetAdapterDisplayModeEx functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	return m_pIDirect3D9Ex->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);
}

/**
* Base CreateDeviceEx functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::CreateDeviceEx(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface)
{
	// hhlle
	// CreateDeviceEx 사용하면 오류남...
#if 0
	LOG(g_hLog, CSLOG_LEVEL_TRACE, 0, "%s %s() : %d", __FILE__, __FUNCTION__, __LINE__);
	// we intercept this call and provide our own "fake" Device Object
	HRESULT hres = m_pIDirect3D9Ex->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);

	extern EAPDirect3DDevice9Ex* gl_pEAPDirect3DDevice9Ex;
	gl_pEAPDirect3DDevice9Ex = new EAPDirect3DDevice9Ex(*ppReturnedDeviceInterface, m_pIDirect3D9Ex);
	*ppReturnedDeviceInterface = gl_pEAPDirect3DDevice9Ex;

	return(hres);
#else
	return m_pIDirect3D9Ex->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);
#endif
}

/**
* Base GetAdapterLUID functionality.
***/
HRESULT WINAPI dkIDirect3D9Ex::GetAdapterLUID(UINT Adapter, LUID * pLUID)
{
	return m_pIDirect3D9Ex->GetAdapterLUID(Adapter, pLUID);
}


HRESULT WINAPI dkIDirect3D9Ex::RegisterSoftwareDevice(void* pInitializeFunction)
{
	return(m_pIDirect3D9Ex->RegisterSoftwareDevice(pInitializeFunction));
}