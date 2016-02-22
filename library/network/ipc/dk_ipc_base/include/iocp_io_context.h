#ifndef _IOCP_IO_CONTEXT_H_
#define _IOCP_IO_CONTEXT_H_

#include <platform.h>

#define MTU 1500
#define MAX_SEND_BUFFER_SIZE	1500
#define MAX_RECV_BUFFER_SIZE	1024*512

#define SEND_RETRY_COUNT	1000
#define SEND_SLEEP_TIME		1

namespace ic
{
	typedef struct _PER_IO_CONTEXT_T
	{
		WSAOVERLAPPED overlapped;
		WSABUF wsabuf;
		char buffer[MTU];
		_PER_IO_CONTEXT_T(void)
		{
			memset(&overlapped, 0x00, sizeof(WSAOVERLAPPED));
			memset(&wsabuf, 0x00, sizeof(WSABUF));
			memset(buffer, 0x00, sizeof(buffer));
		}
	} PER_IO_CONTEXT_T;
}
#endif