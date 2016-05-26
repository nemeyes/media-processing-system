#pragma once

#include "common.h"
#include "MFAttributesImpl.h"
#include "media_sink.h"

namespace debuggerking
{
	class activate : public CMFAttributesImpl<IMFActivate>, public IPersistStream, public IGPUHandler, private mf_base
	{
	public:

		static HRESULT CreateInstance(HWND hwnd, IMFActivate** ppActivate);

		// IUnknown
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP QueryInterface(REFIID riid, __RPC__deref_out _Result_nullonfailure_ void** ppvObject);
		STDMETHODIMP_(ULONG) Release(void);

		// IMFActivate
		STDMETHODIMP ActivateObject(__RPC__in REFIID riid, __RPC__deref_out_opt void** ppvObject);
		STDMETHODIMP DetachObject(void);
		STDMETHODIMP ShutdownObject(void);

		// IPersistStream
		STDMETHODIMP GetSizeMax(__RPC__out ULARGE_INTEGER* pcbSize);
		STDMETHODIMP IsDirty(void);
		STDMETHODIMP Load(__RPC__in_opt IStream* pStream);
		STDMETHODIMP Save(__RPC__in_opt IStream* pStream, BOOL bClearDirty);

		// IPersist (from IPersistStream)
		STDMETHODIMP GetClassID(__RPC__out CLSID* pClassID);

		// ICAPGPUSelector
		STDMETHODIMP SetGPUIndex(UINT index);
		STDMETHODIMP EnablePresent(BOOL enable);

	private:
		activate(void);
		~activate(void);

		long _ref_count;
		UINT _gpu_index;
		BOOL _enable_present;

		IMFMediaSink * _media_sink;
		HWND _hwnd;
	};
}
