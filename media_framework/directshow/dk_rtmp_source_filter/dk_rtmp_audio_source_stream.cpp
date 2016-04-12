#include <tchar.h>
#include <time.h>
#include <dshow.h>
#include <initguid.h> 
#include <atlstr.h>
#include <string.h>
#include <stdlib.h>
#include <streams.h>
#include <dvdmedia.h>
#include <source.h>
#include <mmreg.h>
#include <wmcodecdsp.h>

#include <dk_media_buffering.h>
#include "dk_rtmp_source_filter.h"
#include "dk_rtmp_audio_source_stream.h"

#pragma comment(lib, "wmcodecdspuuid.lib")

//DEFINE_GUID(MEDIASUBTYPE_AAC,
//	0x000000FF, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

DEFINE_GUID(MEDIASUBTYPE_MP3,
	0x00000055, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_AAC	    0x00FF

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

dk_rtmp_audio_source_stream::dk_rtmp_audio_source_stream(HRESULT *hr, CSource *ms, LPCWSTR name)
	: CSourceStream(NAME("dk_rtmp_audio_source_stream"), hr, ms, name)
{

}

dk_rtmp_audio_source_stream::~dk_rtmp_audio_source_stream(VOID)
{

}

/// override this to publicise our interfaces
STDMETHODIMP dk_rtmp_audio_source_stream::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IAMPushSource)
		return GetInterface(static_cast<IAMPushSource*>(this), ppv);
	else
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT dk_rtmp_audio_source_stream::GetMediaType(CMediaType * type)
{
	buffering::asubmedia_type mt = buffering::unknown_audio_type;
	dk_media_buffering::instance().get_audio_submedia_type(mt);
	if (mt == buffering::unknown_audio_type)
		return E_UNEXPECTED;

	int32_t samplerate = 0, channels = 0, bitdepth = 0;
	dk_media_buffering::instance().get_audio_samplerate(samplerate);
	dk_media_buffering::instance().get_audio_channels(channels);
	dk_media_buffering::instance().get_audio_bitdepth(bitdepth);
	if (samplerate<1 || channels<1 || bitdepth<1)
		return E_UNEXPECTED;

	uint8_t configstr[50] = { 0 };
	size_t configstr_size = 0;
	dk_media_buffering::instance().get_configstr(configstr, configstr_size);

	if (mt == buffering::asubmedia_type_aac)
	{
		type->InitMediaType();
		type->SetType(&MEDIATYPE_Audio);
		type->SetSubtype(&MEDIASUBTYPE_RAW_AAC1);
		type->SetFormatType(&FORMAT_WaveFormatEx);
		type->bFixedSizeSamples = FALSE;
		type->SetTemporalCompression(TRUE);

#if 1
		WAVEFORMATEX * wfx = reinterpret_cast<WAVEFORMATEX*>(type->AllocFormatBuffer(sizeof(WAVEFORMATEX) + configstr_size));
		if (!wfx)
			return E_OUTOFMEMORY;
		ZeroMemory(wfx, type->cbFormat);

		wfx->wFormatTag = WAVE_FORMAT_AAC;
		wfx->nChannels = channels;
		wfx->nSamplesPerSec = samplerate;
		wfx->wBitsPerSample = bitdepth;
		wfx->nBlockAlign = (wfx->nChannels*wfx->wBitsPerSample / 8);
		wfx->nAvgBytesPerSec = wfx->nSamplesPerSec*wfx->nBlockAlign;
		wfx->cbSize = configstr_size;
		if (configstr_size > 0)
		{
			uint8_t * wfxex = ((uint8_t*)(wfx)) + sizeof(*wfx);
			memcpy(wfxex, configstr, configstr_size);
		}
		type->SetSampleSize(wfx->nBlockAlign);
#else
		WAVEFORMATEX * wfx = reinterpret_cast<WAVEFORMATEX*>(type->AllocFormatBuffer(sizeof(WAVEFORMATEX)));
		if (!wfx)
			return E_OUTOFMEMORY;
		ZeroMemory(wfx, type->cbFormat);

		wfx->wFormatTag = WAVE_FORMAT_AAC;
		wfx->nChannels = channels;
		wfx->nSamplesPerSec = samplerate;
		wfx->wBitsPerSample = bitdepth;
		wfx->nBlockAlign = (wfx->nChannels*wfx->wBitsPerSample / 8);
		wfx->nAvgBytesPerSec = wfx->nSamplesPerSec*wfx->nBlockAlign;
		wfx->cbSize = 0;
		type->SetSampleSize(wfx->nBlockAlign);
#endif

		return S_OK;
	}
	else if (mt == buffering::asubmedia_type_mp3)
	{
		type->InitMediaType();
		type->SetType(&MEDIATYPE_Audio);
		type->SetSubtype(&MEDIASUBTYPE_MP3);
		type->SetFormatType(&FORMAT_WaveFormatEx);
		type->bFixedSizeSamples = FALSE;
		type->SetTemporalCompression(TRUE);

		WAVEFORMATEX * wfx = reinterpret_cast<WAVEFORMATEX*>(type->AllocFormatBuffer(sizeof(WAVEFORMATEX)));
		if (!wfx)
			return E_OUTOFMEMORY;
		ZeroMemory(wfx, type->cbFormat);

		//wfx->wFormatTag = WAVE_FORMAT_AAC;
		wfx->nChannels = channels;
		wfx->nSamplesPerSec = samplerate;
		wfx->wBitsPerSample = bitdepth;
		wfx->nBlockAlign = (wfx->nChannels*wfx->wBitsPerSample / 8);
		wfx->nAvgBytesPerSec = wfx->nSamplesPerSec*wfx->nBlockAlign;
		wfx->cbSize = 0;
		type->SetSampleSize(wfx->nBlockAlign);
		return S_OK;
	}
	else
	{
		return E_UNEXPECTED;
	}
	
}

HRESULT dk_rtmp_audio_source_stream::DecideBufferSize(IMemAllocator *alloc, ALLOCATOR_PROPERTIES *properties)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	HRESULT hr = NOERROR;

	CheckPointer(alloc, E_POINTER);
	CheckPointer(properties, E_POINTER);

	dk_rtmp_source_filter * parent = static_cast<dk_rtmp_source_filter*>(m_pFilter);

	int32_t samplerate = 0, channels = 0, bitdepth = 0;
	dk_media_buffering::instance().get_audio_samplerate(samplerate);
	dk_media_buffering::instance().get_audio_channels(channels);
	dk_media_buffering::instance().get_audio_bitdepth(bitdepth);

	//if (properties->cBuffers < 1)
	//	properties->cBuffers = 1;

	properties->cbBuffer = channels*samplerate*bitdepth / 8;//AVCODEC_MAX_AUDIO_FRAME_SIZE;
	properties->cBuffers = 1;



	ASSERT(properties->cbBuffer);

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

HRESULT dk_rtmp_audio_source_stream::FillBuffer(IMediaSample *ms)
{
	CheckPointer(ms, E_POINTER);
	dk_rtmp_source_filter * parent = static_cast<dk_rtmp_source_filter*>(m_pFilter);

	BYTE * buffer = NULL;
	size_t size_of_recvd = 0;
	long long timestamp = 0;
	ms->GetPointer(&buffer);

#if 1
	for (int i = 0; i < 1000 && parent->m_State != State_Stopped; i++)
	{
		dk_media_buffering::instance().pop_audio(buffer, size_of_recvd, timestamp);
		if (size_of_recvd > 0)
			break;
		::Sleep(1);
	}
#else
	dk_media_buffering::instance().pop_audio(buffer, size_of_recvd);
#endif

#if 0
	WAVEFORMATEX * wfx = (WAVEFORMATEX*)m_mt.Format();
	CRefTime rt_start = _rt_sample_time;
	_rt_sample_time = rt_start + (REFERENCE_TIME)(UNITS * ms->GetActualDataLength()) / (REFERENCE_TIME)wfx->nAvgBytesPerSec;
	ms->SetTime((REFERENCE_TIME*)&rt_start, (REFERENCE_TIME*)&_rt_sample_time);
#endif
	if (size_of_recvd > 0)
	{
		ms->SetSyncPoint(TRUE);
		ms->SetActualDataLength(size_of_recvd);
	}
	else
		ms->SetActualDataLength(0);

	return NOERROR;
}

//
// OnThreadCreate
//
HRESULT dk_rtmp_audio_source_stream::OnThreadCreate(void)
{
	//_is_first_sample_delivered = FALSE;
	//_sample_media_time_start = 0;
	return CSourceStream::OnThreadCreate();
}


//
// OnThreadDestroy
//
HRESULT dk_rtmp_audio_source_stream::OnThreadDestroy(void)
{
	return CSourceStream::OnThreadDestroy();
}


//
// OnThreadStartPlay
//
HRESULT dk_rtmp_audio_source_stream::OnThreadStartPlay(void)
{
	return CSourceStream::OnThreadStartPlay();
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_audio_source_stream::GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_audio_source_stream::GetPushSourceFlags(ULONG *pFlags)
{
	*pFlags = AM_PUSHSOURCECAPS_PRIVATE_CLOCK; return S_OK;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_audio_source_stream::GetStreamOffset(REFERENCE_TIME *prtOffset)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_audio_source_stream::SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_audio_source_stream::SetPushSourceFlags(ULONG Flags)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_audio_source_stream::SetStreamOffset(REFERENCE_TIME rtOffset)
{
	return E_NOTIMPL;
}

/// From IAMPushSource
STDMETHODIMP dk_rtmp_audio_source_stream::GetLatency(REFERENCE_TIME *prtLatency)
{
	return E_NOTIMPL;
	//TODO: set different latencies for audio and video
	//Set 450ms Latency 
	*prtLatency = 45000000; return S_OK;
}

DWORD dk_rtmp_audio_source_stream::ThreadProc(VOID)
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

HRESULT dk_rtmp_audio_source_stream::DoBufferProcessingLoop(VOID)
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
