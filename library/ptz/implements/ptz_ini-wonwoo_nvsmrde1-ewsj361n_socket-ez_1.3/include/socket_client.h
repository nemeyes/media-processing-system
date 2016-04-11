#if !defined(_SOCKET_CLIENT_H_)
#define _SOCKET_CLIENT_H_


#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32)
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#elif defined(UBUNTU)
#include <unistd.h>
#include <pthread.h>
#endif


class socket_client 
{
public:
	socket_client( char* ip_serv, unsigned short port_serv );
	~socket_client( void );

	unsigned short	send_msg( char* msg, int size, char *recv_buffer=0);

private:
	unsigned short	init_socket( void );
	unsigned short	shutdown_socket( void );


	SOCKET	_socket_connected;
	bool	connected;
	std::string	_ip_serv;
	std::string _port_serv;

};

#endif