#pragma once
#include <strsafe.h>
#include <process.h>
#include <time.h>

#define g_szFilterName L"DK ScreenCapture Filter"

// {DF7C90A9-C202-4506-A4EE-0656C0C02923}
DEFINE_GUID(CLSID_DKScreenCaptureFilter, 
0xdf7c90a9, 0xc202, 0x4506, 0xa4, 0xee, 0x6, 0x56, 0xc0, 0xc0, 0x29, 0x23);

class dk_screencapture_filter_stream;
class dk_screencapture_filter : public CSource,
								public IAMFilterMiscFlags // CSource is CBaseFilter is IBaseFilter is IMediaFilter is IPersist which is IUnknown
{
private:
    dk_screencapture_filter( IUnknown *pUnk, HRESULT *phr );
    ~dk_screencapture_filter();

	dk_screencapture_filter_stream	*m_pPin;
public:
    //////////////////////////////////////////////////////////////////////////
    //  IUnknown
    //////////////////////////////////////////////////////////////////////////
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    STDMETHODIMP			QueryInterface(REFIID riid, void **ppv);

	// ?? compiler error that these be required here? huh?
	ULONG STDMETHODCALLTYPE AddRef() { return CBaseFilter::AddRef(); };
	ULONG STDMETHODCALLTYPE Release() { return CBaseFilter::Release(); };
	
	ULONG STDMETHODCALLTYPE GetMiscFlags() { return AM_FILTER_MISC_FLAGS_IS_SOURCE; } 
	
	// our own method
    IFilterGraph*			GetGraph() {return m_pGraph;}

	// CBaseFilter, some pdf told me I should (msdn agrees)
	STDMETHODIMP			GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State);
	STDMETHODIMP			Stop(); //http://social.msdn.microsoft.com/Forums/en/windowsdirectshowdevelopment/thread/a9e62057-f23b-4ce7-874a-6dd7abc7dbf7
};
