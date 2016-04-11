#include "platform.h"
#include "audio_backchannel_controller.h"
#include "backend_audio_backchannel_controller.h"

audio_backchannel_controller::audio_backchannel_controller( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version )
	: _controller(0)
{
	_controller = new backend_audio_backchannel_controller( vendor, vendor_device, protocol, firmware_version  );
}

audio_backchannel_controller::~audio_backchannel_controller( void )
{
	if( _controller )
		delete _controller;
}

unsigned short audio_backchannel_controller::get_vendor_informations( int ***vendor_ids, char ***vendor_names, int *length )
{
	return backend_audio_backchannel_controller::get_vendor_informations( vendor_ids, vendor_names, length );
}

unsigned short audio_backchannel_controller::get_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length )
{
	return backend_audio_backchannel_controller::get_vendor_device_informations( vendor_id, vendor_device_ids, vendor_device_names, length );
}

unsigned short audio_backchannel_controller::get_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length )
{
	return backend_audio_backchannel_controller::get_vendor_device_protocol_informations( vendor_id, vendor_device_id, vendor_device_protocol_ids, vendor_device_protocol_names, length );
}

unsigned short audio_backchannel_controller::get_vendor_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length )
{
	return backend_audio_backchannel_controller::get_vendor_device_version_informations( vendor_id, vendor_device_id, vendor_device_protocol_id, vendor_device_version_ids, vendor_device_version_names, length );
}

char* audio_backchannel_controller::get_vendor_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_name();
}

char* audio_backchannel_controller::get_vendor_device_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_name();
}

char* audio_backchannel_controller::get_vendor_device_protocol_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_protocol_name();
}

char* audio_backchannel_controller::get_vendor_device_version_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_version_name();
}

unsigned short audio_backchannel_controller::get_vendor_id( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_vendor_id();
}

unsigned short audio_backchannel_controller::get_vendor_device_id( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_vendor_device_id();
}

unsigned short audio_backchannel_controller::get_vendor_device_protocol_id( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_vendor_device_protocol_id();
}

unsigned short audio_backchannel_controller::get_vendor_device_version_id( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_vendor_device_version_id();
}

unsigned short	audio_backchannel_controller::set_host_name( char *host_name )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->set_host_name( host_name );
}

unsigned short	audio_backchannel_controller::set_port_number( unsigned short port_number )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->set_port_number( port_number );
}

unsigned short	audio_backchannel_controller::set_user_id( char *user_id )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->set_user_id( user_id );
}

unsigned short	audio_backchannel_controller::set_user_password( char *password )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->set_user_password( password );
}

unsigned short	audio_backchannel_controller::connect( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->connect();
}

unsigned short	audio_backchannel_controller::disconnect( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->disconnect();
}

unsigned short	audio_backchannel_controller::get_codec_type( AUDIO_BACKCHANNEL_CODEC_TYPE_T &codec_type, unsigned short &bit_depth, unsigned long &sample_rate )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_codec_type( codec_type, bit_depth, sample_rate );
}

unsigned short	audio_backchannel_controller::get_duration( float &duration )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->get_duration( duration );
}


unsigned short	audio_backchannel_controller::process( void *input, int input_size, void *output, int &output_size )
{
	return VMS_AUDIO_BACKCHANNEL_UNSUPPORTED_COMMAND;
}

unsigned short	audio_backchannel_controller::start( bool microphone, char *path, audio_backchannel_progress progress )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->start( microphone, path, progress );
}

unsigned short	audio_backchannel_controller::stop( void )
{
	if( !_controller ) 
		return VMS_AUDIO_BACKCHANNEL_UNDEFINED_DEVICE;
	return _controller->stop();
}


