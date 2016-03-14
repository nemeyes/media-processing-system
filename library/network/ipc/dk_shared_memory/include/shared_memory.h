#ifndef _SHARED_MEMORY_H_
#define _SHARED_MEMORY_H_

#include "dk_shared_memory.h"

namespace ic
{
	class shared_memory_server
	{
	public:
		shared_memory_server(void);
		virtual ~shared_memory_server(void);

		bool create_shared_memory(const char * uuid);
		bool destroy_shared_memory(void);

		const char * uuid(void) const;
		bool check_smb(void);

#if defined(WITH_SERVER_PUBLISH)
		long write(void * buffer, long size, long timeout = INFINITE);
		bool wait_available(long timeout = INFINITE);
#else
		long read(void * buffer, long size, long timeout = INFINITE);
#endif
		shared_memory_block_t * block(long timeout = INFINITE);
		void block(shared_memory_block_t * blk);

	private:
		char _uuid[64];
		HANDLE _map;
		HANDLE _signal;
		HANDLE _available;
		shared_memory_buffer_t * _smb;
	};

	class shared_memory_client
	{
	public:
		shared_memory_client(void);
		virtual ~shared_memory_client(void);

		bool connect_shared_memory(const char * uuid);
		bool disconnect_shared_memory(void);

		const char * uuid(void) const;
		bool check_smb(void);

#if defined(WITH_SERVER_PUBLISH)
		long read(void * buffer, long size, long timeout = INFINITE);
#else
		long write(void * buffer, long size, long timeout = INFINITE);
		bool wait_available(long timeout = INFINITE);
#endif
		shared_memory_block_t * block(long timeout = INFINITE);
		void block(shared_memory_block_t * blk);

	private:
		char _uuid[64];
		HANDLE _map;
		HANDLE _signal;
		HANDLE _available;
		shared_memory_buffer_t * _smb;
	};














};

#endif