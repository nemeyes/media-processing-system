#include "dk_shared_memory.h"
#include "shared_memory.h"

ic::dk_shared_memory_server::dk_shared_memory_server(void)
{
	_sms = new ic::shared_memory_server();
}

ic::dk_shared_memory_server::~dk_shared_memory_server(void)
{
	if (_sms)
	{
		delete _sms;
		_sms = nullptr;
	}
}

bool ic::dk_shared_memory_server::create_shared_memory(const char * uuid)
{
	return _sms->create_shared_memory(uuid);
}

bool ic::dk_shared_memory_server::destroy_shared_memory(void)
{
	return _sms->destroy_shared_memory();
}

const char * ic::dk_shared_memory_server::uuid(void) const
{
	return _sms->uuid();
}

bool ic::dk_shared_memory_server::check_smb(void)
{
	return _sms->check_smb();
}

#if defined(WITH_SERVER_PUBLISH)
long ic::dk_shared_memory_server::write(void * buffer, long size, long timeout)
{
	return _sms->write(buffer, size, timeout);
}

bool ic::dk_shared_memory_server::wait_available(long timeout)
{
	return _sms->wait_available(timeout);
}

#else
long ic::dk_shared_memory_server::read(void * buffer, long size, long timeout)
{
	return _sms->read(buffer, size, timeout);
}
#endif


ic::dk_shared_memory_client::dk_shared_memory_client(void)
{
	_smc = new ic::shared_memory_client();
}

ic::dk_shared_memory_client::~dk_shared_memory_client(void)
{
	if (_smc)
	{
		delete _smc;
		_smc = nullptr;
	}
}

bool ic::dk_shared_memory_client::connect_shared_memory(const char * uuid)
{
	return _smc->connect_shared_memory(uuid);
}

bool ic::dk_shared_memory_client::disconnect_shared_memory(void)
{
	return _smc->disconnect_shared_memory();
}

const char * ic::dk_shared_memory_client::uuid(void) const
{
	return _smc->uuid();
}

bool ic::dk_shared_memory_client::check_smb(void)
{
	return _smc->check_smb();
}

#if defined(WITH_SERVER_PUBLISH)

long ic::dk_shared_memory_client::read(void * buffer, long size, long timeout)
{
	return _smc->read(buffer, size, timeout);
}

#else

long ic::dk_shared_memory_client::write(void * buffer, long size, long timeout)
{
	return _smc->write(buffer, size, timeout);
}

bool ic::dk_shared_memory_client::wait_available(long timeout)
{
	return _smc->wait_availalbe(timeout);
}

#endif