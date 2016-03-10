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

		long read(void * buffer, long size, long timeout = INFINITE);
		const char * address(void) const;

		//Block Functions
		SHARED_MEMORY_BLOCK_T * block(long timeout = INFINITE);
		void block(SHARED_MEMORY_BLOCK_T * block);

	private:
		void create(void);
		void close(void);

	private:
		char * _address;
		HANDLE _map;
		HANDLE _signal;
		HANDLE _available;
		SHARED_MEMORY_BUFFER_T * _smb;
	};
















};

#endif