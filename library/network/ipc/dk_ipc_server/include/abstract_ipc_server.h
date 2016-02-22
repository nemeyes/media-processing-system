#ifndef _ABSTRACT_IPC_SERVER_H_
#define _ABSTRACT_IPC_SERVER_H_

#include <platform.h>
#include <session.h>

namespace ic
{
	class dk_ipc_server;
	class abstract_command;
	class iocp_server;
	class abstract_ipc_server
	{
	public:
		abstract_ipc_server(dk_ipc_server * front, const char * uuid);
		virtual ~abstract_ipc_server(void);

		bool start(char * address, int32_t port_number);
		bool stop(void);

		void data_indication_callback(const char * dst, const char * src, int32_t command_id, const char * msg, size_t length, std::shared_ptr<ic::session> session);
		void data_request(char * dst, int32_t command_id, char * msg, size_t length);
		void data_request(char * dst, char * src, int32_t command_id, char * msg, size_t length);
		void data_request(std::shared_ptr<ic::session> session);

		const char * check_regstered_client(std::shared_ptr<ic::session> session);
		bool register_client(const char * uuid, std::shared_ptr<ic::session> session);
		bool unregister_client(const char * uuid);
		bool unregister_client(std::shared_ptr<ic::session> session);
		std::map<std::string, std::shared_ptr<ic::session>> get_clients(void);

		void add_command(abstract_command * command);
		void remove_command(int32_t command_id);

		void assoc_completion_callback(const char * uuid, std::shared_ptr<ic::session> session);
		void leave_completion_callback(const char * uuid, std::shared_ptr<ic::session> session);

	private:
		void clear_command_list(void);
		static unsigned __stdcall process_cb(void * param);
		void process(void);


	private:
		dk_ipc_server * _front;
		char _uuid[64];
		char _address[128];
		int32_t _port_number;
		iocp_server * _server;
		int32_t _sequence;

		HANDLE _session_lock;
		std::map<std::string, std::shared_ptr<ic::session>> _sessions;

		std::map<int32_t, abstract_command*> _commands;

		HANDLE _thread;
		bool _run;
	};
};


#endif