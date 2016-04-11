#pragma once
#include "socket_client.h"
#include <base_audio_backchannel_controller.h>

//#define _EXPORT_FILE
#define _USE_EXIST_LIBRARY

#if defined(_USE_EXIST_LIBRARY)
struct CNetSession;
typedef struct _INI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T
{
	int				buffer_size;
	unsigned char	*buffer;
} INI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T, *LPINI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T;
#endif
class __declspec(dllexport) ini_audio_backchannel_controller : public base_audio_backchannel_controller, public socket_client
{
public:
	ini_audio_backchannel_controller( void );
	~ini_audio_backchannel_controller( void );

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
	unsigned short	get_duration( float &duration );
	unsigned short	process( void *input, int input_size, void *output, int &output_size );

	void			on_recv_message( char *msg, int size ) {};

private:

	char				_hostname[MAX_PATH];
	unsigned int		_port_number;
	char				_user_id[MAX_PATH];
	char				_user_password[MAX_PATH];

#if defined(_EXPORT_FILE)
	HANDLE				_file;
#endif
#if defined(_USE_EXIST_LIBRARY)
private:
	static void*		process_message( void *arg );
	static void*		process_send_audio( void *arg );
private:

	pthread_t			_tid;
	pthread_t			_send_tid;
	bool				_run_process_message;
	bool				_run_send_audio;
	pthread_mutex_t		_send_audio_mutex;
	struct CNetSession	*_net_session;
	int					_offset;
	char				_send_buffer[1024];
	std::vector<INI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T*>	_render_buffers;
#endif
};

extern "C" __declspec(dllexport) base_audio_backchannel_controller* create( void );
extern "C" __declspec(dllexport) void destroy( base_audio_backchannel_controller **audio_backchannel_controller );