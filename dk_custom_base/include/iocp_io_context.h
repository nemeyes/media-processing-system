#ifndef _IOCP_IO_CONTEXT_H_
#define _IOCP_IO_CONTEXT_H_

#define MAX_BUFFER 1500 //mtu

namespace ic
{
	typedef struct _PER_IO_CONTEXT_T
	{
		WSAOVERLAPPED	overlapped;
		WSABUF			wsaBuf;
		char			buffer[MAX_BUFFER];
	} PER_IO_CONTEXT_T, *LPPER_IO_CONTEXT_T;

	typedef struct _PER_SOCKET_CONTEXT_T
	{
		SOCKET				socket;
		LPPER_IO_CONTEXT_T	recv_context;
		LPPER_IO_CONTEXT_T	send_context;
	} PER_SOCKET_CONTEXT_T, *LPPER_SOCKET_CONTEXT_T;
}
#endif