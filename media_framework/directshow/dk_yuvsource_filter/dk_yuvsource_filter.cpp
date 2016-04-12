#include <streams.h>
#include <initguid.h> 
#include "dk_yuvsource_filter.h"
#include "dk_yuvsource_stream.h"

dk_yuvsource_filter::dk_yuvsource_filter(IUnknown * unk, HRESULT * hr)
	: CSource(g_szFilterName, unk, CLSID_DK_YUVSOURCE_FILTER)
	, _width(1280)
	, _height(720)
	, _fps(30)
{
	memset(_filepath, 0x00, sizeof(_filepath));
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
	else if (riid == IID_IFileSourceFilter)
	{
		return GetInterface(static_cast<IFileSourceFilter*>(this), ppv);
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

STDMETHODIMP dk_yuvsource_filter::GetCurFile(LPOLESTR * file_name, AM_MEDIA_TYPE *type)
{
	DWORD n = sizeof(WCHAR)*(1 + lstrlenW(_filepath));
	*file_name = (LPOLESTR)CoTaskMemAlloc(n);
	CopyMemory(*file_name, _filepath, n);
	return NOERROR;
}

STDMETHODIMP dk_yuvsource_filter::Load(LPCOLESTR file_name, const AM_MEDIA_TYPE *type)
{
	HRESULT hr = E_FAIL;
	if (wcslen(file_name) < 1)
		return hr;

	memset(_filepath, 0x00, sizeof(_filepath));
	wcscpy_s(_filepath, file_name);
	new dk_yuvsource_stream(&hr, this, _filepath, _width, _height, _fps);
	return S_OK;
}

STDMETHODIMP dk_yuvsource_filter::SetWidth(UINT width)
{
	_width = width;
	return S_OK;
}

STDMETHODIMP dk_yuvsource_filter::SetHeight(UINT height)
{
	_height = height;
	return S_OK;
}

STDMETHODIMP dk_yuvsource_filter::SetFPS(UINT fps)
{
	_fps = fps;
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