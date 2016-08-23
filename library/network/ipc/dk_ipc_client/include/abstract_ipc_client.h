#ifndef _ABSTRACT_IPC_CLIENT_H_
#define _ABSTRACT_IPC_CLIENT_H_

#include <platform.h>
#include <session.h>

namespace ic
{
	class dk_ipc_client;
	class abstract_command;
	class iocp_client;
	class abstract_ipc_client
	{
		friend class abstract_command;
		friend class iocp_client;
	public:
		abstract_ipc_client(dk_ipc_client * front);
		abstract_ipc_client(dk_ipc_client * front, const char * uuid);
		virtual ~abstract_ipc_client(void);

		bool connect(const char * address, int32_t port_number, bool retry_connection = true);
		bool disconnect(void);
		bool clear(void);

		const char * uuid(void);
		void uuid(const char * uuid);

		void data_indication_callback(const char * dst, const char * src, int32_t command_id, const char * msg, size_t length, std::shared_ptr<ic::session> session);
		void data_request(char * dst, int32_t command_id, char * msg, int32_t length);
		void data_request(char * dst, char * src, int32_t command_id, char * msg, int32_t length);
		void data_request(std::shared_ptr<ic::session> session);

		void add_command(abstract_command * command);
		void remove_command(int32_t command_id);

		void assoc_completion_callback(std::shared_ptr<ic::session> session);
		void leave_completion_callback(std::shared_ptr<ic::session> session);


	private:
		void clear_command_list(void);
		static unsigned __stdcall process_cb(void * param);
		void process(void);

		static unsigned __stdcall disconnect_process_cb(void * param);
		void disconnect_process(void);

	private:
		dk_ipc_client * _front;
		char _uuid[64];
		char _address[128];
		int32_t _port_number;
		iocp_client * _client;

		std::shared_ptr<ic::session> _session;

		std::map<int32_t, abstract_command*> _commands;

		HANDLE _thread;
		bool _run;

		bool _retry_connection;
		bool _connected;

		HANDLE _disconnect_thread;
		bool _disconnect_run;
		bool _do_disconnect;
	};
};








#endif