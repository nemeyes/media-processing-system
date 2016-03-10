#ifndef _DK_SHARED_MEMORY_H_
#define _DK_SHARED_MEMORY_H_

#include <windows.h>
#include <cstdint>

#define SM_BLOCK_COUNT	512
#define SM_BLOCK_SIZE	4096
#define SM_MAX_ADDR		256

namespace ic
{
	typedef struct _SHARED_MEMORY_BLOCK_T
	{
		long next;
		long prev;
		volatile long rdone;
		volatile long wdone;
		int32_t amount;
		int32_t padding; //padded used to ensure 64bit boundary
		uint8_t data[SM_BLOCK_SIZE];
	} SHARED_MEMORY_BLOCK_T;

	typedef struct _SHARED_MEMORY_BUFFER_T
	{
		SHARED_MEMORY_BLOCK_T	blocks[SM_BLOCK_COUNT];

		//cursors
		volatile long	rend;
		volatile long	rbegin;

		volatile long	wend;
		volatile long	wbegin;
	} SHARED_MEMORY_BUFFER_T;

	static long id(void)
	{
		static volatile long id = 1;
		return (long)InterlockedIncrement((long*)&id);
	};

	class dk_shared_memory_server
	{
	public:
		dk_shared_memory_server(void);
		virtual ~dk_shared_memory_server(void);

		long read(void * buffer, long size, long timeout = INFINITE);
		char * address(void) { return _address; }
		
		//Block Functions
		SHARED_MEMORY_BLOCK_T * block(long timeout = INFINITE);
		void block(SHARED_MEMORY_BLOCK_T * block);

		void create(void);
		void close(void);

	private:
		char * _address;
		HANDLE _map;
		HANDLE _signal;
		HANDLE _available;
		SHARED_MEMORY_BUFFER_T * _smb;
	};

	class dk_shared_memory_client
	{

	};









};
#endif