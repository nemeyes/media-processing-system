//#include <windows.h>
//#include <tchar.h>
//#include <dshow.h>
//#include <commctrl.h>
//#include <commdlg.h>
//#include <stdio.h>
//#include <atlbase.h>
//#include <string.h>
//#include <stdlib.h>
#include <streams.h>	
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <evr.h>
//#include <dvdmedia.h>

#include "dxva2_decoder_sample.h"

dxva2_decoder_sample::dxva2_decoder_sample(dxva2_decoder_allocator * alloc, HRESULT * hr)
	: CMediaSample(NAME("DXVA2 Decoder Sample"), (CBaseAllocator*)alloc, hr, NULL, 0)
	, _d3d9_surface(NULL)
	, _surface_id(0)
{
}

// Note: CMediaSample does not derive from CUnknown, so we cannot use the
//       DECLARE_IUNKNOWN macro that is used by most of the filter classes.
STDMETHODIMP dxva2_decoder_sample::QueryInterface(REFIID riid, void **ppv)
{
	CheckPointer(ppv, E_POINTER);

	if (riid == IID_IMFGetService)
	{
		*ppv = static_cast<IMFGetService*>(this);
		AddRef();
		return S_OK;
	}
	else
	{
		return CMediaSample::QueryInterface(riid, ppv);
	}
}

STDMETHODIMP_(ULONG) dxva2_decoder_sample::AddRef()
{
	return CMediaSample::AddRef();
}

STDMETHODIMP_(ULONG) dxva2_decoder_sample::Release()
{
	// Return a temporary variable for thread safety.
	ULONG cRef = CMediaSample::Release();
	return cRef;
}

// IMFGetService::GetService
STDMETHODIMP dxva2_decoder_sample::GetService(REFGUID guidService, REFIID riid, LPVOID *ppv)
{
	if (guidService != MR_BUFFER_SERVICE)
	{
		return MF_E_UNSUPPORTED_SERVICE;
	}
	else if (_d3d9_surface == NULL)
	{
		return E_NOINTERFACE;
	}
	else
	{
		return _d3d9_surface->QueryInterface(riid, ppv);
	}
}

// Override GetPointer because this class does not manage a system memory buffer.
// The EVR uses the MR_BUFFER_SERVICE service to get the Direct3D surface.
STDMETHODIMP dxva2_decoder_sample::GetPointer(BYTE ** ppBuffer)
{
	return E_NOTIMPL;
}

// Sets the pointer to the Direct3D surface. 
void dxva2_decoder_sample::set_surface(DWORD surface_id, IDirect3DSurface9 * d3d9_surface)
{
	safe_release(&_d3d9_surface);
	_d3d9_surface = d3d9_surface;
	if (_d3d9_surface)
	{
		_d3d9_surface->AddRef();
	}
	_surface_id = surface_id;
}

template <class T>
void dxva2_decoder_sample::safe_release(T ** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}