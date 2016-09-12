#include "dk_rtsp_receiver.h"
#include <dk_recorder_module.h>
#include <dk_string_helper.h>

debuggerking::rtsp_receiver::rtsp_receiver(rtsp_user_unregistered_sei__callback * cb)
	: _cb(cb)
	, _frame_count(0)
	, _osd_enable(false)
	, _osd_x(-1)
	, _osd_y(-1)
	, _last_year(0)
	, _last_month(0)
	, _last_day(0)
	, _last_hour(0)
	, _last_minute(0)
	, _last_second(0)
#if defined(WITH_TIMED_DISCONNECT)
	, _thread(INVALID_HANDLE_VALUE)
	, _run(false)
	, _disconnect_thread(INVALID_HANDLE_VALUE)
	, _disconnect_run(false)
#endif
{
#if defined(WITH_TIMED_DISCONNECT)
	unsigned int thread_id = 0;
	_disconnect_thread = (HANDLE)_beginthreadex(NULL, 0, debuggerking::rtsp_receiver::disconnect_process_cb, this, 0, &thread_id);
	for (int32_t i = 0; !_disconnect_run || i < 50; i++)
		::Sleep(10);
#endif
}

debuggerking::rtsp_receiver::~rtsp_receiver(void)
{
#if defined(WITH_TIMED_DISCONNECT)
	_disconnect_run = false;
	if (_disconnect_thread != INVALID_HANDLE_VALUE)
	{
		if (::WaitForSingleObject(_disconnect_thread, INFINITE) == WAIT_OBJECT_0)
		{
			::CloseHandle(_disconnect_thread);
		}
		_disconnect_thread = INVALID_HANDLE_VALUE;
	}
#endif
}

int32_t debuggerking::rtsp_receiver::enable_osd(bool enable)
{
	_osd_enable = enable;
	return rtsp_receiver::err_code_t::success;
}

int32_t debuggerking::rtsp_receiver::set_osd_position(int32_t x, int32_t y)
{
	_osd_x = x;
	_osd_y = y;
	return rtsp_receiver::err_code_t::success;
}

int32_t debuggerking::rtsp_receiver::get_last_time(int32_t & year, int32_t & month, int32_t & day, int32_t & hour, int32_t & minute, int32_t & second)
{
	year = _last_year;
	month = _last_month;
	day = _last_day;
	hour = _last_hour;
	minute = _last_minute;
	second = _last_second;
	return rtsp_receiver::err_code_t::success;
}

#if defined(WITH_TIMED_DISCONNECT)
int32_t debuggerking::rtsp_receiver::play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, float scale, bool repeat, int32_t second, HWND hwnd)
{
	if (!url || strlen(url) < 1)
		return rtsp_receiver::err_code_t::fail;

	strncpy_s(_url, url, sizeof(_url));
	if (username && strlen(username)>0)
		strncpy_s(_username, username, sizeof(_username));
	if (password && strlen(password)>0)
		strncpy_s(_password, password, sizeof(_password));

	_transport_option = transport_option;
	_recv_option = recv_option;
	_scale = scale;
	_repeat = repeat;
	_second = second;
	_hwnd = hwnd;

	unsigned int thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, debuggerking::rtsp_receiver::process_callback, this, 0, &thrdaddr);
	return rtsp_receiver::err_code_t::success;
}
#else
int32_t debuggerking::rtsp_receiver::play(const char * url, const char * username, const char * password, int32_t transport_option, int32_t recv_option, float scale, bool repeat, HWND hwnd)
{
	_hwnd = hwnd;
	return rtsp_client::play(url, username, password, transport_option, recv_option, 60, scale, repeat);
}
#endif

#if defined(WITH_TIMED_DISCONNECT)
int32_t debuggerking::rtsp_receiver::stop(void)
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

	_transport_option = debuggerking::rtsp_receiver::rtp_over_tcp;
	_recv_option = debuggerking::rtsp_receiver::recv_option_t::video;
	_repeat = false;
	_second = 0;

	return rtsp_receiver::err_code_t::success;
}
#else
int32_t debuggerking::rtsp_receiver::stop(void)
{
	int32_t status = rtsp_client::stop();

	if (_video_decoder)
	{
		_video_decoder->release_decoder();
		delete _video_decoder;
		_video_decoder = nullptr;
	}
	if (_video_decoder_config)
	{
		delete _video_decoder_config;
		_video_decoder_config = nullptr;
	}

	if (_video_renderer)
	{
		_video_renderer->release_renderer();
		delete _video_renderer;
		_video_renderer = nullptr;
	}
	if (_video_renderer_config)
	{
		delete _video_renderer_config;
		_video_renderer_config = nullptr;
	}

	if (_audio_decoder)
	{
		_audio_decoder->release_decoder();
		delete _audio_decoder;
		_audio_decoder = nullptr;
	}
	if (_audio_decoder_config)
	{
		delete _audio_decoder_config;
		_audio_decoder_config = nullptr;
	}

	if (_audio_renderer)
	{
		_audio_renderer->release_renderer();
		delete _audio_renderer;
		_audio_renderer = nullptr;
	}
	if (_audio_renderer_config)
	{
		delete _audio_renderer_config;
		_audio_renderer_config = nullptr;
	}

	return status;
}
#endif

void debuggerking::rtsp_receiver::on_begin_video(int32_t smt, uint8_t * vps, size_t vpssize, uint8_t * sps, size_t spssize, uint8_t * pps, size_t ppssize, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (_video_decoder)
	{
		_video_decoder->release_decoder();
		delete _video_decoder;
		_video_decoder = nullptr;
	}
	if (_video_decoder_config)
	{
		delete _video_decoder_config;
		_video_decoder_config = nullptr;
	}

	if (_video_renderer)
	{
		_video_renderer->release_renderer();
		delete _video_renderer;
		_video_renderer = nullptr;
	}
	if (_video_renderer_config)
	{
		delete _video_renderer_config;
		_video_renderer_config = nullptr;
	}

	if (smt == rtsp_client::video_submedia_type_t::h264)
	{
		_video_decoder = new ff_video_decoder();
		_video_decoder_config = new ff_video_decoder::configuration_t();

#if defined(WITH_BITMAP_RENDERER)
		_video_renderer = new bitmap_renderer();
		_video_renderer_config = new bitmap_renderer::configuration_t();
#else
		_video_renderer = new directdraw_renderer();
		_video_renderer_config = new directdraw_renderer::configuration_t();
#endif

		ff_video_decoder * video_decoder = static_cast<ff_video_decoder*>(_video_decoder);
		ff_video_decoder::configuration_t * video_decoder_config = static_cast<ff_video_decoder::configuration_t*>(_video_decoder_config);

#if defined(WITH_BITMAP_RENDERER)
		bitmap_renderer * video_renderer = static_cast<bitmap_renderer*>(_video_renderer);
		bitmap_renderer::configuration_t * video_renderer_config = static_cast<bitmap_renderer::configuration_t*>(_video_renderer_config);
		video_renderer_config->stretch = true;
#else
		directdraw_renderer * video_renderer = static_cast<directdraw_renderer*>(_video_renderer);
		directdraw_renderer::configuration_t * video_renderer_config = static_cast<directdraw_renderer::configuration_t*>(_video_renderer_config);
		video_renderer_config->stretch = true;
#endif
		do
		{
			if (parse_sps((BYTE*)(sps), spssize, &video_decoder_config->iwidth, &video_decoder_config->iheight, &video_decoder_config->sarwidth, &video_decoder_config->sarheight) > 0)
			{
				video_decoder_config->owidth = video_decoder_config->iwidth;
				video_decoder_config->oheight = video_decoder_config->iheight;
				video_decoder_config->codec = ff_video_decoder::video_submedia_type_t::h264;
				video_decoder_config->cs = ff_video_decoder::video_submedia_type_t::rgb32;

				video_renderer_config->hwnd = _hwnd;
				video_renderer_config->width = video_decoder_config->owidth;
				video_renderer_config->height = video_decoder_config->oheight;

				int32_t decode_err = video_decoder->initialize_decoder(video_decoder_config);
				int32_t render_err = video_renderer->initialize_renderer(video_renderer_config);
				if (decode_err == video_decoder::err_code_t::success)
				{
					video_decoder::entity_t encoded;
					encoded.mem_type = video_decoder::video_memory_type_t::host;

					video_decoder::entity_t decoded; //= { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, _video_buffer, 0, VIDEO_BUFFER_SIZE, dk_ff_video_decoder::PICTURE_TYPE_NONE };
					decoded.mem_type = video_decoder::video_memory_type_t::host;//, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
					decoded.data = _video_buffer;
					decoded.data_capacity = VIDEO_BUFFER_SIZE;

					//sps
					encoded.data = (uint8_t*)sps;
					encoded.data_size = spssize;
					decode_err = video_decoder->decode(&encoded, &decoded);
					if ((decode_err == video_decoder::err_code_t::success) && (decoded.data_size > 0))
					{
						if (render_err == video_renderer::err_code_t::success)
						{
							video_renderer::entity_t render;//= { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
							render.mem_type = video_renderer::video_memory_type_t::host;
							render.data = decoded.data;
							render.data_size = decoded.data_size;
							video_renderer->render(&render);
						}
					}

					//pps
					encoded.data = (uint8_t*)pps;
					encoded.data_size = ppssize;
					decode_err = video_decoder->decode(&encoded, &decoded);
					if ((decode_err == video_decoder::err_code_t::success) && (decoded.data_size > 0))
					{
						if (render_err == video_renderer::err_code_t::success)
						{
							video_renderer::entity_t render;
							render.mem_type = video_decoder::video_memory_type_t::host;
							render.data = decoded.data;
							render.data_size = decoded.data_size;
							video_renderer->render(&render);
						}
					}

					//idr
					encoded.data = (uint8_t*)data;
					encoded.data_size = data_size;
					decode_err = video_decoder->decode(&encoded, &decoded);
					if ((decode_err == video_decoder::err_code_t::success) && (decoded.data_size > 0))
					{
						if (render_err == video_renderer::err_code_t::success)
						{
							video_renderer::entity_t render;
							render.mem_type = video_renderer::video_memory_type_t::host;
							render.data = decoded.data;
							render.data_size = decoded.data_size;
							video_renderer->render(&render);
						}
					}
				}
			}
		} while (0);
	}
}

void debuggerking::rtsp_receiver::on_recv_video(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (smt == rtsp_client::video_submedia_type_t::h264)
	{
		ff_video_decoder * video_decoder = static_cast<ff_video_decoder*>(_video_decoder);
		ff_video_decoder::configuration_t * video_decoder_config = static_cast<ff_video_decoder::configuration_t*>(_video_decoder_config);

#if defined(WITH_BITMAP_RENDERER)
		bitmap_renderer * video_renderer = static_cast<bitmap_renderer*>(_video_renderer);
#else
		directdraw_renderer * video_renderer = static_cast<directdraw_renderer*>(_video_renderer);
#endif

		video_decoder::entity_t encoded;
		encoded.mem_type = video_decoder::video_memory_type_t::host;
		video_decoder::entity_t decoded;
		decoded.mem_type = video_decoder::video_memory_type_t::host;

		encoded.data = (uint8_t*)data;
		encoded.data_size = data_size;

		decoded.data = _video_buffer;
		decoded.data_capacity = VIDEO_BUFFER_SIZE;

		//sei[27] = { 0x06, 0x05, 0x08, 0xbc, 0x97, 0xb8, 0x4d, 0x96, 0x9f, 0x48, 0xb9, 0xbc, 0xe4, 0x7c, 0x1c, 0x1a, 0x39, 0x2f, 0x37, 00, 00, 00, 00, 00, 00, 00, 00 };
		//sei[27] = { , , , , , , , , , , , , , , , , 00, 00, 00, 00, 00, 00, 00, 00 };
		if (((data[4] & 0x1F) == 0x06) && 
			(data[5] == 0x05) && 
			(data[6] == 0x08) &&
			(data[7] == 0xbc) &&
			(data[8] == 0x97) &&
			(data[9] == 0xb8) &&
			(data[10] == 0x4d) &&
			(data[11] == 0x96) &&
			(data[12] == 0x9f) &&
			(data[13] == 0x48) &&
			(data[14] == 0xb9) &&
			(data[15] == 0xbc) &&
			(data[16] == 0xe4) &&
			(data[17] == 0x7c) &&
			(data[18] == 0x1c) &&
			(data[19] == 0x1a) &&
			(data[20] == 0x39) &&
			(data[21] == 0x2f) &&
			(data[22] == 0x37))
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

			int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
#if defined(WITH_MILLISECOND)
			debuggerking::recorder_module::get_time_from_elapsed_millisec_from_epoch(timestamp, year, month, day, hour, minute, second);
#else
			debuggerking::recorder_module::get_time_from_elapsed_microsec_from_epoch(timestamp, year, month, day, hour, minute, second);
#endif
			if (_osd_enable)
			{
				wchar_t time[MAX_PATH] = { 0 };
				_snwprintf_s(time, sizeof(time) / sizeof(wchar_t), L"%.4d-%.2d-%.2d %.2d:%.2d:%.2d", year, month, day, hour, minute, second);

				video_renderer->enable_osd_text(true);
				video_renderer->set_osd_text_color(0xFF, 0xFF, 0xFF);
				if (_osd_x == -1 || _osd_y == -1)
					video_renderer->set_osd_text_position(10, 10);
				else
					video_renderer->set_osd_text_position(_osd_x, _osd_y);
				video_renderer->set_osd_text(time);
			}
			else
			{
				video_renderer->set_osd_text(L"");
				video_renderer->enable_osd_text(false);
			}

			if (_cb != nullptr)
				_cb->invoke(year, month, day, hour, minute, second);

			_last_year = year;
			_last_month = month;
			_last_day = day;
			_last_hour = hour;
			_last_minute = minute;
			_last_second = second;
		}
		else
		{
			int32_t decode_err = video_decoder->decode(&encoded, &decoded);
			if ((decode_err == video_decoder::err_code_t::success) && (decoded.data_size > 0))
			{
				video_renderer::entity_t render;// = { dk_ff_video_decoder::MEMORY_TYPE_HOST, nullptr, nullptr, 0, 0, dk_ff_video_decoder::PICTURE_TYPE_NONE };
				render.mem_type = video_renderer::video_memory_type_t::host;
				render.data = decoded.data;
				render.data_size = decoded.data_size;
				video_renderer->render(&render);
			}
		}
	}
}

void debuggerking::rtsp_receiver::on_begin_audio(int32_t smt, uint8_t * config, size_t config_size, int32_t samplerate, int32_t bitdepth, int32_t channels, const uint8_t * data, size_t data_size, long long timestamp)
{

	if (_audio_decoder)
	{
		_audio_decoder->release_decoder();
		delete _audio_decoder;
		_audio_decoder = nullptr;
	}
	if (_audio_decoder_config)
	{
		delete _audio_decoder_config;
		_audio_decoder_config = nullptr;
	}

	if (_audio_renderer)
	{
		_audio_renderer->release_renderer();
		delete _audio_renderer;
		_audio_renderer = nullptr;
	}
	if (_audio_renderer_config)
	{
		delete _audio_renderer_config;
		_audio_renderer_config = nullptr;
	}

	_audio_renderer = new mmwave_renderer();
	_audio_renderer_config = new mmwave_renderer::configuration_t();
	mmwave_renderer * audio_renderer = static_cast<mmwave_renderer*>(_audio_renderer);
	mmwave_renderer::configuration_t * audio_renderer_config = static_cast<mmwave_renderer::configuration_t*>(_audio_renderer_config);

	if (smt == rtsp_client::audio_submedia_type_t::aac)
	{
		_audio_decoder = new aac_decoder();
		_audio_decoder_config = new aac_decoder::configuration_t();

		aac_decoder * audio_decoder = static_cast<aac_decoder*>(_audio_decoder);
		aac_decoder::configuration_t * audio_decoder_config = static_cast<aac_decoder::configuration_t*>(_audio_decoder_config);

		audio_decoder_config->extradata_size = config_size;
		memcpy(audio_decoder_config->extradata, config, audio_decoder_config->extradata_size);
		audio_decoder_config->samplerate = samplerate;
		audio_decoder_config->bitdepth = bitdepth;
		audio_decoder_config->channels = channels;

		audio_renderer_config->samplerate = samplerate;
		audio_renderer_config->bitdepth = bitdepth;
		audio_renderer_config->channels = 2;

		int32_t decode_err = audio_decoder->initialize_decoder(audio_decoder_config);
		int32_t render_err = audio_renderer->initialize_renderer(audio_renderer_config);

		if (decode_err == audio_decoder::err_code_t::success)
		{
			audio_decoder::entity_t encoded;
			encoded.data = (uint8_t*)data;
			encoded.data_size = data_size;
			audio_decoder::entity_t pcm;
			pcm.data = _audio_buffer;
			pcm.data_size = 0;
			pcm.data_capacity = AUDIO_BUFFER_SIZE;

			decode_err = audio_decoder->decode(&encoded, &pcm);
			if ((decode_err == audio_decoder::err_code_t::success) && (pcm.data_size > 0))
			{
				audio_renderer::entity_t render;
				render.data = pcm.data;
				render.data_size = pcm.data_size;
				audio_renderer->render(&render);
			}
		}

	}
	else if (smt == rtsp_client::audio_submedia_type_t::mp3)
	{
		_audio_decoder = new ff_mp3_decoder();
		_audio_decoder_config = new ff_mp3_decoder::configuration_t();

		ff_mp3_decoder * audio_decoder = static_cast<ff_mp3_decoder*>(_audio_decoder);
		ff_mp3_decoder::configuration_t * audio_decoder_config = static_cast<ff_mp3_decoder::configuration_t*>(_audio_decoder_config);

		audio_decoder_config->samplerate = samplerate;
		audio_decoder_config->bitdepth = bitdepth;
		audio_decoder_config->channels = channels;

		audio_renderer_config->samplerate = samplerate;
		audio_renderer_config->bitdepth = bitdepth;
		audio_renderer_config->channels = 2;

		int32_t decode_err = audio_decoder->initialize_decoder(audio_decoder_config);
		int32_t render_err = audio_renderer->initialize_renderer(audio_renderer_config);
		if (decode_err == audio_decoder::err_code_t::success)
		{
			audio_decoder::entity_t encoded;
			encoded.data = (uint8_t*)data;
			encoded.data_size = data_size;

			audio_decoder::entity_t pcm;
			pcm.data = _audio_buffer;
			pcm.data_size = 0;
			pcm.data_capacity = AUDIO_BUFFER_SIZE;

			decode_err = audio_decoder->decode(&encoded, &pcm);
			if ((decode_err == audio_decoder::err_code_t::success) && (pcm.data_size > 0))
			{
				audio_renderer::entity_t render;
				render.data = pcm.data;
				render.data_size = pcm.data_size;
				audio_renderer->render(&render);
			}
		}
	}
}

void debuggerking::rtsp_receiver::on_recv_audio(int32_t smt, const uint8_t * data, size_t data_size, long long timestamp)
{
	if (smt == rtsp_client::audio_submedia_type_t::aac)
	{
		aac_decoder * audio_decoder = static_cast<aac_decoder*>(_audio_decoder);
		aac_decoder::configuration_t * audio_decoder_config = static_cast<aac_decoder::configuration_t*>(_audio_decoder_config);

		aac_decoder * aac_audio_decoder = static_cast<aac_decoder*>(_audio_decoder);

		audio_decoder::entity_t encoded;
		encoded.data = (uint8_t*)data;
		encoded.data_size = data_size;

		audio_decoder::entity_t pcm;
		pcm.data = _audio_buffer;
		pcm.data_size = 0;
		pcm.data_capacity = AUDIO_BUFFER_SIZE;

		int32_t decode_err = aac_audio_decoder->decode(&encoded, &pcm);
		if ((decode_err == audio_decoder::err_code_t::success) && (pcm.data_size > 0))
		{
			audio_renderer::entity_t render;
			render.data = pcm.data;
			render.data_size = pcm.data_size;
			_audio_renderer->render(&render);
		}
	}
	else if (smt == rtsp_client::audio_submedia_type_t::mp3)
	{
		ff_mp3_decoder * audio_decoder = static_cast<ff_mp3_decoder*>(_audio_decoder);
		ff_mp3_decoder::configuration_t * audio_decoder_config = static_cast<ff_mp3_decoder::configuration_t*>(_audio_decoder_config);

		audio_decoder::entity_t encoded;
		encoded.data = (uint8_t*)data;
		encoded.data_size = data_size;

		audio_decoder::entity_t pcm;
		pcm.data = _audio_buffer;
		pcm.data_size = 0;
		pcm.data_capacity = AUDIO_BUFFER_SIZE;

		int32_t decode_err = audio_decoder->decode(&encoded, &pcm);
		if ((decode_err == audio_decoder::err_code_t::success) && (pcm.data_size > 0))
		{
			audio_renderer::entity_t render;
			render.data = pcm.data;
			render.data_size = pcm.data_size;
			_audio_renderer->render(&render);
		}
	}
}

unsigned __stdcall debuggerking::rtsp_receiver::process_callback(void * param)
{
	rtsp_receiver * self = static_cast<rtsp_receiver*>(param);
	self->process();
	return 0;
}

void debuggerking::rtsp_receiver::process(void)
{
	int status = rtsp_receiver::err_code_t::fail;
	status = rtsp_client::play(_url, _username, _password, _transport_option, _recv_option, 0, _scale, _repeat);
	if (status == rtsp_receiver::err_code_t::success)
	{
		_run = true;
		long long last_timestamp = 0;
		size_t timestamp_unchange_count = 0;

		if (_second == 0)
		{
			while (_run)
				::Sleep(1000);
		}
		else
		{
			for (int32_t index = 0; index < _second && _run; index++)
				::Sleep(1000);
		}

		//status = rtsp_client::stop();
		_do_disconnect = true;
	}
	_run = false;
}


unsigned debuggerking::rtsp_receiver::disconnect_process_cb(void * param)
{
	debuggerking::rtsp_receiver * self = static_cast<debuggerking::rtsp_receiver*>(param);
	self->disconnect_process();
	return 0;
}

void debuggerking::rtsp_receiver::disconnect_process(void)
{
	bool do_disconnect_called = false;
	_disconnect_run = true;
	while (_disconnect_run)
	{
		if (_do_disconnect)
		{
			rtsp_client::stop();
			_do_disconnect = false;
			do_disconnect_called = true;
		}
		::Sleep(10);
	}
	if (!do_disconnect_called)
		rtsp_client::stop();
}