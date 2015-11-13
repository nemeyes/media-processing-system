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
#include "dk_aac_encode_filter_properties.h"
#include "dk_aac_encode_filter.h"

#pragma comment(lib, "wmcodecdspuuid.lib")

#define WAVE_FORMAT_AAC 0x00FF

dk_aac_encode_filter::dk_aac_encode_filter(LPUNKNOWN unk, HRESULT *hr)
	: CTransformFilter(g_szFilterName, unk, CLSID_DK_AAC_ENCODE_FILTER)
	, _frame_size(0)
	//, _max_output_bytes(0)
	, _extra_data_size(0)
	, _got_time(false)
	//, _start_time(0)
{

	m_pInput = new CTransformInputPin(NAME("Input"), this, hr, L"In");
	m_pOutput = new CTransformOutputPin(NAME("Output"), this, hr, L"Out");


	_buffer = (short*)malloc(10 * 2 * 48000 * sizeof(short));
	memset(_extra_data, 0x00, sizeof(_extra_data));
	_encoder = new dk_aac_encoder();
}

dk_aac_encode_filter::~dk_aac_encode_filter(VOID)
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

CUnknown * WINAPI dk_aac_encode_filter::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	*hr = S_OK;
	CUnknown* punk = new dk_aac_encode_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	return punk;
}

STDMETHODIMP dk_aac_encode_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
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

HRESULT  dk_aac_encode_filter::StartStreaming()
{
	//_got_time = false;
	//_start_time = 0;
	//_time_count = 0;
	return NOERROR;
}

HRESULT  dk_aac_encode_filter::StopStreaming()
{
	return NOERROR;
}

// override this to grab extra interfaces on connection
HRESULT  dk_aac_encode_filter::CheckConnect(PIN_DIRECTION direction, IPin *pin)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

// place holder to allow derived classes to release any extra interfaces
HRESULT  dk_aac_encode_filter::BreakConnect(PIN_DIRECTION direction)
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
HRESULT  dk_aac_encode_filter::CompleteConnect(PIN_DIRECTION direction, IPin *pin)
{
	if (direction == PINDIR_INPUT)
	{
		unsigned long ob;
		if (_encoder)
			_encoder->initialize(_config, _frame_size, ob, _extra_data, _extra_data_size);

		_frame_done = 0;
	}

	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

HRESULT  dk_aac_encode_filter::AlterQuality(Quality quality)
{
	UNREFERENCED_PARAMETER(quality);
	return S_FALSE;
}

// EndOfStream received. Default behaviour is to deliver straight
// downstream, since we have no queued data. If you overrode Receive
// and have queue data, then you need to handle this and deliver EOS after
// all queued data is sent
HRESULT  dk_aac_encode_filter::EndOfStream(void)
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
HRESULT  dk_aac_encode_filter::BeginFlush(void)
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
HRESULT  dk_aac_encode_filter::EndFlush(void)
{
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins
	ASSERT(m_pOutput != NULL);
	return m_pOutput->DeliverEndFlush();
	// caller (the input pin's method) will unblock Receives
}


HRESULT  dk_aac_encode_filter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	if (m_pOutput != NULL)
	{
		return m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
	}
	return S_OK;
}

HRESULT  dk_aac_encode_filter::CheckInputType(const CMediaType * mt)
{
	if (mt->majortype != MEDIATYPE_Audio) 
		return E_FAIL;
	if (mt->subtype != MEDIASUBTYPE_PCM) 
		return E_FAIL;
	if (mt->formattype != FORMAT_WaveFormatEx) 
		return E_FAIL;

	return NOERROR;

	/*const GUID* id = type->Subtype();
	const char* media_type = (const char*)&type->Type()->Data1;
	const char* submedia_type = (const char*)&type->Subtype()->Data1;
	const GUID* formaType = type->FormatType();
	if (IsEqualGUID(*type->Type(), MEDIATYPE_Audio))
	{
		if (IsEqualGUID(*(type->Subtype()), MEDIASUBTYPE_PCM))
		{
			if (IsEqualGUID(*(formaType), FORMAT_WaveFormatEx))
			{
				WAVEFORMATEX * wavhdr = reinterpret_cast<WAVEFORMATEX*>(type->Format());
				_config.channels = wavhdr->nChannels;
				_config.sample_rate = wavhdr->nSamplesPerSec;
				_config.bitpersamples = wavhdr->wBitsPerSample;
				switch (_config.bitpersamples)
				{
				case 16 :
					_config.input_format = dk_aac_encoder::FORMAT_TYPE_16BIT;
					break;
				case 24 :
					_config.input_format = dk_aac_encoder::FORMAT_TYPE_24BIT;
					break;
				case 32 :
					_config.input_format = dk_aac_encoder::FORMAT_TYPE_32BIT;
					break;
				default :
					_config.input_format = dk_aac_encoder::FORMAT_TYPE_16BIT;
				}
				return S_OK;
			}
		}
	}
	return E_FAIL;*/
}

HRESULT  dk_aac_encode_filter::GetMediaType(int position, CMediaType *type)
{
	if (!m_pInput->IsConnected())
		return E_UNEXPECTED;

	if (position<0)
		return E_INVALIDARG;

#if 0
	if (position>1)
		return VFW_S_NO_MORE_ITEMS;

	type->SetType(&MEDIATYPE_Audio);
	if (position == 0)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
			return hr;
		type->SetSubtype(&MEDIASUBTYPE_RAW_AAC1); //RAW AAC
		type->SetFormatType(&FORMAT_WaveFormatEx);
		WAVEFORMATEX * wfx = (WAVEFORMATEX*)type->AllocFormatBuffer(sizeof(*wfx) + _extra_data_size);
		memset(wfx, 0x00, sizeof(*wfx));
		wfx->cbSize = _extra_data_size;
		wfx->nChannels = _config.channels;
		wfx->nSamplesPerSec = _config.sample_rate;
		wfx->wFormatTag = WAVE_FORMAT_AAC;
		unsigned char * wfxex = ((unsigned char*)(wfx)) + sizeof(*wfx);
		memcpy(wfxex, _extra_data, _extra_data_size);
		_config.output_format = dk_aac_encoder::FORMAT_TYPE_RAW;
	}
	else if (position == 1)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
			return hr;
		type->SetSubtype(&MEDIASUBTYPE_MPEG_ADTS_AAC); //ADTS AAC
		type->SetFormatType(&FORMAT_WaveFormatEx);
		WAVEFORMATEX * wfx = (WAVEFORMATEX*)type->AllocFormatBuffer(sizeof(*wfx) + _extra_data_size);
		memset(wfx, 0x00, sizeof(*wfx));
		wfx->cbSize = _extra_data_size;
		wfx->nChannels = _config.channels;
		wfx->nSamplesPerSec = _config.sample_rate;
		wfx->wFormatTag = WAVE_FORMAT_AAC;
		unsigned char * wfxex = ((unsigned char*)(wfx)) + sizeof(*wfx);
		memcpy(wfxex, _extra_data, _extra_data_size);
		_config.output_format = dk_aac_encoder::FORMAT_TYPE_ADTS;
	}
#else
	if (position>0)
		return VFW_S_NO_MORE_ITEMS;

	type->SetType(&MEDIATYPE_Audio);
	if (position == 0)
	{
		HRESULT hr = m_pInput->ConnectionMediaType(type);
		if (FAILED(hr))
			return hr;
		type->SetSubtype(&MEDIASUBTYPE_RAW_AAC1); //RAW AAC
		type->SetFormatType(&FORMAT_WaveFormatEx);
		WAVEFORMATEX * wfx = (WAVEFORMATEX*)type->AllocFormatBuffer(sizeof(*wfx) + _extra_data_size);
		memset(wfx, 0x00, sizeof(*wfx));
		wfx->cbSize = _extra_data_size;
		wfx->nChannels = _config.channels;
		wfx->nSamplesPerSec = _config.samplerate;
		wfx->wFormatTag = WAVE_FORMAT_AAC;
		unsigned char * wfxex = ((unsigned char*)(wfx)) + sizeof(*wfx);
		memcpy(wfxex, _extra_data, _extra_data_size);
		_config.output_format = dk_aac_encoder::FORMAT_TYPE_RAW;
	}
#endif
	return S_OK;
}

HRESULT  dk_aac_encode_filter::CheckTransform(const CMediaType *itype, const CMediaType *otype)
{
	if (!IsEqualGUID(*(itype->Type()), MEDIATYPE_Audio) || !IsEqualGUID(*(otype->Type()), MEDIATYPE_Audio))
		return E_FAIL;

	if (!IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_RAW_AAC1) && !IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_MPEG_HEAAC))
		return E_FAIL;

	return S_OK;
}

HRESULT  dk_aac_encode_filter::DecideBufferSize(IMemAllocator *allocator, ALLOCATOR_PROPERTIES *properties)
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

	properties->cBuffers = 2;
	//properties->cbAlign		= 1;
	properties->cbBuffer = 4 * 8 * 2 * _config.channels * 1024;
	//properties->cbPrefix = 0;

	ALLOCATOR_PROPERTIES actual;
	hr = allocator->SetProperties(properties, &actual);

	if (FAILED(hr))
		return hr;

	if (properties->cBuffers>actual.cBuffers || properties->cbBuffer>actual.cbBuffer || properties->cbAlign>actual.cbAlign)
		return E_FAIL;

	return S_OK;
}

HRESULT dk_aac_encode_filter::SetMediaType(PIN_DIRECTION direction, const CMediaType * mt)
{
	if (direction==PINDIR_INPUT)
	{
		WAVEFORMATEX * wfx = (WAVEFORMATEX*)mt->pbFormat;
		if (!wfx) 
			return E_FAIL;

		//if (wfx->wBitsPerSample != 16) 
		//	return E_FAIL;
		_config.channels = wfx->nChannels;
		_config.samplerate = wfx->nSamplesPerSec;
		_config.bitpersamples = wfx->wBitsPerSample;
		switch (_config.bitpersamples)
		{
		case 16:
			_config.input_format = dk_aac_encoder::FORMAT_TYPE_16BIT;
			break;
		case 24:
			_config.input_format = dk_aac_encoder::FORMAT_TYPE_24BIT;
			break;
		case 32:
			_config.input_format = dk_aac_encoder::FORMAT_TYPE_32BIT;
			break;
		default:
			_config.input_format = dk_aac_encoder::FORMAT_TYPE_16BIT;
		}
	}

	return NOERROR;
}

HRESULT dk_aac_encode_filter::Receive(IMediaSample *pSample)
{
	if (!_got_time) 
	{
		// odchytime si casy
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

	/**************************************************************************
	**
	**	Enkodovanie packetov
	**
	***************************************************************************/

	size_t outsize = 32 * 1024;
	BYTE	*outbuf = (BYTE*)malloc(outsize);

	int		cur_sample = 0;
	short	*cur_buf = _buffer;
	while (cur_sample + _frame_size <= _samples)
	{
		// dame zakodovat frame
		size_t bytes_written = 0;
		dk_aac_encoder::ERR_CODE ret = _encoder->encode((int*)cur_buf, _frame_size, outbuf, outsize, bytes_written);//faacEncEncode(encoder, (int*)cur_buf, info.frame_size, outbuf, outsize);
		if (ret == dk_aac_encoder::ERR_CODE_SUCCESS) 
		{

			REFERENCE_TIME		rtStart, rtStop;
			IMediaSample		*sample = NULL;
			HRESULT				hr;

			// dorucime data
			hr = GetDeliveryBuffer(&sample);
			if (FAILED(hr)) 
				break;

			// spocitame timestampy
			int		fs = _frame_size / _config.channels;

			rtStart = (_frame_done*fs * 10000000) / _config.samplerate;
			rtStop = ((((_frame_done + 1)*fs) - 1) * 10000000) / _config.samplerate;
			rtStart += _rt_begin;
			rtStop += _rt_begin;

			sample->SetTime(&rtStart, &rtStop);

			// napiseme data
			BYTE	*out;
			sample->GetPointer(&out);
			memcpy(out, outbuf, bytes_written);
			sample->SetActualDataLength(bytes_written);

			hr = m_pOutput->Deliver(sample);
			sample->Release();
			if (FAILED(hr)) 
				break;

			_frame_done++;
		}

		// na dalsi frame
		cur_sample += _frame_size;
		cur_buf += _frame_size;
	}

	free(outbuf);

	// discardujeme stare data
	if (cur_sample > 0) 
	{
		int	left = _samples - cur_sample;
		if (left > 0) 
			memcpy(_buffer, _buffer + cur_sample, left*sizeof(short));
		_samples = left;
	}

	return hr;
}

HRESULT dk_aac_encode_filter::GetDeliveryBuffer(IMediaSample ** ms)
{
	IMediaSample * oms;
	HRESULT hr = m_pOutput->GetDeliveryBuffer(&oms, NULL, NULL, 0);
	*ms = oms;
	if (FAILED(hr)) 
		return hr;

	// ak sa zmenil type, tak aktualizujeme nase info
	AM_MEDIA_TYPE *mt;
	if (oms->GetMediaType(&mt) == NOERROR) 
	{
		CMediaType _mt(*mt);
		SetMediaType(PINDIR_OUTPUT, &_mt);
		DeleteMediaType(mt);
	}
	return NOERROR;
}


/*HRESULT dk_aac_encode_filter::Transform(IMediaSample *src, IMediaSample *dst)
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
	if (!_got_time)
	{
		hr = src->GetTime(&start_time, &end_time);
		if (hr == NOERROR) 
		{
			_start_time = start_time;
			_got_time = true;
		}
		else 
		{
			return NOERROR;
		}
	}

	int64_t frame_done = 0;
	dk_aac_encoder::ERR_CODE result = _encoder->encode(input_buffer, input_data_size, output_buffer, output_data_size, frame_done);

	if (output_data_size>0)
	{
		int fs = _frame_size / _config.channels;
		start_time = (frame_done * fs * 10000000) / _config.samplerate;
		end_time = ((((frame_done + 1)*fs) - 1) * 10000000) / _config.samplerate;
		start_time += _start_time;
		end_time += _start_time;
		dst->SetTime(&start_time, &end_time);
	}
	dst->SetActualDataLength(output_data_size);
	return S_OK;
}*/

STDMETHODIMP dk_aac_encode_filter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL)
		return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_DK_AAC_ENCODE_FILTER_PROPERTIES;
	return S_OK;
}
