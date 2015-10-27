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

#include <dk_dsutils.h>
#include <dk_video_decode_input_pin.h>
#include <dk_video_decode_output_pin.h>


#define DXVA2_MAX_SURFACES 64
#define DXVA2_QUEUE_SURFACES 4
#define DXVA2_SURFACE_BASE_ALIGN 16

#define FFALIGN(x, a) (((x)+(a)-1)&~((a)-1))

dk_dxva2_decode_filter::dk_dxva2_decode_filter(LPUNKNOWN unk, HRESULT *hr)
	: dk_video_decode_filter(g_szFilterName, unk, CLSID_DK_DXVA2_DECODE_FILTER, hr)
{
	alloc_execute_params(3);
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
	if (direction == PINDIR_OUTPUT)
	{
		configure_dxva2(pin);
		set_evr4dxva2(pin);

		//_dxva2_decoder_service->CreateVideoDecoder(
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

		_fcc_output_format = MAKEFOURCC('N', 'V', '1', '2');
		_surface_width = get_aligned_dimension(_width);
		_surface_height = get_aligned_dimension(_height);
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

	properties->cBuffers = 3;
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

HRESULT dk_dxva2_decode_filter::Transform(IMediaSample * src, IMediaSample * dst)
{
	HRESULT hr = S_OK;
	BYTE * src_buffer = NULL;
	UINT src_data_size = 0;

	hr = src->GetPointer(&src_buffer);
	if (FAILED(hr))
		return S_OK;
	if (!src_buffer)
		return S_OK;
	src_data_size = src->GetActualDataLength();
	if (src_data_size <= 0)
		return S_OK;

	REFERENCE_TIME start_time, end_time;
	if (SUCCEEDED(src->GetTime(&start_time, &end_time)))
	{
		if (-1e7 != start_time)
		{
			start_time = (start_time<0) ? 0 : start_time;
		}
	}


	/*hr = _d3d_device_manager->TestDevice(_dxva2_device);
	if (hr == DXVA2_E_NEW_VIDEO_DEVICE)
	return hr;*/

	CComQIPtr<IMFGetService>	sample_service;
	CComPtr<IDirect3DSurface9>	decoder_render_target;
	sample_service = dst;
	if (sample_service)
	{
		hr = sample_service->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**)&decoder_render_target);
		if (SUCCEEDED(hr))
		{
			hr = _dxva2_decoder->BeginFrame(decoder_render_target, NULL);
		}

		UINT dxva2_buffer_size;
		uint8_t * dxva2_buffer = nullptr;
		hr = _dxva2_decoder->GetBuffer(DXVA2_BitStreamDateBufferType, (void**)&dxva2_buffer, &dxva2_buffer_size);

		if (SUCCEEDED(hr) && (dxva2_buffer_size >= src_data_size))
		{
			memcpy(dxva2_buffer, (BYTE*)src_buffer, src_data_size);

			_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].CompressedBufferType = DXVA2_BitStreamDateBufferType;
			_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].DataSize = src_data_size;
			//_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].NumMBsInBuffer = dwNumMBs;
			_dxva2_execute_params.NumCompBuffers++;
		}

		for (DWORD i = 0; i<_dxva2_execute_params.NumCompBuffers; i++)
		{
			hr = _dxva2_decoder->ReleaseBuffer(_dxva2_execute_params.pCompressedBuffers[i].CompressedBufferType);
			ASSERT(SUCCEEDED(hr));
		}

		hr = _dxva2_decoder->Execute(&_dxva2_execute_params);
		//ASSERT(SUCCEEDED(hr));
		_dxva2_execute_params.NumCompBuffers = 0;

		hr = _dxva2_decoder->EndFrame(NULL);
		ASSERT(SUCCEEDED(hr));
	}

	return S_OK;
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
	DXVA2_VideoDesc dxva2_video_description = { 0, };

	GUID * decoder_guids = NULL; // size = cDecoderGuids
	HANDLE device = INVALID_HANDLE_VALUE;

	// Query the pin for IMFGetService.
	HRESULT hr = pin->QueryInterface(__uuidof(IMFGetService), (void**)&mf_get_service);

	// Get the Direct3D device manager.
	if (SUCCEEDED(hr))
		hr = mf_get_service->GetService(MR_VIDEO_ACCELERATION_SERVICE, __uuidof(IDirect3DDeviceManager9), (void**)&d3d_device_manager);

	// Open a new device handle.
	if (SUCCEEDED(hr))
		hr = d3d_device_manager->OpenDeviceHandle(&device);

	// Get the video decoder service.
	if (SUCCEEDED(hr))
		hr = d3d_device_manager->GetVideoService(device, __uuidof(IDirectXVideoDecoderService), (void**)&dxva2_decoder_service);

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
			hr = find_decoder_configuration(dxva2_decoder_service, decoder_guids[index], &dxva2_video_description, &config, &found_dxva2_configuration);
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


		LPDIRECT3DSURFACE9 surfaces[DXVA2_MAX_SURFACES] = { 0 };
		UINT num_surfaces = max(config.ConfigMinRenderTargetBuffCount, 1);
		hr = _dxva2_decoder_service->CreateSurface(_surface_width, _surface_height, num_surfaces, (D3DFORMAT)_fcc_output_format, D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget, surfaces, nullptr);
		if (FAILED(hr)) 
		{
			DbgLog((LOG_TRACE, 10, L"-> Creation of surfaces failed with hr: %X", hr));
			hr = E_FAIL; // Unable to find a configuration.
		}

		hr = _dxva2_decoder_service->CreateVideoDecoder(_dxva2_decoder_guid, &dxva2_video_description, &config, surfaces, num_surfaces, &_dxva2_decoder);
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

HRESULT dk_dxva2_decode_filter::find_decoder_configuration(IDirectXVideoDecoderService * decoder_service, const GUID & guid_decoder, DXVA2_VideoDesc * dxva2_video_description, DXVA2_ConfigPictureDecode * selected_config, BOOL * found_dxva2_configuration)
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

			_format = (D3DFORMAT)_fcc_output_format;

			// Fill in the video description. Set the width, height, format, 
			// and frame rate.
			//DXVA2_VideoDesc dxva2_video_description = { 0 };

			fill_video_description(dxva2_video_description); // Private helper function.
			dxva2_video_description->Format = d3d_formats[index];

			// Get the available configurations.
			hr = decoder_service->GetDecoderConfigurations(guid_decoder, dxva2_video_description, NULL, &cnt_configurations, &dxva2_config);
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
	//H264_VLD_NoFGT: DXVA2, SD / HD / FHD / 4K
	//DXVA2_ModeH264_E
	//DXVA2_ModeH264_F
	if (IsEqualGUID(mode, DXVA2_ModeH264_E))
		return TRUE;
	else
		return FALSE;
}

// Returns TRUE if the decoder supports a given decoding configuration.
BOOL dk_dxva2_decode_filter::is_supported_decoder_config(const DXVA2_ConfigPictureDecode & config)
{
	return TRUE;
}

// Fills in a DXVA2_VideoDesc structure based on the input format.
void dk_dxva2_decode_filter::fill_video_description(DXVA2_VideoDesc * description)
{
	description->SampleHeight = _height;
	description->SampleWidth = _width;
}

DWORD dk_dxva2_decode_filter::get_aligned_dimension(DWORD dimension)
{
	int align = DXVA2_SURFACE_BASE_ALIGN;

	// MPEG-2 needs higher alignment on Intel cards, and it doesn't seem to harm anything to do it for all cards.
/*	if (m_nCodecId == AV_CODEC_ID_MPEG2VIDEO)
		align <<= 1;
	else if (m_nCodecId == AV_CODEC_ID_HEVC)
		align = 128;*/

	return FFALIGN(dimension, align);
}

HRESULT dk_dxva2_decode_filter::Transform(IMediaSample * src)
{
/*	CAutoLock cAutoLock(&m_csReceive);
	HRESULT			hr;
	BYTE*			pDataIn;
	int				nSize;
	REFERENCE_TIME	rtStart = _I64_MIN;
	REFERENCE_TIME	rtStop = _I64_MIN;

	if (FAILED(hr = src->GetPointer(&pDataIn)))
		return hr;

	nSize = src->GetActualDataLength();
	hr = src->GetTime(&rtStart, &rtStop);


	CComQIPtr<IMFGetService>	sample_service;
	CComPtr<IDirect3DSurface9>	decoder_render_target;
	sample_service = dst;
	if (sample_service)
	{
		hr = sample_service->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**)&decoder_render_target);
		if (SUCCEEDED(hr))
		{
			_dxva2_decoder->BeginFrame(decoder_render_target, NULL);
		}



		UINT dxva2_buffer_size;
		uint8_t * dxva2_buffer = nullptr;
		hr = _dxva2_decoder->GetBuffer(DXVA2_BitStreamDateBufferType, (void**)&dxva2_buffer, &dxva2_buffer_size);

		if (SUCCEEDED(hr) && (dxva2_buffer_size >= input_data_size))
		{
			memcpy(dxva2_buffer, (BYTE*)input_buffer, input_data_size);

			_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].CompressedBufferType = DXVA2_BitStreamDateBufferType;
			_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].DataSize = input_data_size;
			//_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].NumMBsInBuffer = dwNumMBs;
			_dxva2_execute_params.NumCompBuffers++;
		}

		for (DWORD i = 0; i<_dxva2_execute_params.NumCompBuffers; i++)
		{
			hr = _dxva2_decoder->ReleaseBuffer(_dxva2_execute_params.pCompressedBuffers[i].CompressedBufferType);
			ASSERT(SUCCEEDED(hr));
		}

		hr = _dxva2_decoder->Execute(&_dxva2_execute_params);


		_dxva2_decoder->EndFrame(NULL);
	}*/
	return S_OK;
}

HRESULT dk_dxva2_decode_filter::decode(IMediaSample * src, IMediaSample * dst)
{
	HRESULT hr = S_OK;
	BYTE * src_buffer = NULL;
	UINT src_data_size = 0;

	hr = src->GetPointer(&src_buffer);
	if (FAILED(hr))
		return S_OK;
	if (!src_buffer)
		return S_OK;
	src_data_size = src->GetActualDataLength();
	if (src_data_size <= 0)
		return S_OK;

	REFERENCE_TIME start_time, end_time;
	if (SUCCEEDED(src->GetTime(&start_time, &end_time)))
	{
		if (-1e7 != start_time)
		{
			start_time = (start_time<0) ? 0 : start_time;
		}
	}


	/*hr = _d3d_device_manager->TestDevice(_dxva2_device);
	if (hr == DXVA2_E_NEW_VIDEO_DEVICE)
		return hr;*/

	CComQIPtr<IMFGetService>	sample_service;
	CComPtr<IDirect3DSurface9>	decoder_render_target;
	sample_service = dst;
	if (sample_service)
	{
		hr = sample_service->GetService(MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**)&decoder_render_target);
		if (SUCCEEDED(hr))
		{
			hr = _dxva2_decoder->BeginFrame(decoder_render_target, NULL);
		}
			
		UINT dxva2_buffer_size;
		uint8_t * dxva2_buffer = nullptr;
		hr = _dxva2_decoder->GetBuffer(DXVA2_BitStreamDateBufferType, (void**)&dxva2_buffer, &dxva2_buffer_size);

		if (SUCCEEDED(hr) && (dxva2_buffer_size >= src_data_size))
		{
			memcpy(dxva2_buffer, (BYTE*)src_buffer, src_data_size);

			_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].CompressedBufferType = DXVA2_BitStreamDateBufferType;
			_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].DataSize = src_data_size;
			//_dxva2_execute_params.pCompressedBuffers[_dxva2_execute_params.NumCompBuffers].NumMBsInBuffer = dwNumMBs;
			_dxva2_execute_params.NumCompBuffers++;
		}

		for (DWORD i = 0; i<_dxva2_execute_params.NumCompBuffers; i++)
		{
			hr = _dxva2_decoder->ReleaseBuffer(_dxva2_execute_params.pCompressedBuffers[i].CompressedBufferType);
			ASSERT(SUCCEEDED(hr));
		}

		hr = _dxva2_decoder->Execute(&_dxva2_execute_params);
		//ASSERT(SUCCEEDED(hr));
		_dxva2_execute_params.NumCompBuffers = 0;

		hr = _dxva2_decoder->EndFrame(NULL);
		ASSERT(SUCCEEDED(hr));
	}

	return S_OK;
	//_dxva2_decoder->BeginFrame()


	/*dst->SetActualDataLength(output_data_size);

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
	*/
}

void dk_dxva2_decode_filter::alloc_execute_params(int size)
{
	_dxva2_execute_params.pCompressedBuffers = new DXVA2_DecodeBufferDesc[size];

	for (int i = 0; i<size; i++)
		memset(&_dxva2_execute_params.pCompressedBuffers[i], 0, sizeof(DXVA2_DecodeBufferDesc));

	_dxva2_execute_params.NumCompBuffers = 0;
}