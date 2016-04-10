#include "dk_media_player_framework.h"
#include "dk_rtmp_receiver.h"
#include "dk_rtsp_receiver.h"
#include <dk_auto_lock.h>
#include <map>

typedef enum _live_source_type
{
	live_source_rtsp = 0,
	live_source_rtmp,
	live_source_hls
} live_source_type;

typedef struct _live_source_info_t
{
	char url[260];
	char username[260];
	char password[260];
	int transport_option;
	int recv_option;
	HWND hwnd;
	bool repeat;
	bool run;
	dk_rtsp_receiver * receiver;
	_live_source_info_t(void)
		: transport_option(dk_live_rtsp_client::rtp_over_tcp)
		, recv_option(dk_live_rtsp_client::recv_video)
		, hwnd(NULL)
		, repeat(false)
		, run(false)
		, receiver(NULL)
	{
		memset(url, 0x00, sizeof(url));
		memset(username, 0x00, sizeof(username));
		memset(password, 0x00, sizeof(password));
	}

	_live_source_info_t(_live_source_info_t & clone)
	{
		strcpy_s(url, sizeof(url), clone.url);
		strcpy_s(username, sizeof(username), clone.username);
		strcpy_s(password, sizeof(password), clone.password);

		transport_option = clone.transport_option;
		recv_option = clone.recv_option;
		hwnd = clone.hwnd;
		repeat = clone.repeat;
		run = clone.run;
		receiver = clone.receiver;
	}

	_live_source_info_t & operator=(_live_source_info_t & clone)
	{
		strcpy_s(url, sizeof(url), clone.url);
		strcpy_s(username, sizeof(username), clone.username);
		strcpy_s(password, sizeof(password), clone.password);

		transport_option = clone.transport_option;
		recv_option = clone.recv_option;
		hwnd = clone.hwnd;
		repeat = clone.repeat;
		run = clone.run;
		receiver = clone.receiver;
		return (*this);
	}

} live_source_info_t;

CRITICAL_SECTION g_lock;
int32_t g_index_generator;
std::map<int, live_source_info_t*> g_live_source_infos;

void dmpf_initialize(void)
{
	::InitializeCriticalSection(&g_lock);
	g_index_generator = 0;
}

void dmpf_release(void)
{
	{
		dk_auto_lock mutex(&g_lock);
		std::map<int, live_source_info_t*>::iterator iter;
		for (iter = g_live_source_infos.begin(); iter != g_live_source_infos.end(); iter++)
		{
			live_source_info_t * info = iter->second;
			if (info)
			{
				if (info->run)
				{
					info->receiver->stop();
					info->run = false;
				}
			}
			delete info->receiver;
			info->receiver = nullptr;
		}
		g_live_source_infos.clear();
	}

	::DeleteCriticalSection(&g_lock);
}

int dmpf_rtsp_source_add(const char * url, const char * username, const char * password, int transport_option, int recv_option, bool repeat, HWND hwnd)
{
	if (!url || strlen(url) < 1)
		return -1;

	dk_auto_lock mutex(&g_lock);
	int index = g_index_generator;

	live_source_info_t * info = new live_source_info_t();
	strcpy_s(info->url, sizeof(info->url), url);
	if (username && strlen(username) > 0)
		strcpy_s(info->username, sizeof(info->username), username);
	if (password && strlen(password) > 0)
		strcpy_s(info->password, sizeof(info->password), password);
	info->transport_option = transport_option;
	info->recv_option = recv_option;
	info->hwnd = hwnd;
	info->repeat = repeat;
	info->run = false;
	info->receiver = new dk_rtsp_receiver();

	g_live_source_infos.insert(std::make_pair(index, info));

	g_index_generator++;
	return index;
}

void dmpf_rtsp_source_remove(int id)
{
	dk_auto_lock mutex(&g_lock);
	std::map<int, live_source_info_t*>::iterator iter = g_live_source_infos.find(id);
	if (iter != g_live_source_infos.end())
	{
		live_source_info_t * info = iter->second;
		if (info)
		{
			if (info->run)
			{
				info->receiver->stop();
				info->run = false;
			}
		}
		delete info->receiver;
		info->receiver = nullptr;
		g_live_source_infos.erase(id);
	}
}

void dmpf_rtsp_source_play(int id)
{
	dk_auto_lock mutex(&g_lock);
	std::map<int, live_source_info_t*>::iterator iter = g_live_source_infos.find(id);
	if (iter != g_live_source_infos.end())
	{
		live_source_info_t * info = iter->second;
		if (info)
		{
			if (!info->run)
			{
				info->receiver->play(info->url, info->username, info->password, info->transport_option, info->recv_option, info->repeat, info->hwnd);
				info->run = true;
			}
		}
	}
}

void dmpf_rtsp_source_stop(int id)
{
	dk_auto_lock mutex(&g_lock);
	std::map<int, live_source_info_t*>::iterator iter = g_live_source_infos.find(id);
	if (iter != g_live_source_infos.end())
	{
		live_source_info_t * info = iter->second;
		if (info)
		{
			if (info->run)
			{
				info->receiver->stop();
				info->run = false;
			}
		}
	}
}