#include <tchar.h>
#include <time.h>
#include <dshow.h>
#include <atlstr.h>
#include <string.h>
#include <stdlib.h>
#include <streams.h>
#include <dvdmedia.h>
#include <source.h>
#include <mmreg.h>

#include <dk_media_buffering.h>

#include "dk_rtmp_source_filter.h"
#include "dk_rtmp_video_source_stream.h"

dk_rtmp_video_source_stream::dk_rtmp_video_source_stream(HRESULT *hr, CSource *ms, LPCWSTR name)
	: CSourceStream(NAME("dk_rtmp_video_source_stream"), hr, ms, name)
	, _is_first_sample_delivered(FALSE)
	, _sample_media_time_start(0)
{

}

dk_rtmp_video_source_stream::~dk_rtmp_video_source_stream(VOID)
{

}

/// override this to publicise our interfaces
STDMETHODIMP dk_rtmp_video_source_stream::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IAMPushSource)
		return GetInterface(static_cast<IAMPushSource*>(this), ppv);
	else
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT dk_rtmp_video_source_stream::GetMediaType(CMediaType * type)
{
	buffering::vsubmedia_type mt = buffering::unknown_video_type;
	dk_media_buffering::instance().get_video_submedia_type(mt);
	if (mt == buffering::unknown_video_type)
		return E_UNEXPECTED;

	int32_t width = 0, height = 0;
	dk_media_buffering::instance().get_video_width(width);
	dk_media_buffering::instance().get_video_height(height);
	if (width<1 || height<1)
		return E_UNEXPECTED;
	
	if (mt == buffering::vsubmedia_type_avc)
	{
		type->InitMediaType();
		type->SetType(&MEDIATYPE_Video);
		type->SetSubtype(&MEDIASUBTYPE_H264);
		type->SetSampleSize(0);

		VIDEOINFOHEADER2 * vid;
		PBYTE buffer = type->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
		if (NULL == buffer)
			return E_OUTOFMEMORY;
		type->SetFormatType(&FORMAT_VideoInfo2);
		vid = (VIDEOINFOHEADER2 *)buffer;
		ZeroMemory(vid, sizeof(VIDEOINFOHEADER2));
		vid->rcSource.left = 0;
		vid->rcSource.top = 0;
		vid->rcSource.right = width;
		vid->rcSource.bottom = height;
		vid->rcTarget = vid->rcSource;
		//pvid->dwBitRate				= _media_buffer->media_header()->videofmt.bitrate;
		//pvid->dwBitRate				= m_videoMediaInfo.video.bitrate>0?m_videoMediaInfo.video.bitrate:304018;
		//pvid->AvgTimePerFrame			= 400000;
		//pvid->dwPictAspectRatioX		= 4;
		//pvid->dwPictAspectRatioY		= 3;
		vid->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vid->bmiHeader.biWidth = vid->rcSource.right;
		vid->bmiHeader.biHeight = vid->rcSource.bottom;
		vid->bmiHeader.biPlanes = 1;
		vid->bmiHeader.biBitCount = 24;
		vid->bmiHeader.biCompression = MAKEFOURCC('H', '2', '6', '4');
		//pvid->bmiHeader.biCompression	= MAKEFOURCC( 'A', 'V', 'C', '1' );
		vid->bmiHeader.biSizeImage = vid->bmiHeader.biWidth*vid->bmiHeader.biHeight << 1;
		buffer = buffer + sizeof(VIDEOINFOHEADER2);
		//type->SetType(&MEDIATYPE_Video);
		//type->SetSubtype(&MEDIASUBTYPE_H264);
		//type->SetSampleSize(0);
		return S_OK;
	}
	else
	{
		return E_UNEXPECTED;
	}
}

HRESULT dk_rtmp_video_source_stream::DecideBufferSize(IMemAllocator *alloc, ALLOCATOR_PROPERTIES *properties)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	HRESULT hr = NOERROR;

	CheckPointer(alloc, E_POINTER);
	CheckPointer(properties, E_POINTER);

	if (properties->cBuffers == 0)
		properties->cBuffers = 2;

	properties->cbBuffer = (1024*1024*2);
	ASSERT(properties->cbBuffer);

	// Ask the allocator to reserve us some sample memory, NOTE the function
	// can succeed (that is return NOERROR) but still not have allocated the
	// memory that we requested, so we must check we got whatever we wanted
	ALLOCATOR_PROPERTIES Actual;
	hr = alloc->SetProperties(properties, &Actual);
	if (FAILED(hr))
	{
		return hr;
	}

	// Is this allocator unsuitable
	if (Actual.cbBuffer<properties->cbBuffer)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT dk_rtmp_video_source_stream::FillBuffer(IMediaSample *ms)
{
	CheckPointer(ms, E_POINTER);
	dk_rtmp_source_filter * parent = static_cast<dk_rtmp_source_filter*>(m_pFilter);

	BYTE * buffer = NULL;
	size_t size_of_recvd = 0;
	long long timestamp = 0;
	ms->GetPointer(&buffer);

#if 1
	for (int i = 0; i < 1000 && parent->m_State!=State_Stopped; i++)
	{
		dk_media_buffering::instance().pop_video(buffer, size_of_recvd, timestamp);
		if (size_of_recvd > 0)
			break;
		::Sleep(1);
	}
#else
	dk_media_buffering::instance().pop_video(buffer, size_of_recvd);
#endif

	if (size_of_recvd > 0)
	{
		ms->SetActualDataLength(size_of_recvd);
	}
	else
		ms->SetActualDataLength(0);

	return NOERROR;
}

//
// OnThreadCreate
//
HRESULT dk_rtmp_video_source_stream::OnThreadCreate(void)
{
	_is_first_sample_delivered = FALSE;
	_sample_media_time_start = 0;
	return CSourceStream::OnThreadCreate();
}


//
// OnThreadDestroy
//
HRESULT dk_rtmp_video_source_stream::OnThreadDestroy(void)
{
	return CSourceStream::OnThreadDestroy();
}


//
// OnThreadStartPlay
//
HRESULT dk_rtmp_video_source_stream::OnThreadStartPlay(void)
{
	return CSourceStream::OnThreadStartPlay();
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_video_source_stream::GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_video_source_stream::GetPushSourceFlags(ULONG *pFlags)
{
	*pFlags = AM_PUSHSOURCECAPS_PRIVATE_CLOCK; return S_OK;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_video_source_stream::GetStreamOffset(REFERENCE_TIME *prtOffset)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_video_source_stream::SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_video_source_stream::SetPushSourceFlags(ULONG Flags)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_video_source_stream::SetStreamOffset(REFERENCE_TIME rtOffset)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_video_source_stream::GetLatency(REFERENCE_TIME *prtLatency)
{
	return E_NOTIMPL;
	//TODO: set different latencies for audio and video
	//Set 450ms Latency 
	*prtLatency = 45000000; return S_OK;
}

DWORD dk_rtmp_video_source_stream::ThreadProc(VOID)
{
	HRESULT hr;  // the return code from calls
	Command com;
	do
	{
		com = GetRequest();
		if (com != CMD_INIT)
		{
			DbgLog((LOG_ERROR, 1, TEXT("Thread expected init command")));
			Reply((DWORD)E_UNEXPECTED);
		}
	} while (com != CMD_INIT);
	DbgLog((LOG_TRACE, 1, TEXT("CSourceStream worker thread initializing")));

	hr = OnThreadCreate(); // perform set up tasks
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, 1, TEXT("CSourceStream::OnThreadCreate failed. Aborting thread.")));
		OnThreadDestroy();
		Reply(hr);	// send failed return code from OnThreadCreate
		return 1;
	}

	// Initialisation suceeded
	Reply(NOERROR);

	Command cmd;
	do
	{
		cmd = GetRequest();

		switch (cmd)
		{
		case CMD_EXIT:
			Reply(NOERROR);
			break;
		case CMD_RUN:
			DbgLog((LOG_ERROR, 1, TEXT("CMD_RUN received before a CMD_PAUSE???")));
			// !!! fall through???
		case CMD_PAUSE:
			Reply(NOERROR);
			DoBufferProcessingLoop();
			break;
		case CMD_STOP:
			Reply(NOERROR);
			break;
		default:
			DbgLog((LOG_ERROR, 1, TEXT("Unknown command %d received!"), cmd));
			Reply((DWORD)E_NOTIMPL);
			break;
		}
	} while (cmd != CMD_EXIT);

	hr = OnThreadDestroy();	// tidy up.
	if (FAILED(hr))
	{
		DbgLog((LOG_ERROR, 1, TEXT("CSourceStream::OnThreadDestroy failed. Exiting thread.")));
		return 1;
	}

	DbgLog((LOG_TRACE, 1, TEXT("CSourceStream worker thread exiting")));
	return 0;
}

HRESULT dk_rtmp_video_source_stream::DoBufferProcessingLoop(VOID)
{
	Command com;
	OnThreadStartPlay();
	do
	{
		while (!CheckRequest(&com))
		{
			IMediaSample *pSample;
			HRESULT hr = GetDeliveryBuffer(&pSample, NULL, NULL, 0);
			if (FAILED(hr))
			{
				Sleep(1);
				continue;	// go round again. Perhaps the error will go away
				// or the allocator is decommited & we will be asked to
				// exit soon.
			}

			// Virtual function user will override.
			hr = FillBuffer(pSample);
			if (hr == S_OK)
			{
				hr = Deliver(pSample);
				pSample->Release();

				// downstream filter returns S_FALSE if it wants us to
				// stop or an error if it's reporting an error.
				if (hr != S_OK)
				{
					DbgLog((LOG_TRACE, 2, TEXT("Deliver() returned %08x; stopping"), hr));
					return S_OK;
				}
			}
			else if (hr == S_FALSE)
			{
				// derived class wants us to stop pushing data
				pSample->Release();
				DeliverEndOfStream();
				return S_OK;
			}
			else
			{
				// derived class encountered an error
				pSample->Release();
				DbgLog((LOG_ERROR, 1, TEXT("Error %08lX from FillBuffer!!!"), hr));
				DeliverEndOfStream();
				m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
				return hr;
			}
		}

		// For all commands sent to us there must be a Reply call!
		if (com == CMD_RUN || com == CMD_PAUSE)
		{
			Reply(NOERROR);
		}
		else if (com != CMD_STOP)
		{
			Reply((DWORD)E_UNEXPECTED);
			DbgLog((LOG_ERROR, 1, TEXT("Unexpected command!!!")));
		}
	} while (com != CMD_STOP);
	return S_FALSE;
}
