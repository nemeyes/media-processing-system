#include <windows.h>
#include <tchar.h>
#include <dshow.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <atlbase.h>
#include <string.h>
#include <stdlib.h>
#include <streams.h>
#include <dvdmedia.h>

//#include <wmcodecdsp.h>
//#include <d3d9types.h>

//#include <Objbase.h>
//#include <shlguid.h> 

#include "dk_dxva2_decode_filter_properties.h"
#include "dk_dxva2_decode_filter.h"

dk_dxva2_decode_filter::dk_dxva2_decode_filter(LPUNKNOWN unk, HRESULT *hr)
	: CTransformFilter(g_szFilterName, unk, CLSID_DK_DXVA2_DECODE_FILTER)
{
	
}

dk_dxva2_decode_filter::~dk_dxva2_decode_filter(VOID)
{

}

CUnknown * WINAPI dk_dxva2_decode_filter::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	*hr = S_OK;
	CUnknown* punk = new dk_dxva2_decode_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	return punk;
}

STDMETHODIMP dk_dxva2_decode_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
	}
	else
	{
		return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}


// override these two functions if you want to inform something
// about entry to or exit from streaming state.
HRESULT  dk_dxva2_decode_filter::StartStreaming()
{
	return NOERROR;
}

HRESULT  dk_dxva2_decode_filter::StopStreaming()
{
	return NOERROR;
}

// override this to grab extra interfaces on connection
HRESULT  dk_dxva2_decode_filter::CheckConnect(PIN_DIRECTION direction, IPin *pin)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

// place holder to allow derived classes to release any extra interfaces
HRESULT  dk_dxva2_decode_filter::BreakConnect(PIN_DIRECTION direction)
{
	if (direction == PINDIR_INPUT)
	{
		//if (_decoder)
		//	_decoder->release();
	}
	UNREFERENCED_PARAMETER(direction);
	return NOERROR;
}

// Let derived classes know about connection completion
HRESULT  dk_dxva2_decode_filter::CompleteConnect(PIN_DIRECTION direction, IPin *pin)
{
	if (direction == PINDIR_INPUT)
	{
		configure_dxva2(pin);
		//if (_decoder)
		//	_decoder->initialize(_width, _height);
	}

	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

HRESULT  dk_dxva2_decode_filter::AlterQuality(Quality quality)
{
	UNREFERENCED_PARAMETER(quality);
	return S_FALSE;
}

HRESULT  dk_dxva2_decode_filter::SetMediaType(PIN_DIRECTION direction, const CMediaType *type)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(type);
	return NOERROR;
}

// EndOfStream received. Default behaviour is to deliver straight
// downstream, since we have no queued data. If you overrode Receive
// and have queue data, then you need to handle this and deliver EOS after
// all queued data is sent
HRESULT  dk_dxva2_decode_filter::EndOfStream(void)
{
	HRESULT hr = NOERROR;
	if (m_pOutput != NULL)
	{
		hr = m_pOutput->DeliverEndOfStream();
	}
	return hr;
}

// enter flush state. Receives already blocked
// must override this if you have queued data or a worker thread
HRESULT  dk_dxva2_decode_filter::BeginFlush(void)
{
	HRESULT hr = NOERROR;
	if (m_pOutput != NULL)
	{
		// block receives -- done by caller (CBaseInputPin::BeginFlush)
		// discard queued data -- we have no queued data
		// free anyone blocked on receive - not possible in this filter
		// call downstream
		hr = m_pOutput->DeliverBeginFlush();
	}
	return hr;
}

// leave flush state. must override this if you have queued data
// or a worker thread
HRESULT  dk_dxva2_decode_filter::EndFlush(void)
{
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins
	ASSERT(m_pOutput != NULL);
	return m_pOutput->DeliverEndFlush();
	// caller (the input pin's method) will unblock Receives
}


HRESULT  dk_dxva2_decode_filter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	if (m_pOutput != NULL)
	{
		return m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
	}
	return S_OK;
}

HRESULT  dk_dxva2_decode_filter::CheckInputType(const CMediaType *type)
{
	const GUID* id = type->Subtype();
	const char* media_type = (const char*)&type->Type()->Data1;
	const char* submedia_type = (const char*)&type->Subtype()->Data1;
	const GUID* formaType = type->FormatType();
	if (IsEqualGUID(*type->Type(), MEDIATYPE_Video))
	{
		if (IsEqualGUID(*type->Type(), MEDIATYPE_Video))
		{
			if (IsEqualGUID(*(type->Subtype()), MEDIASUBTYPE_H264))
			{
				if (IsEqualGUID(*(formaType), FORMAT_VideoInfo2))
				{
					VIDEOINFOHEADER2 * vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->Format());
					_width = vih2->bmiHeader.biWidth;
					_height = vih2->bmiHeader.biHeight;
					return S_OK;
				}
				else if (IsEqualGUID(*(formaType), FORMAT_VideoInfo))
				{
					VIDEOINFOHEADER * vih = reinterpret_cast<VIDEOINFOHEADER*>(type->Format());
					_width = vih->bmiHeader.biWidth;
					_height = vih->bmiHeader.biHeight;
					return S_OK;
				}
			}
		}
	}
	return E_FAIL;
}

HRESULT  dk_dxva2_decode_filter::GetMediaType(int position, CMediaType *type)
{
	if (!m_pInput->IsConnected())
		return E_UNEXPECTED;

	if (position<0)
		return E_INVALIDARG;
	if (position>0)
		return VFW_S_NO_MORE_ITEMS;

	type->SetType(&MEDIATYPE_Video);
	if (position == 0)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
			return hr;

#if 1

#if 1
		type->SetSubtype(&MEDIASUBTYPE_NV12);
		type->SetFormatType(&FORMAT_VideoInfo2);
		type->SetTemporalCompression(FALSE);
		type->SetSampleSize(_width*_height*1.5);

		VIDEOINFOHEADER2	*vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->pbFormat);//(VIDEOINFOHEADER2*)type->AllocFormatBuffer( sizeof(VIDEOINFOHEADER2) );
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->rcSource.left = 0;
		vih2->rcSource.top = 0;
		vih2->rcSource.right = _width;
		vih2->rcSource.bottom = _height;
		vih2->rcTarget.left = 0;
		vih2->rcTarget.top = 0;
		vih2->rcTarget.right = _width;
		vih2->rcTarget.bottom = _height;
		vih2->dwPictAspectRatioX = _width;
		vih2->dwPictAspectRatioY = _height;
		vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih2->bmiHeader.biWidth = _width;
		vih2->bmiHeader.biHeight = _height;
		vih2->bmiHeader.biPlanes = 1;
		vih2->bmiHeader.biBitCount = 12;
		vih2->bmiHeader.biCompression = MAKEFOURCC('N', 'V', '1', '2');//0x32315659;
		vih2->bmiHeader.biSizeImage = _width*_height*1.5;
#else
		type->SetSubtype(&MEDIASUBTYPE_YV12);
		type->SetFormatType(&FORMAT_VideoInfo2);
		type->SetTemporalCompression(FALSE);
		type->SetSampleSize(_width*_height*1.5);

		VIDEOINFOHEADER2	*vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->pbFormat);//(VIDEOINFOHEADER2*)type->AllocFormatBuffer( sizeof(VIDEOINFOHEADER2) );
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->rcSource.left = 0;
		vih2->rcSource.top = 0;
		vih2->rcSource.right = _width;
		vih2->rcSource.bottom = _height;
		vih2->rcTarget.left = 0;
		vih2->rcTarget.top = 0;
		vih2->rcTarget.right = _width;
		vih2->rcTarget.bottom = _height;
		vih2->dwPictAspectRatioX = _width;
		vih2->dwPictAspectRatioY = _height;
		vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih2->bmiHeader.biWidth = _width;
		vih2->bmiHeader.biHeight = _height;
		vih2->bmiHeader.biPlanes = 1;
		vih2->bmiHeader.biBitCount = 12;
		vih2->bmiHeader.biCompression = MAKEFOURCC('Y', 'V', '1', '2');//0x32315659;
		vih2->bmiHeader.biSizeImage = _width*_height*1.5;
#endif
#else
		type->SetSubtype(&MEDIASUBTYPE_RGB32);
		type->SetFormatType(&FORMAT_VideoInfo2);
		type->SetTemporalCompression(FALSE);
		type->SetSampleSize(_width*_height * 4);

		VIDEOINFOHEADER2	*vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->pbFormat);//(VIDEOINFOHEADER2*)type->AllocFormatBuffer( sizeof(VIDEOINFOHEADER2) );
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->rcSource.left = 0;
		vih2->rcSource.top = 0;
		vih2->rcSource.right = _width;
		vih2->rcSource.bottom = _height;
		vih2->rcTarget.left = 0;
		vih2->rcTarget.top = 0;
		vih2->rcTarget.right = _width;
		vih2->rcTarget.bottom = _height;
		vih2->dwPictAspectRatioX = _width;
		vih2->dwPictAspectRatioY = _height;
		vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih2->bmiHeader.biWidth = _width;
		vih2->bmiHeader.biHeight = _height;
		vih2->bmiHeader.biPlanes = 1;
		vih2->bmiHeader.biBitCount = 32;
		vih2->bmiHeader.biCompression = BI_RGB;//MAKEFOURCC('N', 'V', '1', '2');//0x32315659;
		vih2->bmiHeader.biSizeImage = _width*_height * 4;
		vih2->bmiHeader.biClrImportant = 0;

#endif
	}
	return S_OK;
}

HRESULT  dk_dxva2_decode_filter::CheckTransform(const CMediaType *itype, const CMediaType *otype)
{
	if (!IsEqualGUID(*(itype->Type()), MEDIATYPE_Video) || !IsEqualGUID(*(otype->Type()), MEDIATYPE_Video))
		return E_FAIL;

	VIDEOINFOHEADER2 *ovi = (VIDEOINFOHEADER2 *)(otype->Format());
	if (!ovi)
		return E_FAIL;
	_stride = (UINT)ovi->bmiHeader.biWidth;

#if 1
#if 1
	if (!IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_NV12))
		return VFW_E_TYPE_NOT_ACCEPTED;
#else
	if (!IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_YV12))
		return VFW_E_TYPE_NOT_ACCEPTED;
#endif
#else
	if (!IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_RGB32))
		return VFW_E_TYPE_NOT_ACCEPTED;

#endif
	return S_OK;
}

HRESULT  dk_dxva2_decode_filter::DecideBufferSize(IMemAllocator * allocator, ALLOCATOR_PROPERTIES * properties)
{
	if (!m_pInput->IsConnected())
		return E_UNEXPECTED;

	DXVA2_ModeMPEG2_VLD;
	HRESULT hr;
	IMemAllocator* alloc;
	hr = m_pInput->GetAllocator(&alloc);

	if (FAILED(hr))
		return hr;

	ALLOCATOR_PROPERTIES request;
	hr = alloc->GetProperties(&request);
	alloc->Release();

	if (FAILED(hr))
		return hr;

	properties->cBuffers = 1;
	//properties->cbAlign		= 1;
	properties->cbBuffer = /*_width*_height * 4;*/_stride*_height * 1.5;//_owidth*_oheight*2;//_owidth*_oheight+_owidth*_oheight/2; m_pInput->CurrentMediaType().GetSampleSize();//
	properties->cbPrefix = 0;

	ALLOCATOR_PROPERTIES actual;
	hr = allocator->SetProperties(properties, &actual);

	if (FAILED(hr))
		return hr;

	if (properties->cBuffers>actual.cBuffers || properties->cbBuffer>actual.cbBuffer || properties->cbAlign>actual.cbAlign)
		return E_FAIL;

	return S_OK;
}

HRESULT dk_dxva2_decode_filter::Transform(IMediaSample *src, IMediaSample *dst)
{
	HRESULT hr = S_OK;
	BYTE *input_buffer = NULL;
	UINT input_data_size = 0;
	BYTE *output_buffer = NULL;
	UINT output_data_size = 0;

	hr = src->GetPointer(&input_buffer);
	if (FAILED(hr))
		return S_OK;
	if (!input_buffer)
		return S_OK;
	input_data_size = src->GetActualDataLength();
	if (input_data_size <= 0)
		return S_OK;
	hr = dst->GetPointer(&output_buffer);
	if (FAILED(hr))
		return S_OK;

	//TCHAR debug[100] = {0};
	//_sntprintf( debug, sizeof(debug), _T("input size : %d\n"), input_data_size );
	//OutputDebugString( debug );

	REFERENCE_TIME start_time, end_time;
	if (SUCCEEDED(src->GetTime(&start_time, &end_time)))
	{
		if (-1e7 != start_time)
		{
			start_time = (start_time<0) ? 0 : start_time;
		}
		//dst->SetTime( &start_time, &end_time );
	}
	//int result = _decoder->decode(input_buffer, input_data_size, _stride, output_buffer, output_data_size);

	//_sntprintf( debug, sizeof(debug), _T("output size : %d \n"), output_data_size );
	//OutputDebugString( debug );

	dst->SetActualDataLength(output_data_size);

	//if (result == VMXNET_STATUS_SUCCESS)
	{
		BOOL bSyncPoint;
		BOOL bPreroll;
		BOOL bDiscon;

		end_time = (REFERENCE_TIME)(start_time + (1.0 / 24) * 1e7);
		dst->SetTime(&start_time, &end_time);

		HRESULT hr = src->IsSyncPoint();
		if (hr == S_OK)
		{
			bSyncPoint = TRUE;
			dst->SetSyncPoint(TRUE);
		}
		else if (hr == S_FALSE)
		{
			bSyncPoint = FALSE;
			dst->SetSyncPoint(FALSE);
		}
		else
			return E_UNEXPECTED;

		dst->SetMediaType(NULL);
		hr = src->IsPreroll();
		if (hr == S_OK)
		{
			bPreroll = TRUE;
			dst->SetPreroll(TRUE);
		}
		else if (hr == S_FALSE)
		{
			bPreroll = FALSE;
			dst->SetPreroll(FALSE);
		}
		else
			return E_UNEXPECTED;

		hr = src->IsDiscontinuity();
		if (hr == S_OK)
		{
			bDiscon = TRUE;
			dst->SetDiscontinuity(TRUE);
		}
		else if (hr == S_FALSE)
		{
			bDiscon = FALSE;
			dst->SetDiscontinuity(FALSE);
		}
		else
			return E_UNEXPECTED;

		dst->SetPreroll(FALSE);
	}
	return hr;
}

STDMETHODIMP dk_dxva2_decode_filter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL)
		return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_DK_DXVA2_DECODE_FILTER_PROPERTIES;
	return S_OK;
}

HRESULT dk_dxva2_decode_filter::configure_dxva2(IPin * pin)
{
	UINT cnt_decoder_guids = 0;
	BOOL found_dxva2_configuration = FALSE;
	GUID guid_decoder = GUID_NULL;

	DXVA2_ConfigPictureDecode config;
	ZeroMemory(&config, sizeof(config));

	// Variables that follow must be cleaned up at the end.

	IMFGetService * mf_get_service = NULL;
	IDirect3DDeviceManager9 * d3d_device_manager = NULL;
	IDirectXVideoDecoderService * dxva2_decoder_service = NULL;

	GUID * decoder_guids = NULL; // size = cDecoderGuids
	HANDLE device = INVALID_HANDLE_VALUE;

	// Query the pin for IMFGetService.
	HRESULT hr = pin->QueryInterface(IID_PPV_ARGS(&mf_get_service));

	// Get the Direct3D device manager.
	if (SUCCEEDED(hr))
		hr = mf_get_service->GetService(MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&d3d_device_manager));

	// Open a new device handle.
	if (SUCCEEDED(hr))
		hr = d3d_device_manager->OpenDeviceHandle(&device);

	// Get the video decoder service.
	if (SUCCEEDED(hr))
		hr = d3d_device_manager->GetVideoService(device, IID_PPV_ARGS(&dxva2_decoder_service));

	// Get the decoder GUIDs.
	if (SUCCEEDED(hr))
		hr = dxva2_decoder_service->GetDecoderDeviceGuids(&cnt_decoder_guids, &decoder_guids);

	if (SUCCEEDED(hr))
	{
		// Look for the decoder GUIDs we want.
		for (UINT index = 0; index < cnt_decoder_guids; index++)
		{
			// Do we support this mode?
			if (!is_supported_decoder_mode(decoder_guids[index]))
				continue;

			// Find a configuration that we support. 
			hr = find_decoder_configuration(dxva2_decoder_service, decoder_guids[index], &config, &found_dxva2_configuration);
			if (FAILED(hr))
				break;

			if (found_dxva2_configuration)
			{
				// Found a good configuration. Save the GUID and exit the loop.
				guid_decoder = decoder_guids[index];
				break;
			}
		}
	}

	if (!found_dxva2_configuration)
	{
		hr = E_FAIL; // Unable to find a configuration.
	}

	if (SUCCEEDED(hr))
	{
		// Store the things we will need later.

		safe_release(&_dxva2_decoder_service);
		_dxva2_decoder_service = dxva2_decoder_service;
		_dxva2_decoder_service->AddRef();

		_dxva2_decoder_config = config;
		_dxva2_decoder_guid = guid_decoder;
		_dxva2_device = device;
	}

	if (FAILED(hr))
	{
		if (device != INVALID_HANDLE_VALUE)
		{
			d3d_device_manager->CloseDeviceHandle(device);
		}
	}

	safe_release(&mf_get_service);
	safe_release(&d3d_device_manager);
	safe_release(&dxva2_decoder_service);
	return hr;
}

HRESULT dk_dxva2_decode_filter::set_evr4dxva2(IPin * pin)
{
	HRESULT hr = S_OK;
	IMFGetService * mf_get_service = NULL;
	IDirectXVideoMemoryConfiguration * dx_video_config = NULL;

	// Query the pin for IMFGetService.
	hr = pin->QueryInterface(__uuidof(IMFGetService), (void**)&mf_get_service);

	// Get the IDirectXVideoMemoryConfiguration interface.
	if (SUCCEEDED(hr))
	{
		hr = mf_get_service->GetService(MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&dx_video_config));
	}

	// Notify the EVR. 
	if (SUCCEEDED(hr))
	{
		DXVA2_SurfaceType surface_type;
		for (DWORD iTypeIndex = 0;; iTypeIndex++)
		{
			hr = dx_video_config->GetAvailableSurfaceTypeByIndex(iTypeIndex, &surface_type);

			if (FAILED(hr))
			{
				break;
			}

			if (surface_type == DXVA2_SurfaceType_DecoderRenderTarget)
			{
				hr = dx_video_config->SetSurfaceType(DXVA2_SurfaceType_DecoderRenderTarget);
				break;
			}
		}
	}

	safe_release(&mf_get_service);
	safe_release(&dx_video_config);

	return hr;
}

HRESULT dk_dxva2_decode_filter::find_decoder_configuration(IDirectXVideoDecoderService * decoder_service, const GUID & guid_decoder, DXVA2_ConfigPictureDecode * selected_config, BOOL * found_dxva2_configuration)
{
	HRESULT hr = S_OK;
	UINT cnt_formats = 0;
	UINT cnt_configurations = 0;
	D3DFORMAT * d3d_formats = NULL;// size = cFormats
	DXVA2_ConfigPictureDecode * dxva2_config = NULL;// size = cConfigurations

	// Find the valid render target formats for this decoder GUID.
	hr = decoder_service->GetDecoderRenderTargets(guid_decoder, &cnt_formats, &d3d_formats);
	if (SUCCEEDED(hr))
	{
		// Look for a format that matches our output format.
		for (UINT index = 0; index < cnt_formats; index++)
		{
			if (d3d_formats[index] != (D3DFORMAT)_fcc_output_format)
				continue;

			// Fill in the video description. Set the width, height, format, 
			// and frame rate.
			DXVA2_VideoDesc dxva2_video_description = { 0 };

			fill_video_description(&dxva2_video_description); // Private helper function.
			dxva2_video_description.Format = d3d_formats[index];

			// Get the available configurations.
			hr = decoder_service->GetDecoderConfigurations(guid_decoder, &dxva2_video_description, NULL, &cnt_configurations, &dxva2_config);
			if (FAILED(hr))
				break;

			// Find a supported configuration.
			for (UINT index = 0; index < cnt_configurations; index++)
			{
				if (is_supported_decoder_config(dxva2_config[index]))
				{
					// This configuration is good.
					*found_dxva2_configuration = TRUE;
					*selected_config = dxva2_config[index];
					break;
				}
			}
			CoTaskMemFree(dxva2_config);
			break;

		} // End of formats loop.
	}
	CoTaskMemFree(d3d_formats);
	// Note: It is possible to return S_OK without finding a configuration.
	return hr;
}

// Returns TRUE if the decoder supports a given decoding mode.
BOOL dk_dxva2_decode_filter::is_supported_decoder_mode(const GUID & mode)
{
	return TRUE;
}

// Returns TRUE if the decoder supports a given decoding configuration.
BOOL dk_dxva2_decode_filter::is_supported_decoder_config(const DXVA2_ConfigPictureDecode & config)
{
	return TRUE;
}

// Fills in a DXVA2_VideoDesc structure based on the input format.
void dk_dxva2_decode_filter::fill_video_description(DXVA2_VideoDesc * description)
{

}

template <class T> 
void dk_dxva2_decode_filter::safe_release(T ** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}