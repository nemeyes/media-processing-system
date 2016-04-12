#include <streams.h>
#include <dvdmedia.h>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <dk_simd_colorspace_converter.h>

#include "dk_image_source_stream.h"


// UNITS = 10 ^ 7  
// UNITS / 30 = 30 fps;
// UNITS / 20 = 20 fps, etc
const REFERENCE_TIME FPS_30 = UNITS / 30;
const REFERENCE_TIME FPS_20 = UNITS / 20;
const REFERENCE_TIME FPS_10 = UNITS / 10;
const REFERENCE_TIME FPS_5 = UNITS / 5;
const REFERENCE_TIME FPS_4 = UNITS / 4;
const REFERENCE_TIME FPS_3 = UNITS / 3;
const REFERENCE_TIME FPS_2 = UNITS / 2;
const REFERENCE_TIME FPS_1 = UNITS / 1;

/*	CRefTime _sample_time;
	int _frame_number;
	const REFERENCE_TIME _frame_length;*/

dk_image_source_stream::dk_image_source_stream(HRESULT * hr, CSource * filter, LPCTSTR image_path)
	: CSourceStream(TEXT("dk_image_source_stream"), hr, filter, TEXT("out"))
	, _frame_number(0)
	, _frame_length(FPS_30)
	, _bimgloaded(false)
{
	_bimgloaded = _loader.load(image_path);
}

dk_image_source_stream::~dk_image_source_stream(void)
{
	_loader.empty();
	_bimgloaded = false;
}

STDMETHODIMP dk_image_source_stream::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	//if (riid == IID_IAMPushSource)
	//	return GetInterface(static_cast<IAMPushSource*>(this), ppv);
	//else
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT dk_image_source_stream::CheckMediaType(const CMediaType * mt) 
{
	CAutoLock lock(m_pFilter->pStateLock());

	VIDEOINFOHEADER2 * vih = (VIDEOINFOHEADER2*)(mt->Format());
	if (!vih)
		return E_FAIL;
	_stride = (UINT)vih->bmiHeader.biWidth;

	return NOERROR;
}

HRESULT dk_image_source_stream::GetMediaType(int position, __inout CMediaType * mt)
{
	if (position<0)
		return E_INVALIDARG;
	if (position>0)
		return VFW_S_NO_MORE_ITEMS;

	CheckPointer(mt, E_POINTER);

	mt->InitMediaType();

	if (position == 0)
	{
		BYTE * buffer = mt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
		if (NULL == buffer)
			return E_OUTOFMEMORY;

		VIDEOINFOHEADER2 * vih = (VIDEOINFOHEADER2 *)buffer;
		ZeroMemory(vih, sizeof(VIDEOINFOHEADER2));
		vih->AvgTimePerFrame = _frame_length;

		memcpy(&vih->bmiHeader, _loader.bmih, sizeof(BITMAPINFOHEADER));
		vih->bmiHeader.biHeight = -vih->bmiHeader.biHeight;
		SetRectEmpty(&(vih->rcSource));
		SetRectEmpty(&(vih->rcTarget));
		vih->rcSource.left = 0;
		vih->rcSource.top = 0;
		vih->rcSource.right = _loader.bmih->biWidth;
		vih->rcSource.bottom = _loader.bmih->biHeight;
		vih->rcTarget = vih->rcSource;
		vih->dwPictAspectRatioX = _loader.bmih->biWidth;
		vih->dwPictAspectRatioY = _loader.bmih->biHeight;

		mt->SetType(&MEDIATYPE_Video);
		mt->SetFormatType(&FORMAT_VideoInfo2);
		mt->SetTemporalCompression(FALSE);
		const GUID subtype = GetBitmapSubtype(&vih->bmiHeader);
		mt->SetSubtype(&subtype);
		mt->SetSampleSize(vih->bmiHeader.biSizeImage);
		mt->bFixedSizeSamples = TRUE;
		return S_OK;
	}
	return S_FALSE;
}

/*
HRESULT dk_image_source_stream::GetMediaType(CMediaType * mt)
{
	if (!_bimgloaded)
		return E_FAIL;

	CheckPointer(mt, E_POINTER);

	mt->InitMediaType();

	BYTE * buffer = mt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
	if (NULL == buffer) 
		return E_OUTOFMEMORY;

	VIDEOINFOHEADER2 * vih = (VIDEOINFOHEADER2 *)buffer;
	ZeroMemory(vih, mt->cbFormat);
	vih->AvgTimePerFrame = _frame_length;

	memcpy(&vih->bmiHeader, _bmih, sizeof(BITMAPINFOHEADER));
	//vih->bmiHeader.biHeight = -vih->bmiHeader.biHeight;
	//vih->bmiHeader.biSizeImage = GetBitmapSize(&vih->bmiHeader);
	SetRectEmpty(&(vih->rcSource));
	SetRectEmpty(&(vih->rcTarget));
	vih->rcSource.left = 0;
	vih->rcSource.top = 0;
	vih->rcSource.right = _bmih->biWidth;
	vih->rcSource.bottom = _bmih->biHeight;
	vih->rcTarget = vih->rcSource;
	vih->dwPictAspectRatioX = _bmih->biWidth;
	vih->dwPictAspectRatioY = _bmih->biHeight;

	mt->SetType(&MEDIATYPE_Video);
	mt->SetSubtype(&MEDIASUBTYPE_RGB32);
	mt->SetFormatType(&FORMAT_VideoInfo2);
	mt->SetTemporalCompression(FALSE);
	//const GUID subtype = GetBitmapSubtype(&vih->bmiHeader);
	//mt->SetSubtype(&subtype);
	mt->SetSampleSize(vih->bmiHeader.biSizeImage);
	mt->bFixedSizeSamples = TRUE;
	return S_OK;
}
*/

HRESULT dk_image_source_stream::DecideBufferSize(IMemAllocator * alloc, ALLOCATOR_PROPERTIES * request)
{
	if (!_bimgloaded)
		return E_FAIL;

	CheckPointer(alloc, E_POINTER);
	CheckPointer(request, E_POINTER);

	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*)m_mt.Format();
	if (request->cBuffers == 0)
		request->cBuffers = 2;
	request->cbBuffer = long(_loader.bmih->biHeight * _loader.bmih->biWidth * _loader.Bpp * 1.5);

	ALLOCATOR_PROPERTIES actual;
	HRESULT hr = alloc->SetProperties(request, &actual);
	if (FAILED(hr))
		return hr;
	if (actual.cbBuffer < request->cbBuffer)
		return E_FAIL;

	return S_OK;
}


HRESULT dk_image_source_stream::FillBuffer(IMediaSample * ms)
{
	if (!_bimgloaded)
		return E_FAIL;

	CheckPointer(ms, E_POINTER);

	BYTE * src = _loader.pixel_buffer;
	BYTE * dst = 0;
	ms->GetPointer(&dst);

	LONG length = ms->GetSize();

	VIDEOINFOHEADER2 * vih = (VIDEOINFOHEADER2*)m_mt.Format();
	dk_simd_colorspace_converter::convert_rgba_to_rgba(vih->bmiHeader.biWidth, std::abs(vih->bmiHeader.biHeight), _loader.pixel_buffer, vih->bmiHeader.biWidth * _loader.Bpp, dst, _stride * _loader.Bpp, true);
	REFERENCE_TIME rt_start = _frame_number * _frame_length;
	REFERENCE_TIME rt_stop = rt_start + _frame_length;
	_frame_number++;

	ms->SetTime(&rt_start, &rt_stop);
//	ms->SetSyncPoint(TRUE);
	ms->SetActualDataLength(std::abs(vih->bmiHeader.biHeight) * _stride * _loader.Bpp);
	return S_OK;
}