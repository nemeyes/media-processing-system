#ifndef _DK_PARALLEL_RECORDER_H_
#define _DK_PARALLEL_RECORDER_H_

#include <string>
#include <map>

#define RTSP_RECEIVER	0
#define RTMP_RECEIVER	1

class dk_parallel_recorder_controller;
typedef struct _single_media_source_t
{
	int type;
	char uuid[260];
	char username[260];
	char password[260];
	HWND hwnd;
	bool run;
	void * receiver;
	_single_media_source_t(void)
		: type(RTSP_RECEIVER)
		, hwnd(NULL)
		, run(false)
		, receiver(NULL)
	{
		memset(uuid, 0x00, sizeof(uuid));
		memset(username, 0x00, sizeof(username));
		memset(password, 0x00, sizeof(password));
	}

	_single_media_source_t(_single_media_source_t & clone)
	{
		strcpy_s(uuid, sizeof(uuid), clone.uuid);
		strcpy_s(username, sizeof(username), clone.username);
		strcpy_s(password, sizeof(password), clone.password);

		hwnd = clone.hwnd;
		run = clone.run;
		receiver = clone.receiver;
	}

	_single_media_source_t & operator=(_single_media_source_t & clone)
	{
		strcpy_s(uuid, sizeof(uuid), clone.uuid);
		strcpy_s(username, sizeof(username), clone.username);
		strcpy_s(password, sizeof(password), clone.password);

		hwnd = clone.hwnd;
		run = clone.run;
		receiver = clone.receiver;
		return (*this);
	}
} single_media_source_t;

typedef struct _parallel_recorder_t
{
	char url[260];
	int port_number;
	char username[260];
	char password[260];
	int media_source_index_generator;
	CRITICAL_SECTION media_source_lock;
	dk_parallel_recorder_controller * controller;
	std::map<int, single_media_source_t*> media_sources;
	bool connected;
	bool waiting_request;
	bool waiting_response;
	_parallel_recorder_t(void)
		: port_number(15000)
		, media_source_index_generator(0)
		, controller(nullptr)
		, connected(false)
		, waiting_request(false)
		, waiting_response(false)
	{
		memset(url, 0x00, sizeof(url));
		memset(username, 0x00, sizeof(username));
		memset(password, 0x00, sizeof(password));
		::InitializeCriticalSection(&media_source_lock);
		//controller = new dk_parallel_recorder_controller();
	}

	~_parallel_recorder_t(void)
	{
		//if (controller)
		//	delete controller;
		//controller = nullptr;
		::DeleteCriticalSection(&media_source_lock);
	}
} parallel_recorder_t;











#endif