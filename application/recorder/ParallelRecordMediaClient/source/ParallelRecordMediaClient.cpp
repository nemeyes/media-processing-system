#include "dk_parallel_recorder_controller.h"
#include "dk_rtmp_receiver.h"
#include "dk_rtsp_receiver.h"
#include "dk_rtsp_exportor.h"
#include <dk_auto_lock.h>
#include <dk_string_helper.h>
#include <dk_log4cplus_logger.h>
#include "ParallelRecordMediaClient.h"
#include "dk_parallel_recorder.h"


class rtsp_user_unregistered_sei_callback_impl : public debuggerking::rtsp_user_unregistered_sei__callback
{
public:
	rtsp_user_unregistered_sei_callback_impl(int index, PRMC_PlayTimeCallback cb)
		: _index(index)
		, _cb(cb)
	{}

	virtual void invoke(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second)
	{
		(*_cb)(_index, year, month, day, hour, minute, second);
	}

private:
	int _index;
	PRMC_PlayTimeCallback _cb;
};

class rtsp_exportor_status_callback_impl : public debuggerking::rtsp_exportor_status_callback
{
public:
	rtsp_exportor_status_callback_impl(int index, PRMC_ExportBeginCallback scb, PRMC_ExportEndCallback ecb)
		: _index(index)
		, _scb(scb)
		, _ecb(ecb)
	{}

	virtual void start(void)
	{
		if (_scb)
			(*_scb)(_index);
	}

	virtual void stop(void)
	{
		if (_ecb)
			(*_ecb)(_index);
	}

private:
	int _index;
	PRMC_ExportBeginCallback _scb;
	PRMC_ExportEndCallback _ecb;
};

CRITICAL_SECTION g_lock;
std::map<std::string, debuggerking::parallel_recorder_t*> g_parallel_recorders;


int g_rtsp_source_index_generator;
CRITICAL_SECTION g_rtsp_source_lock;
std::map<int, debuggerking::single_rtsp_source_t*> g_rtsp_sources;

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


int PRMC_Initialize(HWND hwnd)
{
	debuggerking::log4cplus_logger::create("config\\log.properties");

	::InitializeCriticalSection(&g_lock);
	::InitializeCriticalSection(&g_rtsp_source_lock);
	g_rtsp_source_index_generator = 0;

	//g_fullscreen_dlg = CreateDialog(GetModuleHandle(L"dk_media_player_framework.dll"), MAKEINTRESOURCE(IDD_DIALOG_FULLSCREEN), hwnd, fullwnd_proc);
	//if (g_fullscreen_dlg)
	//{
	//	ShowWindow(g_fullscreen_dlg, SW_SHOW);
	//}

	return PRMC_SUCCESS;
}

int PRMC_Release(void)
{
	std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
	for (iter = g_parallel_recorders.begin(); iter != g_parallel_recorders.end(); iter++)
	{
		debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
		if (single_recorder_info)
		{
			debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
			std::map<int, debuggerking::single_media_source_t*>::iterator iter;
			for (iter = single_recorder_info->media_sources.begin(); iter != single_recorder_info->media_sources.end(); iter++)
			{
				debuggerking::single_media_source_t * single_media_source = iter->second;
				if (single_media_source)
				{
					if (single_media_source->run)
					{
						if (single_media_source->type == RTSP_RECEIVER)
						{
							debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
							receiver->stop();
						}
						single_media_source->run = false;
					}
					if (single_media_source->type == RTSP_RECEIVER)
					{
						debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
						delete receiver;
						receiver = nullptr;
					}
					single_media_source->receiver = nullptr;
					if (single_media_source->user_unregistered_sei_callback)
					{
						rtsp_user_unregistered_sei_callback_impl * uusei_callback = static_cast<rtsp_user_unregistered_sei_callback_impl*>(single_media_source->user_unregistered_sei_callback);
						delete uusei_callback;
						uusei_callback = nullptr;
						single_media_source->user_unregistered_sei_callback = nullptr;
					}
					delete single_media_source;
					single_media_source = nullptr;
				}
			}
			single_recorder_info->media_sources.clear();
			if (single_recorder_info->connected && single_recorder_info->controller)
			{
				single_recorder_info->controller->disconnect();
				single_recorder_info->connected = false;
			}
			delete single_recorder_info->controller;
			single_recorder_info->controller = nullptr;
			delete single_recorder_info;
		}
		single_recorder_info = nullptr;
		g_parallel_recorders.erase(iter);
	}

	::DeleteCriticalSection(&g_rtsp_source_lock);
	::DeleteCriticalSection(&g_lock);
	debuggerking::log4cplus_logger::destroy();
	return PRMC_SUCCESS;
}

int PRMC_Connect(const wchar_t * url, int port_number, const wchar_t * username, const wchar_t * password)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url) < 1)
		return result;

	debuggerking::parallel_recorder_t * single_recorder_info = new debuggerking::parallel_recorder_t();
	strncpy_s(single_recorder_info->url, ascii_url, sizeof(single_recorder_info->url));
	single_recorder_info->port_number = port_number;

	char * ascii_username = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)username, &ascii_username);
	if (ascii_username && strlen(ascii_username) > 0)
		strncpy_s(single_recorder_info->username, ascii_username, sizeof(single_recorder_info->username));

	char * ascii_password = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)password, &ascii_password);
	if (ascii_password&& strlen(ascii_password) > 0)
		strncpy_s(single_recorder_info->password, ascii_password, sizeof(single_recorder_info->password));

	single_recorder_info->controller = new debuggerking::parallel_recorder_controller(single_recorder_info);
	bool sock_connected = single_recorder_info->controller->connect(single_recorder_info->url, single_recorder_info->port_number);
	if (!sock_connected)
		result = PRMC_FAIL;

	for (int index = 0; index < 1000; index++)
	{
		//_parallel_recorder->rtsp_port_number_received
		if (single_recorder_info->rtsp_port_number_received)
			break;
		::Sleep(10);
	}

	if (!single_recorder_info->connected)
		result = PRMC_FAIL;
	else
	{
		g_parallel_recorders.insert(std::make_pair(ascii_url, single_recorder_info));
		result = PRMC_SUCCESS;
	}

	if (ascii_url)
		free(ascii_url);
	if (ascii_username)
		free(ascii_username);
	if (ascii_password)
		free(ascii_password);
	ascii_url = 0;
	ascii_username = 0;
	ascii_password = 0;

	return result;
}

int PRMC_Disconnect(const wchar_t * url)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url) < 1)
		return result;

	std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
	iter = g_parallel_recorders.find(ascii_url);
	if (iter != g_parallel_recorders.end())
	{
		debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
		if (single_recorder_info)
		{
			debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
			std::map<int, debuggerking::single_media_source_t*>::iterator iter;
			for (iter = single_recorder_info->media_sources.begin(); iter != single_recorder_info->media_sources.end(); iter++)
			{
				debuggerking::single_media_source_t * single_media_source = iter->second;
				if (single_media_source)
				{
					if (single_media_source->run)
					{
						if (single_media_source->type == RTSP_RECEIVER)
						{
							debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
							receiver->stop();
						}
						single_media_source->run = false;
					}
					if (single_media_source->type == RTSP_RECEIVER)
					{
						debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
						delete receiver;
						receiver = nullptr;
					}
					single_media_source->receiver = nullptr;
					if (single_media_source->user_unregistered_sei_callback)
					{
						rtsp_user_unregistered_sei_callback_impl * uusei_callback = static_cast<rtsp_user_unregistered_sei_callback_impl*>(single_media_source->user_unregistered_sei_callback);
						delete uusei_callback;
						uusei_callback = nullptr;
						single_media_source->user_unregistered_sei_callback = nullptr;
					}
					delete single_media_source;
					single_media_source = nullptr;
				}
			}
			single_recorder_info->media_sources.clear();
			if (single_recorder_info->connected && single_recorder_info->controller)
			{
				single_recorder_info->controller->disconnect();
				single_recorder_info->connected = false;
			}
			delete single_recorder_info->controller;
			single_recorder_info->controller = nullptr;
			delete single_recorder_info;
		}
		single_recorder_info = nullptr;
		g_parallel_recorders.erase(iter);
		result = PRMC_SUCCESS;
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = 0;

	return result;
}

int PRMC_GetYears(const wchar_t * url, const wchar_t * uuid, int years[], int capacity, int & size)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url) < 1)
		return result;

	std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
	iter = g_parallel_recorders.find(ascii_url);
	if (iter != g_parallel_recorders.end())
	{
		char * ascii_uuid = 0;
		debuggerking::string_helper::convert_wide2multibyte((wchar_t*)uuid, &ascii_uuid);
		if (ascii_uuid && strlen(ascii_uuid) > 0)
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				single_recorder_info->controller->get_years(ascii_uuid, years, capacity, size);
				result = PRMC_SUCCESS;
			}
		}
		if (ascii_uuid)
			free(ascii_uuid);
		ascii_uuid = 0;
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = 0;

	return result;
}

int PRMC_GetMonths(const wchar_t * url, const wchar_t * uuid, int year, int months[], int capacity, int & size)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url) < 1)
		return result;

	std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
	iter = g_parallel_recorders.find(ascii_url);
	if (iter != g_parallel_recorders.end())
	{
		char * ascii_uuid = 0;
		debuggerking::string_helper::convert_wide2multibyte((wchar_t*)uuid, &ascii_uuid);
		if (ascii_uuid && strlen(ascii_uuid) > 0)
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				single_recorder_info->controller->get_months(ascii_uuid, year, months, capacity, size);
				result = PRMC_SUCCESS;
			}
		}
		if (ascii_uuid)
			free(ascii_uuid);
		ascii_uuid = 0;
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = 0;

	return result;
}

int PRMC_GetDays(const wchar_t * url, const wchar_t * uuid, int year, int month, int days[], int capacity, int & size)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url) < 1)
		return result;

	std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
	iter = g_parallel_recorders.find(ascii_url);
	if (iter != g_parallel_recorders.end())
	{
		char * ascii_uuid = 0;
		debuggerking::string_helper::convert_wide2multibyte((wchar_t*)uuid, &ascii_uuid);
		if (ascii_uuid && strlen(ascii_uuid) > 0)
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				single_recorder_info->controller->get_days(ascii_uuid, year, month, days, capacity, size);
				result = PRMC_SUCCESS;
			}
		}
		if (ascii_uuid)
			free(ascii_uuid);
		ascii_uuid = 0;
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = 0;

	return result;
}

int PRMC_GetHours(const wchar_t * url, const wchar_t * uuid, int year, int month, int day, int hours[], int capacity, int & size)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url) < 1)
		return result;

	std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
	iter = g_parallel_recorders.find(ascii_url);
	if (iter != g_parallel_recorders.end())
	{
		char * ascii_uuid = 0;
		debuggerking::string_helper::convert_wide2multibyte((wchar_t*)uuid, &ascii_uuid);
		if (ascii_uuid && strlen(ascii_uuid) > 0)
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				single_recorder_info->controller->get_hours(ascii_uuid, year, month, day, hours, capacity, size);
				result = PRMC_SUCCESS;
			}
		}
		if (ascii_uuid)
			free(ascii_uuid);
		ascii_uuid = 0;
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = 0;

	return result;
}

int PRMC_GetMinutes(const wchar_t * url, const wchar_t * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url) < 1)
		return result;

	std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
	iter = g_parallel_recorders.find(ascii_url);
	if (iter != g_parallel_recorders.end())
	{
		char * ascii_uuid = 0;
		debuggerking::string_helper::convert_wide2multibyte((wchar_t*)uuid, &ascii_uuid);
		if (ascii_uuid && strlen(ascii_uuid) > 0)
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				single_recorder_info->controller->get_minutes(ascii_uuid, year, month, day, hour, minutes, capacity, size);
				result = PRMC_SUCCESS;
			}
		}
		if (ascii_uuid)
			free(ascii_uuid);
		ascii_uuid = 0;
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = 0;

	return result;
}

int PRMC_GetSeconds(const wchar_t * url, const wchar_t * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (!ascii_url || strlen(ascii_url) < 1)
		return result;

	std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
	iter = g_parallel_recorders.find(ascii_url);
	if (iter != g_parallel_recorders.end())
	{
		char * ascii_uuid = 0;
		debuggerking::string_helper::convert_wide2multibyte((wchar_t*)uuid, &ascii_uuid);
		if (ascii_uuid && strlen(ascii_uuid) > 0)
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				single_recorder_info->controller->get_seconds(ascii_uuid, year, month, day, hour, minute, seconds, capacity, size);
				result = PRMC_SUCCESS;
			}
		}
		if (ascii_uuid)
			free(ascii_uuid);
		ascii_uuid = 0;
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = 0;

	return result;
}

int PRMC_Add(const wchar_t * url, const wchar_t * uuid, HWND hwnd, PRMC_PlayTimeCallback cb)
{
	if (!url || wcslen(url) < 1)
		return PRMC_FAIL;

	int media_source_index_per_recorder = -1;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				char * ascii_uuid = 0;
				debuggerking::string_helper::convert_wide2multibyte((wchar_t*)uuid, &ascii_uuid);
				if (ascii_uuid && strlen(ascii_uuid) > 0)
				{
					media_source_index_per_recorder = single_recorder_info->media_source_index_generator;

					debuggerking::auto_lock mutext(&single_recorder_info->media_source_lock);
					debuggerking::single_media_source_t * single_media_source = new debuggerking::single_media_source_t();
					strcpy_s(single_media_source->uuid, sizeof(single_media_source->uuid), ascii_uuid);
					if (strlen(single_recorder_info->username) > 0)
						strcpy_s(single_media_source->username, sizeof(single_media_source->username), single_recorder_info->username);
					if (strlen(single_recorder_info->password) > 0)
						strcpy_s(single_media_source->password, sizeof(single_media_source->password), single_recorder_info->password);
					single_media_source->hwnd = hwnd;
					single_media_source->run = false;
					single_media_source->type = RTSP_RECEIVER;
					if (cb)
					{
						rtsp_user_unregistered_sei_callback_impl * uusei_callback = new rtsp_user_unregistered_sei_callback_impl(media_source_index_per_recorder, cb);
						single_media_source->user_unregistered_sei_callback = uusei_callback;
						single_media_source->receiver = new debuggerking::rtsp_receiver(uusei_callback);
					}
					else
					{
						single_media_source->receiver = new debuggerking::rtsp_receiver();
					}
					single_recorder_info->media_sources.insert(std::make_pair(media_source_index_per_recorder, single_media_source));
					single_recorder_info->media_source_index_generator++;
				}

				if (ascii_uuid)
					free(ascii_uuid);
				ascii_uuid = nullptr;
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return media_source_index_per_recorder;
}

int PRMC_Remove(const wchar_t * url, int index)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (single_media_source->run)
						{
							if (single_media_source->type == RTSP_RECEIVER)
							{
								debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
								receiver->stop();
							}
							single_media_source->run = false;
						}

						if (single_media_source->type == RTSP_RECEIVER)
						{
							debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
							delete receiver;
							receiver = nullptr;
						}
						single_media_source->receiver = nullptr;
						if (single_media_source->user_unregistered_sei_callback)
						{
							rtsp_user_unregistered_sei_callback_impl * uusei_callback = static_cast<rtsp_user_unregistered_sei_callback_impl*>(single_media_source->user_unregistered_sei_callback);
							delete uusei_callback;
							uusei_callback = nullptr;
							single_media_source->user_unregistered_sei_callback = nullptr;
						}
						delete single_media_source;
						single_media_source = nullptr;
					}
					single_recorder_info->media_sources.erase(iter);
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

int PRMC_Play(const wchar_t * url, int index, int year, int month, int day, int hour, int minute, int second, float scale, bool repeat)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;
	if (scale < 0.1f)
		scale = 0.1f;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (!single_media_source->run)
						{
							if (single_media_source->type == RTSP_RECEIVER)
							{
								char rtsp_url[260] = { 0 };
								_snprintf_s(rtsp_url, sizeof(rtsp_url), "rtsp://%s:%d/%s/%.4d%.2d%.2d%.2d%.2d%.2d", single_recorder_info->url, single_recorder_info->rtsp_server_port_number, single_media_source->uuid, year, month, day, hour, minute, second);

								debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
								receiver->play(rtsp_url, single_media_source->username, single_media_source->password, 1, debuggerking::rtsp_receiver::recv_option_t::video, scale, repeat, single_media_source->hwnd);
							}
							//else if (single_media_source->type == RTMP_RECEIVER)
							//{
							//	dk_rtmp_receiver * receiver = static_cast<dk_rtmp_receiver*>(single_media_source->receiver);
							//	receiver->play(info->url, single_media_source->username, single_media_source->password, 2, single_media_source->hwnd);
							//}
							single_media_source->run = true;
						}
						
					}
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

int PRMC_Stop(const wchar_t * url, int index)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (single_media_source->run)
						{
							if (single_media_source->type == RTSP_RECEIVER)
							{
								debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
								receiver->stop();
							}
							single_media_source->run = false;
						}

					}
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}


int PRMC_Resume(const wchar_t * url, int index)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;
	float scale = 1.0f;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (!single_media_source->run)
						{
							if (single_media_source->type == RTSP_RECEIVER)
							{
								char rtsp_url[260] = { 0 };
								_snprintf_s(rtsp_url, sizeof(rtsp_url), "rtsp://%s:%d/%s/%.4d%.2d%.2d%.2d%.2d%.2d", single_recorder_info->url, single_recorder_info->rtsp_server_port_number, single_media_source->uuid, single_media_source->last_year, single_media_source->last_month, single_media_source->last_day, single_media_source->last_hour, single_media_source->last_minute, single_media_source->last_second);

								debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
								receiver->play(rtsp_url, single_media_source->username, single_media_source->password, 1, debuggerking::rtsp_receiver::recv_option_t::video, scale, false, single_media_source->hwnd);
							}
							//else if (single_media_source->type == RTMP_RECEIVER)
							//{
							//	dk_rtmp_receiver * receiver = static_cast<dk_rtmp_receiver*>(single_media_source->receiver);
							//	receiver->play(info->url, single_media_source->username, single_media_source->password, 2, single_media_source->hwnd);
							//}
							single_media_source->run = true;
						}

					}
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

int PRMC_Pause(const wchar_t * url, int index)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (single_media_source->run)
						{
							if (single_media_source->type == RTSP_RECEIVER)
							{
								debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
								receiver->get_last_time(single_media_source->last_year, single_media_source->last_month, single_media_source->last_day, single_media_source->last_hour, single_media_source->last_minute, single_media_source->last_second);
								receiver->stop();
							}
							single_media_source->run = false;
						}
					}
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

int PRMC_AddExport(const wchar_t * url, const wchar_t * uuid, const wchar_t * export_file_path, PRMC_ExportBeginCallback scb, PRMC_ExportEndCallback ecb)
{
	if (!url || wcslen(url) < 1)
		return PRMC_FAIL;
	if (!export_file_path || wcslen(export_file_path) < 1)
		return PRMC_FAIL;

	int media_source_index_per_recorder = -1;

	char * ascii_url = nullptr, *ascii_export_file_path = nullptr;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)export_file_path, &ascii_export_file_path);
	if (ascii_url && strlen(ascii_url) > 0 && ascii_export_file_path && strlen(ascii_export_file_path)>0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				char * ascii_uuid = 0;
				debuggerking::string_helper::convert_wide2multibyte((wchar_t*)uuid, &ascii_uuid);
				if (ascii_uuid && strlen(ascii_uuid) > 0)
				{
					media_source_index_per_recorder = single_recorder_info->media_source_index_generator;

					debuggerking::auto_lock mutext(&single_recorder_info->media_source_lock);
					debuggerking::single_media_source_t * single_media_source = new debuggerking::single_media_source_t();
					strcpy_s(single_media_source->uuid, sizeof(single_media_source->uuid), ascii_uuid);
					if (strlen(single_recorder_info->username) > 0)
						strcpy_s(single_media_source->username, sizeof(single_media_source->username), single_recorder_info->username);
					if (strlen(single_recorder_info->password) > 0)
						strcpy_s(single_media_source->password, sizeof(single_media_source->password), single_recorder_info->password);
					single_media_source->hwnd = NULL;
					single_media_source->run = false;
					single_media_source->type = RTSP_EXPORTOR;

					if (scb && ecb)
					{
						rtsp_exportor_status_callback_impl * exp_status_cb = new rtsp_exportor_status_callback_impl(media_source_index_per_recorder, scb, ecb);
						single_media_source->exportor_status_callback = exp_status_cb;
						single_media_source->exportor = new debuggerking::rtsp_exportor(exp_status_cb);
					}
					else
					{
						single_media_source->exportor = new debuggerking::rtsp_exportor();
					}

					strncpy_s(single_media_source->export_file_path, ascii_export_file_path, sizeof(single_media_source->export_file_path));
					single_recorder_info->media_sources.insert(std::make_pair(single_recorder_info->media_source_index_generator, single_media_source));
					single_recorder_info->media_source_index_generator++;
				}

				if (ascii_uuid)
					free(ascii_uuid);
				ascii_uuid = nullptr;
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;
	if (ascii_export_file_path)
		free(ascii_export_file_path);
	ascii_export_file_path = nullptr;

	return media_source_index_per_recorder;
}

int PRMC_RemoveExport(const wchar_t * url, int index)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (single_media_source->run)
						{
							if (single_media_source->type == RTSP_EXPORTOR)
							{
								debuggerking::rtsp_exportor * exportor = static_cast<debuggerking::rtsp_exportor*>(single_media_source->exportor);
								exportor->stop();
								delete exportor;
								exportor = nullptr;
							}
							single_media_source->run = false;
						}
						single_media_source->exportor = nullptr;
						if (single_media_source->exportor_status_callback)
						{
							rtsp_exportor_status_callback_impl * exp_status_cb = static_cast<rtsp_exportor_status_callback_impl*>(single_media_source->exportor_status_callback);
							delete exp_status_cb;
							exp_status_cb = nullptr;
							single_media_source->exportor_status_callback = nullptr;
						}
						delete single_media_source;
						single_media_source = nullptr;
					}
					single_recorder_info->media_sources.erase(iter);
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

int PRMC_PlayExport(const wchar_t * url, int index, int begin_year, int begin_month, int begin_day, int begin_hour, int begin_minute, int begin_second, int end_year, int end_month, int end_day, int end_hour, int end_minute, int end_second)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	float scale = 0.0f;
	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (!single_media_source->run)
						{
							if (single_media_source->type == RTSP_EXPORTOR)
							{
								char rtsp_url[260] = { 0 };
								_snprintf_s(rtsp_url, sizeof(rtsp_url), "rtsp://%s:%d/%s/%.4d%.2d%.2d%.2d%.2d%.2d", single_recorder_info->url, single_recorder_info->rtsp_server_port_number, single_media_source->uuid, begin_year, begin_month, begin_day, begin_hour, begin_minute, begin_second);

								debuggerking::rtsp_exportor * exportor = static_cast<debuggerking::rtsp_exportor*>(single_media_source->exportor);
								exportor->play(rtsp_url, single_media_source->username, single_media_source->password, 1, debuggerking::rtsp_exportor::recv_option_t::video, false, single_media_source->export_file_path, end_year, end_month, end_day, end_hour, end_minute, end_second);
							}
							single_media_source->run = true;
						}

					}
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

int PRMC_StopExport(const wchar_t * url, int index)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (single_media_source->run)
						{
							if (single_media_source->type == RTSP_EXPORTOR)
							{
								debuggerking::rtsp_exportor * exportor = static_cast<debuggerking::rtsp_exportor*>(single_media_source->exportor);
								exportor->stop();
							}
							single_media_source->run = false;
						}

					}
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

int PRMC_EnableOSD(const wchar_t * url, int index, bool enable)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (single_media_source->type == RTSP_RECEIVER)
						{
							debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
							receiver->enable_osd(enable);
						}
					}
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

int PRMC_SetOSDPosition(const wchar_t * url, int index, int x, int y)
{
	int result = PRMC_FAIL;
	if (!url || wcslen(url) < 1)
		return result;

	char * ascii_url = 0;
	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_lock);
		std::map<std::string, debuggerking::parallel_recorder_t*>::iterator iter;
		iter = g_parallel_recorders.find(ascii_url);
		if (iter != g_parallel_recorders.end())
		{
			debuggerking::parallel_recorder_t * single_recorder_info = iter->second;
			if (single_recorder_info && single_recorder_info->connected && single_recorder_info->controller)
			{
				debuggerking::auto_lock mutex(&single_recorder_info->media_source_lock);
				std::map<int, debuggerking::single_media_source_t*>::iterator iter = single_recorder_info->media_sources.find(index);
				if (iter != single_recorder_info->media_sources.end())
				{
					debuggerking::single_media_source_t * single_media_source = iter->second;
					if (single_media_source)
					{
						if (single_media_source->type == RTSP_RECEIVER)
						{
							debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(single_media_source->receiver);
							receiver->set_osd_position(x, y);
						}
					}
					result = PRMC_SUCCESS;
				}
			}
		}
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;

	return result;
}

/*
int g_media_source_index_generator;
CRITICAL_SECTION g_media_source_lock;
std::map<int, single_media_source_t*> g_media_sources;
*/
int PRMC_RTSP_Add(const wchar_t * url, int port_number, const wchar_t * username, const wchar_t * password, HWND hwnd)
{
	if (!url || wcslen(url) < 1)
		return PRMC_FAIL;

	int rtsp_source_index = -1;

	char * ascii_url = 0;
	char * ascii_username = 0;
	char * ascii_password = 0;

	debuggerking::string_helper::convert_wide2multibyte((wchar_t*)url, &ascii_url);
	if (username && wcslen(username)>0)
		debuggerking::string_helper::convert_wide2multibyte((wchar_t*)username, &ascii_username);
	if (password && wcslen(password)>0)
		debuggerking::string_helper::convert_wide2multibyte((wchar_t*)password, &ascii_password);

	if (ascii_url && strlen(ascii_url) > 0)
	{
		debuggerking::auto_lock mutext(&g_rtsp_source_lock);
		debuggerking::single_rtsp_source_t * source = new debuggerking::single_rtsp_source_t();
		strcpy_s(source->url, sizeof(source->url), ascii_url);
		if (ascii_username && strlen(ascii_username) > 0)
			strcpy_s(source->username, sizeof(source->username), ascii_username);
		if (ascii_password && strlen(ascii_password) > 0)
			strcpy_s(source->password, sizeof(source->password), ascii_password);
		source->hwnd = hwnd;
		source->run = false;
		source->receiver = new debuggerking::rtsp_receiver();

		g_rtsp_sources.insert(std::make_pair(g_rtsp_source_index_generator, source));
		rtsp_source_index = g_rtsp_source_index_generator;
		g_rtsp_source_index_generator++;
	}

	if (ascii_url)
		free(ascii_url);
	ascii_url = nullptr;
	if (ascii_username)
		free(ascii_username);
	ascii_username = nullptr;
	if (ascii_password)
		free(ascii_password);
	ascii_password = nullptr;

	return rtsp_source_index;
}

int PRMC_RTSP_Remove(int index)
{
	int result = PRMC_FAIL;

	debuggerking::auto_lock mutex(&g_rtsp_source_lock);
	std::map<int, debuggerking::single_rtsp_source_t*>::iterator iter = g_rtsp_sources.find(index);
	if (iter != g_rtsp_sources.end())
	{
		debuggerking::single_rtsp_source_t * source = iter->second;
		if (source)
		{
			if (source->run)
			{
				debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(source->receiver);
				receiver->stop();
				source->run = false;
			}

			debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(source->receiver);
			delete receiver;
			receiver = nullptr;

			source->receiver = nullptr;
			delete source;
			source = nullptr;
		}
		g_rtsp_sources.erase(iter);
		result = PRMC_SUCCESS;
	}

	return result;
}

int PRMC_RTSP_Play(int index, bool repeat)
{
	int result = PRMC_FAIL;

	debuggerking::auto_lock mutex(&g_rtsp_source_lock);
	std::map<int, debuggerking::single_rtsp_source_t*>::iterator iter = g_rtsp_sources.find(index);
	if (iter != g_rtsp_sources.end())
	{
		debuggerking::single_rtsp_source_t * source = iter->second;
		if (source)
		{
			if (!source->run)
			{
				debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(source->receiver);
				receiver->play(source->url, source->username, source->password, 1, debuggerking::rtsp_receiver::recv_option_t::video, 1.f, repeat, source->hwnd);
				source->run = true;
			}

		}
		result = PRMC_SUCCESS;
	}
	return result;
}

int PRMC_RTSP_Stop(int index)
{
	int result = PRMC_FAIL;

	debuggerking::auto_lock mutex(&g_rtsp_source_lock);
	std::map<int, debuggerking::single_rtsp_source_t*>::iterator iter = g_rtsp_sources.find(index);
	if (iter != g_rtsp_sources.end())
	{
		debuggerking::single_rtsp_source_t * source = iter->second;
		if (source)
		{
			if (source->run)
			{
				debuggerking::rtsp_receiver * receiver = static_cast<debuggerking::rtsp_receiver*>(source->receiver);
				receiver->stop();
				source->run = false;
			}

		}
		result = PRMC_SUCCESS;
	}
	return result;
}

int PRMC_FullScreen(const wchar_t * url, int index, bool enable)
{
	return PRMC_SUCCESS;
}