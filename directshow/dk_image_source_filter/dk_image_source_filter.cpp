#include <streams.h>
#include <initguid.h> 
#include "dk_image_source_filter.h"
#include "dk_image_source_stream.h"

dk_image_source_filter::dk_image_source_filter(IUnknown * unk, HRESULT * hr)
	: CSource(g_szFilterName, unk, CLSID_DK_IMAGE_SOURCE_FILTER)
{
	HRESULT hr1;
	new dk_image_source_stream(&hr1, this, TEXT("image.jpg"));
}


dk_image_source_filter::~dk_image_source_filter(void)
{
}


CUnknown * WINAPI dk_image_source_filter::CreateInstance(IUnknown * unk, HRESULT * hr)
{
	CUnknown * punk = new dk_image_source_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	else
		*hr = S_OK;
	return punk;
}

STDMETHODIMP dk_image_source_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
	}
	else
	{
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}
}

STDMETHODIMP dk_image_source_filter::GetPages(CAUUID * pages)
{
	if (pages == NULL)
		return E_POINTER;
	pages->cElems = 1;
	pages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pages->pElems == NULL)
		return E_OUTOFMEMORY;
	pages->pElems[0] = CLSID_DK_IMAGE_SOURCE_FILTER_PROPERTIES;
	return S_OK;
}