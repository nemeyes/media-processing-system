#include <Platform.h>
#include <command.h>
#pragma comment(lib,"ws2_32.lib")

#include "abstract_ipc_server.h"
#include "iocp_server.h"


ic::iocp_server::iocp_server(ic::abstract_ipc_server * processor)
	: _processor(processor)
	, _listen_socket(INVALID_SOCKET)
	, _send_retry_count(3)
{
	_iocp = new iocp_handler(this);
}

ic::iocp_server::~iocp_server(void)
{
	if (_iocp)
		delete _iocp;
	_iocp = nullptr;
}

bool ic::iocp_server::initialize(void)
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
		return false;
	return true;
}

void ic::iocp_server::release(void)
{
	WSACleanup();
}

bool ic::iocp_server::start(char * address, int32_t port_number)
{
	SOCKADDR_IN sock_addr;
	_listen_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_listen_socket == INVALID_SOCKET)
		return false;

	if (!address || strlen(address)<1)
		sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
	{
		sock_addr.sin_addr.s_addr = inet_addr(address);
	}
	
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons((short)port_number);
	if (bind(_listen_socket, (LPSOCKADDR)&sock_addr, sizeof(sock_addr)))
	{
		closesocket(_listen_socket);
		_listen_socket = INVALID_SOCKET;
		return false;
	}

	if (SOCKET_ERROR == listen(_listen_socket, SOMAXCONN))
	{
		closesocket(_listen_socket);
		_listen_socket = INVALID_SOCKET;
		return false;
	}


	int err_code;
	if (!_iocp->create(0, &err_code))
		return false;

	_iocp->create_thread_pool();

	unsigned int thread_id;
	_thread = (HANDLE)_beginthreadex(NULL, 0, iocp_server::process_cb, this, 0, &thread_id);
	return true;
}

bool ic::iocp_server::stop(void)
{
	closesocket(_listen_socket);
	_listen_socket = INVALID_SOCKET;

	if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
	{
		::CloseHandle(_thread);
	}

	if (_iocp)
	{
		_iocp->close_thread_pool();
	}

	return false;
}

void ic::iocp_server::close(std::shared_ptr<ic::session> session)
{
	_processor->unregister_client(session);
	session->shutdown_fd();
}

bool ic::iocp_server::recv_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes)
{
	session->push_recv_packet(session->recv_context()->buffer, nbytes);
	if (session->get_fd() != INVALID_SOCKET && session->recv_context().get() != NULL && session->send_context().get() != NULL)
	{
		bool value = post_recv(session);
		if (!value)
			return false;
	}
	return true;
}

bool ic::iocp_server::send_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes)
{
	session->pop_front_send_packet();
	post_send(session);
	return true;
}

bool ic::iocp_server::other_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes)
{
	// 현재 여기로 오면 Recv , Send 이외의 이상한 동작을 가리킴 소켓 끊어버리자.
	close(session);
	return true;
}

bool ic::iocp_server::post_recv(std::shared_ptr<ic::session> session)
{
	DWORD nbytes = 0;
	DWORD flags = 0;

	bool status = true;
	if ((session->get_fd() != INVALID_SOCKET))
	{
		session->recv_context()->wsabuf.buf = session->recv_context()->buffer;
		session->recv_context()->wsabuf.len = MTU;
		int value = WSARecv(session->get_fd(), &(session->recv_context()->wsabuf), 1, &nbytes, &flags, &(session->recv_context()->overlapped), NULL);
		if (SOCKET_ERROR == value)
		{
			int err_code = WSAGetLastError();
			if (err_code != WSA_IO_PENDING)
			{
				close(session);
				status = false;
			}
		}
	}
	return status;
}

bool ic::iocp_server::post_send(std::shared_ptr<ic::session> session)
{
	DWORD nbytes = 0;
	DWORD flags = 0;

	bool status = true;
	if ((session->get_fd() != INVALID_SOCKET))
	{
		int32_t length = 0;
		session->front_send_packet(session->send_context()->buffer, length);
		if (length>0)
		{
			session->send_context()->wsabuf.buf = session->send_context()->buffer;
			session->send_context()->wsabuf.len = length;
			int32_t value = WSASend(session->get_fd(), &(session->send_context()->wsabuf), 1, &nbytes, flags, &(session->send_context()->overlapped), NULL);
			if (SOCKET_ERROR == value)
			{
				int32_t err_code = WSAGetLastError();
				if (err_code != WSA_IO_PENDING)
				{
					close(session);
					status = false;
				}
			}
		}
	}
	return status;
}

unsigned __stdcall ic::iocp_server::process_cb(void * param)
{
	iocp_server * self = static_cast<iocp_server*>(param);
	self->process();
	return 0;
}

void ic::iocp_server::process(void)
{
	int err_code = 0;
	int sockaddr_size = sizeof(SOCKADDR_IN);
	SOCKADDR_IN client_sockaddr;

	SOCKET client_socket = INVALID_SOCKET;
	std::shared_ptr<ic::session> session = NULL;
	while (true)
	{
		client_socket = accept(_listen_socket, (LPSOCKADDR)&client_sockaddr, &sockaddr_size);
		if (client_socket == INVALID_SOCKET)
		{
			// 리슨 소켓을 클로즈 하면 이 에러가 나오므로
			// 이 에러시에 Accept 루프를 빠져나간다.
			if (WSAGetLastError() == WSAEINTR)
				return;
		}

		int zero = 0;
		if (SOCKET_ERROR == setsockopt(client_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int)))
			continue;

		zero = 0;
		if (SOCKET_ERROR == setsockopt(client_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int)))
			continue;

		zero = 0;
		if (SOCKET_ERROR == setsockopt(client_socket, SOL_SOCKET, TCP_NODELAY, (char *)&zero, sizeof(int))) //disable nagle algorithm
			continue;

		// 소켓 컨텍스트 할당 -> Completion Key
		session = allocate_session(client_socket);
		if (!session)
			continue;

		// IOCP 커널 객체와 연결
		if (!_iocp->associate(client_socket, reinterpret_cast<ULONG_PTR>(session.get()), &err_code))
			continue;

		// 초기 Recv 요청
		bool value = post_recv(session);
		if (value == false)
			continue;
	}
}

std::shared_ptr<ic::session> ic::iocp_server::allocate_session(SOCKET client_socket)
{
	std::shared_ptr<ic::session> session(new ic::session(_processor, client_socket));
	return session;
}

void ic::iocp_server::execute(void)
{
	ic::session * session = nullptr;
	ic::PER_IO_CONTEXT_T * io_context = nullptr;
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
			close(session->shared_from_this());
		}
	}
}