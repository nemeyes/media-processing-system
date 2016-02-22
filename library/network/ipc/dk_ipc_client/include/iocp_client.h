#ifndef _IOCP_CLIENT_H_
#define _IOCP_CLIENT_H_

#include <platform.h>
#include <session.h>
#include <iocp_handler.h>

namespace ic
{
	class abstract_ipc_client;
	class iocp_client
	{
	public:
		iocp_client(ic::abstract_ipc_client * processor);
		virtual ~iocp_client(void);


		bool initialize(void);
		void release(void);
		std::shared_ptr<ic::session> connect(const char * address, int32_t port_number, bool retry_connection = true);
		bool disconnect(void);

		bool recv_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes);
		bool send_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes);
		bool other_completion_callback(std::shared_ptr<ic::session> session, int32_t nbytes);

		bool post_recv(std::shared_ptr<ic::session> session);
		bool post_send(std::shared_ptr<ic::session> session);

		void execute(void);

	private:
		std::shared_ptr<ic::session> allocate_session(SOCKET socket);


	private:
		abstract_ipc_client * _processor;
		iocp_handler * _iocp;
	};

};

#endif