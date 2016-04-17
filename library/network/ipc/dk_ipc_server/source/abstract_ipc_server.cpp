#include "abstract_ipc_server.h"
#include "dk_ipc_server.h"
#include <scoped_lock.h>
#include <dk_string_helper.h>
#include <command.h>
#include <iocp_server.h>

ic::abstract_ipc_server::abstract_ipc_server(dk_ipc_server * front, const char * uuid)
	: _front(front)
	, _sequence(0)
	, _port_number(15000)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
{
	memset(_address, 0x00, sizeof(_address));

	strcpy_s(_uuid, uuid);
	_server = new iocp_server(this);
	_server->initialize();

	_session_lock = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	::SetEvent(_session_lock);

	add_command(new assoc_req_cmd(this));
#if defined(WITH_LEAVE_CMD)
	add_command(new leave_req_cmd(this));
#endif
#if defined(WITH_KEEPALIVE)
	add_command(new keepalive_req_cmd(this));
	add_command(new keepalive_res_cmd(this));
#endif
}

ic::abstract_ipc_server::abstract_ipc_server(dk_ipc_server * front)
	: _front(front)
	, _sequence(0)
	, _port_number(15000)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
{
	memset(_address, 0x00, sizeof(_address));

	strcpy_s(_uuid, SERVER_UUID);
	_server = new iocp_server(this);
	_server->initialize();

	_session_lock = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	::SetEvent(_session_lock);

	add_command(new assoc_req_cmd(this));
#if defined(WITH_LEAVE_CMD)
	add_command(new leave_req_cmd(this));
#endif
#if defined(WITH_KEEPALIVE)
	add_command(new keepalive_req_cmd(this));
	add_command(new keepalive_res_cmd(this));
#endif
}

ic::abstract_ipc_server::~abstract_ipc_server(void)
{
	if (_server)
	{
		_server->stop();
		_server->release();
		delete _server;
	}
	_server = nullptr;

	clear_command_list();
	::CloseHandle(_session_lock);
	_session_lock = INVALID_HANDLE_VALUE;
}

const char * ic::abstract_ipc_server::uuid(void)
{
	return _uuid;
}

void ic::abstract_ipc_server::uuid(const char * uuid)
{
	strncpy_s(_uuid, uuid, sizeof(uuid));
}

bool ic::abstract_ipc_server::start(char * address, int32_t port_number)
{
	if (address && strlen(address) > 0)
		strncpy_s(_address, address, sizeof(_address));
	_port_number = port_number;

	unsigned int thread_id;
	_thread = (HANDLE)_beginthreadex(NULL, 0, ic::abstract_ipc_server::process_cb, this, 0, &thread_id);
	return true;
}

bool ic::abstract_ipc_server::stop(void)
{
	_run = false;
	if (_thread != INVALID_HANDLE_VALUE)
	{
		::WaitForSingleObject(_thread, INFINITE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}
	return true;
}

void ic::abstract_ipc_server::data_indication_callback(const char * dst, const char * src, int32_t command_id, const char * msg, size_t length, std::shared_ptr<ic::session> session)
{
	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		if (session->assoc_flag() || (command_id == CMD_KEEPALIVE_REQUEST) || (command_id == CMD_KEEPALIVE_RESPONSE) || (command_id == CMD_ASSOC_REQUEST))
		{
			abstract_command * command = (*iter).second;
			command->execute(dst, src, command_id, msg, length, session);
		}
	}
}

void ic::abstract_ipc_server::data_request(char * dst, int32_t command_id, char * msg, size_t length)
{
	data_request(dst, _uuid, command_id, msg, length);
}

void ic::abstract_ipc_server::data_request(char * dst, char * src, int32_t command_id, char * msg, size_t length)
{
	std::map<std::string, std::shared_ptr<ic::session>>::iterator iter;
	{
		scoped_lock mutex(_session_lock);
		iter = _sessions.find(dst);
	}

	if (iter != _sessions.end())
	{
		char src_uuid[64] = { 0 };
		if (!strncmp(_uuid, dst, sizeof(_uuid) - 1) || !strncmp(SERVER_UUID, dst, sizeof(_uuid) - 1))
			strncpy_s(src_uuid, _uuid, sizeof(src_uuid) - 1);
		else
			strncpy_s(src_uuid, dst, sizeof(src_uuid) - 1);

		std::shared_ptr<ic::session> session = (*iter).second;
		session->push_send_packet(_uuid, src_uuid, command_id, msg, length);
	}
}

void ic::abstract_ipc_server::data_request(std::shared_ptr<ic::session> session)
{
	_server->post_send(session);
}

const char * ic::abstract_ipc_server::check_regstered_client(std::shared_ptr<ic::session> session)
{
	scoped_lock mutex(_session_lock);

	std::map<std::string, std::shared_ptr<ic::session>>::iterator iter;
	for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		std::shared_ptr<ic::session> temp = (*iter).second;
		if (temp->fd() == session->fd())
		{
			break;
		}
	}

	if (iter != _sessions.end())
	{
		const char * uuid = (iter->first).c_str();
		return uuid;
	}
	return nullptr;
}

bool ic::abstract_ipc_server::register_client(const char * uuid, std::shared_ptr<ic::session> session)
{
	scoped_lock mutex(_session_lock);

	std::map<std::string, std::shared_ptr<ic::session>>::iterator iter = _sessions.find(uuid);
	if (iter != _sessions.end())
	{
		std::shared_ptr<ic::session> temp = (*iter).second;
		_server->close(temp);
	}
	_sessions.insert(std::make_pair(uuid, session));
	return true;
}

bool ic::abstract_ipc_server::unregister_client(const char * uuid)
{
	scoped_lock mutex(_session_lock);

	std::map<std::string, std::shared_ptr<ic::session>>::iterator iter = _sessions.find(uuid);
	if (iter != _sessions.end())
	{
		_server->close(iter->second);
		_sessions.erase(iter);
		return true;
	}
	return false;
}

bool ic::abstract_ipc_server::unregister_client(std::shared_ptr<ic::session> session)
{
	scoped_lock mutex(_session_lock);

	std::map<std::string, std::shared_ptr<ic::session>>::iterator iter;
	for (iter = _sessions.begin(); iter != _sessions.end(); iter++)
	{
		if (session.get() == (*iter).second.get())
			break;
	}
	if (iter != _sessions.end())
	{
		_sessions.erase(iter);
		return true;
	}
	return false;
}

std::map<std::string, std::shared_ptr<ic::session>> ic::abstract_ipc_server::get_clients(void)
{
	std::map<std::string, std::shared_ptr<ic::session>> sessions;
	{
		scoped_lock mutex(_session_lock);
		sessions = _sessions;
	}
	return sessions;
}

void ic::abstract_ipc_server::add_command(abstract_command * command)
{
	if (command != nullptr)
	{
		if (command->get_processor() == nullptr)
			command->set_processor(this);
		_commands.insert(std::make_pair(command->command_id(), command));
	}
}

void ic::abstract_ipc_server::remove_command(int32_t command_id)
{
	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.erase(command_id);
}

void ic::abstract_ipc_server::assoc_completion_callback(const char * uuid, std::shared_ptr<ic::session> session)
{
	if (_front)
		_front->assoc_completion_callback(uuid);
}

void ic::abstract_ipc_server::leave_completion_callback(const char * uuid, std::shared_ptr<ic::session> session)
{
	if (_front)
		_front->leave_completion_callback(uuid);
}

void ic::abstract_ipc_server::clear_command_list(void)
{
	std::map<int32_t, abstract_command*>::iterator iter;
	for (iter = _commands.begin(); iter != _commands.end(); iter++)
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.clear();
}

unsigned __stdcall ic::abstract_ipc_server::process_cb(void * param)
{
	ic::abstract_ipc_server * self = static_cast<ic::abstract_ipc_server*>(param);
	self->process();
	return 0;
}

void ic::abstract_ipc_server::process(void)
{
	_server->start(_address, _port_number);
	_run = true;
	while (_run)
	{
#if defined(WITH_KEEPALIVE)
		std::map<std::string, std::shared_ptr<ic::session>> sessions = get_clients();
		std::map<std::string, std::shared_ptr<ic::session>>::iterator iter;
		for (iter = sessions.begin(); iter != sessions.end(); iter++)
		{
			std::shared_ptr<ic::session> session = (*iter).second;
			session->update_hb_end_time();
			if (session->check_hb())
			{
				CMD_KEEPALIVE_PAYLOAD_T payload;
				memset(&payload, 0x00, sizeof(CMD_KEEPALIVE_PAYLOAD_T));
				payload.code = CMD_ERR_CODE_FAIL;
				session->push_send_packet(session->uuid(), _uuid, CMD_KEEPALIVE_REQUEST, reinterpret_cast<char*>(&payload), sizeof(CMD_KEEPALIVE_PAYLOAD_T));
				session->update_hb_start_time();
			}
		}
#endif
		::Sleep(500);
	}

	std::map<std::string, std::shared_ptr<ic::session>> sessions = get_clients();
	std::map<std::string, std::shared_ptr<ic::session>>::iterator iter;
	for (iter = sessions.begin(); iter != sessions.end(); iter++)
	{
		std::shared_ptr<ic::session> session = (*iter).second;
		if (session)
		{
			CMD_LEAVE_PAYLOAD_T payload;
			memset(&payload, 0x00, sizeof(CMD_LEAVE_PAYLOAD_T));
			payload.code = CMD_ERR_CODE_SUCCESS;
			session->push_send_packet(session->uuid(), _uuid, CMD_LEAVE_INDICATION, reinterpret_cast<char*>(&payload), sizeof(CMD_LEAVE_PAYLOAD_T));
		}
	}
	_server->stop();
}