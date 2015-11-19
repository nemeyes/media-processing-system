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
#include <wmcodecdsp.h>

#include <ks.h>
#include <ksmedia.h>

#include "dk_celt_decode_filter_properties.h"
#include "dk_celt_decode_filter.h"

#pragma comment(lib, "wmcodecdspuuid.lib")

#define WAVE_FORMAT_AAC 0x00FF

dk_celt_decode_filter::dk_celt_decode_filter(LPUNKNOWN unk, HRESULT *hr)
	: CTransformFilter(g_szFilterName, unk, CLSID_DK_CELT_DECODE_FILTER)
	, _got_time(false)
	, _start_time(0)
	, _time_count(0)
{
	m_pInput = new CTransformInputPin(NAME("Input"), this, hr, L"In");
	m_pOutput = new CTransformOutputPin(NAME("Output"), this, hr, L"Out");

	_decoder = new dk_celt_decoder();
}

dk_celt_decode_filter::~dk_celt_decode_filter(VOID)
{
	if (_decoder)
	{
		delete _decoder;
		_decoder = 0;
	}
}

CUnknown * WINAPI dk_celt_decode_filter::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	*hr = S_OK;
	CUnknown* punk = new dk_celt_decode_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	return punk;
}

STDMETHODIMP dk_celt_decode_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
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

HRESULT  dk_celt_decode_filter::StartStreaming()
{
	return NOERROR;
}

HRESULT  dk_celt_decode_filter::StopStreaming()
{
	return NOERROR;
}

// override this to grab extra interfaces on connection
HRESULT  dk_celt_decode_filter::CheckConnect(PIN_DIRECTION direction, IPin *pin)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

// place holder to allow derived classes to release any extra interfaces
HRESULT  dk_celt_decode_filter::BreakConnect(PIN_DIRECTION direction)
{
	if (direction==PINDIR_INPUT)
	{
		if (_decoder)
			_decoder->release_decoder();
	}
	UNREFERENCED_PARAMETER(direction);
	return NOERROR;
}

// Let derived classes know about connection completion
HRESULT  dk_celt_decode_filter::CompleteConnect(PIN_DIRECTION direction, IPin *pin)
{
	if (direction == PINDIR_INPUT)
	{
		if (_decoder)
		{	
			_decoder->initialize_decoder(&_config);
		}
	}

	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

HRESULT  dk_celt_decode_filter::AlterQuality(Quality quality)
{
	UNREFERENCED_PARAMETER(quality);
	return S_FALSE;
}

HRESULT  dk_celt_decode_filter::SetMediaType(PIN_DIRECTION direction, const CMediaType * mt)
{
	if (direction == PINDIR_INPUT)
	{
		WAVEFORMATEX * wfex = (WAVEFORMATEX *)mt->pbFormat;
		_config.channels = wfex->nChannels;
		_config.samplerate = wfex->nSamplesPerSec;
		_config.bitdepth = wfex->wBitsPerSample;
	}
	return NOERROR;
}

// EndOfStream received. Default behaviour is to deliver straight
// downstream, since we have no queued data. If you overrode Receive
// and have queue data, then you need to handle this and deliver EOS after
// all queued data is sent
HRESULT  dk_celt_decode_filter::EndOfStream(void)
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
HRESULT  dk_celt_decode_filter::BeginFlush(void)
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
HRESULT  dk_celt_decode_filter::EndFlush(void)
{
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins
	ASSERT(m_pOutput != NULL);
	return m_pOutput->DeliverEndFlush();
	// caller (the input pin's method) will unblock Receives
}


HRESULT  dk_celt_decode_filter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	if (m_pOutput != NULL)
	{
		return m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
	}
	return S_OK;
}

HRESULT  dk_celt_decode_filter::CheckInputType(const CMediaType *type)
{
	const GUID* id = type->Subtype();
	const char* media_type = (const char*)&type->Type()->Data1;
	const char* submedia_type = (const char*)&type->Subtype()->Data1;
	const GUID* formaType = type->FormatType();
	if (IsEqualGUID(*type->Type(), MEDIATYPE_Audio))
	{
		if (IsEqualGUID(*(type->Subtype()), MEDIASUBTYPE_CELT))
		{
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT  dk_celt_decode_filter::GetMediaType(int position, CMediaType *type)
{
	if (!m_pInput->IsConnected())
		return E_UNEXPECTED;

	if (position<0)
		return E_INVALIDARG;
	if (position>0)
		return VFW_S_NO_MORE_ITEMS;

	type->SetType(&MEDIATYPE_Audio);
	if (position == 0)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
			return hr;
		type->SetSubtype(&MEDIASUBTYPE_PCM);
		type->SetFormatType(&FORMAT_WaveFormatEx);
		type->bFixedSizeSamples = TRUE;
		type->SetTemporalCompression(FALSE);

#if 0
		type->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		WAVEFORMATEX * wfx = (WAVEFORMATEX*)type->pbFormat;
		memset(wfx, 0x00, sizeof(WAVEFORMATEX));
		wfx->wFormatTag = WAVE_FORMAT_PCM;
		wfx->nChannels = _config.channels;
		wfx->nSamplesPerSec = _config.samplerate;
		wfx->wBitsPerSample = _config.bitdepth;
		wfx->nBlockAlign = (wfx->nChannels * wfx->wBitsPerSample / 8);
		wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;
		//wfx->cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
#else
		type->AllocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));
		WAVEFORMATEX * wfx = (WAVEFORMATEX*)type->pbFormat;
		memset(wfx, 0x00, sizeof(WAVEFORMATEX));
		wfx->wFormatTag = WAVE_FORMAT_PCM;
		wfx->nChannels = _config.channels;
		wfx->nSamplesPerSec = _config.samplerate;
		wfx->wBitsPerSample = _config.bitdepth;
		wfx->nBlockAlign = (wfx->nChannels * wfx->wBitsPerSample / 8);
		wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;
		wfx->cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

		WAVEFORMATEXTENSIBLE * wfext = (WAVEFORMATEXTENSIBLE*)type->pbFormat;
		wfext->Samples.wValidBitsPerSample = wfext->Format.wBitsPerSample;
		wfext->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		wfext->Format.wFormatTag = (_config.channels <= 2) ? WAVE_FORMAT_PCM : WAVE_FORMAT_EXTENSIBLE;
		switch (_config.channels)
		{
		case 1:
			wfext->dwChannelMask = KSAUDIO_SPEAKER_MONO;
			break;
		case 2:
			wfext->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
			break;
		case 3:
			wfext->dwChannelMask = KSAUDIO_SPEAKER_STEREO | SPEAKER_FRONT_CENTER;
			break;
		case 4:
			//wfex.dwChannelMask = KSAUDIO_SPEAKER_QUAD;
			wfext->dwChannelMask = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER);
			break;
		case 5:
			wfext->dwChannelMask = KSAUDIO_SPEAKER_QUAD | SPEAKER_FRONT_CENTER;
			break;
		case 6:
			wfext->dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
			break;
		default:
			wfext->dwChannelMask = KSAUDIO_SPEAKER_DIRECTOUT; // XXX : or SPEAKER_ALL ??
			break;
		}
		
#endif
	}
	return S_OK;
}

HRESULT  dk_celt_decode_filter::CheckTransform(const CMediaType *itype, const CMediaType *otype)
{
	if (!IsEqualGUID(*(itype->Type()), MEDIATYPE_Audio) || !IsEqualGUID(*(otype->Type()), MEDIATYPE_Audio))
		return VFW_E_TYPE_NOT_ACCEPTED;

	if (!IsEqualGUID(*itype->Subtype(), MEDIASUBTYPE_CELT))
		return VFW_E_TYPE_NOT_ACCEPTED;

	if (!IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_PCM))
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

HRESULT  dk_celt_decode_filter::DecideBufferSize(IMemAllocator *allocator, ALLOCATOR_PROPERTIES *properties)
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

	CMediaType mt = m_pInput->CurrentMediaType();
	WAVEFORMATEX * wfex = (WAVEFORMATEX *)mt.pbFormat;

	properties->cBuffers = 5;
	properties->cbBuffer = wfex->nSamplesPerSec * wfex->wBitsPerSample * wfex->nChannels / 8;
	properties->cbAlign = wfex->nBlockAlign;

	ALLOCATOR_PROPERTIES actual;
	hr = allocator->SetProperties(properties, &actual);

	if (FAILED(hr))
		return hr;

	if (properties->cBuffers>actual.cBuffers || properties->cbBuffer>actual.cbBuffer || properties->cbAlign>actual.cbAlign)
		return E_FAIL;

	return S_OK;
}

HRESULT dk_celt_decode_filter::Transform(IMediaSample *src, IMediaSample *dst)
{
	if (m_State == State_Stopped)
	{
		dst->SetActualDataLength(0);
		return S_OK;
	}

	HRESULT hr = S_OK;
	BYTE *input_buffer = NULL;
	UINT input_data_size = 0;
	BYTE *output_buffer = NULL;
	UINT output_data_size = 0;

	input_data_size = src->GetActualDataLength();
	output_data_size = dst->GetSize();
	if (input_data_size <= 0)
		return S_FALSE;
	//if (output_data_size < (_config.channels * 2048 * sizeof(short)))
	//	return S_FALSE;

	hr = src->GetPointer(&input_buffer);
	if (FAILED(hr) || !input_buffer)
		return S_FALSE;
	hr = dst->GetPointer(&output_buffer);
	if (FAILED(hr) || !output_buffer)
		return S_FALSE;

	//REFERENCE_TIME start_time, end_time;
	//hr = src->GetTime(&start_time, &end_time);
	//if (hr == NOERROR)
	//{
	//	_start_time = start_time;
	//	_got_time = true;
	//}
	//else
	//{
	//	return NOERROR;
	//}

	CMediaType mt;
	hr = m_pOutput->ConnectionMediaType(&mt);
	WAVEFORMATEX * wfex = (WAVEFORMATEX*)mt.pbFormat;

	//size_t framesize = output_data_size / (wfex->nChannels * sizeof(int16_t));
	//dk_celt_decoder::ERR_CODE result = _decoder->decode(input_buffer, input_data_size, output_buffer, framesize);
	dk_audio_entity_t encoded = {input_buffer, input_data_size, 0};
	dk_audio_entity_t pcm = { output_buffer, 0, output_data_size };
	dk_celt_decoder::ERR_CODE result = _decoder->decode(&encoded, &pcm);

	if (result != dk_celt_decoder::ERR_CODE_SUCCESS)
		return S_FALSE;

	if (pcm.data_size>0)
	{
		//double duration = (double)framesize / (_config.bitdepth*_config.samplerate) * 10000000.0;
		//start_time = end_time;
		//end_time = start_time+(duration + 0.5);

		//dst->SetTime(&start_time, &end_time);

		dst->SetSyncPoint(TRUE);
		dst->SetActualDataLength(pcm.data_size);
	}
	else
	{
		dst->SetActualDataLength(0);
	}
	//dst->SetDiscontinuity(TRUE);
	//dst->SetMediaTime(NULL, NULL);
	//dst->SetPreroll(FALSE);
	
	
	return S_OK;
}

STDMETHODIMP dk_celt_decode_filter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL)
		return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_DK_CELT_DECODE_FILTER_PROPERTIES;
	return S_OK;
}
