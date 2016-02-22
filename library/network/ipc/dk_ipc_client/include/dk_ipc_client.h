#ifndef _DK_IPC_CLIENT_H_
#define _DK_IPC_CLIENT_H_

#include <cstdint>
#if !defined(WIN32)
#include <pthread.h>
#define EXP_CLASS
#else
#include <winsock2.h>
#include <windows.h>
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#endif

namespace ic
{
	class abstract_command;
	class abstract_ipc_client;
	class EXP_CLASS dk_ipc_client
	{
	public:
		dk_ipc_client(void);
		dk_ipc_client(const char * uuid);
		virtual ~dk_ipc_client(void);

		bool connect(char * address, int32_t port_number);
		bool disconnect(void);

		void data_request(char * dst, int32_t command_id, char * msg, int32_t length);
		void add_command(abstract_command * command);

		virtual void assoc_completion_callback(void) = 0;
		virtual void leave_completion_callback(void) = 0;

	private:
		abstract_ipc_client * _client;
	};
};









#endif