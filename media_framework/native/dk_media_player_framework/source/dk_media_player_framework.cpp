#include "dk_media_player_framework.h"
#include "dk_rtmp_receiver.h"
#include "dk_rtsp_receiver.h"
#include <dk_auto_lock.h>
#include <dk_string_helper.h>
#include <map>
#include "resource.h"

typedef struct _live_source_info_t
{
	int type;
	char url[260];
	char username[260];
	char password[260];
	HWND hwnd;
	bool repeat;
	bool run;
	void * receiver;
	_live_source_info_t(void)
		: type(RTSP_RECEIVER)
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

HWND g_fullscreen_dlg = NULL;

BOOL CALLBACK fullwnd_proc(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		//SetWindowPos(dlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(dlg, 0);
			return TRUE;
		}
		return FALSE;
	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}


void MediaClient_Initialize(HWND hwnd)
{
	::InitializeCriticalSection(&g_lock);
	g_index_generator = 0;

	//g_fullscreen_dlg = CreateDialog(GetModuleHandle(L"dk_media_player_framework.dll"), MAKEINTRESOURCE(IDD_DIALOG_FULLSCREEN), hwnd, fullwnd_proc);
	//if (g_fullscreen_dlg)
	//{
	//	ShowWindow(g_fullscreen_dlg, SW_SHOW);
	//}
}

void MediaClient_Release(void)
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
					if (info->type == RTSP_RECEIVER)
					{
						dk_rtsp_receiver * receiver = static_cast<dk_rtsp_receiver*>(info->receiver);
						receiver->stop();
					}
					else if (info->type == RTMP_RECEIVER)
					{
						dk_rtmp_receiver * receiver = static_cast<dk_rtmp_receiver*>(info->receiver);
						receiver->stop();
					}
					info->run = false;
				}
				if (info->type == RTSP_RECEIVER)
				{
					dk_rtsp_receiver * receiver = static_cast<dk_rtsp_receiver*>(info->receiver);
					delete receiver;
					receiver = nullptr;
				}
				else if (info->type == RTMP_RECEIVER)
				{
					dk_rtmp_receiver * receiver = static_cast<dk_rtmp_receiver*>(info->receiver);
					delete receiver;
					receiver = nullptr;
				}
				info->receiver = nullptr;
				delete info;
				info = nullptr;
			}
		}
		g_live_source_infos.clear();
	}

	::DeleteCriticalSection(&g_lock);
}

int MediaClient_Add(int type, const wchar_t * url, const wchar_t * username, const wchar_t * password, bool repeat, HWND hwnd)
{
	if (!url || wcslen(url) < 1)
		return -1;

	char * ascii_url = 0;
	dk_string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url)<1)
		return -1;

	char * ascii_username = 0;
	if (username && wcslen(username) > 0)
		dk_string_helper::convert_wide2multibyte((wchar_t*)username, &ascii_username);

	char * ascii_password = 0;
	if (password && wcslen(password) > 0)
		dk_string_helper::convert_wide2multibyte((wchar_t*)password, &ascii_password);

	dk_auto_lock mutex(&g_lock);
	int index = g_index_generator;

	live_source_info_t * info = new live_source_info_t();
	strcpy_s(info->url, sizeof(info->url), ascii_url);
	if (ascii_username && strlen(ascii_username) > 0)
		strcpy_s(info->username, sizeof(info->username), ascii_username);
	if (ascii_password && strlen(ascii_password) > 0)
		strcpy_s(info->password, sizeof(info->password), ascii_password);
	info->hwnd = hwnd;
	info->repeat = repeat;
	info->run = false;
	info->type = type;
	if (info->type == RTSP_RECEIVER)
		info->receiver = new dk_rtsp_receiver();
	else if (info->type == RTMP_RECEIVER)
		info->receiver = new dk_rtmp_receiver();

	if (ascii_url)
	{
		free(ascii_url);
		ascii_url = 0;
	}
	if (ascii_username)
	{
		free(ascii_username);
		ascii_username = 0;
	}
	if (ascii_password)
	{
		free(ascii_password);
		ascii_password = 0;
	}

	g_live_source_infos.insert(std::make_pair(index, info));
	g_index_generator++;
	return index;
}

void MediaClient_Remove(int id)
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
				if (info->type == RTSP_RECEIVER)
				{
					dk_rtsp_receiver * receiver = static_cast<dk_rtsp_receiver*>(info->receiver);
					receiver->stop();
				}
				else if (info->type == RTMP_RECEIVER)
				{
					dk_rtmp_receiver * receiver = static_cast<dk_rtmp_receiver*>(info->receiver);
					receiver->stop();
				}
				info->run = false;
			}
			if (info->type == RTSP_RECEIVER)
			{
				dk_rtsp_receiver * receiver = static_cast<dk_rtsp_receiver*>(info->receiver);
				delete receiver;
				receiver = nullptr;
			}
			else if (info->type == RTMP_RECEIVER)
			{
				dk_rtmp_receiver * receiver = static_cast<dk_rtmp_receiver*>(info->receiver);
				delete receiver;
				receiver = nullptr;
			}
			info->receiver = nullptr;
			delete info;
			info = nullptr;
		}
		g_live_source_infos.erase(id);
	}
}

void MediaClient_Play(int id)
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
				if (info->type == RTSP_RECEIVER)
				{
					dk_rtsp_receiver * receiver = static_cast<dk_rtsp_receiver*>(info->receiver);
					receiver->play(info->url, info->username, info->password, 1, 1, info->repeat, info->hwnd);
				}
				else if (info->type == RTMP_RECEIVER)
				{
					dk_rtmp_receiver * receiver = static_cast<dk_rtmp_receiver*>(info->receiver);
					receiver->play(info->url, info->username, info->password, 2, info->hwnd);
				}
				info->run = true;
			}
		}
	}
}

void MediaClient_Stop(int id)
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
				if (info->type == RTSP_RECEIVER)
				{
					dk_rtsp_receiver * receiver = static_cast<dk_rtsp_receiver*>(info->receiver);
					receiver->stop();
				}
				else if (info->type == RTMP_RECEIVER)
				{
					dk_rtmp_receiver * receiver = static_cast<dk_rtmp_receiver*>(info->receiver);
					receiver->stop();
				}
				info->run = false;
			}
		}
	}
}

void MediaClient_FullScreen(int index, bool enable)
{

}