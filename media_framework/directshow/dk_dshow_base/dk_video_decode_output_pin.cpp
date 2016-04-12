#include <dshow.h>
#include <streams.h>
#include <dvdmedia.h>
#include <ks.h>
#include <ksmedia.h>

#include "dk_video_decode_filter.h"
#include "dk_video_decode_output_pin.h"

dk_video_decode_output_pin::dk_video_decode_output_pin(TCHAR * objname, IBaseFilter * filter, HRESULT * hr, LPCWSTR name)
	: CTransformOutputPin(objname, (CTransformFilter *)filter, hr, name)
{
}

dk_video_decode_output_pin::~dk_video_decode_output_pin()
{
}

HRESULT dk_video_decode_output_pin::InitAllocator(IMemAllocator ** allocator)
{
	HRESULT hr = S_FALSE;

	dk_video_decode_filter * filter = static_cast<dk_video_decode_filter*>(m_pTransformFilter);
	hr = filter->init_allocator(allocator);
	if (hr != S_OK)
		hr = CTransformOutputPin::InitAllocator(allocator);

	return hr;
}
