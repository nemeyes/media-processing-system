#pragma once

#pragma pack(1)
typedef struct _WAVEDESCR_T
{
	unsigned char riff[4];
	unsigned long size;
	unsigned char wave[4];
} WAVEDESCR_T, *LPWAVEDESCR_T;

typedef struct _WAVEFORMAT_T
{
	unsigned char	id[4];
	unsigned long	size;
	short			format;
	short			channels;
	unsigned long	sample_rate;
	unsigned long	byte_rate;
	short			block_align;
	short			bits_depth;
} WAVEFORMAT_T, *LPWAVEFORMAT_T;
#pragma pack()

#include "base_audio_backchannel_controller.h"
#include "base_input_source.h"

class backend_audio_backchannel_controller;
class file_input_source : public base_input_source
{
public:
	file_input_source( char *path, audio_backchannel_progress progress );
	virtual ~file_input_source( void );

	void						initialize( backend_audio_backchannel_controller *controller, unsigned short channels, unsigned long sample_per_second, unsigned short bit_per_sample, int sample_size );
	void						release( void );
	static unsigned __stdcall	process( void *param );

private:
	bool							_enable;
	HANDLE							_tid;
	backend_audio_backchannel_controller			*_controller;
	
	char							_path[MAX_PATH];
	FILE							*_file;

	WAVEDESCR_T						_wav_descriptor;
	WAVEFORMAT_T					_wav_format;

	long long						_file_size;
	unsigned short					_input_channels;
	unsigned long					_input_sample_per_second;
	unsigned short					_input_bit_per_sample;

	int								_sample_size;
	unsigned short					_output_channels;
	unsigned long					_output_sample_per_second;
	unsigned short					_output_bit_per_sample;

	char							*_resample_in_buffer;
	char							*_resample_out_buffer;
	int								_resample_in_buffer_size;
	int								_resample_out_buffer_size;

	audio_backchannel_progress		_progress;
};
