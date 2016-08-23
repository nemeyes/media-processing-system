#include "dk_ipc_client.h"
#include <command.h>
#include "abstract_ipc_client.h"

ic::dk_ipc_client::dk_ipc_client(void)
{
	_client = new abstract_ipc_client(this);
}

ic::dk_ipc_client::dk_ipc_client(const char * uuid)
{
	_client = new abstract_ipc_client(this);
}

ic::dk_ipc_client::~dk_ipc_client(void)
{
	if (_client)
	{
		delete _client;
	}
	_client = nullptr;
}

bool ic::dk_ipc_client::connect(char * address, int32_t port_number, bool reconnection)
{
	return _client->connect(address, port_number, reconnection);
}

bool ic::dk_ipc_client::disconnect(void)
{
	return _client->disconnect();
}

void ic::dk_ipc_client::data_request(char * dst, int32_t command_id, char * msg, int32_t length)
{
	_client->data_request(dst, command_id, msg, length);
}

void ic::dk_ipc_client::add_command(abstract_command * command)
{
	_client->add_command(command);
}