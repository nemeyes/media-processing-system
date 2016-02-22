#ifndef _COMMAND_H_
#define _COMMAND_H_

#if defined(WITH_WORKING_AS_SERVER)
#include <rpc.h>
#include "abstract_ipc_server.h"
#else
#include "abstract_ipc_client.h"
#endif

#define COMMAND_SIZE sizeof(int)

#define CMD_ERR_CODE_SUCCESS		0
#define CMD_ERR_CODE_FAIL			1

#define CMD_ASSOC_REQUEST			10
#define CMD_ASSOC_RESPONSE			11
#define CMD_LEAVE_INDICATION		12
#if defined(WITH_LEAVE_CMD)
#define CMD_LEAVE_REQUEST			13
#define CMD_LEAVE_RESPONSE			14
#endif
#define CMD_KEEPALIVE_REQUEST		15
#define CMD_KEEPALIVE_RESPONSE		16



namespace ic
{
	typedef struct _CMD_PAYLOAD_T
	{
		int32_t	code;
	} CMD_PAYLOAD_T;

	typedef struct _CMD_ASSOC_PAYLOAD_T : public _CMD_PAYLOAD_T
	{
		char uuid[64];
	} CMD_ASSOC_PAYLOAD_T;

	typedef struct _CMD_LEAVE_PAYLOAD_T : public _CMD_PAYLOAD_T
	{
	} CMD_LEAVE_PAYLOAD_T;

	typedef struct _CMD_KEEPALIVE_PAYLOAD_T : public _CMD_PAYLOAD_T
	{
	} CMD_KEEPALIVE_PAYLOAD_T;

#if defined(WITH_WORKING_AS_SERVER)
	class abstract_ipc_server;
#else
	class abstract_ipc_client;
#endif
	class abstract_command
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		abstract_command(abstract_ipc_server	* processor, int32_t command_id)
#else
		abstract_command(abstract_ipc_client	* processor, int32_t command_id)
#endif
			: _processor(processor)
			, _command_id(command_id)
		{}

		int32_t command_id(void)
		{
			return _command_id;
		}

		virtual void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session) = 0;

	protected:
#if defined(WITH_WORKING_AS_SERVER)
		abstract_ipc_server	*_processor;
#else
		abstract_ipc_client	*_processor;
#endif
		int32_t	_command_id;
	};

#if defined(WITH_WORKING_AS_SERVER)
	class assoc_req_cmd : public abstract_command
	{
	public:
		assoc_req_cmd(abstract_ipc_server * processor)
			: abstract_command(processor, CMD_ASSOC_REQUEST)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_ASSOC_PAYLOAD_T payload;
			memset(&payload, 0x00, sizeof(CMD_ASSOC_PAYLOAD_T));
			memcpy(&payload, msg, sizeof(CMD_ASSOC_PAYLOAD_T));

			bool code = false;
			const char * prev_uuid = _processor->check_regstered_client(session);
			if (prev_uuid)
			{
				strcpy_s(payload.uuid, prev_uuid);
				session->set_uuid(payload.uuid);
				session->set_assoc_flag(true);
				payload.code = CMD_ERR_CODE_SUCCESS;
			}
			else
			{
				UUID uuid;
				::ZeroMemory(&uuid, sizeof(UUID));
				::UuidCreate(&uuid);


				char * new_uuid = nullptr;
				::UuidToStringA(&uuid, (RPC_CSTR*)&new_uuid);

				if (new_uuid)
				{
					code = _processor->register_client(new_uuid, session);
					if (code)
					{
						strcpy_s(payload.uuid, new_uuid);
						session->set_uuid(payload.uuid);
						session->set_assoc_flag(true);
						payload.code = CMD_ERR_CODE_SUCCESS;
					}
					else
					{
						payload.code = CMD_ERR_CODE_FAIL;
					}

					::RpcStringFreeA((RPC_CSTR*)&new_uuid);
				}
				else
				{
					payload.code = CMD_ERR_CODE_FAIL;
				}
			}
			session->push_send_packet(CMD_ASSOC_RESPONSE, reinterpret_cast<char*>(&payload), sizeof(CMD_ASSOC_PAYLOAD_T));
			if (code)
				_processor->assoc_completion_callback(payload.uuid, session);
		}
	};
#else
	class assoc_res_cmd : public abstract_command
	{
	public:
		assoc_res_cmd(abstract_ipc_client * processor)
			: abstract_command(processor, CMD_ASSOC_RESPONSE)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_ASSOC_PAYLOAD_T payload;
			memset(&payload, 0x00, sizeof(CMD_ASSOC_PAYLOAD_T));
			memcpy(&payload, msg, sizeof(CMD_ASSOC_PAYLOAD_T));

			if (payload.code == CMD_ERR_CODE_SUCCESS)
			{
				session->set_assoc_flag(true);
				session->set_uuid(payload.uuid);
				_processor->set_uuid(payload.uuid);
				_processor->assoc_completion_callback(session);
			}
		}
	};
#endif

#if defined(WITH_WORKING_AS_SERVER)
#if defined(WITH_LEAVE_CMD)
	class leave_req_cmd : public abstract_command
	{
	public:
		leave_req_cmd(abstract_ipc_server * processor)
			: abstract_command(processor, CMD_LEAVE_REQUEST)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_LEAVE_PAYLOAD_T payload;
			memset(&payload, 0x00, sizeof(CMD_LEAVE_PAYLOAD_T));
			memcpy(&payload, msg, sizeof(CMD_LEAVE_PAYLOAD_T));

			bool code = _processor->unregister_client(src);
			payload.code = code ? CMD_ERR_CODE_SUCCESS : CMD_ERR_CODE_FAIL;

			session->push_send_packet(CMD_LEAVE_RESPONSE, reinterpret_cast<char*>(&payload), sizeof(CMD_LEAVE_PAYLOAD_T));
			if (code)
				_processor->leave_completion_callback(src, session);
		}
	};
#endif
#else
	class leave_ind_cmd : public abstract_command
	{
	public:
		leave_ind_cmd(abstract_ipc_client * processor)
			: abstract_command(processor, CMD_LEAVE_INDICATION)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_LEAVE_PAYLOAD_T payload;
			memset(&payload, 0x00, sizeof(CMD_LEAVE_PAYLOAD_T));
			memcpy(&payload, msg, sizeof(CMD_LEAVE_PAYLOAD_T));

			session->shutdown_fd();
			_processor->disconnect(true);
		}
	};

#if defined(WITH_LEAVE_CMD)
	class leave_res_cmd : public abstract_command
	{
	public:
		leave_res_cmd(abstract_ipc_client * processor)
			: abstract_command(processor, CMD_LEAVE_RESPONSE)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_LEAVE_PAYLOAD_T payload;
			memset(&payload, 0x00, sizeof(CMD_LEAVE_PAYLOAD_T));
			memcpy(&payload, msg, sizeof(CMD_LEAVE_PAYLOAD_T));

			session->shutdown_fd();
			_processor->disconnect(true);
		}
	};
#endif
#endif

	class keepalive_req_cmd : public abstract_command
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		keepalive_req_cmd(abstract_ipc_server * processor)
#else
		keepalive_req_cmd(abstract_ipc_client * processor)
#endif
			: abstract_command(processor, CMD_KEEPALIVE_REQUEST)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{
			CMD_KEEPALIVE_PAYLOAD_T payload;
			memset(&payload, 0x00, sizeof(CMD_KEEPALIVE_PAYLOAD_T));
			memcpy(&payload, msg, sizeof(CMD_KEEPALIVE_PAYLOAD_T));
			payload.code = CMD_ERR_CODE_SUCCESS;
			session->push_send_packet(CMD_KEEPALIVE_RESPONSE, reinterpret_cast<char*>(&payload), sizeof(CMD_KEEPALIVE_PAYLOAD_T));
		}
	};

	class keepalive_res_cmd : public abstract_command
	{
	public:
#if defined(WITH_WORKING_AS_SERVER)
		keepalive_res_cmd(abstract_ipc_server * processor)
#else
		keepalive_res_cmd(abstract_ipc_client * processor)
#endif
			: abstract_command(processor, CMD_KEEPALIVE_RESPONSE)
		{}

		void execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
		{

		}
	};
};


#endif