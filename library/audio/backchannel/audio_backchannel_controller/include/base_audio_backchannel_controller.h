#pragma once
#include <string>
#include <vector>
#include <iostream>


#define AUDIO_BACKCHANNEL_DEFAULT_SERVICE_ADDRESS		L"127.0.0.1"
#define AUDIO_BACKCHANNEL_DEFAULT_USER				L"admin"
#define AUDIO_BACKCHANNEL_DEFAULT_PASSWORD			L"admin"

#define AUDIO_BACKCHANNEL_DEFAULT_SERVICE_PORT		9080

enum AUDIO_BACKCHANNEL_ERROR_T
{
	VMS_AUDIO_BACKCHANNEL_SUCCESS = 0,
	VMS_AUDIO_BACKCHANNEL_FAIL,
	VMS_AUDIO_BACKCHANNEL_UNSUPPORTED_COMMAND,
	VMS_AUDIO_BACKCHANNEL_SENSITIVE_VALUE_IS_NOT_VALID,
	VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE,
	VMS_AUDIO_BACKCHANNEL_HOST_NAME_IS_INVALID,
	VMS_AUDIO_BACKCHANNEL_HOST_IS_NOT_CONNECTABLE,
	VMS_AUDIO_BACKCHANNEL_CURRENTLY_WORKING,
};

enum AUDIO_BACKCHANNEL_CODEC_TYPE_T
{
	AUDIO_BACKCHANNEL_CODEC_TYPE_AAC,
	AUDIO_BACKCHANNEL_CODEC_TYPE_G711A,
	AUDIO_BACKCHANNEL_CODEC_TYPE_G711U,
	AUDIO_BACKCHANNEL_CODEC_TYPE_G726_16,
	AUDIO_BACKCHANNEL_CODEC_TYPE_G726_24,
	AUDIO_BACKCHANNEL_CODEC_TYPE_G726_32,
	AUDIO_BACKCHANNEL_CODEC_TYPE_G726_40,
	AUDIO_BACKCHANNEL_CODEC_TYPE_MP4A_LATM,
	AUDIO_BACKCHANNEL_CODEC_TYPE_PCM,
};

typedef void (__stdcall *audio_backchannel_progress)( int progress );

class base_audio_backchannel_controller
{
public:
	virtual char*			get_vendor_name( void )=0;
	virtual char*			get_vendor_device_name( void )=0;
	virtual char*			get_vendor_device_protocol_name( void )=0;
	virtual char*			get_vendor_device_version_name( void )=0;

	virtual unsigned short	get_vendor_id( void )=0;
	virtual unsigned short	get_vendor_device_id( void )=0;
	virtual unsigned short	get_vendor_device_protocol_id( void )=0;
	virtual unsigned short	get_vendor_device_version_id( void )=0;

	virtual unsigned short	set_host_name( char *host_name )=0;
	virtual unsigned short	set_port_number( unsigned short port_number )=0;
	virtual unsigned short	set_user_id( char *user_id )=0;
	virtual unsigned short	set_user_password( char *password )=0;

	virtual unsigned short	connect( void )=0;
	virtual unsigned short	disconnect( void )=0;
	virtual unsigned short	get_duration( float &duration )=0;
	virtual unsigned short	get_codec_type( AUDIO_BACKCHANNEL_CODEC_TYPE_T &codec_type, unsigned short &bit_depth, unsigned long &sample_rate )=0;
	virtual unsigned short	process( void *input, int input_size, void *output, int &output_size )=0;
};
