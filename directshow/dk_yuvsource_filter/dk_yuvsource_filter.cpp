#include <streams.h>
#include <initguid.h> 
#include "dk_yuvsource_filter.h"
#include "dk_yuvsource_stream.h"

dk_yuvsource_filter::dk_yuvsource_filter(IUnknown * unk, HRESULT * hr)
	: CSource(g_szFilterName, unk, CLSID_DK_YUVSOURCE_FILTER)
{
	HRESULT hr1;
	new dk_yuvsource_stream(&hr1, this, TEXT("input.yuv"), 1280, 720, 30);
}

dk_yuvsource_filter::~dk_yuvsource_filter(void)
{
}


CUnknown * WINAPI dk_yuvsource_filter::CreateInstance(IUnknown * unk, HRESULT * hr)
{
	CUnknown * punk = new dk_yuvsource_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	else
		*hr = S_OK;
	return punk;
}

STDMETHODIMP dk_yuvsource_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
	}
	else if (riid == IID_IYUVSource)
	{
		return GetInterface(static_cast<IYUVSource*>(this), ppv);
	}
	else
	{
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}
}


STDMETHODIMP dk_yuvsource_filter::SetFilePath(BSTR filepath)
{
	return S_OK;
}

STDMETHODIMP dk_yuvsource_filter::SetWidth(UINT width)
{
	return S_OK;
}

STDMETHODIMP dk_yuvsource_filter::SetHeight(UINT height)
{
	return S_OK;
}

STDMETHODIMP dk_yuvsource_filter::SetFPS(UINT fps)
{
	return S_OK;
}

STDMETHODIMP dk_yuvsource_filter::GetPages(CAUUID * pages)
{
	if (pages == NULL)
		return E_POINTER;
	pages->cElems = 1;
	pages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pages->pElems == NULL)
		return E_OUTOFMEMORY;
	pages->pElems[0] = CLSID_DK_YUVSOURCE_FILTER_PROPERTIES;
	return S_OK;
}