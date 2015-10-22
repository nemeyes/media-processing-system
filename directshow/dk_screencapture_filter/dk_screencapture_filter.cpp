#include <tchar.h>
#include <dshow.h>
#include <initguid.h> 
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <atlbase.h>
#include <string.h>
#include <stdlib.h>
#include <streams.h>
#include <dvdmedia.h>

#include "dk_screencapture_filter_stream.h"
#include "dk_screencapture_filter.h"
#include "dib_helper.h"

dk_screencapture_filter::dk_screencapture_filter( IUnknown *pUnk, HRESULT *phr )
	: CSource(g_szFilterName, pUnk, CLSID_DKScreenCaptureFilter)
{
    // The pin magically adds itself to our pin array.
	// except its not an array since we just have one [?]
    m_pPin = new dk_screencapture_filter_stream( phr, this );

	if (phr)
	{
		if (m_pPin == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}  
}


dk_screencapture_filter::~dk_screencapture_filter() // parent destructor
{
	// COM should call this when the refcount hits 0...
	// but somebody should make the refcount 0...
    delete m_pPin;
}


CUnknown* WINAPI dk_screencapture_filter::CreateInstance( IUnknown *pUnk, HRESULT *phr )
{
	// the first entry point
    dk_screencapture_filter *pNewFilter = new dk_screencapture_filter( pUnk, phr) ;

	if( phr )
	{
		if( pNewFilter==NULL ) 
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}
    return pNewFilter;
}

HRESULT dk_screencapture_filter::QueryInterface( REFIID riid, void **ppv )
{
    //Forward request for IAMStreamConfig & IKsPropertySet to the pin
    if(riid == _uuidof(IAMStreamConfig) || riid == _uuidof(IKsPropertySet)) {
        return m_paStreams[0]->QueryInterface(riid, ppv);
	}
    else {
        return CSource::QueryInterface(riid, ppv);
	}

}

STDMETHODIMP dk_screencapture_filter::Stop()
{

	CAutoLock filterLock(m_pLock);

	//Default implementation
	HRESULT hr = CBaseFilter::Stop();

	//Reset pin resources
	m_pPin->m_iFrameNumber = 0;

	return hr;
}


// according to msdn...
HRESULT dk_screencapture_filter::GetState(DWORD dw, FILTER_STATE *pState)
{
    CheckPointer(pState, E_POINTER);
    *pState = m_State;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE;
    else
        return S_OK;
}
