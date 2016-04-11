#pragma once

#if defined(EXPORT_VMS_AUDIO_BACKCHANNEL_CONTROLLER)
#define EXPORT_CLASS __declspec(dllexport)
#else
#define EXPORT_CLASS __declspec(dllimport)
#endif

#include "base_audio_backchannel_controller.h"

class backend_audio_backchannel_controller;
class EXPORT_CLASS audio_backchannel_controller : public base_audio_backchannel_controller
{
public:
	audio_backchannel_controller( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version );
	~audio_backchannel_controller( void );

	static unsigned short	get_vendor_informations( int ***vendor_ids, char ***vendor_names, int *length );
	static unsigned short	get_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length );
	static unsigned short	get_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length );
	static unsigned short	get_vendor_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length );

	char*			get_vendor_name( void );
	char*			get_vendor_device_name( void );
	char*			get_vendor_device_protocol_name( void );
	char*			get_vendor_device_version_name( void );

	unsigned short	get_vendor_id( void );
	unsigned short	get_vendor_device_id( void );
	unsigned short	get_vendor_device_protocol_id( void );
	unsigned short	get_vendor_device_version_id( void );

	unsigned short	set_host_name( char *host_name );
	unsigned short	set_port_number( unsigned short port_number );
	unsigned short	set_user_id( char *user_id );
	unsigned short	set_user_password( char *password );


	unsigned short	connect( void );
	unsigned short	disconnect( void );
	unsigned short	get_codec_type( AUDIO_BACKCHANNEL_CODEC_TYPE_T &codec_type, unsigned short &bit_depth, unsigned long &sample_rate );
	unsigned short	get_duration( float &duration );
	unsigned short	process( void *input, int input_size, void *output, int &output_size );
	

	unsigned short	start( bool microphone, char *path, audio_backchannel_progress progress );
	unsigned short	stop( void );

private:
	backend_audio_backchannel_controller			*_controller;
};