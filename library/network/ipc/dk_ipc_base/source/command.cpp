#include <command.h>
#if defined(WITH_WORKING_AS_SERVER)
ic::abstract_command::abstract_command(abstract_ipc_server * processor, int32_t command_id)
#else
ic::abstract_command::abstract_command(abstract_ipc_client * processor, int32_t command_id)
#endif
	: _processor(processor)
	, _command_id(command_id)
{}


#if defined(WITH_WORKING_AS_SERVER)
ic::abstract_command::abstract_command(int32_t command_id)
#else
ic::abstract_command::abstract_command(int32_t command_id)
#endif
	: _processor(nullptr)
	, _command_id(command_id)
{}

ic::abstract_command::~abstract_command(void)
{}

#if defined(WITH_WORKING_AS_SERVER)
void ic::abstract_command::set_processor(ic::abstract_ipc_server * processor)
{
	_processor = processor;
}

ic::abstract_ipc_server * ic::abstract_command::get_processor(void)
{
	return _processor;
}
#else
void ic::abstract_command::set_processor(ic::abstract_ipc_client * processor)
{
	_processor = processor;
}

ic::abstract_ipc_client * ic::abstract_command::get_processor(void)
{
	return _processor;
}
#endif

const char * ic::abstract_command::uuid(void)
{
	return _processor->uuid();
}

int32_t ic::abstract_command::command_id(void)
{
	return _command_id;
}

#if defined(WITH_WORKING_AS_SERVER)
ic::assoc_req_cmd::assoc_req_cmd(abstract_ipc_server * processor)
	: abstract_command(processor, CMD_ASSOC_REQUEST)
{}

ic::assoc_req_cmd::~assoc_req_cmd(void)
{}

void ic::assoc_req_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{
	CMD_ASSOC_PAYLOAD_T payload;
	memset(&payload, 0x00, sizeof(CMD_ASSOC_PAYLOAD_T));
	memcpy(&payload, msg, sizeof(CMD_ASSOC_PAYLOAD_T));

	bool code = false;
	const char * prev_uuid = _processor->check_regstered_client(session);
	if (prev_uuid)
	{
		strcpy_s(payload.uuid, prev_uuid);
		session->uuid(payload.uuid);
		session->assoc_flag(true);
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
				session->uuid(payload.uuid);
				session->assoc_flag(true);
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
	session->push_send_packet(session->uuid(), uuid(), CMD_ASSOC_RESPONSE, reinterpret_cast<char*>(&payload), sizeof(CMD_ASSOC_PAYLOAD_T));
	if (code)
		_processor->assoc_completion_callback(payload.uuid, session);
}
#else
ic::assoc_res_cmd::assoc_res_cmd(abstract_ipc_client * processor)
	: abstract_command(processor, CMD_ASSOC_RESPONSE)
{}

ic::assoc_res_cmd::~assoc_res_cmd(void)
{}

void ic::assoc_res_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{
	CMD_ASSOC_PAYLOAD_T payload;
	memset(&payload, 0x00, sizeof(CMD_ASSOC_PAYLOAD_T));
	memcpy(&payload, msg, sizeof(CMD_ASSOC_PAYLOAD_T));

	if (payload.code == CMD_ERR_CODE_SUCCESS)
	{
		session->assoc_flag(true);
		session->uuid(payload.uuid);
		_processor->uuid(payload.uuid);
		_processor->assoc_completion_callback(session);
	}
}
#endif

#if defined(WITH_WORKING_AS_SERVER)
#if defined(WITH_LEAVE_CMD)
ic::leave_req_cmd::leave_req_cmd(abstract_ipc_server * processor)
	: abstract_command(processor, CMD_LEAVE_REQUEST)
{}

ic::leave_req_cmd::~leave_req_cmd(void)
{}

void ic::leave_req_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{
	CMD_LEAVE_PAYLOAD_T payload;
	memset(&payload, 0x00, sizeof(CMD_LEAVE_PAYLOAD_T));
	memcpy(&payload, msg, sizeof(CMD_LEAVE_PAYLOAD_T));

	bool code = _processor->unregister_client(src);
	payload.code = code ? CMD_ERR_CODE_SUCCESS : CMD_ERR_CODE_FAIL;

	session->push_send_packet(session->uuid(), uuid(), CMD_LEAVE_RESPONSE, reinterpret_cast<char*>(&payload), sizeof(CMD_LEAVE_PAYLOAD_T));
	if (code)
		_processor->leave_completion_callback(src, session);
}
#endif
#else
ic::leave_ind_cmd::leave_ind_cmd(abstract_ipc_client * processor)
	: abstract_command(processor, CMD_LEAVE_INDICATION)
{}

ic::leave_ind_cmd::~leave_ind_cmd(void)
{}

void ic::leave_ind_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{
	CMD_LEAVE_PAYLOAD_T payload;
	memset(&payload, 0x00, sizeof(CMD_LEAVE_PAYLOAD_T));
	memcpy(&payload, msg, sizeof(CMD_LEAVE_PAYLOAD_T));

	session->shutdown_fd();
	_processor->disconnect(true);
}

#if defined(WITH_LEAVE_CMD)
ic::leave_res_cmd::leave_res_cmd(abstract_ipc_client * processor)
	: abstract_command(processor, CMD_LEAVE_RESPONSE)
{}

ic::leave_res_cmd::~leave_res_cmd(void)
{}

void ic::leave_res_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{
	CMD_LEAVE_PAYLOAD_T payload;
	memset(&payload, 0x00, sizeof(CMD_LEAVE_PAYLOAD_T));
	memcpy(&payload, msg, sizeof(CMD_LEAVE_PAYLOAD_T));

	session->shutdown_fd();
	_processor->disconnect(true);
}
#endif
#endif

#if defined(WITH_WORKING_AS_SERVER)
ic::keepalive_req_cmd::keepalive_req_cmd(abstract_ipc_server * processor)
#else
ic::keepalive_req_cmd::keepalive_req_cmd(abstract_ipc_client * processor)
#endif
	: abstract_command(processor, CMD_KEEPALIVE_REQUEST)
{}

ic::keepalive_req_cmd::~keepalive_req_cmd(void)
{}

void ic::keepalive_req_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{
	CMD_KEEPALIVE_PAYLOAD_T payload;
	memset(&payload, 0x00, sizeof(CMD_KEEPALIVE_PAYLOAD_T));
	memcpy(&payload, msg, sizeof(CMD_KEEPALIVE_PAYLOAD_T));
	payload.code = CMD_ERR_CODE_SUCCESS;
	session->push_send_packet(SERVER_UUID, uuid(), CMD_KEEPALIVE_RESPONSE, reinterpret_cast<char*>(&payload), sizeof(CMD_KEEPALIVE_PAYLOAD_T));
}

#if defined(WITH_WORKING_AS_SERVER)
ic::keepalive_res_cmd::keepalive_res_cmd(abstract_ipc_server * processor)
#else
ic::keepalive_res_cmd::keepalive_res_cmd(abstract_ipc_client * processor)
#endif
	: abstract_command(processor, CMD_KEEPALIVE_RESPONSE)
{}

ic::keepalive_res_cmd::~keepalive_res_cmd(void)
{}

void ic::keepalive_res_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{

}