#if !defined(_SOCKET_CLIENT_H_)
#define _SOCKET_CLIENT_H_


#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment (lib, "mswsock.lib")
#pragma comment (lib, "advapi32.lib")
#elif defined(UBUNTU)
#include <unistd.h>
#endif
#include <pthread.h>

typedef struct _SOCKET_MESSAGE_T
{
	char	*data;
	int		data_size;
	int		delay_time;
	_SOCKET_MESSAGE_T( char *src, int size, int delay )
		: delay_time(delay)
	{
		data_size = size;
		if( data_size>0 )
		{
			data = static_cast<char*>( malloc(data_size) );
			memset( data, 0x00, data_size );
			memcpy( data, src, data_size );
		}
	}
	~_SOCKET_MESSAGE_T( void )
	{
		if( data )
		{
			free( data );
			data = nullptr;
		}
		data_size = 0;
		delay_time = 0;
	}
} SOCKET_MESSAGE_T, *LPSOCKET_MESSAGE_T;

class socket_client 
{
public:
	socket_client( void );
	~socket_client( void );

	unsigned short	connect( char *server_address, unsigned short server_port );
	unsigned short	disconnect( void );

	unsigned short	post_send_message( char* msg, int size, int delay=0 );
	unsigned short	post_recv_message( int size );
	virtual void	on_recv_message( char *msg, int size )=0;

protected:
	bool					_connected;

private:
	static void*	run( void *param );
	static void*	delayed_send_run( void *param );

	fd_set					_wset;
	fd_set					_rset;
	int						_recv_size;
	SOCKET					_socket;
	char					_server_address[MAX_PATH];
	char					_server_port[MAX_PATH];

	pthread_mutex_t			_mutex;
	pthread_t				_tid;
	pthread_t				_send_tid;
	bool					_is_run;


	std::vector<LPSOCKET_MESSAGE_T> _send_queue;
	pthread_mutex_t					_delayed_mutex;
	std::vector<LPSOCKET_MESSAGE_T> _delayed_send_queue;

};

#endif