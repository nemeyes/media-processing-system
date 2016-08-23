#include "Platform.h"
#include <command.h>
#pragma comment(lib,"ws2_32.lib")

#include "abstract_ipc_client.h"
#include "iocp_client.h"


ic::iocp_client::iocp_client(ic::abstract_ipc_client * processor)
	: _processor(processor)
{
	_iocp = new iocp_handler(this);
}

ic::iocp_client::~iocp_client(void)
{
	if (_iocp)
		delete _iocp;
	_iocp = nullptr;

}

bool ic::iocp_client::initialize(void)
{
	return true;
}

void ic::iocp_client::release(void)
{

}

std::shared_ptr<ic::session> ic::iocp_client::connect(const char * address, int32_t port_number, bool retry_connection)
{
	SOCKADDR_IN	sockaddr;
	SOCKET socket = INVALID_SOCKET;

	int32_t err_code;
	if (!_iocp->create(2, &err_code))
		return std::shared_ptr<ic::session>();
	_iocp->create_thread_pool();

	int	value = -1;
	int	length = -1;
	int ErrCode = 0;
	socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
		return std::shared_ptr<ic::session>();

	memset(&sockaddr, 0x00, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr(address);
	sockaddr.sin_port = htons(port_number);

	value = WSAConnect(socket, (SOCKADDR*)(&sockaddr), sizeof(sockaddr), NULL, NULL, NULL, NULL);
	if ((value == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
	{
		closesocket(socket);
		return std::shared_ptr<ic::session>();
	}
	else
	{
		int zero = 0;
		if (SOCKET_ERROR == setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int)))
		{
			closesocket(socket);
			return std::shared_ptr<ic::session>();
		}

		zero = 0;
		if (SOCKET_ERROR == setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int)))
		{
			closesocket(socket);
			return std::shared_ptr<ic::session>();
		}

		zero = 0;
		if (SOCKET_ERROR == setsockopt(socket, SOL_SOCKET, TCP_NODELAY, (char *)&zero, sizeof(int))) //disable nagle algorithm
		{
			closesocket(socket);
			return std::shared_ptr<ic::session>();
		}

		std::shared_ptr<ic::session> session = allocate_session(socket);
		if (!_iocp->associate((session)->fd(), reinterpret_cast<ULONG_PTR>(session.get()), &err_code))
		{
			return std::shared_ptr<ic::session>();
		}

		if (!post_recv((session)))
		{
			session->shutdown_fd();
			return std::shared_ptr<ic::session>();
		}
		return session;
	}
}

bool ic::iocp_client::disconnect(void)
{
	_iocp->close_thread_pool();
	return true;
}

bool ic::iocp_client::recv_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes)
{
	session->push_recv_packet(session->recv_context()->buffer, nbytes);
	if (session->fd() != INVALID_SOCKET && session->recv_context().get() != NULL && session->send_context().get() != NULL)
	{
		bool value = post_recv(session);
		if (!value)
			return false;
	}
	return true;
}

bool ic::iocp_client::send_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes)
{
	session->pop_front_send_packet();
	post_send(session);
	return true;
}

bool ic::iocp_client::other_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes)
{
	// 현재 여기로 오면 Recv , Send 이외의 이상한 동작을 가리킴 소켓 끊어버리자.
	session->shutdown_fd();
	return true;
}

bool ic::iocp_client::post_recv(std::shared_ptr<ic::session> session)
{
	DWORD nbytes = 0;
	DWORD flags = 0;

	if ((session->fd() != INVALID_SOCKET))
	{
		session->recv_context()->wsabuf.buf = session->recv_context()->buffer;
		session->recv_context()->wsabuf.len = MTU;
		int value = WSARecv(session->fd(), &(session->recv_context()->wsabuf), 1, &nbytes, &flags, &(session->recv_context()->overlapped), NULL);
		if (SOCKET_ERROR == value)
		{
			int err_code = WSAGetLastError();
			if (err_code != WSA_IO_PENDING)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

bool ic::iocp_client::post_send(std::shared_ptr<ic::session> session)
{
	DWORD nbytes = 0;
	DWORD flags = 0;

	bool status = true;
	if ((session->fd() != INVALID_SOCKET))
	{
		int32_t length = 0;
		session->front_send_packet(session->send_context()->buffer, length);
		if (length>0)
		{
			session->send_context()->wsabuf.buf = session->send_context()->buffer;
			session->send_context()->wsabuf.len = length;
			int32_t value = WSASend(session->fd(), &(session->send_context()->wsabuf), 1, &nbytes, flags, &(session->send_context()->overlapped), NULL);
			if (SOCKET_ERROR == value)
			{
				int32_t err_code = WSAGetLastError();
				if (err_code != WSA_IO_PENDING)
					status = false;
			}
		}
	}
	return status;
}

std::shared_ptr<ic::session> ic::iocp_client::allocate_session(SOCKET client_socket)
{
	std::shared_ptr<ic::session> session(new ic::session(_processor, client_socket));
	return session;
}

void ic::iocp_client::execute(void)
{
	ic::session * session = NULL;
	PER_IO_CONTEXT_T * io_context = NULL;
	DWORD nbytes = 0;
	int32_t err_code = 0;

	while (1)
	{
		// IO Completion Packet 얻어온다.
		bool value = _iocp->get_completion_status(reinterpret_cast<ULONG_PTR*>(&session), &nbytes, reinterpret_cast<LPOVERLAPPED*>(&io_context), &err_code);
		if (value)
		{
			if (((int32_t)session) == KILL_THREAD)
				break;

			if (io_context == NULL)
				continue;
		}
		else
		{
			if (io_context == NULL)
				continue;
		}

		try
		{
			// 클라이언트가 연결 끊음 
			if (nbytes == 0)
			{
				throw _T("dwBytestransferred==0");
			}

			// IO 성격에 따라 그에 따른 처리
			if (io_context == session->recv_context().get())
			{
				if (!recv_completion_callback(session->shared_from_this(), nbytes))
					throw _T("RecvCompleteEvent Error");
			}
			else if (io_context == session->send_context().get())
			{
				if (!send_completion_callback(session->shared_from_this(), nbytes))
					throw _T("SendCompleteEvent Error");
			}
			else
			{
				if (!other_completion_callback(session->shared_from_this(), nbytes))
					throw _T("OtherCompleteEvent Error");
			}

		}
		catch (TCHAR*)
		{
			//session->shutdown_fd();
			//_processor->disconnect(true);
			//_processor->clear();
		}
	}

	int a = 2;
}