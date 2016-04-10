#ifndef _MMWAVE_RENDERER_H_
#define _MMWAVE_RENDERER_H_

#include "dk_mmwave_renderer.h"
#include <mmsystem.h>
#include <vector>

typedef struct _AUDIOSINK_WAVEHDR_T
{
	WAVEHDR waveHdr;
	BOOL bLock;
} AUDIOSINK_WAVEHDR_T;

class mmwave_renderer
{
public:
	mmwave_renderer(void);
	~mmwave_renderer(void);

	dk_mmwave_renderer::err_code initialize_renderer(dk_mmwave_renderer::configuration_t * config);
	dk_mmwave_renderer::err_code release_renderer(void);
	dk_mmwave_renderer::err_code render(dk_mmwave_renderer::dk_audio_entity_t * pcm);

private:
	void init_buffer(int buffer_size, int data_size);
	void set_buffer(void *data, DWORD size);
	void clear_buffer(void);

	BOOL begin_audio_render(DWORD sample_per_second, WORD bitdepth, WORD channels = 1);
	void end_audio_render(void);

	void set_volume(unsigned long lVolume);
	DWORD get_volume(void);
	BOOL is_initialized(void) const { return (_wave_out != NULL) ? TRUE : FALSE; }
	static void CALLBACK speaker_callback(HWAVEOUT wave_out, UINT msg, DWORD instance, DWORD param1, DWORD param2);

private:
	unsigned int _waveout_device_id;
	HWAVEOUT _wave_out;
	AUDIOSINK_WAVEHDR_T	*_wave_hdr;
	DWORD _per_data_size;
	WAVEFORMATEX _wfx;

	static DWORD _volume;
	CRITICAL_SECTION _cs;

	int _current_position;
	int	 _buffer_size;
};

#endif