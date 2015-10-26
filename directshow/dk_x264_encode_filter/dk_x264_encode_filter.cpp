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

#include "dk_x264_encode_filter_properties.h"
#include "dk_x264_encode_filter.h"

dk_x264_encode_filter::dk_x264_encode_filter(LPUNKNOWN unk, HRESULT *hr)
	: CTransformFilter(g_szFilterName, unk, CLSID_DK_X264_ENCODE_FILTER)
{
	_encoder = new dk_x264_encoder();
}

dk_x264_encode_filter::~dk_x264_encode_filter(VOID)
{
	if (_encoder)
		delete _encoder;
}

CUnknown * WINAPI dk_x264_encode_filter::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	*hr = S_OK;
	CUnknown* punk = new dk_x264_encode_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	return punk;
}

STDMETHODIMP dk_x264_encode_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
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
HRESULT  dk_x264_encode_filter::StartStreaming()
{
	return NOERROR;
}

HRESULT  dk_x264_encode_filter::StopStreaming()
{
	return NOERROR;
}

// override this to grab extra interfaces on connection
HRESULT  dk_x264_encode_filter::CheckConnect(PIN_DIRECTION direction, IPin *pin)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

// place holder to allow derived classes to release any extra interfaces
HRESULT  dk_x264_encode_filter::BreakConnect(PIN_DIRECTION direction)
{
	if (direction == PINDIR_INPUT)
	{
		if (_encoder)
			_encoder->release();
	}
	UNREFERENCED_PARAMETER(direction);
	return NOERROR;
}

// Let derived classes know about connection completion
HRESULT  dk_x264_encode_filter::CompleteConnect(PIN_DIRECTION direction, IPin *pin)
{
	if (direction == PINDIR_INPUT)
	{
		if (_encoder)
			_encoder->initialize(_config);
	}

	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

HRESULT  dk_x264_encode_filter::AlterQuality(Quality quality)
{
	UNREFERENCED_PARAMETER(quality);
	return S_FALSE;
}

HRESULT  dk_x264_encode_filter::SetMediaType(PIN_DIRECTION direction, const CMediaType *type)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(type);
	return NOERROR;
}

// EndOfStream received. Default behaviour is to deliver straight
// downstream, since we have no queued data. If you overrode Receive
// and have queue data, then you need to handle this and deliver EOS after
// all queued data is sent
HRESULT  dk_x264_encode_filter::EndOfStream(void)
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
HRESULT  dk_x264_encode_filter::BeginFlush(void)
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
HRESULT  dk_x264_encode_filter::EndFlush(void)
{
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins
	ASSERT(m_pOutput != NULL);
	return m_pOutput->DeliverEndFlush();
	// caller (the input pin's method) will unblock Receives
}


HRESULT  dk_x264_encode_filter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	if (m_pOutput != NULL)
	{
		return m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
	}
	return S_OK;
}

HRESULT  dk_x264_encode_filter::CheckInputType(const CMediaType *type)
{
	const GUID* id = type->Subtype();
	const char* media_type = (const char*)&type->Type()->Data1;
	const char* submedia_type = (const char*)&type->Subtype()->Data1;
	const GUID* formaType = type->FormatType();
	if (IsEqualGUID(*type->Type(), MEDIATYPE_Video))
	{
		/*
		if (IsEqualGUID(*(type->Subtype()), MEDIASUBTYPE_YUY2))
		{
			if (IsEqualGUID(*(formaType), FORMAT_VideoInfo2))
			{
				VIDEOINFOHEADER2 *vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->Format());
				_imedia_type = cu_h264_encoder2::COLOR_SPACE_YUY2;
				_iwidth = vih2->bmiHeader.biWidth;
				_iheight = vih2->bmiHeader.biHeight;
				if (_owidth == 0 || _oheight == 0)
				{
					_owidth = _iwidth;
					_oheight = _iheight;
				}
				return S_OK;
			}
			else if (IsEqualGUID(*(formaType), FORMAT_VideoInfo))
			{
				VIDEOINFOHEADER *vih = reinterpret_cast<VIDEOINFOHEADER*>(type->Format());
				_imedia_type = cu_h264_encoder2::COLOR_SPACE_YUY2;
				_iwidth = vih->bmiHeader.biWidth;
				_iheight = vih->bmiHeader.biHeight;
				if (_owidth == 0 || _oheight == 0)
				{
					_owidth = _iwidth;
					_oheight = _iheight;
				}
				return S_OK;
			}
		}
		else */if (IsEqualGUID(*(type->Subtype()), MEDIASUBTYPE_YV12))
		{
			if (IsEqualGUID(*(formaType), FORMAT_VideoInfo2))
			{
				VIDEOINFOHEADER2 *vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->Format());
				_config.width = vih2->bmiHeader.biWidth;
				if (vih2->bmiHeader.biHeight<0)
					_config.height = -vih2->bmiHeader.biHeight;
				else
					_config.height = vih2->bmiHeader.biHeight;
				_config.cs = dk_x264_encoder::COLOR_SPACE_YV12;
				return S_OK;
			}
			else if (IsEqualGUID(*(formaType), FORMAT_VideoInfo))
			{
				VIDEOINFOHEADER *vih = reinterpret_cast<VIDEOINFOHEADER*>(type->Format());
				_config.width = vih->bmiHeader.biWidth;
				if (vih->bmiHeader.biHeight<0)
					_config.height = -vih->bmiHeader.biHeight;
				else
					_config.height = vih->bmiHeader.biHeight;
				return S_OK;
			}
		}
		/*else if (IsEqualGUID(*(type->Subtype()), MEDIASUBTYPE_NV12))
		{
			if (IsEqualGUID(*(formaType), FORMAT_VideoInfo2))
			{
				VIDEOINFOHEADER2* vih2 = reinterpret_cast<VIDEOINFOHEADER2*>(type->Format());
				_imedia_type = cu_h264_encoder2::COLOR_SPACE_NV12;
				_iwidth = vih2->bmiHeader.biWidth;
				_iheight = vih2->bmiHeader.biHeight;
				if (_owidth == 0 || _oheight == 0)
				{
					_owidth = _iwidth;
					_oheight = _iheight;
				}
				return S_OK;
			}
			else if (IsEqualGUID(*(formaType), FORMAT_VideoInfo))
			{
				VIDEOINFOHEADER* vih = reinterpret_cast<VIDEOINFOHEADER*>(type->Format());
				_imedia_type = cu_h264_encoder2::COLOR_SPACE_NV12;
				_iwidth = vih->bmiHeader.biWidth;
				_iheight = vih->bmiHeader.biHeight;
				if (_owidth == 0 || _oheight == 0)
				{
					_owidth = _iwidth;
					_oheight = _iheight;
				}
				return S_OK;
			}
		}*/
	}
	return E_FAIL;
}

HRESULT  dk_x264_encode_filter::GetMediaType(int position, CMediaType *type)
{
	if (!m_pInput->IsConnected())
		return E_UNEXPECTED;

	if (position<0)
		return E_INVALIDARG;
	if (position>1)
		return VFW_S_NO_MORE_ITEMS;

	type->SetType(&MEDIATYPE_Video);
	if (position == 0)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
		{
			return hr;
		}

		type->SetSubtype(&MEDIASUBTYPE_H264);
		type->SetFormatType(&FORMAT_VideoInfo2);
		type->SetTemporalCompression(TRUE);
		//type->SetSampleSize(_owidth*_oheight*1.5);
		type->SetVariableSize();

		VIDEOINFOHEADER2 * vih2 = /*reinterpret_cast<VIDEOINFOHEADER2*>(type->pbFormat);//*/(VIDEOINFOHEADER2*)type->AllocFormatBuffer( sizeof(VIDEOINFOHEADER2) );
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->rcSource.left = 0;
		vih2->rcSource.top = 0;
		vih2->rcSource.right = _config.width;
		vih2->rcSource.bottom = _config.height;
		vih2->rcTarget = vih2->rcSource;
		vih2->dwPictAspectRatioX = _config.width;
		vih2->dwPictAspectRatioY = _config.height;
		//vih2->AvgTimePerFrame = (UNITS / MILLISECONDS)*(LONGLONG)(UNITS / FPS_30);
		vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih2->bmiHeader.biWidth = _config.width;
		vih2->bmiHeader.biHeight = _config.height;
		vih2->bmiHeader.biPlanes = 1;
		vih2->bmiHeader.biBitCount = 24;
		vih2->bmiHeader.biCompression = MAKEFOURCC('H', '2', '6', '4');
		vih2->bmiHeader.biSizeImage = vih2->bmiHeader.biWidth*vih2->bmiHeader.biHeight;
		//vih2->bmiHeader.biSizeImage = _owidth*_oheight*1.5;
		//		
	}
	return S_OK;
}

HRESULT  dk_x264_encode_filter::CheckTransform(const CMediaType *itype, const CMediaType *otype)
{
	if (!IsEqualGUID(*(itype->Type()), MEDIATYPE_Video) || !IsEqualGUID(*(otype->Type()), MEDIATYPE_Video))
		return E_FAIL;

	VIDEOINFOHEADER2 *ovi = (VIDEOINFOHEADER2 *)(otype->Format());
	if (!ovi)
		return E_FAIL;
	return S_OK;
}

HRESULT  dk_x264_encode_filter::DecideBufferSize(IMemAllocator *allocator, ALLOCATOR_PROPERTIES *properties)
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
	properties->cbBuffer = _config.width*_config.height * 2;
	properties->cbPrefix = 0;

	ALLOCATOR_PROPERTIES actual;
	hr = allocator->SetProperties(properties, &actual);

	if (FAILED(hr))
		return hr;

	if (properties->cBuffers>actual.cBuffers || properties->cbBuffer>actual.cbBuffer || properties->cbAlign>actual.cbAlign)
		return E_FAIL;

	return S_OK;
}

HRESULT dk_x264_encode_filter::Transform(IMediaSample *src, IMediaSample *dst)
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

	REFERENCE_TIME start_time, end_time;
	if (SUCCEEDED(src->GetTime(&start_time, &end_time)))
	{
		if (-1e7 != start_time)
		{
			start_time = (start_time<0) ? 0 : start_time;
		}
		//dst->SetTime( &start_time, &end_time );
	}
	dk_x264_encoder::PIC_TYPE pic_type;
	int result = _encoder->encode(input_buffer, input_data_size, output_buffer, output_data_size, pic_type);

	dst->SetActualDataLength(output_data_size);


	end_time = (REFERENCE_TIME)(start_time + (1.0 / 24) * 1e7);
	dst->SetTime(&start_time, &end_time);

	return S_OK;
	/*BOOL bSyncPoint;
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

	return hr;*/
}

STDMETHODIMP dk_x264_encode_filter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL)
		return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_DK_X264_ENCODE_FILTER_PROPERTIES;
	return S_OK;
}