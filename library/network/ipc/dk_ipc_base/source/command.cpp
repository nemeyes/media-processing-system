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
	if (session->assoc_flag())	//이미 Assoc이 되어 있으면 처리를 하지 않는다.
		return;

	CMD_ASSOC_RES_T res;
	memset(&res, 0x00, sizeof(CMD_ASSOC_RES_T));

	bool code = false;
	if (!strncmp(src, UNDEFINED_UUID, strlen(UNDEFINED_UUID))) //UUID값이 정의되지 않은 UUID일경우 새로 생성.
	{
		UUID gen_uuid;
		::ZeroMemory(&gen_uuid, sizeof(UUID));
		::UuidCreate(&gen_uuid);

		char * new_uuid = nullptr;
		::UuidToStringA(&gen_uuid, (RPC_CSTR*)&new_uuid);

		std::string ret_uuid = (char *)new_uuid;
		std::transform(ret_uuid.begin(), ret_uuid.end(), ret_uuid.begin(), toupper);

		memcpy(new_uuid, ret_uuid.c_str(), ret_uuid.length());
		if (new_uuid)
		{
			code = _processor->register_client(new_uuid, session);
			if (code)
			{
				strcpy_s(res.uuid, new_uuid);
				session->uuid(res.uuid);
				session->assoc_flag(true);
				res.code = CMD_ERR_CODE_SUCCESS;
			}
			else
				res.code = CMD_ERR_CODE_FAIL;

			::RpcStringFreeA((RPC_CSTR*)&new_uuid);
		}
		else
			res.code = CMD_ERR_CODE_FAIL;
	}
	else //UUID값이 정의된 경우
	{
		if (strlen(src)>0)
		{
			code = _processor->register_client(src, session);
			if (code)
			{
				strcpy_s(res.uuid, src);
				session->uuid(res.uuid);
				session->assoc_flag(true);
				res.code = CMD_ERR_CODE_SUCCESS;
			}
			else
				res.code = CMD_ERR_CODE_FAIL;
		}
		else
			res.code = CMD_ERR_CODE_FAIL;
	}
	session->push_send_packet(session->uuid(), uuid(), CMD_ASSOC_RESPONSE, reinterpret_cast<char*>(&res), sizeof(CMD_ASSOC_RES_T));
	if (code)
		_processor->assoc_completion_callback(res.uuid, session);
}
#else
ic::assoc_res_cmd::assoc_res_cmd(abstract_ipc_client * processor)
	: abstract_command(processor, CMD_ASSOC_RESPONSE)
{}

ic::assoc_res_cmd::~assoc_res_cmd(void)
{}

void ic::assoc_res_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{
	CMD_ASSOC_RES_T payload;
	memset(&payload, 0x00, sizeof(CMD_ASSOC_RES_T));
	memcpy(&payload, msg, sizeof(CMD_ASSOC_RES_T));

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
ic::leave_ind_cmd::leave_ind_cmd(abstract_ipc_server * processor)
	: abstract_command(processor, CMD_LEAVE_INDICATION)
{}
#else
ic::leave_ind_cmd::leave_ind_cmd(abstract_ipc_client * processor)
	: abstract_command(processor, CMD_LEAVE_INDICATION)
{}
#endif

ic::leave_ind_cmd::~leave_ind_cmd(void)
{}

void ic::leave_ind_cmd::execute(const char * dst, const char * src, int32_t command_id, const char * msg, int32_t length, std::shared_ptr<ic::session> session)
{
#if defined(WITH_WORKING_AS_SERVER)

#else
	_processor->leave_completion_callback(session);
	_processor->clear();
#endif
}

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
	session->push_send_packet(SERVER_UUID, uuid(), CMD_KEEPALIVE_RESPONSE, nullptr, 0);
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