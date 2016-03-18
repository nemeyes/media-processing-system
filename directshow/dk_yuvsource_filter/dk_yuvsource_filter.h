#pragma once
#pragma once

#include <process.h>
#include <time.h>
#include <dk_basic_type.h>
#include <dk_media_source.h>

#define g_szFilterName L"DK YUV Source filter"

// {0B46BC4E-729E-4496-A7E0-82353D1B5FC0}
DEFINE_GUID(CLSID_DK_YUVSOURCE_FILTER,
	0xb46bc4e, 0x729e, 0x4496, 0xa7, 0xe0, 0x82, 0x35, 0x3d, 0x1b, 0x5f, 0xc0);

// {4B3D448B-0152-4147-BDFA-11C18E4E763E}
DEFINE_GUID(CLSID_DK_YUVSOURCE_FILTER_PROPERTIES,
	0x4b3d448b, 0x152, 0x4147, 0xbd, 0xfa, 0x11, 0xc1, 0x8e, 0x4e, 0x76, 0x3e);

class dk_yuvsource_filter : public CSource,
							public ISpecifyPropertyPages,
							public IYUVSource
{
public:
	static CUnknown * WINAPI CreateInstance(IUnknown * unk, HRESULT * hr);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP GetPages(CAUUID *pages);

	STDMETHODIMP SetFilePath(BSTR filepath);
	STDMETHODIMP SetWidth(UINT width);
	STDMETHODIMP SetHeight(UINT height);
	STDMETHODIMP SetFPS(UINT fps);

private:
	dk_yuvsource_filter(IUnknown * unk, HRESULT * hr);
	~dk_yuvsource_filter(void);
	DISALLOW_IMPLICIT_CONSTRUCTORS(dk_yuvsource_filter);
};