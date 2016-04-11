#pragma once

#define NUM_OF_BUFFER 30
#define RECVBUFFERSIZE 1280000
#define SENDBUFFERSIZE 1280000
#define WAVEOUTSIZE 12800
#include "base_input_source.h"

class backend_audio_backchannel_controller;
class mmwave_input_source : public base_input_source
{
public:
	mmwave_input_source( void );
	virtual ~mmwave_input_source( void );

	void					initialize( backend_audio_backchannel_controller *controller, unsigned short channels, unsigned long sample_per_second, unsigned short bit_per_sample, int sample_size );
	void					release( void );

private:
	void					initialize_wave( unsigned short channels=1, unsigned long sample_per_second=8000, unsigned short bit_per_sample=16 );
	void					release_wave( void );

	void					initialize_wave_header( int buffer_size );
	void					release_wave_header( void );

	static void CALLBACK	wave_in_callback( HWAVEIN hwi, UINT msg, DWORD instance, DWORD param1, DWORD param2 );

private:
	HWAVEIN			_wave_in;
	WAVEFORMATEX	_wave_format_ex;
	WAVEHDR			_wave_hdr[NUM_OF_BUFFER];
	unsigned char	*_wave_buffer[NUM_OF_BUFFER];

	HANDLE			_wave_in_lock;

	int				_sample_size;
	bool			_enable;

	backend_audio_backchannel_controller *_controller;
};
