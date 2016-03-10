#include "shared_memory.h"
#include <cstdint>
#include <cstdio>

ic::shared_memory_server::shared_memory_server(void)
	: _map(INVALID_HANDLE_VALUE)
	, _signal(INVALID_HANDLE_VALUE)
	, _available(INVALID_HANDLE_VALUE)
	, _smb(nullptr)
	, _address(nullptr)
{
	create();
}

ic::shared_memory_server::~shared_memory_server(void)
{
	if (_address)
	{
		free(_address);
		_address = nullptr;
	}
	close();
}

long ic::shared_memory_server::read(void * buffer, long size, long timeout)
{

}

const char * ic::shared_memory_server::address(void) const
{ 
	return _address; 
}

//Block Functions
ic::SHARED_MEMORY_BLOCK_T * ic::shared_memory_server::block(long timeout)
{

}

void ic::shared_memory_server::block(ic::SHARED_MEMORY_BLOCK_T * block)
{

}

void ic::shared_memory_server::create(void)
{
	DWORD pid = ::GetCurrentProcessId();
	DWORD tid = ::GetCurrentThreadId();
	DWORD sid = ic::id();

	_address = (char*)malloc(SM_MAX_ADDR);
	if (!_address)
		return;
	_snprintf(_address, SM_MAX_ADDR, "IPC_%04u_%04u_%04u", pid, tid, sid);

	char * evt_available = (char*)malloc(SM_MAX_ADDR);
	if (!evt_available)
		return;
	_snprintf(evt_available, SM_MAX_ADDR, "%s_evt_available", _address);

	char * evt_filled = (char*)malloc(SM_MAX_ADDR);
	if (!evt_filled)
	{
		free(evt_available);
		return;
	}
	_snprintf(evt_filled, SM_MAX_ADDR, "%s_evt_filled", _address);

	char * evt_sm = (char*)malloc(SM_MAX_ADDR);
	if (!evt_sm)
	{
		free(evt_filled);
		free(evt_available);
		return;
	}
	_snprintf(evt_sm, SM_MAX_ADDR, "%s_evt_sm", _address);

	_signal = CreateEventA(NULL, FALSE, FALSE, (LPCSTR)evt_filled);
	if (_signal == NULL || _signal == INVALID_HANDLE_VALUE)
	{
		free(evt_sm);
		free(evt_filled);
		free(evt_available);
		return;
	}
	_available = CreateEventA(NULL, FALSE, FALSE, (LPCSTR)evt_available);
	if (_available == NULL || _available == INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_signal);
		free(evt_sm);
		free(evt_filled);
		free(evt_available);
		return;
	}

	_map = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHARED_MEMORY_BUFFER_T), (LPCSTR)evt_sm);
	if (_map == NULL || _map == INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_available);
		::CloseHandle(_signal);
		free(evt_sm);
		free(evt_filled);
		free(evt_available);
		return;
	}

	_smb = (SHARED_MEMORY_BUFFER_T*)MapViewOfFile(_map, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SHARED_MEMORY_BUFFER_T));
	if (_smb == NULL)
	{
		::CloseHandle(_map);
		::CloseHandle(_available);
		::CloseHandle(_signal);
		free(evt_sm);
		free(evt_filled);
		free(evt_available);
	}



}

void ic::shared_memory_server::close(void)
{

}