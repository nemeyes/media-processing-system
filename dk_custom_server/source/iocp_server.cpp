#pragma once
//#include "MemoryPooler.h"

class net_processor;
class iocp_handler;
class iocp_server
{
public:
	iocp_server(net_processor * processor);
	~iocp_server(void);


	bool initialize(void);
	void release(void);
	bool start(void);
	bool stop(void);

	void close_client(LPPER_SOCKET_CONTEXT_T socket_context, BOOL graceful);

	bool on_receive_completion(LPPER_SOCKET_CONTEXT_T socket_context, DWORD bytes_of_transfered);
	bool on_send_completion(LPPER_SOCKET_CONTEXT_T socket_context, DWORD bytes_of_transfered);
	bool on_other_completion(LPPER_SOCKET_CONTEXT_T socket_context, DWORD bytes_of_transfered);

	bool PostReceiving(LPPER_SOCKET_CONTEXT_T socket_context);
	bool PostSending(CHAR *payload, UINT length, LPPER_SOCKET_CONTEXT_T socket_context);

	VOID	Execute(VOID);


private:
	static UINT CALLBACK	Process(VOID *param);
	LPPER_IO_CONTEXT_T		AllocPerIoContextForRecv(VOID);
	LPPER_IO_CONTEXT_T		AllocPerIoContextForSend(VOID);
	LPPER_SOCKET_CONTEXT_T	AllocPerSocketContext(SOCKET client_socket);
	VOID					DeallocPerIoContextForSend(LPPER_IO_CONTEXT_T io_context);
	VOID					DeallocPerIoContextForRecv(LPPER_IO_CONTEXT_T io_context);
	VOID					DeallocPerSocketContext(LPPER_SOCKET_CONTEXT_T io_context);

	BOOL					ConvertWide2Multibyte(WCHAR *source, char **destination);
	BOOL					ConvertMultibyte2Wide(char *source, WCHAR **destination);

private:
	SOCKET								_listen_socket;
	IocpHandler							*_iocp;
	BusinessProcessor*					_processor;
	CRITICAL_SECTION					_lock;

	MemoryPooler<PER_SOCKET_CONTEXT_T>	*_socket_context_mempool;
	MemoryPooler<PER_IO_CONTEXT_T>		*_recv_mempool;
	MemoryPooler<PER_IO_CONTEXT_T>		*_send_mempool;
};

