#include "mmwave_renderer.h"
#pragma comment(lib, "winmm")

DWORD debuggerking::mmwave_core::_volume = 30;

debuggerking::mmwave_core::mmwave_core(void)
{
	_wave_out = nullptr;
	_waveout_device_id = WAVE_MAPPER;
	_current_position = 0;
	_buffer_size = 0;
	_per_data_size = 0;

	init_buffer(48, 8192);
	InitializeCriticalSection(&_cs);
	ZeroMemory(&_wfx, sizeof(_wfx));
}

debuggerking::mmwave_core::~mmwave_core(void)
{
	end_audio_render();
	clear_buffer();
	DeleteCriticalSection(&_cs);
}

int32_t debuggerking::mmwave_core::initialize_renderer(mmwave_renderer::configuration_t * config)
{
	int32_t value = mmwave_renderer::err_code_t::fail;
	BOOL ret = begin_audio_render(config->samplerate, config->bitdepth, config->channels);
	if (ret)
		value = mmwave_renderer::err_code_t::success;
	return value;
}

int32_t debuggerking::mmwave_core::release_renderer(void)
{
	end_audio_render();
	return mmwave_renderer::err_code_t::success;
}

int32_t debuggerking::mmwave_core::render(mmwave_renderer::entity_t * pcm)
{
	set_buffer(pcm->data, pcm->data_size);
	return mmwave_renderer::err_code_t::success;
}

BOOL debuggerking::mmwave_core::begin_audio_render(DWORD samplerate, WORD bitdepth, WORD channels)
{
	if (samplerate == 0 || bitdepth == 0)
		return FALSE;

	BOOL bResult = TRUE;
	end_audio_render();

	ZeroMemory(&_wfx, sizeof(_wfx));
	_wfx.wFormatTag = WAVE_FORMAT_PCM;
	_wfx.nChannels = channels;
	_wfx.nSamplesPerSec = samplerate;
	_wfx.wBitsPerSample = bitdepth;
	_wfx.nBlockAlign = _wfx.wBitsPerSample / 8 * _wfx.nChannels;
	_wfx.nAvgBytesPerSec = _wfx.nSamplesPerSec * _wfx.nBlockAlign;

	MMRESULT mr = NOERROR;
	//mr = waveOutOpen(&m_hWaveOut, m_nWaveoutDeviceID, &m_wfx, NULL, 0, 0);
	mr = waveOutOpen(&_wave_out, _waveout_device_id, &_wfx, (DWORD_PTR)speaker_callback, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if (mr == MMSYSERR_NOERROR)
	{
		get_volume();
	}
	return bResult;
}

void debuggerking::mmwave_core::end_audio_render(void)
{
	if (!_wave_out)
		return;

	__try
	{
		EnterCriticalSection(&_cs);

		MMRESULT mr;
		mr = waveOutReset(_wave_out);
		if (mr != MMSYSERR_NOERROR)
			return;

		AUDIOSINK_WAVEHDR_T *phdr;
		for (int i = 0; i<_buffer_size; i++)
		{
			phdr = _wave_hdr + i;
			waveOutUnprepareHeader(_wave_out, &phdr->waveHdr, sizeof(WAVEHDR));
		}
		mr = waveOutClose(_wave_out);
		_wave_out = nullptr;
	}
	__finally
	{
		_current_position = 0;
		LeaveCriticalSection(&_cs);
	}
}

void debuggerking::mmwave_core::init_buffer(int buffer_size, int data_size)
{
	clear_buffer();
	_wave_hdr = new AUDIOSINK_WAVEHDR_T[buffer_size];
	ZeroMemory(_wave_hdr, sizeof(AUDIOSINK_WAVEHDR_T) * buffer_size);

	AUDIOSINK_WAVEHDR_T *phdr;

	for (int i = 0; i<buffer_size; i++)
	{
		phdr = _wave_hdr + i;
		phdr->waveHdr.lpData = new char[data_size];
	}
	_buffer_size = buffer_size;
	_per_data_size = data_size;
}

void debuggerking::mmwave_core::clear_buffer(void)
{
	AUDIOSINK_WAVEHDR_T *phdr;
	if (_buffer_size>0)
	{
		for (int i = 0; i<_buffer_size; i++)
		{
			phdr = _wave_hdr + i;
			delete[] phdr->waveHdr.lpData;
		}

		delete[] _wave_hdr;
		_wave_hdr = nullptr;
	}
}

void debuggerking::mmwave_core::set_buffer(void *data, DWORD size)
{
	__try
	{
		EnterCriticalSection(&_cs);

		if (!_wave_out)
			return;
		if (_buffer_size == 0)
			return;
		if (size > _per_data_size)
			return;

		// 16비트인 경우 데이타 swap
		//if(m_wfx.wBitsPerSample == 16)
		//	SwapAudioData((BYTE*)pData, dwSize);

		MMRESULT mr;
		if (_wave_hdr[_current_position].bLock)
			return;

		AUDIOSINK_WAVEHDR_T *phdr = &_wave_hdr[_current_position++];
		_current_position %= _buffer_size;

		if (phdr->waveHdr.dwFlags & WHDR_DONE)
		{
			waveOutUnprepareHeader(_wave_out, &phdr->waveHdr, sizeof(WAVEHDR));
			phdr->waveHdr.dwFlags = 0;
		}

		phdr->bLock = TRUE;
		phdr->waveHdr.dwBufferLength = size;
		phdr->waveHdr.dwFlags = 0;
		CopyMemory(phdr->waveHdr.lpData, data, size);


		mr = waveOutPrepareHeader(_wave_out, &phdr->waveHdr, sizeof(WAVEHDR));
		if (mr != MMSYSERR_NOERROR)
			return;

		mr = waveOutWrite(_wave_out, &phdr->waveHdr, sizeof(WAVEHDR));

	}
	__finally
	{
		LeaveCriticalSection(&_cs);
	}
}

void debuggerking::mmwave_core::set_volume(unsigned long lVolume)
{
	if (!_wave_out)
		return;

	lVolume = min(100, lVolume);
	unsigned long normalizedVolume = (lVolume * 0xffff) / 100;
	unsigned long volumeToSet = (normalizedVolume << 16) + normalizedVolume;

	MMRESULT mr;
	mr = waveOutSetVolume(_wave_out, volumeToSet);
	_volume = lVolume;
}

DWORD debuggerking::mmwave_core::get_volume(void)
{
	if (_wave_out)
	{
		DWORD dwVolume;
		waveOutGetVolume(_wave_out, &dwVolume);
		_volume = LOWORD(dwVolume) * 100 / 0xffff;
	}
	return _volume;
}

void CALLBACK debuggerking::mmwave_core::speaker_callback(HWAVEOUT wave_out, UINT msg, DWORD instance, DWORD param1, DWORD param2)
{
	AUDIOSINK_WAVEHDR_T *phdr;
	switch (msg)
	{
	case WOM_OPEN:
		break;

	case WOM_CLOSE:
		break;

	case WOM_DONE:
		phdr = reinterpret_cast<AUDIOSINK_WAVEHDR_T*>(param1);
		phdr->bLock = FALSE;
		break;
	}
}