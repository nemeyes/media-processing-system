#include "dk_ipc_server.h"
#include <command.h>
#include "abstract_ipc_server.h"

ic::dk_ipc_server::dk_ipc_server(const char * uuid)
{
	_server = new abstract_ipc_server(this, uuid);
}

ic::dk_ipc_server::dk_ipc_server(void)
{
	_server = new abstract_ipc_server(this);
}

ic::dk_ipc_server::~dk_ipc_server(void)
{
	if (_server)
	{
		delete _server;
	}
	_server = nullptr;
}

void ic::dk_ipc_server::uuid(const char * uuid)
{
	return _server->uuid(uuid);
}

bool ic::dk_ipc_server::start(char * address, int32_t port_number)
{
	return _server->start(address, port_number);
}

bool ic::dk_ipc_server::stop(void)
{
	return _server->stop();
}

void ic::dk_ipc_server::data_request(char * dst, int32_t command_id, char * msg, int32_t length)
{
	_server->data_request(dst, command_id, msg, length);
}

void ic::dk_ipc_server::add_command(abstract_command * command)
{
	_server->add_command(command);
}