#include "shared_memory.h"
#include <cstdint>
#include <cstdio>

ic::shared_memory_server::shared_memory_server(void)
	: _map(INVALID_HANDLE_VALUE)
	, _signal(INVALID_HANDLE_VALUE)
	, _available(INVALID_HANDLE_VALUE)
	, _smb(nullptr)
{
}

ic::shared_memory_server::~shared_memory_server(void)
{
}

bool ic::shared_memory_server::create_shared_memory(const char * uuid)
{
	strncpy_s(_uuid, uuid, sizeof(_uuid));
	char * evt_available = nullptr;
	char * evt_filled = nullptr;
	char * evt_sm = nullptr;

	evt_available = (char*)malloc(SM_MAX_ADDR);
	if (!evt_available)
		goto fail;
	_snprintf(evt_available, SM_MAX_ADDR, "%s_evt_available", _uuid);

	evt_filled = (char*)malloc(SM_MAX_ADDR);
	if (!evt_filled)
		goto fail;
	_snprintf(evt_filled, SM_MAX_ADDR, "%s_evt_filled", _uuid);

	evt_sm = (char*)malloc(SM_MAX_ADDR);
	if (!evt_sm)
		goto fail;
	_snprintf(evt_sm, SM_MAX_ADDR, "%s_evt_sm", _uuid);

	_signal = CreateEventA(NULL, FALSE, FALSE, (LPCSTR)evt_filled);
	if (_signal == NULL || _signal == INVALID_HANDLE_VALUE)
		goto fail;

	_available = CreateEventA(NULL, FALSE, FALSE, (LPCSTR)evt_available);
	if (_available == NULL || _available == INVALID_HANDLE_VALUE)
		goto fail;

	_map = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHARED_MEMORY_BUFFER_T), (LPCSTR)evt_sm);
	if (_map == NULL || _map == INVALID_HANDLE_VALUE)
		goto fail;

	_smb = (SHARED_MEMORY_BUFFER_T*)MapViewOfFile(_map, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SHARED_MEMORY_BUFFER_T));
	if (_smb == NULL)
		goto fail;
	::ZeroMemory(_smb, sizeof(SHARED_MEMORY_BUFFER_T));

	//create circular linked list
	int32_t index = 1;
	_smb->blocks[0].next = 1;
	_smb->blocks[0].prev = (SM_BLOCK_COUNT - 1);
	for (; index < SM_BLOCK_COUNT - 1; index++)
	{
		//add this block into available list
		_smb->blocks[index].next = (index + 1);
		_smb->blocks[index].prev = (index - 1);
	}

	_smb->blocks[index].next = 0;
	_smb->blocks[index].prev = (SM_BLOCK_COUNT - 2); //??

	//initilaize the pointers
	_smb->rend = 0;
	_smb->rbegin = 0;
	_smb->wend = 0;
	_smb->wbegin = 0;

	if (evt_sm)
		free(evt_sm);
	if (evt_filled)
		free(evt_filled);
	if (evt_available)
		free(evt_available);

	return true;

fail:
	if (_map != NULL && _map != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_map);
		_map = INVALID_HANDLE_VALUE;
	}
	if (_available != NULL && _available != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_available);
		_available = INVALID_HANDLE_VALUE;
	}
	if (_signal != NULL && _signal != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_signal);
		_signal = INVALID_HANDLE_VALUE;
	}

	if (evt_sm)
		free(evt_sm);
	if (evt_filled)
		free(evt_filled);
	if (evt_available)
		free(evt_available);

	return false;
}

bool ic::shared_memory_server::destroy_shared_memory(void)
{
	if (_signal != NULL && _signal != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_signal);
		_signal = INVALID_HANDLE_VALUE;
	}

	if (_available != NULL && _available != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_available);
		_available = INVALID_HANDLE_VALUE;
	}

	//unmap shared memory
	if (_smb)
	{
		UnmapViewOfFile(_smb);
		_smb = nullptr;
	}

	if (_map != NULL && _map != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_map);
		_map = INVALID_HANDLE_VALUE;
	}

	return true;
}

const char * ic::shared_memory_server::uuid(void) const
{
	return _uuid;
}

bool ic::shared_memory_server::check_smb(void)
{
	if (_smb)
		return true;
	else
		return false;
}

#if defined(WITH_SERVER_PUBLISH)

long ic::shared_memory_server::write(void * buffer, long size, long timeout)
{
	SHARED_MEMORY_BLOCK_T * blk = block(timeout);
	if (!blk)
		return 0;

	long amount = min(size, SM_BLOCK_SIZE);
	memmove(blk->data, buffer, amount);
	blk->amount = amount;

	block(blk);

	return amount;

}

bool ic::shared_memory_server::wait_available(long timeout)
{
	if (::WaitForSingleObject(_available, timeout) != WAIT_OBJECT_0)
		return false;

	return true;
}

ic::SHARED_MEMORY_BLOCK_T * ic::shared_memory_server::block(long timeout)
{
	for (;;)
	{
		long blk_index = _smb->wbegin;
		SHARED_MEMORY_BLOCK_T * blk = _smb->blocks + blk_index;
		if (blk->next == _smb->rend)
		{
			if (::WaitForSingleObject(_available, timeout) == WAIT_OBJECT_0)
				continue;

			return nullptr;
		}

		//make sure the operation is atomic
		if (::InterlockedCompareExchange(&_smb->wbegin, blk->next, blk_index) == blk_index)
			return blk;

		continue;
	}
}

void ic::shared_memory_server::block(ic::SHARED_MEMORY_BLOCK_T * blk)
{
	blk->wdone = 1;

	for (;;)
	{
		long blk_index = _smb->wend;
		blk = _smb->blocks + blk_index;
		if (::InterlockedCompareExchange(&blk->wdone, 0, 1) != 1)
		{
			return;
}

		::InterlockedCompareExchange(&_smb->wend, blk->next, blk_index);

		if (blk->prev == _smb->rbegin)
			::SetEvent(_signal);
	}
}

#else

long ic::shared_memory_server::read(void * buffer, long size, long timeout)
{
	ic::SHARED_MEMORY_BLOCK_T * block = this->block(timeout);
	if (!block)
		return 0;

	long amount = min(block->amount, size);
	memmove(buffer, block->data, amount);

	this->block(block);
	return amount;
}

//Block Functions
ic::SHARED_MEMORY_BLOCK_T * ic::shared_memory_server::block(long timeout)
{
	for (;;)
	{
		long blk_index = _smb->rbegin;
		ic::SHARED_MEMORY_BLOCK_T * blk = _smb->blocks + blk_index;
		if (blk->next == _smb->wend)
		{
			if (::WaitForSingleObject(_signal, timeout) == WAIT_OBJECT_0)
				continue;
			return nullptr;
		}

		if (::InterlockedCompareExchange(&_smb->rbegin, blk->next, blk_index) == blk_index)
			return blk;

		continue;
	}
}

void ic::shared_memory_server::block(ic::SHARED_MEMORY_BLOCK_T * blk)
{
	//set the done flag for this block
	blk->rdone = 1;

	//move the read pointer as far forward as we can
	for (;;)
	{
		long blk_index = _smb->rend;
		blk = _smb->blocks + blk_index;
		if (::InterlockedCompareExchange(&blk->rdone, 0, 1) != 1)
			return;

		//move the pointer one forward(interlock protected)
		::InterlockedCompareExchange(&_smb->rend, blk->next, blk_index);

		if (blk->prev == _smb->wbegin)
			::SetEvent(_available);
	}
}

#endif

ic::shared_memory_client::shared_memory_client(void)
	: _map(INVALID_HANDLE_VALUE)
	, _signal(INVALID_HANDLE_VALUE)
	, _available(INVALID_HANDLE_VALUE)
	, _smb(NULL)
{
}

ic::shared_memory_client::~shared_memory_client(void)
{
}

bool ic::shared_memory_client::connect_shared_memory(const char * uuid)
{
	char * evt_available = nullptr;
	char * evt_filled = nullptr;
	char * evt_sm = nullptr;

	evt_available = static_cast<char*>(malloc(SM_MAX_ADDR));
	if (!evt_available)
		goto fail;
	_snprintf(evt_available, SM_MAX_ADDR, "%s_evt_available", _uuid);

	evt_filled = static_cast<char*>(malloc(SM_MAX_ADDR));
	if (!evt_filled)
		goto fail;
	_snprintf(evt_filled, SM_MAX_ADDR, "%s_evt_filled", _uuid);

	evt_sm = static_cast<char*>(malloc(SM_MAX_ADDR));
	if (!evt_sm)
		goto fail;
	_snprintf(evt_sm, SM_MAX_ADDR, "%s_evt_sm", _uuid);

	_signal = ::CreateEventA(NULL, FALSE, FALSE, (LPCSTR)evt_filled);
	if (_signal == NULL || _signal == INVALID_HANDLE_VALUE)
		goto fail;

	_available = ::CreateEventA(NULL, FALSE, FALSE, (LPCSTR)evt_available);
	if (_available == NULL || _available == INVALID_HANDLE_VALUE)
		goto fail;

	_map = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, (LPCSTR)evt_sm);
	if (_map == NULL || _map == INVALID_HANDLE_VALUE)
		goto fail;

	_smb = static_cast<SHARED_MEMORY_BUFFER_T*>(::MapViewOfFile(_map, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SHARED_MEMORY_BUFFER_T)));
	if (_smb == NULL)
		goto fail;

	if (evt_sm)
		free(evt_sm);
	if (evt_filled)
		free(evt_filled);
	if (evt_available)
		free(evt_available);

	return true;

fail:
	if (_map != NULL && _map != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_map);
		_map = INVALID_HANDLE_VALUE;
	}
	if (_available != NULL && _available != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_available);
		_available = INVALID_HANDLE_VALUE;
	}
	if (_signal != NULL && _signal != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_signal);
		_signal = INVALID_HANDLE_VALUE;
	}

	if (evt_sm)
		free(evt_sm);
	if (evt_filled)
		free(evt_filled);
	if (evt_available)
		free(evt_available);

	return false;
}

bool ic::shared_memory_client::disconnect_shared_memory(void)
{
	if (_signal != NULL && _signal != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_signal);
		_signal = INVALID_HANDLE_VALUE;
	}

	if (_available != NULL && _available != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_available);
		_available = INVALID_HANDLE_VALUE;
	}

	if (_smb)
	{
		UnmapViewOfFile(_smb);
		_smb = nullptr;
	}

	if (_map != NULL && _map != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_map);
		_map = INVALID_HANDLE_VALUE;
	}

	return true;
}

const char * ic::shared_memory_client::uuid(void) const
{
	return _uuid;
}

bool ic::shared_memory_client::check_smb(void)
{
	if (_smb)
		return true;
	else
		return false;
}

#if defined(WITH_SERVER_PUBLISH)

long ic::shared_memory_client::read(void * buffer, long size, long timeout)
{
	ic::SHARED_MEMORY_BLOCK_T * block = this->block(timeout);
	if (!block)
		return 0;

	long amount = min(block->amount, size);
	memmove(buffer, block->data, amount);

	this->block(block);
	return amount;
}

//Block Functions
ic::SHARED_MEMORY_BLOCK_T * ic::shared_memory_client::block(long timeout)
{
	for (;;)
	{
		long blk_index = _smb->rbegin;
		ic::SHARED_MEMORY_BLOCK_T * blk = _smb->blocks + blk_index;
		if (blk->next == _smb->wend)
		{
			if (::WaitForSingleObject(_signal, timeout) == WAIT_OBJECT_0)
				continue;
			return nullptr;
		}

		if (::InterlockedCompareExchange(&_smb->rbegin, blk->next, blk_index) == blk_index)
			return blk;

		continue;
	}
}

void ic::shared_memory_client::block(ic::SHARED_MEMORY_BLOCK_T * blk)
{
	//set the done flag for this block
	blk->rdone = 1;

	//move the read pointer as far forward as we can
	for (;;)
	{
		long blk_index = _smb->rend;
		blk = _smb->blocks + blk_index;
		if (::InterlockedCompareExchange(&blk->rdone, 0, 1) != 1)
			return;

		//move the pointer one forward(interlock protected)
		::InterlockedCompareExchange(&_smb->rend, blk->next, blk_index);

		if (blk->prev == _smb->wbegin)
			::SetEvent(_available);
	}
}

#else

long ic::shared_memory_client::write(void * buffer, long size, long timeout)
{
	SHARED_MEMORY_BLOCK_T * blk = block(timeout);
	if (!blk)
		return 0;

	long amount = min(size, SM_BLOCK_SIZE);
	memmove(blk->data, buffer, amount);
	blk->amount = amount;

	block(blk);

	return amount;

}

bool ic::shared_memory_client::wait_available(long timeout)
{
	if (::WaitForSingleObject(_available, timeout) != WAIT_OBJECT_0)
		return false;

	return true;
}

ic::SHARED_MEMORY_BLOCK_T * ic::shared_memory_client::block(long timeout)
{
	for (;;)
	{
		long blk_index = _smb->wbegin;
		SHARED_MEMORY_BLOCK_T * blk = _smb->blocks + blk_index;
		if (blk->next == _smb->rend)
		{
			if (::WaitForSingleObject(_available, timeout) == WAIT_OBJECT_0)
				continue;

			return nullptr;
		}

		//make sure the operation is atomic
		if (::InterlockedCompareExchange(&_smb->wbegin, blk->next, blk_index) == blk_index)
			return blk;

		continue;
	}
}

void ic::shared_memory_client::block(ic::SHARED_MEMORY_BLOCK_T * blk)
{
	blk->wdone = 1;

	for (;;)
	{
		long blk_index = _smb->wend;
		blk = _smb->blocks + blk_index;
		if (::InterlockedCompareExchange(&blk->wdone, 0, 1) != 1)
		{
			return;
		}

		::InterlockedCompareExchange(&_smb->wend, blk->next, blk_index);

		if (blk->prev == _smb->rbegin)
			::SetEvent(_signal);
	}
}

#endif
