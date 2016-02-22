#ifndef _IOCP_SERVER_H_
#define _IOCP_SERVER_H_

#include <platform.h>
#include <session.h>
#include <iocp_handler.h>

namespace ic
{
	class abstract_ipc_server;
	class iocp_server
	{
	public:
		iocp_server(ic::abstract_ipc_server * processor);
		virtual ~iocp_server(void);


		bool initialize(void);
		void release(void);
		bool start(char * address, int32_t port_number);
		bool stop(void);

		void close(std::shared_ptr<ic::session> session);

		bool recv_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes);
		bool send_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes);
		bool other_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes);

		bool post_recv(std::shared_ptr<ic::session> session);
		bool post_send(std::shared_ptr<ic::session> session);
		//bool post_send(std::shared_ptr<ic::PER_SOCKET_CONTEXT_T> socket_context);

		void execute(void);

	private:
		static unsigned __stdcall process_cb(void * param);
		void process(void);
		std::shared_ptr<ic::session> allocate_session(SOCKET client_socket);
	private:
		abstract_ipc_server * _processor;
		SOCKET _listen_socket;
		iocp_handler * _iocp;
		HANDLE _thread;
		int32_t _send_retry_count;
	};

};





#endif