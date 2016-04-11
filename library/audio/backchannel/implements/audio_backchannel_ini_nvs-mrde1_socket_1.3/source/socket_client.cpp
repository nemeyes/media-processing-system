#include "platform.h"
#include "socket_client.h"
#include <sstream>
#include <fcntl.h>

#define DEFAULT_BUFLEN	512

socket_client::socket_client( void ) 
	: _mutex(PTHREAD_MUTEX_INITIALIZER)
	, _delayed_mutex(PTHREAD_MUTEX_INITIALIZER)
	, _connected(false)
	, _recv_size(0)
{
	pthread_mutex_init( &_mutex, 0 );
	pthread_mutex_init( &_delayed_mutex, 0 );
}

socket_client::~socket_client( void ) 
{
	//disconnect();
	pthread_mutex_destroy( &_mutex );
	pthread_mutex_destroy( &_delayed_mutex );
}

unsigned short socket_client::connect( char *server_address, unsigned short server_port ) 
{
	if( strlen(server_address)>0 )
		strncpy( _server_address, server_address, sizeof(_server_address) );
	snprintf( _server_port, sizeof(_server_port), "%d", server_port );

	int err;
/*
	WSADATA	wsaData;
	err = WSAStartup( MAKEWORD(2,2), &wsaData );
	if( err!=0 ) 
		return 1;
*/
	// Create a socket
	struct addrinfo *result = nullptr, *ptr = nullptr, hints;

	memset( &hints, 0x00, sizeof(hints) );
	hints.ai_family		= AF_UNSPEC;
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_protocol	= IPPROTO_TCP;

	// Resolve the server address and port
	err = getaddrinfo( _server_address, _server_port, &hints, &result );
	if( err!=0 ) 
	{
		//WSACleanup();
		return 1;
	}

	_socket = 0;
	// Attemp to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;
	// Create a SOCKET for connecting to server
	_socket = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol );
	if( _socket==0 ) 
	{
		freeaddrinfo( result );
		//WSACleanup();
		return 1;
	}


#ifdef WIN32
//    int nonblocking =1;
//    ioctlsocket( _socket, FIONBIO, (unsigned long*) &nonblocking );
#else
    flags = fcntl( _socket, F_GETFL, 0 );
    fcntl( sockfd, F_SETFL, flags | O_NONBLOCK );
#endif

	// Connect to server
	err = ::connect( _socket, ptr->ai_addr, (int) ptr->ai_addrlen );
	if( (err==-1) /*|| (err!=EINPROGRESS && err!=EWOULDBLOCK)*/ ) 
	{
		closesocket( _socket );
		_socket = 0;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo( result );
	if( _socket==0 ) 
	{
		//WSACleanup();
		return 1;
	}

	_connected = true;
	pthread_create( &_tid, nullptr, (void* (*)(void*))socket_client::run, (void*)this );
	pthread_create( &_send_tid, nullptr, (void* (*)(void*))socket_client::delayed_send_run, (void*)this );
	return 0;
}

unsigned short socket_client::disconnect( void ) 
{
	// shutdown the send half of the connection since no more data will be sent
	if( _connected )
	{
		_connected = false;
		pthread_join( _tid, nullptr );
		pthread_join( _send_tid, nullptr );
	}


	//release send queue
	std::vector<LPSOCKET_MESSAGE_T>::iterator iter;
	pthread_mutex_lock( &_mutex );
	for( iter=_send_queue.begin(); iter!=_send_queue.end(); iter++ )
	{
		LPSOCKET_MESSAGE_T message = (*iter);
		delete message;
		message = 0;
	}
	_send_queue.clear();
	pthread_mutex_unlock( &_mutex );

	//release delayed send queue
	pthread_mutex_lock( &_delayed_mutex );
	for( iter=_delayed_send_queue.begin(); iter!=_delayed_send_queue.end(); iter++ )
	{
		LPSOCKET_MESSAGE_T message = (*iter);
		delete message;
		message = 0;
	}
	_delayed_send_queue.clear();
	pthread_mutex_unlock( &_delayed_mutex );

	int was_value;
	was_value = shutdown( _socket, SD_SEND );
	if( was_value==-1 ) 
	{
		closesocket( _socket );
		//WSACleanup();
		return 1;
	}

	// cleanup
	closesocket( _socket );
	//WSACleanup();
	return 0;
}

unsigned short socket_client::post_send_message( char* msg, int size, int delay )
{
	if( _connected )
	{
		if( delay==0 )
		{
			LPSOCKET_MESSAGE_T message = new SOCKET_MESSAGE_T( msg, size, 0 );
			pthread_mutex_lock( &_mutex );
			if( _send_queue.size()>50 )
			{
				pthread_mutex_unlock( &_mutex );
				delete message;
				return 0;
			}
			_send_queue.push_back( message );
			pthread_mutex_unlock( &_mutex );
		}
		else
		{
			LPSOCKET_MESSAGE_T message = new SOCKET_MESSAGE_T( msg, size, delay );
			pthread_mutex_lock( &_delayed_mutex );
			if( _delayed_send_queue.size()>50 )
			{
				pthread_mutex_unlock( &_delayed_mutex );
				return 0;
			}
			_delayed_send_queue.push_back( message );
			pthread_mutex_unlock( &_delayed_mutex );
		}
	}
	return 0;
}

unsigned short socket_client::post_recv_message( int size )
{
	if( _connected )
	{
		_recv_size = size;
		//FD_SET( _socket, &_rset );
	}
	return 0;
}

void* socket_client::delayed_send_run( void *param )
{
	socket_client *self = static_cast<socket_client*>( param );
	if( !self )
		return 0;

	int		value = 0;
	while( self->_connected ) 
	{
		LPSOCKET_MESSAGE_T message = 0;
		std::vector<LPSOCKET_MESSAGE_T>::iterator iter;
		pthread_mutex_lock( &self->_delayed_mutex );
		iter = self->_delayed_send_queue.begin();
		if( iter!=self->_delayed_send_queue.end() )
		{
			message = (*iter);
			self->_delayed_send_queue.erase( iter );
		}
		pthread_mutex_unlock( &self->_delayed_mutex );

		if( message )
		{
			Sleep( message->delay_time );
			pthread_mutex_lock( &self->_mutex );
			if( self->_send_queue.size()>50 )
			{
				delete message;
				message = 0;
				pthread_mutex_unlock( &self->_mutex );
				continue;
			}
			self->_send_queue.push_back( message );
			pthread_mutex_unlock( &self->_mutex );
		}
		Sleep( 10 );
	}
	return 0;
}

void* socket_client::run( void *param )
{
	socket_client *self = static_cast<socket_client*>( param );

	if( !self )
		return 0;

	char	recvbuf[DEFAULT_BUFLEN];
	int		value = 0;

    struct timeval waitd;					// the max wait time for an event
	int sel;								// holds return value for select();
	while( self->_connected ) 
	{
		FD_ZERO( &self->_wset );
		FD_SET( self->_socket, &self->_wset );
		self->_rset = self->_wset;
	
        waitd.tv_sec = 10;           
        sel = select( self->_socket+1, &self->_rset, &self->_wset, (fd_set*)0, &waitd );
        if( sel<=0 ) 
		{
			Sleep( 10 );
			continue;
		}

		//socket ready for writing		
        if( FD_ISSET(self->_socket, &self->_wset) ) 
		{          
			std::vector<LPSOCKET_MESSAGE_T>::iterator iter;
			pthread_mutex_lock( &self->_mutex );
			for( iter=self->_send_queue.begin(); iter!=self->_send_queue.end(); iter++ )
			{
				LPSOCKET_MESSAGE_T message = (*iter);
				value = send( self->_socket, message->data, message->data_size, 0 );
				if( value==-1 ) 
				{
					self->_connected = false;
					closesocket( self->_socket );
					//WSACleanup();
					break;
				}
				delete message;
				message = 0;
			}
			self->_send_queue.clear();
			pthread_mutex_unlock( &self->_mutex );
			if( value==-1 )
			{
				break;
			}
			//else
			//	FD_CLR( self->_socket, &write_flags_ori );
        }	

        //socket ready for reading
        if( FD_ISSET(self->_socket, &self->_rset) ) 
		{
            //FD_CLR( self->_socket, &read_flags );
            memset( &recvbuf, 0, DEFAULT_BUFLEN );
			if( self->_recv_size>0 )
			{
				value = recv( self->_socket, recvbuf, self->_recv_size, 0 );
				if( value<0 ) 
				{
					self->_connected = false;
					closesocket( self->_socket );
					//WSACleanup();
					break;
				}
				else if( value==0 )
				{
					//self->_connected = false;
					//break;
					Sleep( 10 );
					continue;
				}			
				else
				{
					self->on_recv_message( recvbuf, value );
				}
			}
        }
    }
	return 0;
}
