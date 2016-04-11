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
	//WSADATA	wsaData;
	int iResult;
	// Initialze Winsock
	/*
	iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if (iResult != 0) {
		printf( "WSAStartup failed: %d\n", iResult );
		return 1;
	}
	*/

	// Create a socket
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;

	memset(&hints,0x00,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo( _ip_serv.c_str(), _port_serv.c_str(), &hints, &result);
	if (iResult != 0) {
		printf( "getaddrinfo failed: %d\n", iResult );
		//WSACleanup();
		return 1;
	}


	int ConnectSocket = 0;

	// Attemp to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (ConnectSocket == 0) {
		printf("Error at socket(): %ld\n", WSAGetLastError() );
		freeaddrinfo( result );
		//WSACleanup();
		return 1;
	}

	// Connect to server
	iResult = connect( ConnectSocket, ptr->ai_addr, (int) ptr->ai_addrlen );
	if (iResult == -1) {
		closesocket( ConnectSocket );
		ConnectSocket = 0;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo( result );
	if (ConnectSocket == 0) {
		printf("Unable to connect to server!\n");
		//WSACleanup();
		return 1;
	}
	
	connected = true;
	_socket_connected = ConnectSocket;	

	return 0;

}

//unsigned short socket_client::send_msg( char* msg, int size ) {
//
//	int recvbuflen = DEFAULT_BUFLEN;
//
//	char recvbuf[DEFAULT_BUFLEN];
//
//	int iResult;
//
//	// Send an initial buffer
//	iResult = send( _socket_connected, msg, size, 0 );
//	if (iResult == -1) {
//		printf("send failed: %d\n", WSAGetLastError() );
//		closesocket( _socket_connected );
//		WSACleanup();
//	}
//	
//	printf( "Bytes Sent: %ld\n", iResult );
//
//	// shutdown the connection for sending since no more data will be sent
//	// the client can still use the ConnectSocket for receiving data
//	iResult = shutdown( _socket_connected, SD_SEND );
//	if( iResult == -1 ) {
//		printf("shutdown failed: %d\n", WSAGetLastError() );
//		closesocket( _socket_connected );
//		WSACleanup();
//		return 1;
//	}
//
//	// Receive data until the server closes the connection
//	do {
//		iResult = recv( _socket_connected, recvbuf, recvbuflen, 0);
//		if (iResult > 0)
//			printf("Bytes received: %d\n", iResult);
//		else if (iResult == 0)
//			printf("Connection closed\n");
//		else
//			printf("recv failed: %d\n", WSAGetLastError() );
//	} while (iResult > 0);
//
//
//
//	return 0;
//
//}

unsigned short socket_client::send_msg( char* msg, int size, char* recv_buffer ) {

	int recvbuflen = DEFAULT_BUFLEN;

	char recvbuf[DEFAULT_BUFLEN];

	int iResult = 0;

	fd_set read_flags_ori,write_flags_ori; // the flag sets to be used
	fd_set read_flags,write_flags; // the flag sets to be used
    struct timeval waitd;          // the max wait time for an event
	int sel;                      // holds return value for select();
	FD_ZERO(&read_flags_ori);
    FD_ZERO(&write_flags_ori);
	FD_SET(_socket_connected, &read_flags_ori);
    FD_SET(_socket_connected, &write_flags_ori);	
	while(1) {
	
        waitd.tv_sec = 10;           
		read_flags = read_flags_ori;
		write_flags = write_flags_ori;

        sel = select(_socket_connected+1, &read_flags, &write_flags, (fd_set*)0, &waitd);
        if(sel <= 0) break;

		//socket ready for writing		
        if(FD_ISSET(_socket_connected, &write_flags)) 
		{          
			iResult = send( _socket_connected, msg, size, 0 );
			if (iResult == -1) {
				printf("send failed: %d\n", WSAGetLastError() );
				closesocket( _socket_connected );
				//WSACleanup();
				break;
			}
			else
			{
				printf( "Bytes Sent: %ld\n", iResult );
				FD_CLR(_socket_connected, &write_flags_ori);
			}					
        }	

        //socket ready for reading
        if(FD_ISSET(_socket_connected, &read_flags)) 
		{
            FD_CLR(_socket_connected, &read_flags);

            memset(&recvbuf, 0, recvbuflen);

			iResult = recv(_socket_connected, recvbuf, sizeof(recvbuf), 0);
            if(iResult <= 0) {
				closesocket( _socket_connected );
				//WSACleanup();
				printf("recv failed: %d\n", WSAGetLastError() );
                break;
            }
			else if (iResult == 0)
			{
				printf("Connection closed\n");
				break;
			}			
            else
			{
				printf("Bytes received: %d\n", iResult);
				FD_CLR(_socket_connected, &read_flags_ori);				
				if( recv_buffer!=NULL && strlen(recvbuf)!=0 )
					memcpy(recv_buffer, recvbuf, recvbuflen);
			}
        }   //end if ready for read        

    }   //end while

	return 0;

}

socket_client::~socket_client( void ) {
	shutdown_socket();
}

unsigned short socket_client::shutdown_socket( void ) {
	// shutdown the send half of the connection since no more data will be sent
	int iResult;
	iResult = shutdown( _socket_connected, SD_SEND );
	if (iResult == -1) {
		printf("shutdown failed: %d\n", WSAGetLastError() );
		closesocket( _socket_connected );
		//WSACleanup();
		return 1;
	}

	// cleanup
	closesocket( _socket_connected );
	//WSACleanup();

	return 0;
}