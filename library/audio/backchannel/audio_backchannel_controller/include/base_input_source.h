#pragma once


class backend_audio_backchannel_controller;
class base_input_source
{
public:
	virtual void initialize( backend_audio_backchannel_controller *controller, unsigned short channels, unsigned long sample_per_second, unsigned short bit_per_sample, int sample_size )=0;
	virtual void release( void )=0;
};