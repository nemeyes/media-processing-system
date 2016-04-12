#include <streams.h>
#include <dvdmedia.h>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <dk_string_helper.h>
#include "dk_yuvsource_stream.h"
#include "dk_yuvsource_filter.h"


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

dk_yuvsource_stream::dk_yuvsource_stream(HRESULT * hr, CSource * filter, LPCTSTR file_path, int32_t width, int32_t height, int32_t fps)
	: CSourceStream(TEXT("dk_yuvsource_stream"), hr, filter, TEXT("out"))
	, _frame_number(0)
	, _frame_length(UNITS/fps)
{
	char * mb_filepath = nullptr;
	dk_string_helper::convert_wide2multibyte((LPTSTR)file_path, &mb_filepath);
	if (mb_filepath && strlen(mb_filepath)>0)
	{
		_width = width;
		_height = height;
		_reader = &(static_cast<dk_yuvsource_filter*>(filter)->_reader);
		_reader->initialize_reader(mb_filepath, width, height, fps);
		free(mb_filepath);
		mb_filepath = nullptr;
	}
}

dk_yuvsource_stream::~dk_yuvsource_stream(void)
{
	//_reader->release_reader();
}

STDMETHODIMP dk_yuvsource_stream::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	//if (riid == IID_IAMPushSource)
	//	return GetInterface(static_cast<IAMPushSource*>(this), ppv);
	//else
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT dk_yuvsource_stream::CheckMediaType(const CMediaType * mt) 
{
	CAutoLock lock(m_pFilter->pStateLock());

	VIDEOINFOHEADER2 * vih = (VIDEOINFOHEADER2*)(mt->Format());
	if (!vih)
		return E_FAIL;
	_stride = (UINT)vih->bmiHeader.biWidth;

	return NOERROR;
}

HRESULT dk_yuvsource_stream::GetMediaType(int position, __inout CMediaType * mt)
{
	if (position<0)
		return E_INVALIDARG;
	if (position>0)
		return VFW_S_NO_MORE_ITEMS;

	CheckPointer(mt, E_POINTER);

	mt->InitMediaType();
	mt->SetType(&MEDIATYPE_Video);
	if (position == 0)
	{
		mt->SetSubtype(&MEDIASUBTYPE_YV12);
		mt->SetFormatType(&FORMAT_VideoInfo2);
		mt->SetTemporalCompression(FALSE);
		mt->SetSampleSize(_width*_height*1.5);
		mt->bFixedSizeSamples = TRUE;
		BYTE * buffer = mt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
		if (NULL == buffer)
			return E_OUTOFMEMORY;
		VIDEOINFOHEADER2 * vih2 = (VIDEOINFOHEADER2 *)buffer;
		ZeroMemory(vih2, sizeof(VIDEOINFOHEADER2));
		vih2->AvgTimePerFrame = _frame_length;
		vih2->rcSource.left = 0;
		vih2->rcSource.top = 0;
		vih2->rcSource.right = _width;
		vih2->rcSource.bottom = _height;
		vih2->rcTarget = vih2->rcSource;
		vih2->dwPictAspectRatioX = _width;
		vih2->dwPictAspectRatioY = _height;
		vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vih2->bmiHeader.biWidth = _width;
		vih2->bmiHeader.biHeight = -_height;
		vih2->bmiHeader.biPlanes = 1;
		vih2->bmiHeader.biBitCount = 12;
		vih2->bmiHeader.biCompression = MAKEFOURCC('Y', 'V', '1', '2');
		vih2->bmiHeader.biSizeImage = _width*_height*1.5;
		return S_OK;
	}
	return S_FALSE;
}

/*
HRESULT dk_yuvsource_stream::GetMediaType(CMediaType * mt)
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

HRESULT dk_yuvsource_stream::DecideBufferSize(IMemAllocator * alloc, ALLOCATOR_PROPERTIES * request)
{
	CheckPointer(alloc, E_POINTER);
	CheckPointer(request, E_POINTER);

	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*)m_mt.Format();
	if (request->cBuffers == 0)
		request->cBuffers = 1;
	request->cbBuffer = _height * _stride * 1.5;

	ALLOCATOR_PROPERTIES actual;
	HRESULT hr = alloc->SetProperties(request, &actual);
	if (FAILED(hr))
		return hr;
	if (actual.cbBuffer < request->cbBuffer)
		return E_FAIL;

	return S_OK;
}


HRESULT dk_yuvsource_stream::FillBuffer(IMediaSample * ms)
{
	CheckPointer(ms, E_POINTER);

	BYTE * dst = 0;
	ms->GetPointer(&dst);
	LONG length = ms->GetSize();

	_reader->read(dst, _stride);
	REFERENCE_TIME rt_start = _frame_number * _frame_length;
	REFERENCE_TIME rt_stop = rt_start + _frame_length;
	_frame_number++;

	ms->SetTime(&rt_start, &rt_stop);
	ms->SetSyncPoint(TRUE);
	ms->SetActualDataLength(_height * _stride * 1.5);
	return S_OK;
}