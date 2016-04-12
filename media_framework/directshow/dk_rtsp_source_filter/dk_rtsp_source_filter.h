#pragma once

#include <process.h>
#include <time.h>

#define E_SEEK (0x80008888L)
#define g_sz_filter_name L"DK rtsp source filter"

// {02C772F7-7F7A-48CA-819F-B6D74F072E49}
DEFINE_GUID(CLSID_DK_RTSP_SOURCE_FILTER,
	0x2c772f7, 0x7f7a, 0x48ca, 0x81, 0x9f, 0xb6, 0xd7, 0x4f, 0x7, 0x2e, 0x49);

// {2AFF77BE-DDAA-4D30-AF2A-93717B1EB354}
DEFINE_GUID(CLSID_DK_RTSP_SOURCE_FILTER_PROPERTIES,
	0x2aff77be, 0xddaa, 0x4d30, 0xaf, 0x2a, 0x93, 0x71, 0x7b, 0x1e, 0xb3, 0x54);

class dk_rtsp_source_filter : public CSource,
								public IFileSourceFilter,
								public IAMFilterMiscFlags,
								public ISpecifyPropertyPages
{
	friend class dk_rtsp_source_stream;

public:


};
