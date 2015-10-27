#include <dshow.h>
#include <streams.h>
#include <dvdmedia.h>
#include <ks.h>
#include <ksmedia.h>
#include <mmintrin.h>
#include "dk_video_decode_filter.h"

#include <dk_dsutils.h>
#include <dk_video_decode_input_pin.h>
#include <dk_video_decode_output_pin.h>

dk_video_decode_filter::dk_video_decode_filter(TCHAR * name, LPUNKNOWN unk, REFCLSID clsid, HRESULT * hr)
	: CTransformFilter(name, unk, clsid)
	, _dxva2_decoder_service(nullptr)
	, _width(0)
	, _height(0)
	, _format(0)
{
	*hr = S_OK;
	m_pInput = new dk_video_decode_input_pin(TEXT("dk_video_decode_input_pin"), this, hr, L"Input");
	ASSERT(SUCCEEDED(*hr));

	m_pOutput = new dk_video_decode_output_pin(TEXT("dk_video_decode_output_pin"), this, hr, L"Output");
	ASSERT(SUCCEEDED(*hr));
}

dk_video_decode_filter::~dk_video_decode_filter(void)
{
	if (_dxva2_allocator)
	{
		safe_release(&_dxva2_allocator);
	}
}

/*HRESULT dk_video_decode_filter::Receive(IMediaSample* pIn)
{
#ifndef _WIN64
	// TODOX64 : fixme!
	_mm_empty(); // just for safety
#endif

	CAutoLock cAutoLock(&m_csReceive);

	HRESULT hr;

	AM_SAMPLE2_PROPERTIES* const pProps = m_pInput->SampleProps();
	if (pProps->dwStreamId != AM_STREAM_MEDIA)
		return m_pOutput->Deliver(pIn);

	AM_MEDIA_TYPE* pmt;
	if (SUCCEEDED(pIn->GetMediaType(&pmt)) && pmt)
	{
		CMediaType mt(*pmt);
		m_pInput->SetMediaType(&mt);
		DeleteMediaType(pmt);
	}

	if (FAILED(hr = Transform(pIn)))
		return hr;

	return S_OK;
}*/

HRESULT dk_video_decode_filter::initialize_d3d(void)
{
	HRESULT hr = S_OK;
	//_d3d = Direct3DCreate9(D3D_SDK_VERSION);
	//if (!_d3d) 
	//{
	//	DbgLog((LOG_ERROR, 10, L"-> Failed to acquire IDirect3D9"));
	//	return E_FAIL;
	//}

	//UINT lAdapter = D3DADAPTER_DEFAULT;
	/*D3DADAPTER_IDENTIFIER9 d3dai = { 0 };

retry_default:
	hr = _d3d->GetAdapterIdentifier(lAdapter, 0, &d3dai);
	if (FAILED(hr)) {
		// retry if the adapter is invalid
		if (lAdapter != D3DADAPTER_DEFAULT) {
			lAdapter = D3DADAPTER_DEFAULT;
			goto retry_default;
		}
		DbgLog((LOG_TRACE, 10, L"-> Querying of adapter identifier failed with hr: %X", hr));
		return E_FAIL;
	}

	const char *vendor = "Unknown";
	for (int i = 0; vendors[i].id != 0; i++) {
		if (vendors[i].id == d3dai.VendorId) {
			vendor = vendors[i].name;
			break;
		}
	}*/

	//D3DPRESENT_PARAMETERS d3dpp;
	//D3DDISPLAYMODE d3ddm;

	//ZeroMemory(&d3dpp, sizeof(d3dpp));
	//_d3d->GetAdapterDisplayMode(lAdapter, &d3ddm);

	//d3dpp.Windowed = TRUE;
	//d3dpp.BackBufferWidth = 640;
	//d3dpp.BackBufferHeight = 480;
	//d3dpp.BackBufferCount = 0;
	//d3dpp.BackBufferFormat = d3ddm.Format;
	//d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	//d3dpp.Flags = D3DPRESENTFLAG_VIDEO;

	//hr = _d3d->CreateDevice(lAdapter, D3DDEVTYPE_HAL, GetShellWindow(), D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &d3dpp, &_d3d_device);
	//if (FAILED(hr)) {
	//	DbgLog((LOG_TRACE, 10, L"-> Creation of device failed with hr: %X", hr));
	//	return E_FAIL;
	//}

	//hr = CreateD3DDeviceManager(_d3d_device, &_d3d_reset_token, &_d3d_device_manager);
	//if (FAILED(hr)) {
	//	DbgLog((LOG_TRACE, 10, L"-> Creation of Device manager failed with hr: %X", hr));
	//	return E_FAIL;
	//}

	return hr;
}

STDMETHODIMP dk_video_decode_filter::init_allocator(IMemAllocator ** allocator)
{
	HRESULT hr = S_OK;

	_dxva2_allocator = new dk_dxva2_allocator(this, &hr);
	if (!_dxva2_allocator)
		return E_OUTOFMEMORY;
	
	if (FAILED(hr)) 
	{
		safe_release(&_dxva2_allocator);
		return hr;
	}

	return _dxva2_allocator->QueryInterface(__uuidof(IMemAllocator), (void **)allocator);
}
