#ifndef _DK_SHARED_MEMORY_H_
#define _DK_SHARED_MEMORY_H_

#include <cstdint>
#if !defined(WIN32)
#include <pthread.h>
#define EXP_CLASS
#else
#include <winsock2.h>
#include <windows.h>
#if defined(EXPORT_SHARED_MEMORY_LIB)
#define EXP_SHARED_MEMORY_CLASS __declspec(dllexport)
#else
#define EXP_SHARED_MEMORY_CLASS __declspec(dllimport)
#endif
#endif

#define SM_BLOCK_COUNT	512
#define SM_BLOCK_SIZE	4096
#define SM_MAX_ADDR		256

namespace ic
{
	typedef EXP_SHARED_MEMORY_CLASS struct _shared_memory_block_t
	{
		long next;
		long prev;
		volatile long rdone;
		volatile long wdone;
		int32_t amount;
		int32_t padding; //padded used to ensure 64bit boundary
		uint8_t data[SM_BLOCK_SIZE];
	} shared_memory_block_t;

	typedef EXP_SHARED_MEMORY_CLASS struct _shared_memory_buffer_t
	{
		shared_memory_block_t blocks[SM_BLOCK_COUNT];
		//cursors
		volatile long rend;
		volatile long rbegin;
		volatile long wend;
		volatile long wbegin;
	} shared_memory_buffer_t;

	class shared_memory_server;
	class EXP_SHARED_MEMORY_CLASS dk_shared_memory_server
	{
	public:
		dk_shared_memory_server(void);
		virtual ~dk_shared_memory_server(void);

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

	private:
		shared_memory_server * _sms;
	};

	class shared_memory_client;
	class EXP_SHARED_MEMORY_CLASS  dk_shared_memory_client
	{
	public:
		dk_shared_memory_client(void);
		virtual ~dk_shared_memory_client(void);

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

	private:
		shared_memory_client * _smc;
	};
};
#endif