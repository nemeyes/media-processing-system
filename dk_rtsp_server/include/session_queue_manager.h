#pragma once
#include <map>
#include <vector>
#include <list>
#include <string>


#include "buffers/shared_memory_queue_manager.h"



#define MAX_SESSION_SIZE				128

typedef struct _SINGLE_STREAM_QUEUE_T
{
	HANDLE					sm;
	HANDLE					sm_lock;
	SHARED_VIDEO_QUEUE_T	*sm_queue;

	unsigned char			sps[100];
	unsigned char			pps[100];
	int						sps_size;
	int						pps_size;
	_SINGLE_STREAM_QUEUE_T(VOID)
	{
		sm = INVALID_HANDLE_VALUE;
		sm_lock = INVALID_HANDLE_VALUE;
		sm_queue = 0;
		memset(sps, 0x00, sizeof(sps));
		memset(pps, 0x00, sizeof(pps));
		sps_size = 0;
		pps_size = 0;
	}

} SINGLE_STREAM_QUEUE_T;

class session_queue_manager
{
public:
	static session_queue_manager & instance(void);
	session_queue_manager(void);
	~session_queue_manager(void);

	std::string					create_stream_session(const char *stream_name);
	bool						delete_stream_session(const char *stream_name, char * session_id);

	const unsigned char*		get_sps(const char *stream_name);
	const unsigned char*		get_pps(const char *stream_name);
	int							get_sps_size(const char *stream_name);
	int							get_pps_size(const char *stream_name);
	bool						pop(const char *stream_name, char *session_id, unsigned char *frame, int *frame_size);

private:
	std::string					generate_uuid(void);
private:
	GUID											*_guid_seed;
	std::map<std::string, SINGLE_STREAM_QUEUE_T*>	_queues;
	CRITICAL_SECTION								_lock;
};