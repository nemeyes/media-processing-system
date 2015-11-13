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

#include "dk_aac_decode_filter_properties.h"
#include "dk_aac_decode_filter.h"

#pragma comment(lib, "wmcodecdspuuid.lib")

#define WAVE_FORMAT_AAC 0x00FF

dk_aac_decode_filter::dk_aac_decode_filter(LPUNKNOWN unk, HRESULT *hr)
	: CTransformFilter(g_szFilterName, unk, CLSID_DK_AAC_DECODE_FILTER)
	, _extra_data_size(0)
	, _got_time(false)
	, _start_time(0)
	, _time_count(0)
{
	memset(_extra_data, 0x00, sizeof(_extra_data));
	_decoder = new dk_aac_decoder();
}

dk_aac_decode_filter::~dk_aac_decode_filter(VOID)
{
	if (_decoder)
	{
		delete _decoder;
		_decoder = 0;
	}
}

CUnknown * WINAPI dk_aac_decode_filter::CreateInstance(LPUNKNOWN unk, HRESULT *hr)
{
	*hr = S_OK;
	CUnknown* punk = new dk_aac_decode_filter(unk, hr);
	if (punk == NULL)
		*hr = E_OUTOFMEMORY;
	return punk;
}

STDMETHODIMP dk_aac_decode_filter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
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

HRESULT  dk_aac_decode_filter::StartStreaming()
{
	return NOERROR;
}

HRESULT  dk_aac_decode_filter::StopStreaming()
{
	return NOERROR;
}

// override this to grab extra interfaces on connection
HRESULT  dk_aac_decode_filter::CheckConnect(PIN_DIRECTION direction, IPin *pin)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

// place holder to allow derived classes to release any extra interfaces
HRESULT  dk_aac_decode_filter::BreakConnect(PIN_DIRECTION direction)
{
	if (direction==PINDIR_INPUT)
	{
		if (_decoder)
			_decoder->release();
	}
	UNREFERENCED_PARAMETER(direction);
	return NOERROR;
}

// Let derived classes know about connection completion
HRESULT  dk_aac_decode_filter::CompleteConnect(PIN_DIRECTION direction, IPin *pin)
{
	if (direction == PINDIR_INPUT)
	{
		if (_decoder)
		{	
			int channels, samplerate;
			_decoder->initialize(_config, _extra_data, _extra_data_size, samplerate, channels);
			_config.samplerate = samplerate;
			_config.channels = channels;
		}
	}

	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pin);
	return NOERROR;
}

HRESULT  dk_aac_decode_filter::AlterQuality(Quality quality)
{
	UNREFERENCED_PARAMETER(quality);
	return S_FALSE;
}

HRESULT  dk_aac_decode_filter::SetMediaType(PIN_DIRECTION direction, const CMediaType *type)
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(type);
	return NOERROR;
}

// EndOfStream received. Default behaviour is to deliver straight
// downstream, since we have no queued data. If you overrode Receive
// and have queue data, then you need to handle this and deliver EOS after
// all queued data is sent
HRESULT  dk_aac_decode_filter::EndOfStream(void)
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
HRESULT  dk_aac_decode_filter::BeginFlush(void)
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
HRESULT  dk_aac_decode_filter::EndFlush(void)
{
	// sync with pushing thread -- we have no worker thread
	// ensure no more data to go downstream -- we have no queued data
	// call EndFlush on downstream pins
	ASSERT(m_pOutput != NULL);
	return m_pOutput->DeliverEndFlush();
	// caller (the input pin's method) will unblock Receives
}


HRESULT  dk_aac_decode_filter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	if (m_pOutput != NULL)
	{
		return m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
	}
	return S_OK;
}

HRESULT  dk_aac_decode_filter::CheckInputType(const CMediaType *type)
{
	const GUID* id = type->Subtype();
	const char* media_type = (const char*)&type->Type()->Data1;
	const char* submedia_type = (const char*)&type->Subtype()->Data1;
	const GUID* formaType = type->FormatType();
	if (IsEqualGUID(*type->Type(), MEDIATYPE_Audio))
	{
		if (IsEqualGUID(*(type->Subtype()), MEDIASUBTYPE_RAW_AAC1))
		{
			if (IsEqualGUID(*(formaType), FORMAT_WaveFormatEx))
			{
				WAVEFORMATEX * wavhdr = reinterpret_cast<WAVEFORMATEX*>(type->Format());
				if (wavhdr->wFormatTag != WAVE_FORMAT_AAC)
					return VFW_E_TYPE_NOT_ACCEPTED;

				if (wavhdr->cbSize < 2)
					return VFW_E_TYPE_NOT_ACCEPTED;

				_extra_data_size = wavhdr->cbSize;
				memcpy(_extra_data, (char*)wavhdr + sizeof(WAVEFORMATEX), _extra_data_size);
				return S_OK;
			}
		}
	}
	return E_FAIL;
}

HRESULT  dk_aac_decode_filter::GetMediaType(int position, CMediaType *type)
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
		type->SetSubtype(&MEDIASUBTYPE_PCM); //RAW AAC
		type->SetFormatType(&FORMAT_WaveFormatEx);
		type->SetTemporalCompression(FALSE);

		//type->Format();

		WAVEFORMATEXTENSIBLE wfex;
		memset(&wfex, 0x00, sizeof(wfex));
		wfex.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		wfex.Format.wFormatTag = (_config.channels <= 2) ? WAVE_FORMAT_PCM : WAVE_FORMAT_EXTENSIBLE;
		wfex.Format.cbSize = (_config.channels <= 2) ? 0 : sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		wfex.Format.nChannels = (unsigned short)_config.channels;
		wfex.Format.nSamplesPerSec = (unsigned short)_config.samplerate;
		wfex.Format.wBitsPerSample = _config.bitpersamples;
		wfex.Format.nBlockAlign = (unsigned short)((wfex.Format.nChannels * wfex.Format.wBitsPerSample) / 8);
		wfex.Format.nAvgBytesPerSec = wfex.Format.nSamplesPerSec * wfex.Format.nBlockAlign;
		switch (_config.channels)
		{
		case 1:
			wfex.dwChannelMask = KSAUDIO_SPEAKER_MONO;
			break;
		case 2:
			wfex.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
			break;
		case 3:
			wfex.dwChannelMask = KSAUDIO_SPEAKER_STEREO | SPEAKER_FRONT_CENTER;
			break;
		case 4:
			//wfex.dwChannelMask = KSAUDIO_SPEAKER_QUAD;
			wfex.dwChannelMask = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER);
			break;
		case 5:
			wfex.dwChannelMask = KSAUDIO_SPEAKER_QUAD | SPEAKER_FRONT_CENTER;
			break;
		case 6:
			wfex.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
			break;
		default:
			wfex.dwChannelMask = KSAUDIO_SPEAKER_DIRECTOUT; // XXX : or SPEAKER_ALL ??
			break;
		}
		wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;
		type->SetFormat((BYTE*)&wfex, sizeof(WAVEFORMATEX) + wfex.Format.cbSize);
	}
	return S_OK;
}

HRESULT  dk_aac_decode_filter::CheckTransform(const CMediaType *itype, const CMediaType *otype)
{
	if (!IsEqualGUID(*(itype->Type()), MEDIATYPE_Audio) || !IsEqualGUID(*(otype->Type()), MEDIATYPE_Audio))
		return VFW_E_TYPE_NOT_ACCEPTED;

	if (!IsEqualGUID(*(otype->Subtype()), MEDIASUBTYPE_PCM))
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

HRESULT  dk_aac_decode_filter::DecideBufferSize(IMemAllocator *allocator, ALLOCATOR_PROPERTIES *properties)
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

	// 960 for LD or else 1024 (expanded to 2048 for HE-AAC)
	// Buffer Size for decoded PCM: 1s of 192kHz 32-bit with 8 channels
	// 192000 (Samples) * 4 (Bytes per Sample) * 8 (channels)
	//#define LAV_AUDIO_BUFFER_SIZE 6144000
	properties->cBuffers = 4;//8;
	//properties->cbAlign		= 1;
	properties->cbBuffer = 6144000;// _config.channels * 2048 * sizeof(short);
	properties->cbPrefix = 0;

	ALLOCATOR_PROPERTIES actual;
	hr = allocator->SetProperties(properties, &actual);

	if (FAILED(hr))
		return hr;

	if (properties->cBuffers>actual.cBuffers || properties->cbBuffer>actual.cbBuffer || properties->cbAlign>actual.cbAlign)
		return E_FAIL;

	return S_OK;
}

HRESULT dk_aac_decode_filter::Transform(IMediaSample *src, IMediaSample *dst)
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
	if (output_data_size < (_config.channels * 2048 * sizeof(short)))
		return S_FALSE;

	hr = src->GetPointer(&input_buffer);
	if (FAILED(hr) || !input_buffer)
		return S_FALSE;
	hr = dst->GetPointer(&output_buffer);
	if (FAILED(hr) || !output_buffer)
		return S_FALSE;

	REFERENCE_TIME start_time, end_time;
	//if (!_got_time)
	//{
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
	//}

	dk_aac_decoder::ERR_CODE result = _decoder->decode(input_buffer, input_data_size, output_buffer, output_data_size);

	if (result != dk_aac_decoder::ERR_CODE_SUCCESS)
		return S_FALSE;

	if (output_data_size>0)
	{
		double duration = (double)output_data_size / (_config.bitpersamples*_config.samplerate) * 10000000.0;
		start_time = end_time;
		end_time = start_time+(duration + 0.5);

		//start_time += _start_time;
		//end_time += _start_time;
		dst->SetTime(&start_time, &end_time);
	}
	//dst->SetDiscontinuity(TRUE);
	dst->SetMediaTime(NULL, NULL);
	dst->SetPreroll(FALSE);
	dst->SetSyncPoint(TRUE);
	dst->SetActualDataLength(output_data_size);
	return S_OK;
}

STDMETHODIMP dk_aac_decode_filter::GetPages(CAUUID *pPages)
{
	if (pPages == NULL)
		return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL)
		return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_DK_AAC_DECODE_FILTER_PROPERTIES;
	return S_OK;
}
