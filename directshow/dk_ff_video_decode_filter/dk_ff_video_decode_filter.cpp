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

#include "dk_ff_video_decode_filter_properties.h"
#include "dk_ff_video_decode_filter.h"

dk_ff_video_decode_filter::dk_ff_video_decode_filter(LPUNKNOWN unk, HRESULT *hr)
	: CTransformFilter(g_szFilterName, unk, CLSID_DK_FFMPEG_VIDEO_DECODE_FILTER)
{
	_decoder = new dk_ff_video_decoder();
}

dk_ff_video_decode_filter::~dk_ff_video_decode_filter(VOID)
{
	if (_decoder)
		delete _decoder;
}

CUnknown * WINAPI dk_ff_video_decode_filter::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	*hr = S_OK;
	CUnknown* punk = new dk_ff_video_decode_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	return punk;
}

STDMETHODIMP dk_ff_video_decode_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
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
HRESULT  dk_ff_video_decode_filter::StartStreaming()
{
	return NOERROR;
}

HRESULT  dk_ff_video_decode_filter::StopStreaming()
{
	return NOERROR;
}

// override this to grab extra interfaces on connection
HRESULT  dk_ff_video_decode_filter::CheckConnect(PIN_DIRECTION direction, IPin *pin)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

// place holder to allow derived classes to release any extra interfaces
HRESULT  dk_ff_video_decode_filter::BreakConnect(PIN_DIRECTION direction)
{
	if (direction == PINDIR_INPUT)
	{
		if (_decoder)
			_decoder->release_decoder();
	}
	UNREFERENCED_PARAMETER(direction);
	return NOERROR;
}

// Let derived classes know about connection completion
HRESULT  dk_ff_video_decode_filter::CompleteConnect(PIN_DIRECTION direction, IPin *pin)
{
	if (direction == PINDIR_INPUT)
	{
		if (_decoder)
			_decoder->initialize_decoder(&_config);
	}

	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

HRESULT  dk_ff_video_decode_filter::AlterQuality(Quality quality)
{
	UNREFERENCED_PARAMETER(quality);
	return S_FALSE;
}

HRESULT  dk_ff_video_decode_filter::SetMediaType(PIN_DIRECTION direction, const CMediaType *type)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(type);
	return NOERROR;
}

// EndOfStream received. Default behaviour is to deliver straight
// downstream, since we have no queued data. If you overrode Receive
// and have queue data, then you need to handle this and deliver EOS after
// all queued data is sent
HRESULT  dk_ff_video_decode_filter::EndOfStream(void)
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
HRESULT  dk_ff_video_decode_filter::BeginFlush(void)
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
HRESULT  dk_ff_video_decode_filter::EndFlush(void)
{
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins
	ASSERT(m_pOutput != NULL);
	return m_pOutput->DeliverEndFlush();
	// caller (the input pin's method) will unblock Receives
}


HRESULT  dk_ff_video_decode_filter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	if (m_pOutput != NULL)
	{
		return m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
	}
	return S_OK;
}

HRESULT  dk_ff_video_decode_filter::CheckInputType(const CMediaType *type)
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
					_config.ismt = dk_ff_video_decoder::SUBMEDIA_TYPE_H264;
					_config.iwidth = vih2->bmiHeader.biWidth;
					_config.iheight = vih2->bmiHeader.biHeight;
					_config.owidth = _config.iwidth;
					_config.oheight = _config.iheight;
					return S_OK;
				}
				else if (IsEqualGUID(*(formaType), FORMAT_VideoInfo))
				{
					VIDEOINFOHEADER * vih = reinterpret_cast<VIDEOINFOHEADER*>(type->Format());
					_config.ismt = dk_ff_video_decoder::SUBMEDIA_TYPE_H264;
					_config.iwidth = vih->bmiHeader.biWidth;
					_config.iheight = vih->bmiHeader.biHeight;
					_config.owidth = _config.iwidth;
					_config.oheight = _config.iheight;
					return S_OK;
				}
			}
		}
	}
	return E_FAIL;
}

HRESULT  dk_ff_video_decode_filter::GetMediaType(int position, CMediaType *type)
{
	if (!m_pInput->IsConnected())
		return E_UNEXPECTED;

	if (position<0)
		return E_INVALIDARG;
	if (position>2)
		return VFW_S_NO_MORE_ITEMS;

	type->SetType(&MEDIATYPE_Video);

	if (position == 0)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
		{
			return hr;
		}

		type->SetSubtype(&MEDIASUBTYPE_YV12);
		type->SetFormatType(&FORMAT_VideoInfo2);
		type->SetTemporalCompression(FALSE);
		type->SetSampleSize(_config.owidth*_config.oheight*1.5);

		VIDEOINFOHEADER2	*vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->pbFormat);//(VIDEOINFOHEADER2*)type->AllocFormatBuffer( sizeof(VIDEOINFOHEADER2) );
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->rcSource.left = 0;
		vih2->rcSource.top = 0;
		vih2->rcSource.right = _config.owidth;
		vih2->rcSource.bottom = _config.oheight;
		vih2->rcTarget.left = 0;
		vih2->rcTarget.top = 0;
		vih2->rcTarget.right = _config.owidth;
		vih2->rcTarget.bottom = _config.oheight;
		vih2->dwPictAspectRatioX = _config.owidth;
		vih2->dwPictAspectRatioY = _config.oheight;
		vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih2->bmiHeader.biWidth = _config.owidth;
		vih2->bmiHeader.biHeight = _config.oheight;
		vih2->bmiHeader.biPlanes = 1;
		vih2->bmiHeader.biBitCount = 12;
		vih2->bmiHeader.biCompression = MAKEFOURCC('Y', 'V', '1', '2');//0x32315659;
		vih2->bmiHeader.biSizeImage = _config.owidth*_config.oheight*1.5;
	}
	else if (position == 1)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
		{
			return hr;
		}

		type->SetSubtype(&MEDIASUBTYPE_RGB32);
		type->SetFormatType(&FORMAT_VideoInfo2);
		type->SetTemporalCompression(FALSE);
		type->SetSampleSize(_config.owidth * _config.oheight * 4);

		VIDEOINFOHEADER2	*vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->pbFormat);//(VIDEOINFOHEADER2*)type->AllocFormatBuffer( sizeof(VIDEOINFOHEADER2) );
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->rcSource.left = 0;
		vih2->rcSource.top = 0;
		vih2->rcSource.right = _config.owidth;
		vih2->rcSource.bottom = _config.oheight;
		vih2->rcTarget.left = 0;
		vih2->rcTarget.top = 0;
		vih2->rcTarget.right = _config.owidth;
		vih2->rcTarget.bottom = _config.oheight;
		vih2->dwPictAspectRatioX = _config.owidth;
		vih2->dwPictAspectRatioY = _config.oheight;
		vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih2->bmiHeader.biWidth = _config.owidth;
		vih2->bmiHeader.biHeight = _config.oheight;
		vih2->bmiHeader.biPlanes = 1;
		vih2->bmiHeader.biBitCount = 32;
		vih2->bmiHeader.biCompression = BI_RGB;//MAKEFOURCC('N', 'V', '1', '2');//0x32315659;
		vih2->bmiHeader.biSizeImage = _config.owidth * _config.oheight * 4;
		vih2->bmiHeader.biClrImportant = 0;
	}
	else if (position == 2)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
		{
			return hr;
		}

		type->SetSubtype(&MEDIASUBTYPE_RGB24);
		type->SetFormatType(&FORMAT_VideoInfo2);
		type->SetTemporalCompression(FALSE);
		type->SetSampleSize(_config.owidth*_config.oheight * 3);

		VIDEOINFOHEADER2	*vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->pbFormat);//(VIDEOINFOHEADER2*)type->AllocFormatBuffer( sizeof(VIDEOINFOHEADER2) );
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->rcSource.left = 0;
		vih2->rcSource.top = 0;
		vih2->rcSource.right = _config.owidth;
		vih2->rcSource.bottom = _config.oheight;
		vih2->rcTarget.left = 0;
		vih2->rcTarget.top = 0;
		vih2->rcTarget.right = _config.owidth;
		vih2->rcTarget.bottom = _config.oheight;
		vih2->dwPictAspectRatioX = _config.owidth;
		vih2->dwPictAspectRatioY = _config.oheight;
		vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih2->bmiHeader.biWidth = _config.owidth;
		vih2->bmiHeader.biHeight = _config.oheight;
		vih2->bmiHeader.biPlanes = 1;
		vih2->bmiHeader.biBitCount = 24;
		vih2->bmiHeader.biCompression = BI_RGB;
		vih2->bmiHeader.biSizeImage = _config.owidth*_config.oheight * 3;
	}
	return S_OK;
}

HRESULT  dk_ff_video_decode_filter::CheckTransform(const CMediaType *itype, const CMediaType *otype)
{
	if (!IsEqualGUID(*(itype->Type()), MEDIATYPE_Video) || !IsEqualGUID(*(otype->Type()), MEDIATYPE_Video))
		return E_FAIL;

	VIDEOINFOHEADER2 *ovi = (VIDEOINFOHEADER2 *)(otype->Format());
	if (!ovi)
		return E_FAIL;

	if (IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_RGB32))
	{
		_config.ostride = _config.owidth * 4;
		_config.osmt = dk_ff_video_decoder::SUBMEDIA_TYPE_RGB32;
	}
	else if (IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_RGB24))
	{
		_config.ostride = _config.owidth * 3;
		_config.osmt = dk_ff_video_decoder::SUBMEDIA_TYPE_RGB24;
	}
	else if (IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_YV12))
	{
		_config.ostride = (UINT)ovi->bmiHeader.biWidth;
		_config.osmt = dk_ff_video_decoder::SUBMEDIA_TYPE_YV12;
	}

	return S_OK;
}

HRESULT  dk_ff_video_decode_filter::DecideBufferSize(IMemAllocator *allocator, ALLOCATOR_PROPERTIES *properties)
{
	if (!m_pInput->IsConnected())
		return E_UNEXPECTED;

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
	if (_config.osmt == dk_ff_video_decoder::SUBMEDIA_TYPE_RGB32)
		properties->cbBuffer = _config.owidth*_config.oheight * 4;
	else if (_config.osmt == dk_ff_video_decoder::SUBMEDIA_TYPE_RGB24)
		properties->cbBuffer = _config.owidth*_config.oheight * 3;
	else
		properties->cbBuffer = _config.ostride*_config.oheight * 1.5;

	properties->cbPrefix = 0;

	ALLOCATOR_PROPERTIES actual;
	hr = allocator->SetProperties(properties, &actual);

	if (FAILED(hr))
		return hr;

	if (properties->cBuffers>actual.cBuffers || properties->cbBuffer>actual.cbBuffer || properties->cbAlign>actual.cbAlign)
		return E_FAIL;

	return S_OK;
}

HRESULT dk_ff_video_decode_filter::Transform(IMediaSample *src, IMediaSample *dst)
{
	HRESULT hr = S_OK;
	dk_video_entity_t encoded = { 0, };
	dk_video_entity_t decoded = { 0, };

	/*BYTE *input_buffer = NULL;
	UINT input_data_size = 0;
	BYTE *output_buffer = NULL;
	UINT output_data_size = 0;*/

	hr = src->GetPointer(&encoded.data);
	if (FAILED(hr))
		return S_OK;
	if (!encoded.data)
		return S_OK;
	encoded.data_size = src->GetActualDataLength();
	if (encoded.data_size <= 0)
		return S_OK;
	hr = dst->GetPointer(&decoded.data);
	if (FAILED(hr))
		return S_OK;

	REFERENCE_TIME start_time, end_time;
	if (SUCCEEDED(src->GetTime(&start_time, &end_time)))
	{
		if (-1e7 != start_time)
		{
			start_time = (start_time<0) ? 0 : start_time;
		}
		//dst->SetTime( &start_time, &end_time );
	}

	int result = _decoder->decode(&encoded, &decoded);

	dst->SetActualDataLength(decoded.data_size);


	end_time = (REFERENCE_TIME)(start_time + (1.0 / 24) * 1e7);
	dst->SetTime(&start_time, &end_time);
	dst->SetSyncPoint(TRUE);

	return S_OK;

	/*
	BOOL bSyncPoint;
	BOOL bPreroll;
	BOOL bDiscon;

	end_time = (REFERENCE_TIME)(start_time + (1.0 / 24) * 1e7);
	dst->SetTime(&start_time, &end_time);

	hr = src->IsSyncPoint();
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

	return hr;
	*/
}

STDMETHODIMP dk_ff_video_decode_filter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL)
		return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_DK_FFMPEG_VIDEO_DECODE_FILTER_PROPERTIES;
	return S_OK;
}
