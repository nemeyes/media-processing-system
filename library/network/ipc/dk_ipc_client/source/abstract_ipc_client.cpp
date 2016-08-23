#include "abstract_ipc_client.h"
#include "dk_ipc_client.h"
#include <scoped_lock.h>
#include <dk_string_helper.h>
#include <command.h>
#include <iocp_client.h>

ic::abstract_ipc_client::abstract_ipc_client(dk_ipc_client * front)
	: _front(front)
	, _port_number(15000)
	, _retry_connection(true)
	, _connected(false)
	, _run(false)
{
	memcpy(_uuid, UNDEFINED_UUID, sizeof(_uuid));
	memset(_address, 0x00, sizeof(_address));

	_client = new iocp_client(this);
	_client->initialize();

	unsigned int thread_id = 0;
	_disconnect_thread = (HANDLE)_beginthreadex(NULL, 0, ic::abstract_ipc_client::disconnect_process_cb, this, 0, &thread_id);
	for (int32_t i = 0; !_disconnect_run || i < 50; i++)
		::Sleep(10);

	add_command(new assoc_res_cmd(this));
	add_command(new leave_ind_cmd(this));
#if defined(WITH_KEEPALIVE)
	add_command(new keepalive_req_cmd(this));
	add_command(new keepalive_res_cmd(this));
#endif
}

ic::abstract_ipc_client::abstract_ipc_client(dk_ipc_client * front, const char * uuid)
	: _front(front)
	, _port_number(15000)
	, _retry_connection(true)
	, _connected(false)
	, _run(false)
{
	memcpy(_uuid, UNDEFINED_UUID, sizeof(_uuid));
	memset(_address, 0x00, sizeof(_address));

	if (uuid && strlen(uuid)>0)
		strncpy_s(_uuid, uuid, sizeof(_uuid));


	_client = new iocp_client(this);
	_client->initialize();

	unsigned int thread_id = 0;
	_disconnect_thread = (HANDLE)_beginthreadex(NULL, 0, ic::abstract_ipc_client::disconnect_process_cb, this, 0, &thread_id);
	for (int32_t i = 0; !_disconnect_run || i < 500; i++)
		::Sleep(1);

	add_command(new assoc_res_cmd(this));
	add_command(new leave_ind_cmd(this));
#if defined(WITH_KEEPALIVE)
	add_command(new keepalive_req_cmd(this));
	add_command(new keepalive_res_cmd(this));
#endif
}

ic::abstract_ipc_client::~abstract_ipc_client(void)
{
	disconnect();

	_disconnect_run = false;
	if (_disconnect_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_disconnect_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_disconnect_thread);
		}
		_disconnect_thread = INVALID_HANDLE_VALUE;
	}

	if (_client)
	{
		//_client->disconnect();
		_client->release();
		delete _client;
		_client = nullptr;
	}

	clear_command_list();
}

bool ic::abstract_ipc_client::connect(const char * address, int32_t port_number, bool retry_connection)
{
	if (!address || strlen(address) < 1)
		return false;
	strcpy_s(_address, address);
	_port_number = port_number;
	_retry_connection = retry_connection;

	unsigned int thread_id = 0;
	_thread = (HANDLE)_beginthreadex(NULL, 0, ic::abstract_ipc_client::process_cb, this, 0, &thread_id);
	if (_thread == NULL || _thread == INVALID_HANDLE_VALUE)
		return false;
	for (int32_t i = 0; !_run || i < 50; i++)
		::Sleep(10);

	return true;
}

bool ic::abstract_ipc_client::disconnect(void)
{
	_run = false;
	_retry_connection = false;
	if (_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_thread);
		}
		_thread = INVALID_HANDLE_VALUE;
	}
	return true;
}

bool ic::abstract_ipc_client::clear(void)
{
	_do_disconnect = true;
	return true;
}

const char * ic::abstract_ipc_client::uuid(void)
{
	return _uuid;
}

void ic::abstract_ipc_client::uuid(const char * uuid)
{
	strcpy_s(_uuid, uuid);
}

void ic::abstract_ipc_client::data_indication_callback(const char * dst, const char * src, int32_t command_id, const char * msg, size_t length, std::shared_ptr<ic::session> session)
{
	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		if (session->assoc_flag() || (command_id == CMD_KEEPALIVE_REQUEST) || (command_id == CMD_KEEPALIVE_RESPONSE) || (command_id == CMD_ASSOC_RESPONSE))
		{
			abstract_command * command = (*iter).second;
			command->execute(dst, src, command_id, msg, length, session);
		}
	}
}

void ic::abstract_ipc_client::data_request(char * dst, int32_t command_id, char * msg, int32_t length)
{
	data_request(dst, _uuid, command_id, msg, length);
}

void ic::abstract_ipc_client::data_request(char * dst, char * src, int32_t command_id, char * msg, int32_t length)
{
	_session->push_send_packet(dst, src, command_id, msg, length);
}

void ic::abstract_ipc_client::data_request(std::shared_ptr<ic::session> session)
{
	_client->post_send(session);
}

void ic::abstract_ipc_client::add_command(abstract_command * command)
{
	if (command != nullptr)
	{
		if (command->get_processor() == nullptr)
			command->set_processor(this);
		_commands.insert(std::make_pair(command->command_id(), command));
	}
}

void ic::abstract_ipc_client::remove_command(int32_t command_id)
{
	std::map<int32_t, abstract_command*>::iterator iter = _commands.find(command_id);
	if (iter != _commands.end())
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.erase(command_id);
}

void ic::abstract_ipc_client::assoc_completion_callback(std::shared_ptr<ic::session> session)
{
	_session = session;
	if (_front)
		_front->assoc_completion_callback();
}

void ic::abstract_ipc_client::leave_completion_callback(std::shared_ptr<ic::session> session)
{
	if (_session.get() == session.get())
	{
		if (_front)
			_front->leave_completion_callback();
	}
}

void ic::abstract_ipc_client::clear_command_list(void)
{
	std::map<int32_t, abstract_command*>::iterator iter;
	for (iter = _commands.begin(); iter != _commands.end(); iter++)
	{
		abstract_command * command = (*iter).second;
		delete command;
	}
	_commands.clear();
}

unsigned __stdcall ic::abstract_ipc_client::process_cb(void * param)
{
	ic::abstract_ipc_client * self = static_cast<ic::abstract_ipc_client*>(param);
	self->process();
	return 0;
}

void ic::abstract_ipc_client::process(void)
{
	do
	{
		_session = _client->connect(_address, _port_number);
		_run = true;
		DWORD msleep = 500;
		long long elapsed_millisec = 0;
		while (_run)
		{
			if (_session)
			{
				if (!_session->assoc_flag() && (elapsed_millisec % 3000) == 0)
				{
					_session->push_send_packet(SERVER_UUID, _uuid, CMD_ASSOC_REQUEST, nullptr, 0);
					::Sleep(3000);
				}
#if defined(WITH_KEEPALIVE)
			_session->update_hb_end_time();
			if (_session->check_hb())
			{
				CMD_KEEPALIVE_PAYLOAD_T payload;
				memset(&payload, 0x00, sizeof(CMD_KEEPALIVE_PAYLOAD_T));
				payload.code = CMD_ERR_CODE_FAIL;
				_session->push_send_packet(SERVER_UUID, _uuid, CMD_KEEPALIVE_REQUEST, reinterpret_cast<char*>(&payload), sizeof(CMD_KEEPALIVE_PAYLOAD_T));
				_session->update_hb_start_time();
			}
#endif
			}
			else
			{
				break;
			}
			::Sleep(msleep);
			elapsed_millisec += msleep;
		}

		if (_session && _session->assoc_flag())
			_session->push_send_packet(SERVER_UUID, _uuid, CMD_LEAVE_INDICATION, nullptr, 0);

		_do_disconnect = true;

		::Sleep(500);

	} while (_retry_connection);
}

unsigned ic::abstract_ipc_client::disconnect_process_cb(void * param)
{
	ic::abstract_ipc_client * self = static_cast<ic::abstract_ipc_client*>(param);
	self->disconnect_process();
	return 0;
}

void ic::abstract_ipc_client::disconnect_process(void)
{
	_disconnect_run = true;
	while (_disconnect_run)
	{
		if (_client && _do_disconnect)
		{
			if (_session)
			{
				_session->shutdown_fd();
			}
			_client->disconnect();
			_do_disconnect = false;
		}
		::Sleep(10);
	}

	if (_client)
	{
		if (_session)
		{
			_session->shutdown_fd();
		}
		_client->disconnect();
	}
}
