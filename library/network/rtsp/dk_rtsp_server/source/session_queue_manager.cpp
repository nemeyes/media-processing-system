#include "GroupsockHelper.hh"
#include "buffers/session_queue_manager.h"
#include <windows.h>
#include <process.h>
#include <time.h>
#include "utils/scoped_lock.h"
#include "log/logger.h"

#include <atlstr.h>
#pragma comment(lib, "rpcrt4.lib")



session_queue_manager & session_queue_manager::instance(void)
{
	static session_queue_manager _instance;
	return _instance;
}

session_queue_manager::session_queue_manager(void)
{
	::InitializeCriticalSection(&_lock);
	_guid_seed = new GUID();
}

session_queue_manager::~session_queue_manager(void)
{
	if (_guid_seed)
	{
		delete _guid_seed;
		_guid_seed = 0;
	}
	::DeleteCriticalSection(&_lock);
}

std::string session_queue_manager::create_stream_session(const char *stream_name)
{
	scoped_lock lock(&_lock);

	std::map<std::string, SINGLE_STREAM_QUEUE_T*>::iterator iter;
	iter = _queues.find(stream_name);
	if (iter == _queues.end())
	{
		HANDLE sm = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, stream_name);
		if (sm == 0 || sm == INVALID_HANDLE_VALUE)
			return 0;

		char lock_name[100] = { 0 };
		_snprintf(lock_name, sizeof(lock_name), "%s_lock", stream_name);

		SINGLE_STREAM_QUEUE_T *stream_queue = new SINGLE_STREAM_QUEUE_T;
		stream_queue->sm = sm;
		stream_queue->sm_lock = CreateMutexA(NULL, FALSE, lock_name);
		stream_queue->sm_queue = (SHARED_VIDEO_QUEUE_T*)::MapViewOfFile(sm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SHARED_VIDEO_QUEUE_T));

		do
		{
			if (stream_queue->sm_queue->sps_size<1 || stream_queue->sm_queue->pps_size<1)
			{
				Sleep(10);
				continue;
			}
			stream_queue->sps_size = stream_queue->sm_queue->sps_size;
			memcpy(stream_queue->sps, stream_queue->sm_queue->sps, stream_queue->sps_size);
			stream_queue->pps_size = stream_queue->sm_queue->pps_size;
			memcpy(stream_queue->pps, stream_queue->sm_queue->pps, stream_queue->pps_size);
			break;
		} while (1);

		_queues.insert(std::make_pair(stream_name, stream_queue));
		std::string uuid = generate_uuid();
		return uuid;
	}
	else
		return "";
}

bool session_queue_manager::delete_stream_session(const char *stream_name, char * session_id)
{
	scoped_lock lock(&_lock);

	std::map<std::string, SINGLE_STREAM_QUEUE_T*>::iterator iter;
	iter = _queues.find(stream_name);
	if (iter != _queues.end())
	{
		SINGLE_STREAM_QUEUE_T* stream_queue = (*iter).second;
		::UnmapViewOfFile(stream_queue->sm_queue);
		::CloseHandle(stream_queue->sm_lock);

		_queues.erase(iter);
		return true;
	}
	else
		return false;
}

const unsigned char* session_queue_manager::get_sps(const char *stream_name)
{
	std::map<std::string, SINGLE_STREAM_QUEUE_T*>::iterator iter;
	iter = _queues.find(stream_name);
	if (iter != _queues.end())
	{
		SINGLE_STREAM_QUEUE_T* stream_queue = (*iter).second;
		return stream_queue->sps;
	}
	return NULL;
}

const unsigned char* session_queue_manager::get_pps(const char *stream_name)
{
	std::map<std::string, SINGLE_STREAM_QUEUE_T*>::iterator iter;
	iter = _queues.find(stream_name);
	if (iter != _queues.end())
	{
		SINGLE_STREAM_QUEUE_T* stream_queue = (*iter).second;
		return stream_queue->pps;
	}
	return NULL;
}

int session_queue_manager::get_sps_size(const char *stream_name)
{
	std::map<std::string, SINGLE_STREAM_QUEUE_T*>::iterator iter;
	iter = _queues.find(stream_name);
	if (iter != _queues.end())
	{
		SINGLE_STREAM_QUEUE_T* stream_queue = (*iter).second;
		return stream_queue->sps_size;
	}
	return 0;
}

int session_queue_manager::get_pps_size(const char *stream_name)
{
	std::map<std::string, SINGLE_STREAM_QUEUE_T*>::iterator iter;
	iter = _queues.find(stream_name);
	if (iter != _queues.end())
	{
		SINGLE_STREAM_QUEUE_T* stream_queue = (*iter).second;
		return stream_queue->pps_size;
	}
	return 0;
}

bool session_queue_manager::pop(const char *stream_name, char *session_id, unsigned char *frame, int *frame_size)
{
	scoped_lock lock(&_lock);

	(*frame_size) = 0;
	std::map<std::string, SINGLE_STREAM_QUEUE_T*>::iterator iter;
	iter = _queues.find(stream_name);
	if (iter != _queues.end())
	{
		SINGLE_STREAM_QUEUE_T* stream_queue = (*iter).second;

		if (::WaitForSingleObject(stream_queue->sm_lock, INFINITE) == WAIT_OBJECT_0)
		{
			(*frame_size) = 0;
			unsigned char *video_frame = 0;
			if (shared_memory_queue_manager::pop(stream_queue->sm_queue, &video_frame, frame_size))
			{
				if ((*frame_size)>0)
				{
					memcpy(frame, video_frame, (*frame_size));
				}
			}
			else
			{
				logger::instance().make_system_debug_log("shared queue is empty");
			}
			::ReleaseMutex(stream_queue->sm_lock);
		}
	}
	return true;
}

std::string session_queue_manager::generate_uuid(void)
{
	CStringA strGUID;
	CoCreateGuid(_guid_seed);
	RPC_CSTR guid;
	UuidToStringA(_guid_seed, &guid);
	strGUID.Format("%s", guid);
	RpcStringFreeA(&guid);
	return (std::string)(LPSTR)(LPCSTR)strGUID;
}