#ifndef _DK_PARALLEL_RECORDER_H_
#define _DK_PARALLEL_RECORDER_H_

#include <string>
#include <map>

#define RTSP_RECEIVER	0
#define RTMP_RECEIVER	1
#define RTSP_EXPORTOR	3

namespace debuggerking
{ 
	class parallel_recorder_controller;
	typedef struct _single_media_source_t
	{
		int type;
		char uuid[MAX_PATH];
		char username[MAX_PATH];
		char password[MAX_PATH];
		HWND hwnd;
		bool run;
		void * receiver;
		void * exportor;
		char export_file_path[MAX_PATH];
		_single_media_source_t(void)
			: type(RTSP_RECEIVER)
			, hwnd(NULL)
			, run(false)
			, receiver(NULL)
			, exportor(NULL)
		{
			memset(uuid, 0x00, sizeof(uuid));
			memset(username, 0x00, sizeof(username));
			memset(password, 0x00, sizeof(password));
			memset(export_file_path, 0x00, sizeof(export_file_path));
		}

		_single_media_source_t(_single_media_source_t & clone)
		{
			memset(uuid, 0x00, sizeof(uuid));
			memset(username, 0x00, sizeof(username));
			memset(password, 0x00, sizeof(password));
			memset(export_file_path, 0x00, sizeof(export_file_path));

			if (strlen(clone.uuid)>0)
				strcpy_s(uuid, sizeof(uuid), clone.uuid);
			if (strlen(clone.username)>0)
				strcpy_s(username, sizeof(username), clone.username);
			if (strlen(clone.password)>0)
				strcpy_s(password, sizeof(password), clone.password);
			if (strlen(clone.export_file_path)>0)
				strcpy_s(export_file_path, sizeof(export_file_path), clone.export_file_path);

			hwnd = clone.hwnd;
			run = clone.run;
			receiver = clone.receiver;
			exportor = clone.exportor;
		}

		_single_media_source_t & operator=(_single_media_source_t & clone)
		{
			memset(uuid, 0x00, sizeof(uuid));
			memset(username, 0x00, sizeof(username));
			memset(password, 0x00, sizeof(password));
			memset(export_file_path, 0x00, sizeof(export_file_path));

			if (strlen(clone.uuid)>0)
				strcpy_s(uuid, sizeof(uuid), clone.uuid);
			if (strlen(clone.username)>0)
				strcpy_s(username, sizeof(username), clone.username);
			if (strlen(clone.password)>0)
				strcpy_s(password, sizeof(password), clone.password);
			if (strlen(clone.export_file_path)>0)
				strcpy_s(export_file_path, sizeof(export_file_path), clone.export_file_path);

			hwnd = clone.hwnd;
			run = clone.run;
			receiver = clone.receiver;
			exportor = clone.exportor;
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
		parallel_recorder_controller * controller;
		std::map<int, single_media_source_t*> media_sources;
		int rtsp_server_port_number;
		bool connected;
		bool rtsp_port_number_received;
		bool waiting_request;
		bool waiting_response;
		_parallel_recorder_t(void)
			: port_number(15000)
			, media_source_index_generator(0)
			, controller(nullptr)
			, rtsp_server_port_number(554)
			, connected(false)
			, rtsp_port_number_received(false)
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

	typedef struct _single_rtsp_source_t
	{
		char url[260];
		char username[260];
		char password[260];
		HWND hwnd;
		bool run;
		void * receiver;
		_single_rtsp_source_t(void)
			: hwnd(NULL)
			, run(false)
			, receiver(NULL)
		{
			memset(url, 0x00, sizeof(url));
			memset(username, 0x00, sizeof(username));
			memset(password, 0x00, sizeof(password));
		}

		_single_rtsp_source_t(_single_rtsp_source_t & clone)
		{
			strcpy_s(url, sizeof(url), clone.url);
			strcpy_s(username, sizeof(username), clone.username);
			strcpy_s(password, sizeof(password), clone.password);

			hwnd = clone.hwnd;
			run = clone.run;
			receiver = clone.receiver;
		}

		_single_rtsp_source_t & operator=(_single_rtsp_source_t & clone)
		{
			strcpy_s(url, sizeof(url), clone.url);
			strcpy_s(username, sizeof(username), clone.username);
			strcpy_s(password, sizeof(password), clone.password);

			hwnd = clone.hwnd;
			run = clone.run;
			receiver = clone.receiver;
			return (*this);
		}
	} single_rtsp_source_t;

}









#endif