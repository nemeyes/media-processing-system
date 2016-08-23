#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <cstdint>
#include <memory>
#include <session.h>

#if defined(WITH_WORKING_AS_SERVER)
#include <rpc.h>
#include "abstract_ipc_server.h"
#else
#include "abstract_ipc_client.h"
#endif

#define SERVER_UUID		"00000000-0000-0000-0000-000000000000"
#define BROADCAST_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"
#define UNDEFINED_UUID	"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"

#define COMMAND_SIZE sizeof(int)

#define CMD_ERR_CODE_SUCCESS		0
#define CMD_ERR_CODE_FAIL			1

#define CMD_ASSOC_REQUEST			10
#define CMD_ASSOC_RESPONSE			11
#define CMD_LEAVE_INDICATION		12
#define CMD_LEAVE_REQUEST			13
#define CMD_LEAVE_RESPONSE			14
#define CMD_KEEPALIVE_REQUEST		15
#define CMD_KEEPALIVE_RESPONSE		16


namespace ic
{
	typedef struct _CMD_PAYLOAD_T
	{
		int32_t	code;
	} CMD_PAYLOAD_T;

	typedef struct _CMD_ASSOC_RES_T : public _CMD_PAYLOAD_T
	{
		char uuid[64];
	} CMD_ASSOC_RES_T;

#if defined(WITH_WORKING_AS_SERVER)
	class abstract_ipc_server;
#else
	class abstract_ipc_client;
#endif
	class abstract_command
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		abstract_command(abstract_ipc_server * processor, int32_t command_id);
		abstract_command(int32_t command_id);
		void set_processor(abstract_ipc_server * processor);
		abstract_ipc_server * get_processor(void);
#else
		abstract_command(abstract_ipc_client * processor, int32_t command_id);
		abstract_command(int32_t command_id);
		void set_processor(abstract_ipc_client * processor);
		abstract_ipc_client * get_processor(void);
#endif
		virtual ~abstract_command(void);

		const char * uuid(void);
		int32_t command_id(void);
		virtual void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session) = 0;

	protected:
#if defined(WITH_WORKING_AS_SERVER)
		abstract_ipc_server	* _processor;
#else
		abstract_ipc_client	* _processor;
#endif
		int32_t	_command_id;
	};

#if defined(WITH_WORKING_AS_SERVER)
	class assoc_req_cmd : public abstract_command
	{
	public:
		assoc_req_cmd(abstract_ipc_server * processor);
		virtual ~assoc_req_cmd(void);
		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session);
	};
#else
	class assoc_res_cmd : public abstract_command
	{
	public:
		assoc_res_cmd(abstract_ipc_client * processor);
		virtual ~assoc_res_cmd(void);
		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session);
	};
#endif


	class leave_ind_cmd : public abstract_command
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		leave_ind_cmd(abstract_ipc_server * processor);
#else
		leave_ind_cmd(abstract_ipc_client * processor);
#endif
		virtual ~leave_ind_cmd(void);
		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session);
	};

	class keepalive_req_cmd : public abstract_command
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		keepalive_req_cmd(abstract_ipc_server * processor);
#else
		keepalive_req_cmd(abstract_ipc_client * processor);
#endif
		virtual ~keepalive_req_cmd(void);
		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session);
	};

	class keepalive_res_cmd : public abstract_command
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		keepalive_res_cmd(abstract_ipc_server * processor);
#else
		keepalive_res_cmd(abstract_ipc_client * processor);
#endif
		virtual ~keepalive_res_cmd(void);
		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session);
	};
};


#endif