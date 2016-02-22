#ifndef _IOCP_HANDLER_H_
#define _IOCP_HANDLER_H_

#include <platform.h>

namespace ic
{
#if defined(WITH_WORKING_AS_SERVER)
	class iocp_server;
#else
	class iocp_client;
#endif
	class iocp_handler
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		iocp_handler(iocp_server * iocp_server);
#else
		iocp_handler(iocp_client * iocp_client);
#endif
		virtual ~iocp_handler(void);

		bool create(int32_t number_of_pooled_threads = 0, int32_t * error_code = NULL);
		bool associate(SOCKET socket, ULONG_PTR key, int32_t * error_code = NULL);
		bool associate(HANDLE handle, ULONG_PTR key, int32_t * error_code = NULL);
		bool post_completion_status(ULONG_PTR key, DWORD bytes_of_transfered = 0, OVERLAPPED * overlapped = NULL, int32_t * error_code = NULL);
		bool get_completion_status(ULONG_PTR * key, LPDWORD bytes_of_transfered, LPOVERLAPPED * overlapped, int32_t * error_code = NULL, DWORD waiting_time = INFINITE);
		void create_thread_pool(void);
		void close_thread_pool(void);

	private:
		static unsigned __stdcall process(void * param);

	private:
#if defined(WITH_WORKING_AS_SERVER)
		iocp_server * _iocp_server;
#else
		iocp_client * _iocp_client;
#endif
		int32_t _number_of_threads;
		std::vector<HANDLE>	_threads;
		HANDLE _iocp_handle;
	};
}





#endif