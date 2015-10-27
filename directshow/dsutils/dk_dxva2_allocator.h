#pragma once

#include "dk_video_decode_filter.h"

class dk_dxva2_allocator : public CBaseAllocator
{
	friend class dk_video_decode_filter;
public:
	dk_dxva2_allocator(dk_video_decode_filter * filter, HRESULT * hr);
	virtual ~dk_dxva2_allocator(void);

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	HRESULT Alloc(void);
	void Free(void);

private:
	template <class T> void safe_release(T ** ppT);

private:
	dk_video_decode_filter * _filter;
	IDirect3DSurface9 ** _rt_surface_array;
	UINT _count_surface_array;
};

