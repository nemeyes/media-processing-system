#include "platform.h"
#include "backend_audio_backchannel_controller.h"
#include "file_input_source.h"

#define LENGTH_SEC 0.5

file_input_source::file_input_source( char *path, audio_backchannel_progress progress )
	: _sample_size(0)
	, _input_channels(0)
	, _input_sample_per_second(0)
	, _input_bit_per_sample(0)
	, _output_channels(0)
	, _output_sample_per_second(0)
	, _output_bit_per_sample(0)
	, _resample_in_buffer(0)
	, _resample_out_buffer(0)
	, _resample_in_buffer_size(0)
	, _resample_out_buffer_size(0)
	, _enable(false)
	, _file(0)
	, _progress(progress)
{

	if( strlen(path)>0 )
	{
		memset( _path, 0x00, sizeof(_path) );
		strcpy( _path, path );
	}
}

file_input_source::~file_input_source( void )
{

}

void file_input_source::initialize( backend_audio_backchannel_controller *controller, unsigned short channels, unsigned long sample_per_second, unsigned short bit_per_sample, int sample_size )
{
	_controller					= controller;
	_output_channels			= channels;
	_output_sample_per_second	= sample_per_second;
	_output_bit_per_sample		= bit_per_sample;
	_sample_size				= sample_size;

	if( _controller && strlen(_path)>0 )
	{
		_file = fopen( _path, "rb" );
		if( _file )
		{
			_resample_out_buffer_size	= LENGTH_SEC*_output_sample_per_second*_output_channels*_output_bit_per_sample/8;
			_resample_out_buffer		= static_cast<char*>( malloc(_resample_out_buffer_size) );
			fread( &_wav_descriptor, sizeof(WAVEDESCR_T), 1, _file );
			if( !strncmp((LPCSTR)_wav_descriptor.wave, "WAVE", 4) )
			{
				fread( &_wav_format, sizeof(WAVEFORMAT_T), 1, _file );
				if( !strncmp((LPCSTR)_wav_format.id, "fmt", 3) && (_wav_format.format==1) )
				{
					_input_channels				= _wav_format.channels;
					_input_sample_per_second	= _wav_format.sample_rate;
					_input_bit_per_sample		= _wav_format.bits_depth;
					_resample_in_buffer_size	= LENGTH_SEC*_input_sample_per_second*_input_channels*_input_bit_per_sample/8;
					_resample_in_buffer			= static_cast<char*>( malloc(_resample_in_buffer_size) );


					unsigned thrdid = 0;
					_tid = reinterpret_cast<HANDLE>( _beginthreadex(NULL, 0, file_input_source::process, this, 0, &thrdid) );
				}
			}
		}
	}
}

void file_input_source::release( void )
{
	_enable = false;
	::WaitForSingleObject( _tid, INFINITE );
	CloseHandle( _tid );

	_resample_in_buffer_size = 0;
	_resample_out_buffer_size = 0;

	if( _resample_in_buffer )
	{
		free( _resample_in_buffer );
		_resample_in_buffer = 0;
	}
	if( _resample_out_buffer )
	{
		free( _resample_out_buffer );
		_resample_out_buffer = 0;
	}
	if( _file )
	{
		fclose( _file );
		_file = 0;
	}
	memset( _path, 0x00, sizeof(_path) );
}

unsigned __stdcall file_input_source::process( void *param )
{
	file_input_source *self	= static_cast<file_input_source*>( param );
	if( !self->_controller || !self->_file )
		return 0;

	self->_enable	= true;

	BYTE id[4]		= {0};
	DWORD size		= 0;
	fread( id, sizeof(BYTE), 4, self->_file );
	fread( &size, sizeof(DWORD), 1, self->_file );
	DWORD offset = ftell( self->_file );
	float duration = 0.0;

	self->_file_size = self->_wav_descriptor.size;
	long long total_bytes_read = 0;

	if( self->_input_channels==self->_output_channels && 
		self->_input_sample_per_second==self->_output_sample_per_second &&
		self->_input_bit_per_sample==self->_output_bit_per_sample )
	{
		while( self->_enable && offset<self->_wav_descriptor.size ) 
		{
			if( !strncmp((LPCSTR)id, "data", 4) )
			{
				__try
				{
					unsigned int bytes_read = 0;

					while( self->_enable && !feof(self->_file) && !ferror(self->_file) )
					{
						memset( self->_resample_in_buffer, 0x00, self->_resample_in_buffer_size );
						bytes_read = fread( self->_resample_in_buffer, 1, self->_resample_in_buffer_size/2, self->_file );
						total_bytes_read += bytes_read;

						int output_size = 0;
						if( self->_controller )
							self->_controller->process( self->_resample_in_buffer, bytes_read, 0, output_size ); 

						/*
						if( self->_progress && self->_file_size>0 )
						{
							long long progress = (long long)((total_bytes_read*(long long)100)/self->_file_size);
							(*self->_progress)( (int)progress );
						}
						*/
						//Sleep( (LENGTH_SEC)*1000 );
						self->_controller->get_duration(duration);
						Sleep( (LENGTH_SEC-duration)*1000 );
					}
				}
				__except( EXCEPTION_EXECUTE_HANDLER )
				{
					break;
				}
			}
			else
			{
				fseek( self->_file,  size, SEEK_CUR );
				total_bytes_read += size;
				if( self->_progress && self->_file_size>0 )
				{
					long long progress = (long long)((total_bytes_read*(long long)100)/self->_file_size);
					(*self->_progress)( (int)progress );
				}
			}
			if( self->_enable )
			{
				fread( id, sizeof(BYTE), 4, self->_file );
				total_bytes_read += 4;
				fread( &size, sizeof(DWORD), 1, self->_file );
				total_bytes_read += sizeof(DWORD);
				offset = ftell( self->_file );
				if( self->_progress && self->_file_size>0 )
				{
					long long progress = (long long)((total_bytes_read*(long long)100)/self->_file_size);
					(*self->_progress)( (int)progress );
				}
			}
		} 
	}
	else
	{
		while( self->_enable && offset<self->_wav_descriptor.size ) 
		{
			if( !strncmp((LPCSTR)id, "data", 4) )
			{
				ReSampleContext	*resample_ctx = av_audio_resample_init( self->_output_channels, self->_input_channels,
																self->_output_sample_per_second, self->_input_sample_per_second, 
																AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16, 
																32, 
																10, 
																1, 
																1.0 );
				__try
				{
					unsigned int bytes_read = 0;

					while( self->_enable && !feof(self->_file) && !ferror(self->_file) )
					{
						memset( self->_resample_in_buffer, 0x00, self->_resample_in_buffer_size );
						bytes_read = fread( self->_resample_in_buffer, 1, self->_resample_in_buffer_size/2, self->_file );
						total_bytes_read += bytes_read;
						int samples_output = audio_resample( resample_ctx, 
															 (short*)self->_resample_out_buffer, 
															 (short*)self->_resample_in_buffer, 
															 self->_input_sample_per_second*LENGTH_SEC );
						int output_size = 0;
						if( self->_controller )
							self->_controller->process( self->_resample_out_buffer, samples_output, 0, output_size ); 

						/*
						if( self->_progress && self->_file_size>0 )
						{
							long long progress = (long long)((total_bytes_read*(long long)100)/self->_file_size);
							(*self->_progress)( (int)progress );
						}
						*/
						//Sleep( (LENGTH_SEC-0.35)*1000 );
						//Sleep( (LENGTH_SEC-0.18)*1000 );
						self->_controller->get_duration(duration);
						Sleep( (LENGTH_SEC-duration)*1000 );
					}
					audio_resample_close( resample_ctx );
				}
				__except( EXCEPTION_EXECUTE_HANDLER )
				{
					audio_resample_close( resample_ctx );
					break;
				}
			}
			else
			{
				fseek( self->_file,  size, SEEK_CUR );
				total_bytes_read += size;
				if( self->_progress && self->_file_size>0 )
				{
					long long progress = (long long)((total_bytes_read*(long long)100)/self->_file_size);
					(*self->_progress)( (int)progress );
				}
			}
			if( self->_enable )
			{
				fread( id, sizeof(BYTE), 4, self->_file );
				total_bytes_read += 4;
				fread( &size, sizeof(DWORD), 1, self->_file );
				total_bytes_read += sizeof(DWORD);
				offset = ftell( self->_file );
				if( self->_progress && self->_file_size>0 )
				{
					long long progress = (long long)((total_bytes_read*(long long)100)/self->_file_size);
					(*self->_progress)( (int)progress );
				}
			}
		} 
	}

	if( self->_progress )
		(*self->_progress)( 100 );
	return 0;
}
