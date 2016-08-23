#include "dk_rtsp_exportor.h"
#include <dk_recorder_module.h>
#include <dk_string_helper.h>

debuggerking::rtsp_exportor::rtsp_exportor(rtsp_exportor_status_callback * cb)
	: _transport_option(debuggerking::rtsp_exportor::rtp_over_tcp)
	, _recv_option(debuggerking::rtsp_exportor::recv_option_t::video)
	, _repeat(false)
	, _rcvd_timestamp(false)
	, _year(0)
	, _month(0)
	, _day(0)
	, _hour(0)
	, _minute(0)
	, _second(0)
	, _millisec(0)
	, _rcvd_year(0)
	, _rcvd_month(0)
	, _rcvd_day(0)
	, _rcvd_hour(0)
	, _rcvd_minute(0)
	, _rcvd_second(0)
	, _rcvd_millisec(0)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
	, _cb(cb)
	, _disconnect_thread(INVALID_HANDLE_VALUE)
	, _disconnect_run(false)
{
	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	memset(_ts_file_path, 0x00, sizeof(_ts_file_path));

	unsigned int thread_id = 0;
	_disconnect_thread = (HANDLE)_beginthreadex(NULL, 0, debuggerking::rtsp_exportor::disconnect_process_cb, this, 0, &thread_id);
	for (int32_t i = 0; !_disconnect_run || i < 50; i++)
		::Sleep(10);
}

debuggerking::rtsp_exportor::~rtsp_exportor(void)
{
	_disconnect_run = false;
	if (_disconnect_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_disconnect_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_disconnect_thread);
		}
		_disconnect_thread = INVALID_HANDLE_VALUE;
	}
}

int32_t debuggerking::rtsp_exportor::play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, bool repeat, char * export_file_path, int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second)
{
	if (!url || strlen(url) < 1)
		return rtsp_exportor::err_code_t::fail;
	if (!export_file_path || strlen(export_file_path) < 1)
		return rtsp_exportor::err_code_t::fail;

	strncpy_s(_url, url, sizeof(_url));
	if (username && strlen(username)>0)
		strncpy_s(_username, username, sizeof(_username));
	if (password && strlen(password)>0)
		strncpy_s(_password, password, sizeof(_password));

	_transport_option = transport_option;
	_recv_option = recv_option;
	_repeat = repeat;

	strncpy_s(_ts_file_path, export_file_path, sizeof(_ts_file_path));

	_year = year;
	_month = month;
	_day = day;
	_hour = hour;
	_minute = minute;
	_second = second;

	_rcvd_timestamp = false;

	unsigned int thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, debuggerking::rtsp_exportor::process_callback, this, 0, &thrdaddr);
	return rtsp_exportor::err_code_t::success;
}

int32_t debuggerking::rtsp_exportor::stop(void)
{
	if (_thread && _thread != INVALID_HANDLE_VALUE)
	{
		_run = false;
		::WaitForSingleObject(_thread, INFINITE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}

	memset(_url, 0x00, sizeof(_url));
	memset(_username, 0x00, sizeof(_username));
	memset(_password, 0x00, sizeof(_password));
	memset(_ts_file_path, 0x00, sizeof(_ts_file_path));

	_transport_option = debuggerking::rtsp_exportor::rtp_over_tcp;
	_recv_option = debuggerking::rtsp_exportor::recv_option_t::video;
	_repeat = false;
	_rcvd_timestamp = false;
	_year = 0;
	_month = 0;
	_day = 0;
	_hour = 0;
	_minute = 0;
	_second = 0;
	_rcvd_year = 0;
	_rcvd_month = 0;
	_rcvd_day = 0;
	_rcvd_hour = 0;
	_rcvd_minute = 0;
	_rcvd_second = 0;

	if (_tsmuxer)
	{
		_tsmuxer->release();
		delete _tsmuxer;
		_tsmuxer = nullptr;
	}

	if (_tsmuxer_config)
	{
		delete _tsmuxer_config;
		_tsmuxer_config = nullptr;
	}
	_rcvd_timestamp = false;

	return rtsp_exportor::err_code_t::success;
}

void debuggerking::rtsp_exportor::on_begin_video(int32_t smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (_tsmuxer)
	{
		_tsmuxer->release();
		delete _tsmuxer;
		_tsmuxer = nullptr;
	}
	if (_tsmuxer_config)
	{
		delete _tsmuxer_config;
		_tsmuxer_config = nullptr;
	}

	if (smt == rtsp_client::video_submedia_type_t::h264)
	{
		int32_t width = 0;
		int32_t height = 0;
		int32_t sarwidth = 0;
		int32_t sarheight = 0;
		int32_t fps = 30;
		int32_t bitrate = 4000000;
		do
		{
			if (parse_sps((BYTE*)(sps), spssize, &width, &height, &sarwidth, &sarheight) > 0)
			{

				_tsmuxer_config = new ff_tsmuxer::configuration_t();
				strncpy_s(_tsmuxer_config->vconfig.file_path, _ts_file_path, sizeof(_tsmuxer_config->vconfig.file_path));
				_tsmuxer_config->vconfig.width = width;
				_tsmuxer_config->vconfig.height = height;
				_tsmuxer_config->vconfig.fps = fps;
				_tsmuxer_config->vconfig.stream_index = 0;
				_tsmuxer_config->vconfig.bitrate = bitrate;

				memset(_tsmuxer_config->vconfig.extradata, 0x00, sizeof(_tsmuxer_config->vconfig.extradata));
				memcpy(_tsmuxer_config->vconfig.extradata, sps, spssize);
				memcpy(_tsmuxer_config->vconfig.extradata + spssize, pps, ppssize);
				_tsmuxer_config->vconfig.extradata_size = spssize + ppssize;
				_tsmuxer_config->vconfig.width = width;
				_tsmuxer_config->vconfig.height = height;
				_tsmuxer_config->vconfig.fps = fps;
				_tsmuxer_config->vconfig.stream_index = 0;
				_tsmuxer_config->vconfig.bitrate = bitrate;

				_tsmuxer = new ff_tsmuxer();
				_tsmuxer->initialize(_tsmuxer_config);
				_tsmuxer->put_video_stream(data, data_size, 0, true);
			}
		} while (0);
	}
}

void debuggerking::rtsp_exportor::on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (smt == rtsp_exportor::video_submedia_type_t::h264)
	{
		if (_tsmuxer)
		{
			if ((data[4] & 0x1F) == 0x06)
			{
				/*sei[19] = (timestamp & 0xFF00000000000000) >> 56;
				sei[20] = (timestamp & 0x00FF000000000000) >> 48;
				sei[21] = (timestamp & 0x0000FF0000000000) >> 40;
				sei[22] = (timestamp & 0x000000FF00000000) >> 32;
				sei[23] = (timestamp & 0x00000000FF000000) >> 24;
				sei[24] = (timestamp & 0x0000000000FF0000) >> 16;
				sei[25] = (timestamp & 0x000000000000FF00) >> 8;
				sei[26] = (timestamp & 0x00000000000000FF);*/
				const uint8_t * sei = data + 4;
				long long timestamp = 0;
				memcpy(&timestamp, &sei[19], sizeof(timestamp));

#if defined(WITH_MILLISECOND)
				debuggerking::recorder_module::get_time_from_elapsed_millisec_from_epoch(timestamp, _rcvd_year, _rcvd_month, _rcvd_day, _rcvd_hour, _rcvd_minute, _rcvd_second);
#else
				debuggerking::recorder_module::get_time_from_elapsed_microsec_from_epoch(timestamp, _rcvd_year, _rcvd_month, _rcvd_day, _rcvd_hour, _rcvd_minute, _rcvd_second);
#endif
				_rcvd_timestamp = true;
			}
			else if ((data[4] & 0x1F) == 0x05)
			{
				_tsmuxer->put_video_stream(data, data_size, 0, true);
			}
			else
			{
				_tsmuxer->put_video_stream(data, data_size, 0, false);
			}
		}
	}
}

void debuggerking::rtsp_exportor::on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (smt == rtsp_exportor::audio_submedia_type_t::aac)
	{

	}
	else if (smt == rtsp_exportor::audio_submedia_type_t::mp3)
	{

	}
}

void debuggerking::rtsp_exportor::on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (smt == rtsp_exportor::audio_submedia_type_t::aac)
	{

	}
	else if (smt == rtsp_exportor::audio_submedia_type_t::mp3)
	{

	}
}

unsigned __stdcall debuggerking::rtsp_exportor::process_callback(void * param)
{
	rtsp_exportor * self = static_cast<rtsp_exportor*>(param);
	self->process();
	return 0;
}

void debuggerking::rtsp_exportor::process(void)
{
	int status = rtsp_exportor::err_code_t::fail;
	status = rtsp_client::play(_url, _username, _password, _transport_option, _recv_option, 3, 8.f, _repeat);
	if (status == rtsp_exportor::err_code_t::success)
	{
		if (_cb)
			_cb->start();
		_run = true;
		long long last_timestamp = 0;
		size_t timestamp_unchange_count = 0;
		while (_run)
		{
			if (_rcvd_timestamp)
			{
#if defined(WITH_MILLISECOND)
				long long destination_timestamp = debuggerking::recorder_module::get_elapsed_millisec_from_epoch(_year, _month, _day, _hour, _minute, _second);
				long long current_timestamp = debuggerking::recorder_module::get_elapsed_millisec_from_epoch(_rcvd_year, _rcvd_month, _rcvd_day, _rcvd_hour, _rcvd_minute, _rcvd_second);
#else
				long long destination_timestamp = debuggerking::recorder_module::get_elapsed_microsec_from_epoch(_year, _month, _day, _hour, _minute, _second, _millisec);
				long long current_timestamp = debuggerking::recorder_module::get_elapsed_microsec_from_epoch(_rcvd_year, _rcvd_month, _rcvd_day, _rcvd_hour, _rcvd_minute, _rcvd_second, _rcvd_millisec);
#endif
				if (destination_timestamp <= current_timestamp)
					break;
				else
					if (timestamp_unchange_count > 30)
						break;

				if (last_timestamp != current_timestamp)
				{
					last_timestamp = current_timestamp;
					timestamp_unchange_count = 0;
				}
				else
					timestamp_unchange_count++;
			}
			::Sleep(100);
		}
		//status = rtsp_client::stop();

		{ //tsmuxer 파일 관련 점유 해제
			if (_tsmuxer)
			{
				_tsmuxer->release();
				delete _tsmuxer;
				_tsmuxer = nullptr;
			}

			if (_tsmuxer_config)
			{
				delete _tsmuxer_config;
				_tsmuxer_config = nullptr;
			}
			_rcvd_timestamp = false;
		}

		_do_disconnect = true;

		if (_cb)
			_cb->stop();
	}
	_run = false;
}


unsigned debuggerking::rtsp_exportor::disconnect_process_cb(void * param)
{
	debuggerking::rtsp_exportor * self = static_cast<debuggerking::rtsp_exportor*>(param);
	self->disconnect_process();
	return 0;
}

void debuggerking::rtsp_exportor::disconnect_process(void)
{
	_disconnect_run = true;
	while (_disconnect_run)
	{
		if (_do_disconnect)
		{
			rtsp_client::stop();
			_do_disconnect = false;
		}
		::Sleep(10);
	}
	rtsp_client::stop();
}