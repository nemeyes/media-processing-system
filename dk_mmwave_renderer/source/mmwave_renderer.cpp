#include "stdafx.h"
#include "FFMpegBase.h"
#include "MMWaveRenderer.h"
#pragma comment(lib, "winmm")

DWORD MMWaveRenderer::_volume = 30;

MMWaveRenderer::MMWaveRenderer(BaseMediaController *controller)
	: BaseMediaRenderer(controller)
{
	_wave_out = NULL;
	_waveout_device_id = WAVE_MAPPER;
	_current_position = 0;
	_buffer_size = 0;
	_per_data_size = 0;
	_sample_rate = 0;
	_bit_per_sample = 0;

	InitBuffer(48, 8192);
	InitializeCriticalSection(&_cs);
	ZeroMemory(&_wfx, sizeof(_wfx));
}

MMWaveRenderer::~MMWaveRenderer(void)
{
	EndAudioRender();
	ClearBuffer();
	DeleteCriticalSection(&_cs);
}

unsigned short MMWaveRenderer::Initialize(LPSUBMEDIA_PROCESS_ELEMENT_T subpe)
{
	unsigned short value = VMS_STATUS_FAIL;
	BOOL ret = BeginAudioRender(subpe->media_info.fmt_audio.sample_rate, 16/*subpe->media_info.fmt_audio.bits_per_sample*/, subpe->media_info.fmt_audio.channels);
	if (ret)
		value = VMS_STATUS_SUCCESS;
	return value;
}

unsigned short MMWaveRenderer::Release(LPSUBMEDIA_PROCESS_ELEMENT_T subpe)
{
	unsigned short value = VMS_STATUS_SUCCESS;
	EndAudioRender();
	return value;
}

unsigned short MMWaveRenderer::Process(LPSUBMEDIA_PROCESS_ELEMENT_T subpe)
{
	unsigned short value = VMS_STATUS_SUCCESS;
	SetBuffer(subpe->buffer_of_filter, subpe->buffer_size_of_filter);
	return value;
}

void MMWaveRenderer::InitBuffer(int buffer_size, int data_size)
{
	ClearBuffer();
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

void MMWaveRenderer::ClearBuffer(void)
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
		_wave_hdr = NULL;
	}
}

BOOL MMWaveRenderer::BeginAudioRender(DWORD sample_per_second, WORD bit_per_sample, WORD channels)
{
	if (sample_per_second == 0 || bit_per_sample == 0)
		return FALSE;

	BOOL bResult = TRUE;
	EndAudioRender();

	ZeroMemory(&_wfx, sizeof(_wfx));
	_wfx.wFormatTag = WAVE_FORMAT_PCM;
	//m_wfx.nChannels = 1;	// mono
	//m_wfx.nChannels = 2;	// streo
	_wfx.nChannels = channels;
	_wfx.nSamplesPerSec = sample_per_second;
	_wfx.wBitsPerSample = bit_per_sample;
	_wfx.nBlockAlign = _wfx.wBitsPerSample / 8 * _wfx.nChannels;
	_wfx.nAvgBytesPerSec = _wfx.nSamplesPerSec * _wfx.nBlockAlign;

	MMRESULT mr = NOERROR;
	//mr = waveOutOpen(&m_hWaveOut, m_nWaveoutDeviceID, &m_wfx, NULL, 0, 0);
	mr = waveOutOpen(&_wave_out, _waveout_device_id, &_wfx, (DWORD_PTR)SpeakerCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if (mr == MMSYSERR_NOERROR)
	{
		_sample_rate = sample_per_second;
		_bit_per_sample = bit_per_sample;
		GetVolume();
	}
	return bResult;
}

void MMWaveRenderer::EndAudioRender(void)
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
		_wave_out = NULL;
	}
	__finally
	{
		_current_position = 0;
		LeaveCriticalSection(&_cs);
	}
}

void MMWaveRenderer::SetBuffer(void *data, DWORD size)
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

void MMWaveRenderer::SetVolume(unsigned long lVolume)
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

DWORD MMWaveRenderer::GetVolume(void)
{
	if (_wave_out)
	{
		DWORD dwVolume;
		waveOutGetVolume(_wave_out, &dwVolume);
		_volume = LOWORD(dwVolume) * 100 / 0xffff;
	}
	return _volume;
}

void CALLBACK MMWaveRenderer::SpeakerCallback(HWAVEOUT wave_out, UINT msg, DWORD instance, DWORD param1, DWORD param2)
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

/*
MMWaveRenderer::MMWaveRenderer( BaseMediaController *controller )
: BaseMediaRenderer(controller)
, _wave_out(NULL)
, _prev_read_index(-1)
, _read_index(0)
, _buffer_count(0)
, _write_index(0)
, _max_audio_buffer(64)
{
OSVERSIONINFO osvi;
ZeroMemory( &osvi, sizeof(OSVERSIONINFO) );
osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
GetVersionEx( &osvi );
_os_major_version = osvi.dwMajorVersion;
_os_minor_version = osvi.dwMinorVersion;
if( (_os_major_version==6) && (_os_minor_version==1 || _os_minor_version==2) )
_max_audio_buffer = 64;
else
_max_audio_buffer = 32;
}

MMWaveRenderer::~MMWaveRenderer( void )
{

}

unsigned short MMWaveRenderer::Initialize( LPSUBMEDIA_PROCESS_ELEMENT_T subpe )
{
unsigned short value = VMS_STATUS_FAIL;
__try
{
for( unsigned char index=0; index<_max_audio_buffer; index++ )
{
_audio_data[index] = static_cast<BYTE*>( malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE*sizeof(BYTE)) );
_wave_hdr[index].lpData = (CHAR*)_audio_data[index];
}
_prev_read_index	= -1;
_read_index			= 0;
_buffer_count		= 0;
_write_index		= 0;

_wave_format.wFormatTag			= WAVE_FORMAT_PCM;
_wave_format.nChannels			= subpe->media_info.fmt_audio.channels;
_wave_format.nSamplesPerSec		= subpe->media_info.fmt_audio.sample_rate;
_wave_format.wBitsPerSample		= subpe->media_info.fmt_audio.bits_per_sample;
_wave_format.nBlockAlign		= subpe->media_info.fmt_audio.block_align;
_wave_format.nAvgBytesPerSec	= subpe->media_info.fmt_audio.bytes_per_frame;
_wave_format.cbSize				= sizeof(WAVEFORMATEX);

MMRESULT mmResult = waveOutOpen( &_wave_out, WAVE_MAPPER,  &_wave_format, NULL, NULL, CALLBACK_NULL );
if( mmResult==MMSYSERR_NOERROR )
{
value = VMS_STATUS_SUCCESS;
}
else if( mmResult==MMSYSERR_BADDEVICEID )
{
value = VMS_STATUS_MEDIA_RENDERER_DOES_NOT_INITIALIZED;
}
value = VMS_STATUS_MEDIA_RENDERER_DOES_NOT_INITIALIZED;
}
__except(EXCEPTION_EXECUTE_HANDLER)
{
value = VMS_STATUS_MEDIA_RENDERER_DOES_NOT_INITIALIZED;
}
return value;
}

unsigned short MMWaveRenderer::Release( LPSUBMEDIA_PROCESS_ELEMENT_T subpe )
{
unsigned short value = VMS_STATUS_FAIL;
__try
{
waveOutReset( _wave_out );
waveOutClose( _wave_out );

for( unsigned char index=0; index<_max_audio_buffer; index++ )
{
if( _audio_data[index] )
{
free( _audio_data[index] );
_audio_data[index] = NULL;
}
}

_prev_read_index	= -1;
_read_index			= 0;
_buffer_count		= 0;
_write_index		= 0;

_wave_out = NULL;
}
__except(EXCEPTION_EXECUTE_HANDLER)
{
value = VMS_STATUS_SUCCESS;
}
value = VMS_STATUS_SUCCESS;
return value;
}

unsigned short MMWaveRenderer::Process( LPSUBMEDIA_PROCESS_ELEMENT_T subpe )
{
if( _prev_read_index>-1 )
waveOutUnprepareHeader( _wave_out, &_wave_hdr[_prev_read_index], sizeof(WAVEHDR) );

memcpy( _audio_data[_write_index], subpe->buffer_of_filter, subpe->buffer_size_of_filter );
_wave_hdr[_write_index].dwBufferLength	= subpe->buffer_size_of_filter;
_wave_hdr[_write_index].dwLoops			= 0;
_wave_hdr[_write_index].dwUser			= 0;
if( (_os_major_version==6) && (_os_minor_version==1 || _os_minor_version==2) )
_wave_hdr[_write_index].dwFlags		= WHDR_PREPARED;
else
_wave_hdr[_write_index].dwFlags		= 0;

_write_index++;
if( _write_index>=_max_audio_buffer )
_write_index = 0;
_buffer_count++;

unsigned short value = VMS_STATUS_SUCCESS;
if( _buffer_count>(_max_audio_buffer/2) )
{
while( _buffer_count>2 )
{
__try
{
MMRESULT result = waveOutPrepareHeader( _wave_out, &_wave_hdr[_read_index], sizeof(WAVEHDR) );
if( result==MMSYSERR_NOERROR )
result = waveOutWrite( _wave_out, &_wave_hdr[_read_index], sizeof(WAVEHDR) );
else
value = VMS_STATUS_MEDIA_RENDERER_PROCESS_FAIL;
}
__except(EXCEPTION_EXECUTE_HANDLER)
{
value = VMS_STATUS_MEDIA_RENDERER_PROCESS_FAIL;
}

_prev_read_index = _read_index;
_read_index++;
if( _read_index>=_max_audio_buffer )
_read_index=0;
_buffer_count--;
}
}
return value;
}
*/