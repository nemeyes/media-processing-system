#pragma once
#include <base_audio_backchannel_controller.h>

//#define _EXPORT_FILE

class __declspec(dllexport) hanil_audio_backchannel_controller : public base_audio_backchannel_controller
{
public:
	hanil_audio_backchannel_controller( void );
	~hanil_audio_backchannel_controller( void );

	char*			get_vendor_name( void );
	char*			get_vendor_device_name( void );
	char*			get_vendor_device_protocol_name( void );
	char*			get_vendor_device_version_name( void );

	unsigned short	get_vendor_id( void );
	unsigned short	get_vendor_device_id( void );
	unsigned short	get_vendor_device_protocol_id( void );
	unsigned short	get_vendor_device_version_id( void );

	unsigned short	set_host_name( char *hostname );
	unsigned short	set_port_number( unsigned short port_number );
	unsigned short	set_user_id( char *user_id );
	unsigned short	set_user_password( char *password );

	unsigned short	connect( void );
	unsigned short	disconnect( void );
	unsigned short	get_codec_type( AUDIO_BACKCHANNEL_CODEC_TYPE_T &codec_type, unsigned short &bit_depth, unsigned long &sample_rate );
	unsigned short  get_duration( float &duration );
	unsigned short	process( void *input, int input_size, void *output, int &output_size );

private:
	int					_sender_id;

private:
	char				_hostname[MAX_PATH];
	unsigned int		_port_number;
	char				_user_id[MAX_PATH];
	char				_user_password[MAX_PATH];

#if defined(_EXPORT_FILE)
	HANDLE				_file;
#endif

};

extern "C" __declspec(dllexport) base_audio_backchannel_controller* create( void );
extern "C" __declspec(dllexport) void destroy( base_audio_backchannel_controller **audio_backchannel_controller );