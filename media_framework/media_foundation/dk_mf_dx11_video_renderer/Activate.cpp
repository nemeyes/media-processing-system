#include "Activate.h"
#include <atlbase.h>
#include <dk_gpu_handler.h>

HRESULT debuggerking::activate::CreateInstance(HWND hwnd, IMFActivate ** ppactivate)
{
	if (ppactivate == NULL)
	{
		return E_POINTER;
	}

	activate * act = new activate();
	if (act == NULL)
	{
		return E_OUTOFMEMORY;
	}

	act->AddRef();

	HRESULT hr = S_OK;

	do
	{
		hr = act->Initialize();
		if (FAILED(hr))
		{
			break;
		}

		hr = act->QueryInterface(IID_PPV_ARGS(ppactivate));
		if (FAILED(hr))
		{
			break;
		}

		act->_hwnd = hwnd;
	} while (FALSE);

	safe_release(act);
	return hr;
}

// IUnknown
ULONG debuggerking::activate::AddRef(void)
{
	return InterlockedIncrement(&_ref_count);
}

// IUnknown
HRESULT debuggerking::activate::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void ** ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}
	if (iid == IID_IUnknown)
	{
		*ppv = static_cast<IUnknown*>(static_cast<IMFActivate*>(this));
	}
	else if (iid == __uuidof(IMFActivate))
	{
		*ppv = static_cast<IMFActivate*>(this);
	}
	else if (iid == __uuidof(IPersistStream))
	{
		*ppv = static_cast<IPersistStream*>(this);
	}
	else if (iid == __uuidof(IPersist))
	{
		*ppv = static_cast<IPersist*>(this);
	}
	else if (iid == __uuidof(IMFAttributes))
	{
		*ppv = static_cast<IMFAttributes*>(this);
	}
	else if (iid == __uuidof(IGPUHandler))
	{
		*ppv = static_cast<IGPUHandler*>(this);
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
ULONG debuggerking::activate::Release(void)
{
	ULONG lRefCount = InterlockedDecrement(&_ref_count);
	if (lRefCount == 0)
	{
		delete this;
	}
	return lRefCount;
}

// IMFActivate
HRESULT debuggerking::activate::ActivateObject(__RPC__in REFIID riid, __RPC__deref_out_opt void ** ppvobj)
{
	HRESULT hr = S_OK;
	ATL::CComPtr<IMFGetService> sink_get_service;
	ATL::CComPtr<IMFVideoDisplayControl> sink_video_Display_control;

	do
	{
		if (_media_sink == NULL)
		{
			hr = media_sink::CreateInstance(IID_PPV_ARGS(&_media_sink));
			if (FAILED(hr))
			{
				break;
			}

			ATL::CComQIPtr<IGPUHandler> gpu_selector(_media_sink);
			if (gpu_selector == NULL)
				break;
			hr = gpu_selector->SetGPUIndex(_gpu_index);
			if (FAILED(hr))
				break;

			hr = gpu_selector->EnablePresent(_enable_present);
			if (FAILED(hr))
				break;

			hr = _media_sink->QueryInterface(IID_PPV_ARGS(&sink_get_service));
			if (FAILED(hr))
			{
				break;
			}

			hr = sink_get_service->GetService(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&sink_video_Display_control));
			if (FAILED(hr))
			{
				break;
			}

			hr = sink_video_Display_control->SetVideoWindow(_hwnd);
			if (FAILED(hr))
			{
				break;
			}
		}

		hr = _media_sink->QueryInterface(riid, ppvobj);
		if (FAILED(hr))
		{
			break;
		}
	} while (FALSE);
	return hr;
}

// IMFActivate
HRESULT debuggerking::activate::DetachObject(void)
{
	safe_release(_media_sink);
	return S_OK;
}

// IMFActivate
HRESULT debuggerking::activate::ShutdownObject(void)
{
	if (_media_sink != NULL)
	{
		_media_sink->Shutdown();
		safe_release(_media_sink);
	}
	return S_OK;
}

// IPersistStream
HRESULT debuggerking::activate::GetSizeMax(__RPC__out ULARGE_INTEGER* pcbSize)
{
	return E_NOTIMPL;
}

// IPersistStream
HRESULT debuggerking::activate::IsDirty(void)
{
	return E_NOTIMPL;
}

// IPersistStream
HRESULT debuggerking::activate::Load(__RPC__in_opt IStream * stream)
{
	return E_NOTIMPL;
}

// IPersistStream
HRESULT debuggerking::activate::Save(__RPC__in_opt IStream * stream, BOOL bcleardirty)
{
	return E_NOTIMPL;
}

// IPersist
HRESULT debuggerking::activate::GetClassID(__RPC__out CLSID * clsid)
{
	if (clsid == NULL)
	{
		return E_POINTER;
	}
	*clsid = CLSID_D3D11VideoRendererActivate;
	return S_OK;
}


// IGPUHandler
HRESULT debuggerking::activate::SetGPUIndex(UINT index)
{
	_gpu_index = index;
	return NOERROR;
}

STDMETHODIMP debuggerking::activate::EnablePresent(BOOL enable)
{
	_enable_present = enable;
	return NOERROR;
}

// ctor
debuggerking::activate::activate(void)
	: _ref_count(0)
	, _media_sink(NULL)
	, _hwnd(NULL)
{}

// dtor
debuggerking::activate::~activate(void)
{
	safe_release(_media_sink);
}
