#include "marker.h"

//////////////////////
// CMarker class
// Holds information from IMFStreamSink::PlaceMarker
//
debuggerking::marker::marker(MFSTREAMSINK_MARKER_TYPE eMarkerType)
	: _ref_count(1)
	, _marker_type(eMarkerType)
{
	PropVariantInit(&_marker_value);
	PropVariantInit(&_marker_context_value);
}

debuggerking::marker::~marker(void)
{
    assert(_ref_count == 0);
    PropVariantClear(&_marker_value);
	PropVariantClear(&_marker_context_value);
}

/* static */
HRESULT debuggerking::marker::create(MFSTREAMSINK_MARKER_TYPE marker_type, const PROPVARIANT * marker_value, const PROPVARIANT * marker_context_value, IMarker ** ppmarker)
{
	if (ppmarker == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
	debuggerking::marker * pmarker = new debuggerking::marker(marker_type);
	if (pmarker == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    // Copy the marker data.
    if (SUCCEEDED(hr))
    {
        if (marker_value)
            hr = PropVariantCopy(&pmarker->_marker_value, marker_value);
    }

    if (SUCCEEDED(hr))
    {
        if (marker_context_value)
        {
			hr = PropVariantCopy(&pmarker->_marker_context_value, marker_context_value);
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppmarker = pmarker;
		(*ppmarker)->AddRef();
    }

    safe_release(pmarker);
    return hr;
}

// IUnknown methods.
ULONG debuggerking::marker::AddRef(void)
{
    return InterlockedIncrement(&_ref_count);
}

ULONG debuggerking::marker::Release(void)
{
    ULONG count = InterlockedDecrement(&_ref_count);
    if (count == 0)
        delete this;
    
	// For thread safety, return a temporary variable.
    return count;
}

HRESULT debuggerking::marker::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void ** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(IMarker))
    {
        *ppv = static_cast<IMarker*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

// IMarker methods
HRESULT debuggerking::marker::GetMarkerType(MFSTREAMSINK_MARKER_TYPE * type)
{
	if (type == NULL)
        return E_POINTER;

    *type = _marker_type;
    return S_OK;
}

HRESULT debuggerking::marker::GetMarkerValue(PROPVARIANT * pvar)
{
    if (pvar == NULL)
    {
        return E_POINTER;
    }
    return PropVariantCopy(pvar, &_marker_value);

}
HRESULT debuggerking::marker::GetContext(PROPVARIANT * pvar)
{
    if (pvar == NULL)
    {
        return E_POINTER;
    }
    return PropVariantCopy(pvar, &_marker_context_value);
}
