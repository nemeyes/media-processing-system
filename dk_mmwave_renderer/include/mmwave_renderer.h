#include "BaseMediaRenderer.h"
#include <MMSystem.h>
#include <vector>

typedef struct _AUDIOSINK_WAVEHDR_T
{
	WAVEHDR waveHdr;
	BOOL bLock;
} AUDIOSINK_WAVEHDR_T;

class MMWaveRenderer : public BaseMediaRenderer
{
public:
	MMWaveRenderer(BaseMediaController *controller);
	virtual ~MMWaveRenderer(void);

	unsigned short		Initialize(LPSUBMEDIA_PROCESS_ELEMENT_T subpe);
	unsigned short		Release(LPSUBMEDIA_PROCESS_ELEMENT_T subpe);
	unsigned short		Process(LPSUBMEDIA_PROCESS_ELEMENT_T subpe);

private:
	void	InitBuffer(int buffer_size, int data_size);
	void	ClearBuffer(void);
	BOOL	BeginAudioRender(DWORD sample_per_second, WORD bit_per_sample, WORD channels = 1);
	void	EndAudioRender(void);
	void	SetBuffer(void *data, DWORD size);

	void	SetVolume(unsigned long lVolume);
	DWORD	GetVolume(void);
	BOOL	IsInitialized(void) const { return (_wave_out != NULL) ? TRUE : FALSE; }
	static void CALLBACK SpeakerCallback(HWAVEOUT wave_out, UINT msg, DWORD instance, DWORD param1, DWORD param2);

private:
	int					_sample_rate;	// sample rate
	int					_bit_per_sample;	// bit per sample

	unsigned int		_waveout_device_id;
	HWAVEOUT			_wave_out;
	AUDIOSINK_WAVEHDR_T	*_wave_hdr;
	DWORD				_per_data_size;
	WAVEFORMATEX		_wfx;

	static DWORD		_volume;
	CRITICAL_SECTION	_cs;

	int					_current_position;
	int					_buffer_size;
};

/*
#define MAX_AUDIO_BUFFER 64
class MMWaveRenderer : public BaseMediaRenderer
{
public:
MMWaveRenderer( BaseMediaController *controller );
virtual ~MMWaveRenderer( void );

unsigned short		Initialize( LPSUBMEDIA_PROCESS_ELEMENT_T subpe );
unsigned short		Release( LPSUBMEDIA_PROCESS_ELEMENT_T subpe );

unsigned short		Process( LPSUBMEDIA_PROCESS_ELEMENT_T subpe );

private:
WAVEHDR					_wave_hdr[MAX_AUDIO_BUFFER];
BYTE					*_audio_data[MAX_AUDIO_BUFFER];

HWAVEOUT				_wave_out;
WAVEFORMATEX			_wave_format;

int						_prev_read_index;
unsigned short			_read_index;
unsigned short			_buffer_count;
unsigned short			_write_index;

unsigned short			_max_audio_buffer;

DWORD					_os_major_version;
DWORD					_os_minor_version;

};
*/