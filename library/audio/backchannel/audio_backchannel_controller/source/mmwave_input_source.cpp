#include "platform.h"
#include "mmwave_input_source.h"
#include "backend_audio_backchannel_controller.h"

mmwave_input_source::mmwave_input_source( void )
	: _sample_size(0)
	, _enable(false)
{
	_wave_in_lock = CreateEvent( NULL, TRUE, FALSE, _T("wave_in_lock") );
	SetEvent( _wave_in_lock );
}

mmwave_input_source::~mmwave_input_source( void )
{
	CloseHandle( _wave_in_lock );
}

void mmwave_input_source::initialize( backend_audio_backchannel_controller *controller, unsigned short channels, unsigned long sample_per_second, unsigned short bit_per_sample, int sample_size )
{
	_controller		= controller;
	_sample_size	= sample_size;
	if( _controller )
	{
		_enable = true;
		initialize_wave( channels, sample_per_second, bit_per_sample );
		initialize_wave_header( _sample_size );
	}
}

void mmwave_input_source::release( void )
{

	if( _controller )
	{
		release_wave_header();
		release_wave();
	}
	_sample_size = 0;
}

void mmwave_input_source::initialize_wave( unsigned short channels, unsigned long sample_per_second, unsigned short bit_per_sample )
{
	_wave_format_ex.wFormatTag		= WAVE_FORMAT_PCM;
	_wave_format_ex.nChannels		= channels;
	_wave_format_ex.nSamplesPerSec	= sample_per_second;			
	_wave_format_ex.wBitsPerSample	= bit_per_sample; 
	_wave_format_ex.nAvgBytesPerSec	= _wave_format_ex.nSamplesPerSec*_wave_format_ex.nChannels*_wave_format_ex.wBitsPerSample/8;	
	_wave_format_ex.nBlockAlign		= _wave_format_ex.nChannels*_wave_format_ex.wBitsPerSample/8;			
	_wave_format_ex.cbSize			= 0;
	waveInOpen( &_wave_in, WAVE_MAPPER, &_wave_format_ex, (DWORD_PTR)wave_in_callback, (DWORD_PTR)this, CALLBACK_FUNCTION );
}

void mmwave_input_source::release_wave( void )
{
	if( _wave_in ) 
		waveInClose( _wave_in );
}

void mmwave_input_source::initialize_wave_header( int buffer_size )
{
	for( int i=0;i<NUM_OF_BUFFER;i++ )
	{
		_wave_buffer[i]				= (unsigned char*)malloc( buffer_size );
		_wave_hdr[i].lpData			= (char*)_wave_buffer[i];
		_wave_hdr[i].dwBufferLength	= buffer_size;
		_wave_hdr[i].dwFlags		= 0;
		_wave_hdr[i].dwLoops		= 0;
	}

	for( int i=0;i<NUM_OF_BUFFER;i++ )
	{
		waveInPrepareHeader( _wave_in, &_wave_hdr[i],sizeof(WAVEHDR) );
		waveInAddBuffer( _wave_in, &_wave_hdr[i], sizeof(WAVEHDR) );
	}
	waveInStart( _wave_in );
}

void mmwave_input_source::release_wave_header( void )
{
	_enable = false;
	::WaitForSingleObject( _wave_in_lock, INFINITE );
	MMRESULT value = waveInStop( _wave_in );
	for( int i=0;i<NUM_OF_BUFFER;i++ )
	{
		if( _wave_buffer[i] )
			free( _wave_buffer[i] );
		_wave_buffer[i] = 0;
	}
}

void CALLBACK mmwave_input_source::wave_in_callback( HWAVEIN hwi, UINT msg, DWORD instance, DWORD param1, DWORD param2 )
{
	WAVEHDR	*wave_hdr			= (WAVEHDR*)param1;
	mmwave_input_source *self	= (mmwave_input_source*)instance;
	if( !self->_enable )
		return;

	ResetEvent( self->_wave_in_lock );
	int output_size = 0;
	WAVEHDR out_wave_hdr;
	switch( msg )
	{
		case WIM_DATA:
		{
			out_wave_hdr.lpData				= (LPSTR)wave_hdr->lpData;
			out_wave_hdr.dwBufferLength		= wave_hdr->dwBufferLength;
			out_wave_hdr.dwBytesRecorded	= wave_hdr->dwBytesRecorded;
			out_wave_hdr.dwUser				= 0;
			out_wave_hdr.dwFlags			= 0;
			out_wave_hdr.dwLoops			= 0;

			if( self->_controller )
			{
				self->_controller->process( out_wave_hdr.lpData, out_wave_hdr.dwBytesRecorded, 0, output_size ); 
			}

			memset( wave_hdr->lpData, 0x00, wave_hdr->dwBufferLength );
			waveInPrepareHeader( hwi, (WAVEHDR*)param1, sizeof(WAVEHDR) );
			waveInAddBuffer( hwi, (WAVEHDR*)param1, sizeof(WAVEHDR) );
		}
	}
	SetEvent( self->_wave_in_lock );
}