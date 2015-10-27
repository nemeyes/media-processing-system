#include <streams.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <evr.h>
#include "dk_dxva2_sample.h"
#include "dk_dxva2_allocator.h"


dk_dxva2_allocator::dk_dxva2_allocator(dk_video_decode_filter * filter, HRESULT * hr)
	: CBaseAllocator(NAME("dk_dxva2_allocator"), nullptr, hr)
	, _filter(filter)
{

}

dk_dxva2_allocator::~dk_dxva2_allocator(void)
{

}

// IUnknown
STDMETHODIMP dk_dxva2_allocator::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return CBaseAllocator::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT dk_dxva2_allocator::Alloc(void)
{
	CAutoLock lock(this);

	HRESULT hr = S_OK;

	if (_filter->_dxva2_decoder_service == NULL)
	{
		return E_UNEXPECTED;
	}

	hr = CBaseAllocator::Alloc();

	// If the requirements have not changed, do not reallocate.
	if (hr == S_FALSE)
	{
		return S_OK;
	}

	if (SUCCEEDED(hr))
	{
		// Free the old resources.
		Free();

		// Allocate a new array of pointers.
		_rt_surface_array = new IDirect3DSurface9*[m_lCount];
		if (_rt_surface_array == NULL)
		{
			hr = E_OUTOFMEMORY;
		}
		else
		{
			ZeroMemory(_rt_surface_array, sizeof(IDirect3DSurface9*) * m_lCount);
		}
	}

	// Allocate the surfaces.
	if (SUCCEEDED(hr))
	{
		hr = _filter->_dxva2_decoder_service->CreateSurface(_filter->_width,
															_filter->_height,
															m_lCount - 1,
															(D3DFORMAT)_filter->_format,
															D3DPOOL_DEFAULT,
															0,
															DXVA2_VideoDecoderRenderTarget,
															_rt_surface_array,
															NULL);
	}

	if (SUCCEEDED(hr))
	{
		for (m_lAllocated = 0; m_lAllocated < m_lCount; m_lAllocated++)
		{
			dk_dxva2_sample * pSample = new dk_dxva2_sample(this, &hr);

			if (pSample == NULL)
			{
				hr = E_OUTOFMEMORY;
				break;
			}
			if (FAILED(hr))
			{
				break;
			}
			// Assign the Direct3D surface pointer and the index.
			pSample->set_surface(m_lAllocated, _rt_surface_array[m_lAllocated]);
			// Add to the sample list.
			m_lFree.Add(pSample);
		}
	}

	if (SUCCEEDED(hr))
	{
		m_bChanged = FALSE;
	}
	return hr;
}

void dk_dxva2_allocator::Free()
{
	CMediaSample *pSample = NULL;
	do
	{
		pSample = m_lFree.RemoveHead();
		if (pSample)
		{
			delete pSample;
		}
	} while (pSample);

	if (_rt_surface_array)
	{
		for (long i = 0; i < m_lAllocated; i++)
		{
			safe_release(&_rt_surface_array[i]);
		}

		delete[] _rt_surface_array;
	}
	m_lAllocated = 0;
}


template <class T>
void dk_dxva2_allocator::safe_release(T ** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}