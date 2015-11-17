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
#include <mmreg.h>
#include "dk_celt_encode_filter_properties.h"
#include "dk_celt_encode_filter.h"

//#pragma comment(lib, "wmcodecdspuuid.lib")

dk_celt_encode_filter::dk_celt_encode_filter(LPUNKNOWN unk, HRESULT *hr)
	: CTransformFilter(g_szFilterName, unk, CLSID_DK_CELT_ENCODE_FILTER)
	, _frame_size(20)
	, _got_time(false)
	//, _start_time(0)
{

	m_pInput = new CTransformInputPin(NAME("Input"), this, hr, L"In");
	m_pOutput = new CTransformOutputPin(NAME("Output"), this, hr, L"Out");


	_buffer = (short*)malloc(10 * 2 * 48000 * sizeof(short));
	_encoder = new dk_celt_encoder();
}

dk_celt_encode_filter::~dk_celt_encode_filter(VOID)
{
	if (_encoder)
	{
		delete _encoder;
		_encoder = nullptr;
	}
	if (_buffer)
	{
		free(_buffer);
		_buffer = nullptr;
	}
}

CUnknown * WINAPI dk_celt_encode_filter::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	*hr = S_OK;
	CUnknown* punk = new dk_celt_encode_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	return punk;
}

STDMETHODIMP dk_celt_encode_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
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

HRESULT  dk_celt_encode_filter::StartStreaming()
{
	//_got_time = false;
	//_start_time = 0;
	//_time_count = 0;
	return NOERROR;
}

HRESULT  dk_celt_encode_filter::StopStreaming()
{
	return NOERROR;
}

// override this to grab extra interfaces on connection
HRESULT  dk_celt_encode_filter::CheckConnect(PIN_DIRECTION direction, IPin *pin)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

// place holder to allow derived classes to release any extra interfaces
HRESULT  dk_celt_encode_filter::BreakConnect(PIN_DIRECTION direction)
{
	if (direction == PINDIR_INPUT)
	{
		if (_encoder)
			_encoder->release_encoder();
	}
	UNREFERENCED_PARAMETER(direction);
	return NOERROR;
}

// Let derived classes know about connection completion
HRESULT  dk_celt_encode_filter::CompleteConnect(PIN_DIRECTION direction, IPin *pin)
{
	if (direction == PINDIR_INPUT)
	{
		if (_encoder)
			_encoder->initialize_encoder(&_config);
		_frame_size = _frame_size * _config.samplerate / 1000;
		_frame_done = 0;
	}

	UNREFERENCED_PARAMETER(direction);	
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

HRESULT  dk_celt_encode_filter::AlterQuality(Quality quality)
{
	UNREFERENCED_PARAMETER(quality);
	return S_FALSE;
}

// EndOfStream received. Default behaviour is to deliver straight
// downstream, since we have no queued data. If you overrode Receive
// and have queue data, then you need to handle this and deliver EOS after
// all queued data is sent
HRESULT  dk_celt_encode_filter::EndOfStream(void)
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
HRESULT  dk_celt_encode_filter::BeginFlush(void)
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
HRESULT  dk_celt_encode_filter::EndFlush(void)
{
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins
	ASSERT(m_pOutput != NULL);
	return m_pOutput->DeliverEndFlush();
	// caller (the input pin's method) will unblock Receives
}


HRESULT  dk_celt_encode_filter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	if (m_pOutput != NULL)
	{
		return m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
	}
	return S_OK;
}

HRESULT  dk_celt_encode_filter::CheckInputType(const CMediaType * mt)
{
	if ((mt->majortype != MEDIATYPE_Audio) ||
		(mt->subtype != MEDIASUBTYPE_PCM) ||
		(mt->formattype != FORMAT_WaveFormatEx) ||
		(mt->cbFormat < sizeof(WAVEFORMATEX)))
	{
		return E_FAIL;
	}

	WAVEFORMATEX * wfx = (WAVEFORMATEX*)mt->pbFormat;
	if (!wfx)
		return E_FAIL;
	if (wfx->wFormatTag != WAVE_FORMAT_PCM && wfx->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
		return E_FAIL;
	if (wfx->wBitsPerSample != 16)
		return E_FAIL;
	if ((wfx->nChannels < 0) || (wfx->nChannels > 2))
		return E_FAIL;

#if 0
	switch (wfx->nSamplesPerSec)
	{
	case 48000:
	case 24000:
	case 16000:
	case 12000:
	case 8000:
		break;
	default:
		return E_FAIL;
	}
#endif

	return NOERROR;
}

HRESULT  dk_celt_encode_filter::GetMediaType(int position, CMediaType *type)
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
		type->SetSubtype(&MEDIASUBTYPE_CELT); //CELT(OPUS)
		type->bFixedSizeSamples = FALSE;
		type->SetTemporalCompression(FALSE);
		type->SetFormatType(&FORMAT_WaveFormatEx);
		type->AllocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));

		WAVEFORMATEX * wfx = (WAVEFORMATEX*)type->pbFormat;
		wfx->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfx->nChannels = _config.channels;
		wfx->nSamplesPerSec = _config.codingrate;
		wfx->wBitsPerSample = 16;
		wfx->nBlockAlign = wfx->nChannels * wfx->wBitsPerSample / 8;
		wfx->cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

		WAVEFORMATEXTENSIBLE * wfext = (WAVEFORMATEXTENSIBLE*)type->pbFormat;
		wfext->Samples.wReserved = 0;
		if (_config.channels == 1)
			wfext->dwChannelMask = SPEAKER_FRONT_CENTER;
		else
			wfext->dwChannelMask = SPEAKER_FRONT_LEFT && SPEAKER_FRONT_RIGHT;
		wfext->SubFormat = KSDATAFORMAT_SUBTYPE_CELT;
	}

	return S_OK;
}

HRESULT  dk_celt_encode_filter::CheckTransform(const CMediaType *itype, const CMediaType *otype)
{
	if (!IsEqualGUID(*(itype->Type()), MEDIATYPE_Audio) || !IsEqualGUID(*(otype->Type()), MEDIATYPE_Audio))
		return E_FAIL;

	if (!IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_CELT))
		return E_FAIL;

	return S_OK;
}

HRESULT  dk_celt_encode_filter::DecideBufferSize(IMemAllocator *allocator, ALLOCATOR_PROPERTIES *properties)
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
	WAVEFORMATEX * wfx = (WAVEFORMATEX*)mt.pbFormat;

	properties->cBuffers = 5;
	properties->cbBuffer = wfx->nSamplesPerSec * wfx->wBitsPerSample * wfx->nChannels / 8;
	properties->cbAlign = wfx->nBlockAlign;

	ALLOCATOR_PROPERTIES actual;
	hr = allocator->SetProperties(properties, &actual);

	if (FAILED(hr))
		return hr;

	if (properties->cBuffers>actual.cBuffers || properties->cbBuffer>actual.cbBuffer || properties->cbAlign>actual.cbAlign)
		return E_FAIL;

	return S_OK;
}

HRESULT dk_celt_encode_filter::SetMediaType(PIN_DIRECTION direction, const CMediaType * mt)
{
	if (direction == PINDIR_INPUT)
	{
		if ((mt->majortype != MEDIATYPE_Audio) ||
			(mt->subtype != MEDIASUBTYPE_PCM) ||
			(mt->formattype != FORMAT_WaveFormatEx) ||
			(mt->cbFormat < sizeof(WAVEFORMATEX)))
		{
			return E_FAIL;
		}

		WAVEFORMATEX * wfx = (WAVEFORMATEX*)mt->pbFormat;
		if (!wfx)
			return E_FAIL;
		if (wfx->wFormatTag != WAVE_FORMAT_PCM && wfx->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
			return E_FAIL;
		if (wfx->wBitsPerSample != 16)
			return E_FAIL;
		if ((wfx->nChannels < 0) || (wfx->nChannels > 2))
			return E_FAIL;

#if 0
		switch (wfx->nSamplesPerSec)
		{
		case 48000:
		case 24000:
		case 16000:
		case 12000:
		case 8000:
			break;
		default:
			return E_FAIL;
		}
		_config.channels = wfx->nChannels;
		_config.samplerate = wfx->nSamplesPerSec;
		_config.codingrate = wfx->nSamplesPerSec;
#else

		_config.channels = wfx->nChannels;
		_config.samplerate = wfx->nSamplesPerSec;

		if (_config.samplerate > 24000)
			_config.codingrate = 48000;
		else if (_config.samplerate > 16000)
			_config.codingrate = 24000;
		else if (_config.samplerate > 12000)
			_config.codingrate = 16000;
		else if (_config.samplerate > 8000)
			_config.codingrate = 12000;
		else
			_config.codingrate = 8000;
#endif
	}
	return NOERROR;
}

HRESULT dk_celt_encode_filter::Receive(IMediaSample *pSample)
{
	if (!_got_time) 
	{
		REFERENCE_TIME rt_begin, rt_end;
		HRESULT hr = pSample->GetTime(&rt_begin, &rt_end);
		if (hr == NOERROR) 
		{
			_rt_begin = rt_begin;
			_got_time = true;
		}
		else 
		{
			return NOERROR;
		}
	}

	if (!m_pOutput->IsConnected()) 
	{
		return NOERROR;
	}

	HRESULT	hr = NOERROR;
	BYTE	*buf;
	long	size;
	pSample->GetPointer(&buf);
	size = pSample->GetActualDataLength();

	short	*in_buf = (short*)buf;
	long	in_samples = size / sizeof(short);

	memcpy(_buffer + _samples, in_buf, in_samples * sizeof(short));
	_samples += in_samples;

	size_t outsize = 32 * 1024;
	BYTE	*outbuf = (BYTE*)malloc(outsize);

	int		cur_sample = 0;
	short	*cur_buf = _buffer;
	while (cur_sample + (_frame_size * _config.channels) <= _samples)
	{
		dk_audio_entity_t pcm = { cur_buf, _frame_size, 0 };
		dk_audio_entity_t encoded = { outbuf, outsize, 0 };
		dk_celt_encoder::ERR_CODE ret = _encoder->encode(&pcm, &encoded);
		if (ret == dk_celt_encoder::ERR_CODE_SUCCESS)
		{

			REFERENCE_TIME		rtStart, rtStop;
			IMediaSample		*sample = NULL;
			HRESULT				hr;

			hr = GetDeliveryBuffer(&sample);
			if (FAILED(hr)) 
				break;

			int	fs = _frame_size * _config.channels;

			rtStart = (_frame_done*fs * 10000000) / _config.samplerate;
			rtStop = ((((_frame_done + 1)*fs) - 1) * 10000000) / _config.samplerate;
			rtStart += _rt_begin;
			rtStop += _rt_begin;

			sample->SetTime(&rtStart, &rtStop);

			BYTE	*out;
			sample->GetPointer(&out);
			memcpy(out, outbuf, encoded.data_size);
			sample->SetActualDataLength(encoded.data_size);

			hr = m_pOutput->Deliver(sample);
			sample->Release();
			if (FAILED(hr)) 
				break;

			_frame_done++;
		}
		cur_sample += _frame_size * _config.channels;
		cur_buf += _frame_size * _config.channels;
	}

	free(outbuf);

	if (cur_sample > 0) 
	{
		int	left = _samples - cur_sample;
		if (left > 0) 
			memcpy(_buffer, _buffer + cur_sample, left*sizeof(short));
		_samples = left;
	}

	return hr;
}

HRESULT dk_celt_encode_filter::GetDeliveryBuffer(IMediaSample ** ms)
{
	IMediaSample * oms;
	HRESULT hr = m_pOutput->GetDeliveryBuffer(&oms, NULL, NULL, 0);
	*ms = oms;
	if (FAILED(hr)) 
		return hr;

	AM_MEDIA_TYPE *mt;
	if (oms->GetMediaType(&mt) == NOERROR) 
	{
		CMediaType _mt(*mt);
		SetMediaType(PINDIR_OUTPUT, &_mt);
		DeleteMediaType(mt);
	}
	return NOERROR;
}

/*HRESULT dk_celt_encode_filter::Transform(IMediaSample *src, IMediaSample *dst)
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
	output_data_size = dst->GetSize();
	if (input_data_size <= 0)
		return S_OK;
	hr = dst->GetPointer(&output_buffer);
	if (FAILED(hr))
		return S_OK;

	REFERENCE_TIME start_time, end_time;
	if (!_got_time)
	{
		hr = src->GetTime(&start_time, &end_time);
		if (hr == NOERROR) 
		{
			_rt_begin = start_time;
			_got_time = true;
		}
		else 
		{
			return NOERROR;
		}
	}

	CMediaType mt;
	hr = m_pOutput->ConnectionMediaType(&mt);
	WAVEFORMATEX * wfex = (WAVEFORMATEX*)mt.pbFormat;

	size_t framesize = input_data_size / (wfex->nChannels * sizeof(int16_t));
	//output_data_size = 3 * 1276;
	dk_celt_encoder::ERR_CODE result = _encoder->encode((int16_t*)input_buffer, framesize, output_buffer, output_data_size);

	if (output_data_size>0)
	{
		int		fs = _frame_size / _config.channels;
		start_time = (_frame_done*fs * 10000000) / _config.samplerate;
		end_time = ((((_frame_done + 1)*fs) - 1) * 10000000) / _config.samplerate;
		start_time += _rt_begin;
		end_time += _rt_begin;
		dst->SetTime(&start_time, &end_time);
	}
	_frame_done++;
	dst->SetActualDataLength(output_data_size);
	return S_OK;
}*/

STDMETHODIMP dk_celt_encode_filter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL)
		return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_DK_CELT_ENCODE_FILTER_PROPERTIES;
	return S_OK;
}
