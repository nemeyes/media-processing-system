#include <streams.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <evr.h>
#include "dxva2_decoder_sample.h"
#include "dxva2_decoder_allocator.h"


dxva2_decoder_allocator::dxva2_decoder_allocator(IDirectXVideoDecoderService * dxva2_decoder_service, DWORD width, DWORD height, DWORD format, HRESULT * hr)
	: CBaseAllocator(NAME("dxva2_decoder_allocator"), nullptr, hr)
	, _dxva2_decoder_service(dxva2_decoder_service)
	, _width(width)
	, _height(height)
	, _format(format)
{
}

dxva2_decoder_allocator::~dxva2_decoder_allocator(void)
{
	//if (m_pDec && m_pDec->m_pDXVA2Allocator == this)
	//	m_pDec->m_pDXVA2Allocator = nullptr;
}

// IUnknown
STDMETHODIMP dxva2_decoder_allocator::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return CBaseAllocator::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT dxva2_decoder_allocator::Alloc(void)
{
	CAutoLock lock(this);

	HRESULT hr = S_OK;

	if (_dxva2_decoder_service == NULL)
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
		hr = _dxva2_decoder_service->CreateSurface(
			_width,
			_height,
			m_lCount - 1,
			(D3DFORMAT)_format,
			D3DPOOL_DEFAULT,
			0,
			DXVA2_VideoDecoderRenderTarget,
			_rt_surface_array,
			NULL
			);
	}

	if (SUCCEEDED(hr))
	{
		for (m_lAllocated = 0; m_lAllocated < m_lCount; m_lAllocated++)
		{
			dxva2_decoder_sample *pSample = new dxva2_decoder_sample(this, &hr);

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

void dxva2_decoder_allocator::Free()
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
void dxva2_decoder_allocator::safe_release(T ** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}