#ifndef _SHARED_MEMORY_H_
#define _SHARED_MEMORY_H_

#include "dk_shared_memory.h"

namespace ic
{
	class shared_memory_server
	{
	public:
		shared_memory_server(const char * uuid);
		virtual ~shared_memory_server(void);

		const char * uuid(void) const;
		bool check_smb(void);

#if defined(WITH_SERVER_PUBLISH)
		long write(void * buffer, long size, long timeout = INFINITE);
		bool wait_available(long timeout = INFINITE);
#else
		long read(void * buffer, long size, long timeout = INFINITE);
#endif
		SHARED_MEMORY_BLOCK_T * block(long timeout = INFINITE);
		void block(SHARED_MEMORY_BLOCK_T * blk);

	private:
		void create(void);
		void close(void);

	private:
		char _uuid[64];
		HANDLE _map;
		HANDLE _signal;
		HANDLE _available;
		SHARED_MEMORY_BUFFER_T * _smb;
	};

	class shared_memory_client
	{
	public:
		shared_memory_client(const char * uuid);
		virtual ~shared_memory_client(void);

		const char * uuid(void) const;
		bool check_smb(void);

#if defined(WITH_SERVER_PUBLISH)
		long read(void * buffer, long size, long timeout = INFINITE);
#else
		long write(void * buffer, long size, long timeout = INFINITE);
		bool wait_available(long timeout = INFINITE);
#endif
		SHARED_MEMORY_BLOCK_T * block(long timeout = INFINITE);
		void block(SHARED_MEMORY_BLOCK_T * blk);

	private:
		char _uuid[64];
		HANDLE _map;
		HANDLE _signal;
		HANDLE _available;
		SHARED_MEMORY_BUFFER_T * _smb;
	};














};

#endif