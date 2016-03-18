#pragma once
#pragma once

#include <process.h>
#include <time.h>
#include <dk_basic_type.h>

#define g_szFilterName L"DK Image Source filter"

// {47EB2952-FBF7-4D27-BEA9-8BE6A44BBAA2}
DEFINE_GUID(CLSID_DK_IMAGE_SOURCE_FILTER,
	0x47eb2952, 0xfbf7, 0x4d27, 0xbe, 0xa9, 0x8b, 0xe6, 0xa4, 0x4b, 0xba, 0xa2);

// {7C267847-A5BE-47ED-B270-D5BA4093F01B}
DEFINE_GUID(CLSID_DK_IMAGE_SOURCE_FILTER_PROPERTIES,
	0x7c267847, 0xa5be, 0x47ed, 0xb2, 0x70, 0xd5, 0xba, 0x40, 0x93, 0xf0, 0x1b);

class dk_image_source_filter : public CSource,
							   public ISpecifyPropertyPages
{
public:
	static CUnknown * WINAPI CreateInstance(IUnknown * unk, HRESULT * hr);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP GetPages(CAUUID *pages);


private:
	dk_image_source_filter(IUnknown * unk, HRESULT * hr);
	~dk_image_source_filter(void);
	DISALLOW_IMPLICIT_CONSTRUCTORS(dk_image_source_filter);
};