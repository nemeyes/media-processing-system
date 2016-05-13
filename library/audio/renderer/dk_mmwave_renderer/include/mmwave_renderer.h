#ifndef _MMWAVE_RENDERER_H_
#define _MMWAVE_RENDERER_H_

#include "dk_mmwave_renderer.h"
#include <mmsystem.h>
#include <vector>

namespace debuggerking
{
	typedef struct _AUDIOSINK_WAVEHDR_T
	{
		WAVEHDR waveHdr;
		BOOL bLock;
	} AUDIOSINK_WAVEHDR_T;

	class mmwave_core
	{
	public:
		mmwave_core(void);
		~mmwave_core(void);

		int32_t initialize_renderer(mmwave_renderer::configuration_t * config);
		int32_t release_renderer(void);
		int32_t render(mmwave_renderer::entity_t * pcm);

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
};

#endif