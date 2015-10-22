#include <dshow.h>
#include <streams.h>
#include <dvdmedia.h>
#include <ks.h>
#include <ksmedia.h>

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
	//hr = m_pFilter->m_Decoder.InitAllocator(allocator);
	if (hr != S_OK)
		hr = __super::InitAllocator(allocator);

	return hr;
}
