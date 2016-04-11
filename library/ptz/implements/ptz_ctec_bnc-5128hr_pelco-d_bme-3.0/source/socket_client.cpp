#include "platform.h"
#include "socket_client.h"
#include <sstream>

#define DEFAULT_BUFLEN	512

socket_client::socket_client( char* ip_serv, unsigned short port_serv ) {
	_ip_serv = ip_serv;

	std::stringstream ushort2str;
	ushort2str << port_serv;
	ushort2str >> _port_serv;

	init_socket();
}

unsigned short socket_client::init_socket() {
	WSADATA	wsaData;
	int iResult;
	// Initialze Winsock
	iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if (iResult != 0) {
		printf( "WSAStartup failed: %d\n", iResult );
		return 1;
	}

	// Create a socket
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo( _ip_serv.c_str(), _port_serv.c_str(), &hints, &result);
	if (iResult != 0) {
		printf( "getaddrinfo failed: %d\n", iResult );
		WSACleanup();
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attemp to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError() );
		freeaddrinfo( result );
		WSACleanup();
		return 1;
	}

	// Connect to server
	iResult = connect( ConnectSocket, ptr->ai_addr, (int) ptr->ai_addrlen );
	if (iResult == SOCKET_ERROR) {
		closesocket( ConnectSocket );
		ConnectSocket = INVALID_SOCKET;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo( result );
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	connected = true;
	_socket_connected = ConnectSocket;

	return 0;

}

unsigned short socket_client::send_msg( char* msg ) {

	int recvbuflen = DEFAULT_BUFLEN;

	char recvbuf[DEFAULT_BUFLEN];

	int iResult;

	// Send an initial buffer
	iResult = send( _socket_connected, msg, (int) strlen(msg), 0 );
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError() );
		closesocket( _socket_connected );
		WSACleanup();
	}

	printf( "Bytes Sent: %ld\n", iResult );

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown( _socket_connected, SD_SEND );
	if( iResult == SOCKET_ERROR ) {
		printf("shutdown failed: %d\n", WSAGetLastError() );
		closesocket( _socket_connected );
		WSACleanup();
		return 1;
	}

	// Receive data until the server closes the connection
	do {
		iResult = recv( _socket_connected, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError() );
	} while (iResult > 0);



	return 0;

}

socket_client::~socket_client( void ) {
	shutdown_socket();
}

unsigned short socket_client::shutdown_socket( void ) {
	// shutdown the send half of the connection since no more data will be sent
	int iResult;
	iResult = shutdown( _socket_connected, SD_SEND );
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError() );
		closesocket( _socket_connected );
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket( _socket_connected );
	WSACleanup();

	return 0;
}