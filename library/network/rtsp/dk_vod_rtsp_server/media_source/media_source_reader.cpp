#include <windows.h>
#include <process.h>
#include <cstdio>
#include <dk_misc_helper.h>
#include <dk_time_helper.h>
#include <dk_auto_lock.h>
#include "media_source_reader.h"
#include <dk_record_module.h>
#include <boost/date_time/local_time/local_time.hpp>
#include <dk_log4cplus_logger.h>

#define VIDEO_BUFFER_QUEUE_SIZE	1024*1024*4
#define VIDEO_BUFFER_SIZE		1024*1024*2
#define MAX_VIDEO_QUEUE_COUNT	10

#if defined(WITH_RECORD_SERVER)
debuggerking::media_source_reader::media_source_reader(void)
	: _scale(1.f)
#if defined(WITH_RECORD_MODULE)
#if defined(WITH_BUFFERING_MODE)
	, _video_thread(INVALID_HANDLE_VALUE)
	, _video_run(false)
	, _video_queue_count(0)
	, _video_buffer(nullptr)
	, _video_buffer_size(0)
#endif
#endif
{
#if defined(WITH_RECORD_MODULE)
#if defined(WITH_BUFFERING_MODE)
	_video_buffer_size = VIDEO_BUFFER_SIZE;
	_video_buffer = static_cast<uint8_t*>(malloc(_video_buffer_size));

	::InitializeCriticalSection(&_video_mutex);
	_video_queue = circular_buffer_t::create(VIDEO_BUFFER_QUEUE_SIZE);
	_video_root = static_cast<video_buffer_t*>(malloc(sizeof(video_buffer_t)));
	init_video(_video_root);
#endif
#endif
}

debuggerking::media_source_reader::~media_source_reader(void)
{
	close();

#if defined(WITH_RECORD_MODULE)
#if defined(WITH_BUFFERING_MODE)
	video_buffer_t * vbuffer = _video_root->next;
	while (vbuffer)
	{
		vbuffer->prev->next = vbuffer->next;
		free(vbuffer);
		vbuffer = nullptr;
	}
	free(_video_root);
	_video_root = nullptr;
	circular_buffer_t::destroy(_video_queue);
	::DeleteCriticalSection(&_video_mutex);

	if (_video_buffer)
		free(_video_buffer);
	_video_buffer = nullptr;
	_video_buffer_size = 0;
#endif
#endif
}

bool debuggerking::media_source_reader::open(const char * stream_name, long long timestamp, int32_t & vsmt, int32_t & asmt)
{
	if (!stream_name || strlen(stream_name) < 1)
		return media_source_reader::err_code_t::fail;

	strncpy(_stream_name, stream_name, sizeof(_stream_name));

	//open video file
	char contents_path[260] = { 0 };
	char * module_path = nullptr;
	dk_misc_helper::retrieve_absolute_module_path("dk_vod_rtsp_server.dll", &module_path);
	if (module_path && strlen(module_path)>0)
	{
		_snprintf_s(contents_path, sizeof(contents_path), "%s%s", module_path, "contents\\");
		free(module_path);
	}

	char ms_uuid[260] = { 0 };
	char seek_time[260] = { 0 };
	char * slash = (char*)strrchr(stream_name, '/');
	if (slash != NULL)
	{
		strncpy_s(seek_time, slash + 1, strlen(slash + 1));
		memcpy(ms_uuid, stream_name, slash - stream_name);
		if (strlen(seek_time) == 14)
		{
			char single_media_source_path[260] = { 0 };
			_snprintf_s(single_media_source_path, sizeof(single_media_source_path), "%s%s", contents_path, ms_uuid);
			//_snprintf_s(single_media_source_path, sizeof(single_media_source_path), "%s%s\\*", contents_path, ms_uuid);

			int32_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
			sscanf(seek_time, "%4d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);
			char time_string[100] = { 0 };
			_snprintf_s(time_string, sizeof(time_string), "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.000", year, month, day, hour, minute, second);

			boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
			boost::posix_time::ptime pseek_time = boost::posix_time::time_from_string(time_string);
			boost::posix_time::time_duration elapsed_seek_time = pseek_time - epoch;
			long long elapsed_seek_time_millsec = elapsed_seek_time.total_milliseconds();

			char time[260] = { 0 };
#if defined(WITH_RECORD_MODULE)
			debuggerking::recorder_module::get_time_from_elapsed_msec_from_epoch(elapsed_seek_time_millsec, time, sizeof(time));
			bool result = _record_module.seek(single_media_source_path, elapsed_seek_time_millsec);
			if (!result)
				return false;
#else
			get_time_from_elapsed_msec_from_epoch(elapsed_seek_time_millsec, time, sizeof(time));
			//dk_log4cplus_logger::instance().make_system_debug_log("parallel.record.streamer", time);

			bool result = _record_module_seeker.seek(single_media_source_path, elapsed_seek_time_millsec);
			if (!result)
				return false;
#endif
		}
		else
		{ 
			//live
			strncpy_s(ms_uuid, slash + 1, strlen(slash + 1));


			return false;
		}
	}
	else
	{ 
		//live

		return false;
	}

	_vsmt = media_source_reader::video_submedia_type_t::h264;
	_asmt = media_source_reader::audio_submedia_type_t::unknown;
	vsmt = _vsmt;
	asmt = _asmt;

#if defined(WITH_RECORD_MODULE)
#if defined(WITH_BUFFERING_MODE)
	unsigned thrd_addr = 0;
	_video_thread = (HANDLE)::_beginthreadex(NULL, 0, &media_source_reader::video_process_callback, this, 0, &thrd_addr);
#endif
#endif
	return true;
}

bool debuggerking::media_source_reader::close(void)
{
#if defined(WITH_RECORD_MODULE)
#if defined(WITH_BUFFERING_MODE)
	if (_video_thread && _video_thread != INVALID_HANDLE_VALUE)
	{
		::WaitForSingleObject(_video_thread, INFINITE);
		::CloseHandle(_video_thread);
		_video_thread = INVALID_HANDLE_VALUE;
	}
#endif
#endif
	return true;
}

bool debuggerking::media_source_reader::read(int32_t mt, uint8_t * data, size_t data_capacity, size_t & data_size, long long & timestamp)
{
	if (mt == media_source_reader::media_type_t::video)
	{
		if (_vsmt == media_source_reader::video_submedia_type_t::h264)
		{
#if defined(WITH_RECORD_MODULE)
#if defined(WITH_BUFFERING_MODE)
			uint8_t nalu_type = recorder_module::nalu_type_t::none;
			int32_t status = pop_video(data, data_size, nalu_type, timestamp);
			if(status != media_source_reader::err_code_t::success)
				return false;
#else
			_record_module.read(data, data_size, timestamp);
#endif
#else
			_record_module_seeker.read(data, data_size, timestamp);
#endif
		}
	}
	else if (mt == media_source_reader::media_type_t::audio)
	{
		return false;
	}
	return true;
}

const uint8_t * debuggerking::media_source_reader::get_sps(size_t & sps_size)
{
#if defined(WITH_RECORD_MODULE)
	return _record_module.get_sps(sps_size);
#else
	return _record_module_seeker.get_sps(sps_size);
#endif
}

const uint8_t * debuggerking::media_source_reader::get_pps(size_t & pps_size)
{
#if defined(WITH_RECORD_MODULE)
	return _record_module.get_pps(pps_size);
#else
	return _record_module_seeker.get_pps(pps_size);
#endif
}

#if defined(WITH_RECORD_MODULE)
#if defined(WITH_BUFFERING_MODE)
int32_t debuggerking::media_source_reader::push_video(uint8_t * bs, size_t size, uint8_t nalu_type, long long timestamp)
{
	int32_t status = media_source_reader::err_code_t::success;
	dk_auto_lock lock(&_video_mutex);
	if (bs && size > 0)
	{
		video_buffer_t * vbuffer = _video_root;
		vbuffer->amount = VIDEO_BUFFER_QUEUE_SIZE;
		//move to tail
		do
		{
			if (!vbuffer->next)
				break;
			vbuffer = vbuffer->next;
		} while (1);

		vbuffer->next = static_cast<video_buffer_t*>(malloc(sizeof(video_buffer_t)));
		init_video(vbuffer->next);
		vbuffer->next->prev = vbuffer;
		vbuffer = vbuffer->next;

		vbuffer->amount = size;
		vbuffer->nalu_type = nalu_type;
		vbuffer->timestamp = timestamp;
		int32_t result = circular_buffer_t::write(_video_queue, bs, vbuffer->amount);
		if (result == -1)
		{
			if (vbuffer->prev)
				vbuffer->prev->next = nullptr;
			free(vbuffer);
			vbuffer = nullptr;
			status = media_source_reader::err_code_t::fail;
		}
	}
	return status;
}

int32_t debuggerking::media_source_reader::pop_video(uint8_t * bs, size_t & size, uint8_t & nalu_type, long long & timestamp)
{
	int32_t status = media_source_reader::err_code_t::success;
	size = 0;
	dk_auto_lock lock(&_video_mutex);
	video_buffer_t * vbuffer = _video_root->next;
	if (vbuffer)
	{
		vbuffer->prev->next = vbuffer->next;
		int32_t result = circular_buffer_t::read(_video_queue, bs, vbuffer->amount);
		if (result == -1)
		{
			status = media_source_reader::err_code_t::fail;
		}
		else
		{
			size = vbuffer->amount;
			nalu_type = vbuffer->nalu_type;
			timestamp = vbuffer->timestamp;
		}
		free(vbuffer);
		vbuffer = nullptr;
	}
	return status;
}

int32_t debuggerking::media_source_reader::init_video(video_buffer_t * buffer)
{
	buffer->timestamp = 0;
	buffer->amount = 0;
	buffer->nalu_type = recorder_module::nalu_type_t::none;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return media_source_reader::err_code_t::success;
}

unsigned debuggerking::media_source_reader::video_process_callback(void * param)
{
	media_source_reader * self = static_cast<media_source_reader*>(param);
	self->video_process();
	return 0;
}

void debuggerking::media_source_reader::video_process(void)
{
	_video_run = true;
	while (_video_run)
	{
		if (_video_queue_count < MAX_VIDEO_QUEUE_COUNT)
		{
			long long timestamp = 0;
			size_t video_buffer_size = 0;
			_record_module.read(_video_buffer, video_buffer_size, timestamp);

			uint8_t nalu_type = recorder_module::nalu_type_t::none;
			if ((_video_buffer[4] & 0x1F) == 0x07)
				nalu_type = recorder_module::nalu_type_t::sps;
			else if ((_video_buffer[4] & 0x1F) == 0x08)
				nalu_type = recorder_module::nalu_type_t::pps;
			else if ((_video_buffer[4] & 0x1F) == 0x05)
				nalu_type = recorder_module::nalu_type_t::idr;
			else
				nalu_type = recorder_module::nalu_type_t::vcl;

			int32_t status = push_video(_video_buffer, video_buffer_size, nalu_type, timestamp);
			if (status == media_source_reader::err_code_t::success)
				_video_queue_count++;
		}
		::Sleep(10);
	}
}
#endif
#endif

#if !defined(WITH_RECORD_MODULE)
void media_source_reader::get_time_from_elapsed_msec_from_epoch(long long elapsed_time, char * time_string, int time_string_size)
{
	boost::posix_time::time_duration elapsed = boost::posix_time::millisec(elapsed_time);
	boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
	boost::posix_time::ptime current_time = epoch + elapsed;

	std::string tmp_time = boost::posix_time::to_simple_string(current_time);
	//strncpy_s(time_string, time_string_size, tmp_time.c_str(), (size_t)time_string_size);
	strcpy_s(time_string, time_string_size, tmp_time.c_str());
}
#endif
#else

#define VIDEO_BUFFER_SIZE 1024 * 1024 * 6
#define AUDIO_BUFFER_SIZE 48000 * 2 * 8 //48000hz * 16bitdetph * 8 channels ex) for 2channel 192000

#define MAX_AUDIO_FRAME_SIZE 192000

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

#define AV_RB32(x)  \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) | \
               (((const uint8_t*)(x))[1] << 16) | \
               (((const uint8_t*)(x))[2] <<  8) | \
                ((const uint8_t*)(x))[3])

#define MIN(a,b) ((a) > (b) ? (b) : (a))

media_source_reader::media_source_reader(void)
{
	_video_buffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));
	_audio_buffer = static_cast<uint8_t*>(malloc(AUDIO_BUFFER_SIZE)); //48000hz * 16bitdetph * 8 channels
}

media_source_reader::~media_source_reader(void)
{
	close();
	if (_video_buffer)
		free(_video_buffer);
	if (_audio_buffer)
		free(_audio_buffer);
}

bool media_source_reader::open(const char * stream_name, long long timestamp, media_source_reader::vsubmedia_type & vsubmedia_type, media_source_reader::asubmedia_type & asubmedia_type)
{
	if (!stream_name || strlen(stream_name) < 1)
		return media_source_reader::err_code_fail;

	strncpy(_stream_name, stream_name, sizeof(_stream_name));
	_format_ctx = nullptr;

	_video_ctx = nullptr;
	_video_stream_index = -1;
	_video_stream = nullptr;
	//_video_timer = (double)av_gettime() / 1000000.0;
	_video_last_dts = 0;
	//_video_last_delay = 40e-3;
	//_video_current_dts = 0;
	//_video_current_dts_time = av_gettime();
	//_video_clock = 0;
	_video_recv_keyframe = false;

	//open video file

#if defined(WITH_RECORD_SERVER)
	//if (avformat_open_input(&_format_ctx, "C:\\workspace\\01.reference\\media-processing-system\\build\\win32\\x86\\debug\\bin\\contents\\9344D189-202F-4248-905E-F02E9A2288A2\\1458465130.ts", NULL, NULL) != 0)
	//	return false; //couldn't open file

	char contents_path[260] = { 0 };
	char * module_path = nullptr;
	dk_misc_helper::retrieve_absolute_module_path("dk_vod_rtsp_server.dll", &module_path);
	if (module_path && strlen(module_path)>0)
	{
		_snprintf_s(contents_path, sizeof(contents_path), "%s%s", module_path, "contents\\");
		free(module_path);
	}

#if 1
	char ms_uuid[260] = { 0 };
	char seek_time[260] = { 0 };
	char * slash = (char*)strrchr(stream_name, '/');
	if (slash != NULL)
	{
		strncpy_s(seek_time, slash + 1, strlen(slash + 1));
		memcpy(ms_uuid, stream_name, slash - stream_name);

		char single_ms_path[260] = { 0 };
		_snprintf_s(single_ms_path, sizeof(single_ms_path), "%s%s\\*", contents_path, ms_uuid);

		HANDLE bfind = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA wfd;
		bfind = ::FindFirstFileA(single_ms_path, &wfd);
		if (bfind == INVALID_HANDLE_VALUE)
			return false;
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				char * recorded_file_name = &wfd.cFileName[0];
				char single_ms[260] = { 0 };
				_snprintf_s(single_ms, sizeof(single_ms), "%s%s\\%s", contents_path, ms_uuid, recorded_file_name);
				if (avformat_open_input(&_format_ctx, single_ms, NULL, NULL) != 0)
					return false; //couldn't open file
				break;
			}
		} while (::FindNextFileA(bfind, &wfd));

		::FindClose(bfind);
	}
	else
	{
		return false;
	}
#else
	char ms_uuid[260] = { 0 };
	char seek_time[260] = { 0 };
	char * slash = (char*)strrchr(stream_name, '/');
	if (slash != NULL)
	{
		strncpy_s(seek_time, slash + 1, strlen(slash + 1));
		memcpy(ms_uuid, stream_name, slash - stream_name);

		char single_ms_path[260] = { 0 };
		_snprintf_s(single_ms_path, sizeof(single_ms_path), "%s%s\\*", contents_path, ms_uuid);

		if (strlen(seek_time) != 14) //20160320154030 : 2015-03-20 15::40::30
			return false;

		SYSTEMTIME req_utc_time = { 0 };
		if (!dk_time_helper::convert_local_string_time_to_utc_time(seek_time, req_utc_time))
			return false;

		HANDLE bfind = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA wfd;
		bfind = ::FindFirstFileA(single_ms_path, &wfd);
		if (bfind == INVALID_HANDLE_VALUE)
			return false;
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				char * recorded_file_name = &wfd.cFileName[0];
				char * dot = (char*)strrchr(recorded_file_name, '.');
				char recorded_file_creation_time[260] = { 0 };
				memcpy(recorded_file_creation_time, recorded_file_name, dot - recorded_file_name);
				unsigned long recorded_file_creation_elapsed_utc_time = 0;
				sscanf(recorded_file_creation_time, "%lu", &recorded_file_creation_elapsed_utc_time);

				SYSTEMTIME recored_file_creation_utc_systemtime = { 0 };
				dk_time_helper::convert_elasped_utc_time_to_utc_system_time(recorded_file_creation_elapsed_utc_time, recored_file_creation_utc_systemtime);

				SYSTEMTIME recored_file_creation_local_systemtime = { 0 };
				dk_time_helper::convert_elasped_utc_time_to_local_system_time(recorded_file_creation_elapsed_utc_time, recored_file_creation_local_systemtime);
			}
		} while (::FindNextFileA(bfind, &wfd));
	}
	else
	{
		return false;
	}
#endif
#else //defined(WITH_RECORD_SERVER)
	if (avformat_open_input(&_format_ctx, /*_stream_name*/"C:\\workspace\\03.movie\\COSTA RICA IN 4K 60fps (ULTRA HD) (4k).mp4", NULL, NULL) != 0)
		return false; //couldn't open file
#endif

	//retrieve stream information
	if (avformat_find_stream_info(_format_ctx, NULL) < 0)
		return false; //couldn't find stream information

	_video_stream_index = -1;
	_audio_stream_index = -1;
	for (uint32_t index = 0; index<_format_ctx->nb_streams; index++)
	{
		if (_format_ctx->streams[index]->codec->codec_type == AVMEDIA_TYPE_VIDEO && _video_stream_index < 0)
			_video_stream_index = index;
		if (_format_ctx->streams[index]->codec->codec_type == AVMEDIA_TYPE_AUDIO && _audio_stream_index < 0)
			_audio_stream_index = index;
	}

	if (_video_stream_index != -1)
	{
		_video_stream = _format_ctx->streams[_video_stream_index];
		_video_ctx = _video_stream->codec;
		switch (_video_ctx->codec_id)
		{
			case AV_CODEC_ID_MJPEG:
			{
				_vsubmedia_type = vsubmedia_type_jpeg;
				break;
			}
			case AV_CODEC_ID_MPEG4:
			{
				_vsubmedia_type = vsubmedia_type_mpeg4;
				break;
			}
			case AV_CODEC_ID_H264:
			{
				_vsubmedia_type = vsubmedia_type_h264;

#if defined(WITH_RECORD_SERVER)
				uint8_t * data = _video_ctx->extradata;
				int32_t remained = _video_ctx->extradata_size;
				uint8_t * begin = data;
				uint8_t * end = data;
				bool exit = false;
				while (begin < data + remained)
				{
					uint8_t * nalu = nullptr;
					size_t nalu_size = 0;
					int nalu_begin, nalu_end;
					int size = media_source_reader::next_nalu(begin, (data + remained) - begin, &nalu_begin, &nalu_end);
					if (size == 0)
					{
						nalu = nullptr;
						nalu_size = 0;
						exit = true;
					}
					else if (size < 0)
					{
						begin += nalu_begin;
						nalu = begin - 4;
						nalu_size = (data + remained) - begin + 4;
						exit = true;
					}
					else
					{
						begin += nalu_begin;
						end += nalu_end;
						nalu = begin - 4;
						nalu_size = nalu_end - nalu_begin + 4;
					}

					if (nalu && nalu_size > 0)
					{
						bool is_sps = media_source_reader::is_sps(_vsubmedia_type, nalu[4] & 0x1F);
						if (is_sps)
						{
							set_sps(nalu, nalu_size);
						}
						bool is_pps = media_source_reader::is_pps(_vsubmedia_type, nalu[4] & 0x1F);
						if (is_pps)
						{
							set_pps(nalu, nalu_size);
						}
					}

					if (exit)
						break;

					remained -= nalu_size;//nalu size itself
					data += nalu_size;
				}
#else
				uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };

				uint8_t * sps = _video_ctx->extradata + 8;
				size_t sps_size = sps[-1];
				if (_video_ctx->extradata_size >= (8 + sps_size))
				{
					memmove(_video_extradata, start_code, sizeof(start_code));
					memmove(_video_extradata + sizeof(start_code), sps, sps_size);
					set_sps(_video_extradata, sizeof(start_code) + sps_size);
					_video_extradata_size = sizeof(start_code) + sps_size;
				}

				uint8_t * pps = _video_ctx->extradata + 8 + sps_size + 3;
				size_t pps_size = pps[-1];
				if (_video_ctx->extradata_size >= (8 + sps_size + 3 + pps_size))
				{
					memmove(_video_extradata + sizeof(start_code) + sps_size, start_code, sizeof(start_code));
					memmove(_video_extradata + sizeof(start_code) + sps_size + sizeof(start_code), pps, pps_size);
					set_pps(_video_extradata + sizeof(start_code) + sps_size, sizeof(start_code) + pps_size);
					_video_extradata_size = _video_extradata_size + sizeof(start_code) + pps_size;
				}
#endif
				break;
			}
			default:
			{
				_vsubmedia_type = unknown_video_type;
				break;
			}
		}
	}
	else
		_vsubmedia_type = unknown_video_type;

	if (_audio_stream_index != -1)
	{
		_audio_stream = _format_ctx->streams[_audio_stream_index];
		_audio_ctx = _audio_stream->codec;
		switch (_audio_ctx->codec_id)
		{
		case AV_CODEC_ID_MP3:
			_asubmedia_type = asubmedia_type_mp3;
			break;
		case AV_CODEC_ID_AAC:
			_asubmedia_type = asubmedia_type_aac;
			break;
		default:
			_asubmedia_type = unknown_audio_type;
			break;
		}
	}
	else
		_asubmedia_type = unknown_audio_type;

	vsubmedia_type = _vsubmedia_type;
	asubmedia_type = _asubmedia_type;

	unsigned thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, media_source_reader::process_cb, this, 0, &thrdaddr);
	return true;
}

bool media_source_reader::close(void)
{
	_run = false;
	if (_thread != INVALID_HANDLE_VALUE)
	{
		::WaitForSingleObject(_thread, INFINITE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
	}

	int32_t code = 0;
	AVPacket packet;
	while (1)
	{
		code = packet_queue_pop(&_video_packet_queue, &packet);
		if (code < 0 || code == 0)
			break;
		av_free_packet(&packet);
	}
	while (1)
	{
		code = packet_queue_pop(&_audio_packet_queue, &packet);
		if (code < 0 || code == 0)
			break;
		av_free_packet(&packet);
	}

	if (_format_ctx)
	{
		avformat_close_input(&_format_ctx);
		_format_ctx = nullptr;
	}

	return true;
}

bool media_source_reader::read(media_source_reader::media_type mt, uint8_t * data, size_t data_capacity, size_t & data_size, long long & timestamp)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;

	double dts = 0.f;
	double delay = 0.f;
	double actual_delay = 0.f;

	data_size = 0;
	timestamp = 0;
	if (mt == media_source_reader::media_type_video)
	{
		int32_t code = packet_queue_pop(&_video_packet_queue, &packet);
		if (code <= 0)
			return false;


		if (packet.dts != AV_NOPTS_VALUE)
			dts = packet.dts;
		if (dts != 0)
		{
			dts *= av_q2d(_video_stream->time_base);
			delay = dts - _video_last_dts;
		}
		else
		{
			delay = 0;
		}
		_video_last_dts = dts;
		actual_delay = delay * 1000; //milli second
		timestamp = actual_delay * 1000; //micro second

		if (_vsubmedia_type == vsubmedia_type_h264)
		{
			uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
			int32_t index = 0;
			int32_t packet_size = packet.size;

#if defined(WITH_RECORD_SERVER)
			uint8_t * bitstream = &packet.data[0];
			int32_t remained = packet_size;
			uint8_t * begin = bitstream;
			uint8_t * end = bitstream;
			bool exit = false;
			while (begin < bitstream + remained)
			{
				uint8_t * nalu = nullptr;
				size_t nalu_size = 0;
				int nalu_begin, nalu_end;
				int size = media_source_reader::next_nalu(begin, (bitstream + remained) - begin, &nalu_begin, &nalu_end);
				if (size == 0)
				{
					nalu = nullptr;
					nalu_size = 0;
					exit = true;
				}
				else if (size < 0)
				{
					begin += nalu_begin;
					nalu = begin - 4;
					nalu_size = (bitstream + remained) - begin + 4;
					exit = true;
				}
				else
				{
					begin += nalu_begin;
					end += nalu_end;
					nalu = begin - 4;
					nalu_size = nalu_end - nalu_begin + 4;
				}

				if (nalu && nalu_size>0)
				{
					bool is_idr = media_source_reader::is_idr(_vsubmedia_type, nalu[4] & 0x1F);
					if (is_idr && !_video_recv_keyframe)
					{
						_video_recv_keyframe = true;
						size_t sps_size = 0;
						size_t pps_size = 0;
						const uint8_t * sps = get_sps(sps_size);
						const uint8_t * pps = get_pps(pps_size);

						memmove(data + data_size, sps, sps_size);
						data_size += sps_size;
						memmove(data + data_size, pps, pps_size);
						data_size += pps_size;
						memmove(data + data_size, nalu, nalu_size);
						data_size += nalu_size;
					}
					else if (_video_recv_keyframe)
					{
						memmove(data + data_size, nalu, nalu_size);
						data_size += nalu_size;
					}
				}
				if (exit)
					break;
			}
#else
			for (int32_t x = 0; index < packet_size; x++)
			{
				if (packet_size < (index + 4))
					break;

				int32_t nalu_size = (packet.data[index] << 24) + (packet.data[index + 1] << 16) + (packet.data[index + 2] << 8) + packet.data[index + 3];
				index += sizeof(start_code);

				uint8_t * nalu = &packet.data[index];
				long long timestamp = 0;
				if (x == 0)
					timestamp = actual_delay * 1000;

				bool is_idr = media_source_reader::is_idr(_vsubmedia_type, nalu[0] & 0x1F);
				if (is_idr && !_video_recv_keyframe)
				{
					size_t sps_size = 0;
					size_t pps_size = 0;
					const uint8_t * sps = get_sps(sps_size);
					const uint8_t * pps = get_pps(pps_size);

					if (sps_size>0 && pps_size>0)
					{
						_video_recv_keyframe = true;

						memmove(data + data_size, sps, sps_size);
						data_size += sps_size;
						memmove(data + data_size, pps, pps_size);
						data_size += pps_size;
						memmove(data + data_size, start_code, sizeof(start_code));
						data_size += sizeof(start_code);
						memmove(data + data_size, nalu, nalu_size);
						data_size += nalu_size;
					}
				}
				else if (_video_recv_keyframe)
				{
					memmove(data + data_size, start_code, sizeof(start_code));
					data_size += sizeof(start_code);
					memmove(data + data_size, nalu, nalu_size);
					data_size += nalu_size;
				}
				index += nalu_size;
			}
#endif
		}
		av_free_packet(&packet);
	}
	else if (mt == media_source_reader::media_type_audio)
	{
		return false;
	}

	return true;
}

void media_source_reader::packet_queue_init(media_source_reader::packet_queue_t * q)
{
	memset(q, 0, sizeof(media_source_reader::packet_queue_t));
	q->lock = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	::SetEvent(q->lock);
}

int32_t media_source_reader::packet_queue_push(media_source_reader::packet_queue_t * q, AVPacket * pkt)
{
	AVPacket dup_packet;
	AVPacketList * pkt1;

	if (av_copy_packet(&dup_packet, pkt) < 0)
		return -1;

	pkt1 = static_cast<AVPacketList*>(av_malloc(sizeof(AVPacketList)));
	if (!pkt1)
		return -1;
	pkt1->pkt = dup_packet;
	pkt1->next = NULL;

	scoped_lock mutex(q->lock);

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;

	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	return 0;
}

int32_t media_source_reader::packet_queue_pop(media_source_reader::packet_queue_t * q, AVPacket * pkt)
{
	AVPacketList *pkt1;
	int ret;

	scoped_lock mutex(q->lock);
	pkt1 = q->first_pkt;
	if (pkt1)
	{
		q->first_pkt = pkt1->next;
		if (!q->first_pkt)
			q->last_pkt = NULL;
		q->nb_packets--;
		q->size -= pkt1->pkt.size;
		*pkt = pkt1->pkt;
		av_free(pkt1);
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	return ret;
}

void media_source_reader::set_vps(uint8_t * vps, size_t size)
{
	_vps_size = size;
	memcpy(_vps, vps, size);
}

void media_source_reader::set_sps(uint8_t * sps, size_t size)
{
	_sps_size = size;
	memcpy(_sps, sps, size);
}

void media_source_reader::set_pps(uint8_t * pps, size_t size)
{
	_pps_size = size;
	memcpy(_pps, pps, size);
}

const uint8_t * media_source_reader::get_vps(size_t & size)
{
	size = _vps_size;
	return _vps;
}

const uint8_t * media_source_reader::get_sps(size_t & size)
{
	size = _sps_size;
	return _sps;
}

const uint8_t * media_source_reader::get_pps(size_t & size)
{
	size = _pps_size;
	return _pps;
}

unsigned media_source_reader::process_cb(void * param)
{
	media_source_reader * self = static_cast<media_source_reader*>(param);
	self->process();
	return 0;
}

void media_source_reader::process(void)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;
	_run = true;

#if 1
	while(_run)
	{
		if (_video_packet_queue.nb_packets<10 /*|| _audio_packet_queue.nb_packets<10*/)
		{
			if (av_read_frame(_format_ctx, &packet) >= 0)
			{
				if (packet.stream_index == _video_stream_index)
				{
					packet_queue_push(&_video_packet_queue, &packet);
				}
				else if (packet.stream_index == _audio_stream_index)
				{
					//packet_queue_push(&_audio_packet_queue, &packet);
				}
			}
		}
		av_free_packet(&packet);
		::Sleep(10);
	}
#else
	double dts = 0;
	double delay = 0.f;
	double actual_delay = 0.f;

	for (; _run && av_read_frame(_format_ctx, &packet) >= 0;)
	{
		if (packet.stream_index == _video_stream_index)
		{
			if (packet.dts != AV_NOPTS_VALUE)
				dts = packet.dts;
			if (dts != 0)
			{
				dts *= av_q2d(_video_stream->time_base);
				delay = dts - _video_last_dts;
			}
			else
			{
				delay = 0;
			}


			//delay += av_q2d(_video_ctx->time_base);
			//if (delay <= 0 || delay >= 1.0)
			//{
			//	delay = _video_last_delay;
			//}
			_video_last_delay = delay;
			_video_last_dts = dts;
			actual_delay = delay;
			actual_delay = actual_delay * 350;

			/*if (dts != 0)
				_video_clock = dts;
			else
				dts = _video_clock;

			delay = av_q2d(_video_ctx->time_base);
			//delay += src_frame->repeat_pict * (delay * 0.5);
			_video_clock += delay;

			_video_current_dts = dts;
			_video_current_dts_time = av_gettime();
			delay = _video_current_dts - _video_last_dts;
			if (delay <= 0 || delay >= 1.0)
			{
				delay = _video_last_delay;
			}
			_video_last_delay = delay;
			_video_last_dts = _video_current_dts;

			_video_timer += delay;
			actual_delay = _video_timer - (av_gettime() / 1000000.0);
			if (actual_delay < 0.010)
			{
				actual_delay = 0.010;
			}
			actual_delay = actual_delay * 1000 + 0.5;
			*/

			if (_vsubmedia_type == vsubmedia_type_h264)
			{
				uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
				uint8_t * video_data = _video_buffer;
				int32_t video_data_size = 0;
				int32_t index = 0;
				int32_t packet_size = packet.size;

				for (int32_t x = 0; index < packet_size; x++)
				{
					if (packet_size < (index + 4))
						break;
					video_data_size = (packet.data[index] << 24) + (packet.data[index + 1] << 16) + (packet.data[index + 2] << 8) + packet.data[index + 3];
					memmove(video_data, start_code, sizeof(start_code));
					index += sizeof(start_code);

					if (packet_size < (index + video_data_size) || (UINT32_MAX - index) < video_data_size)
						break;

					memcpy(video_data + index, &packet.data[index], video_data_size);
#if 0
					bool is_idr = media_source_reader::is_idr(_vsubmedia_type, video_data[4] & 0x1F);
					if (is_idr && !_video_recv_keyframe)
					{
						uint8_t * sps = _video_ctx->extradata + 8;
						size_t sps_size = sps[-1];
						if (_video_ctx->extradata_size < (8 + sps_size))
							break; // We don't have a complete SPS
						else
						{
							memmove(_video_extradata, start_code, sizeof(start_code));
							memmove(_video_extradata + sizeof(start_code), sps, sps_size);
							_media_buffer_manager.set_sps(_video_extradata, sizeof(start_code) + sps_size);
							_video_extradata_size = sizeof(start_code) + sps_size;
						}

						uint8_t * pps = _video_ctx->extradata + 8 + sps_size + 3;
						size_t pps_size = pps[-1];
						if (_video_ctx->extradata_size < (8 + sps_size + 3 + pps_size))
							break;
						else
						{
							memmove(_video_extradata + sizeof(start_code) + sps_size, start_code, sizeof(start_code));
							memmove(_video_extradata + sizeof(start_code) + sps_size + sizeof(start_code), pps, pps_size);
							_media_buffer_manager.set_pps(_video_extradata + sizeof(start_code) + sps_size, sizeof(start_code) + pps_size);
							_video_extradata_size = _video_extradata_size + sizeof(start_code) + pps_size;
						}
						_video_recv_keyframe = true;
						long long timestamp = actual_delay * 1000;
						_media_buffer_manager.push_video(video_data, video_data_size, timestamp);
					}

					if (_video_recv_keyframe)
					{
						long long timestamp = actual_delay * 1000;
						_media_buffer_manager.push_video(video_data, video_data_size, timestamp);
					}
#else
					long long timestamp = 0;
					if (x == 0)
						timestamp = actual_delay * 1000;

					bool is_idr = media_source_reader::is_idr(_vsubmedia_type, video_data[4] & 0x1F);
					if (is_idr && !_video_recv_keyframe)
					{
						size_t sps_size = 0;
						size_t pps_size = 0;
						const uint8_t * sps = _media_buffer_manager.get_sps(sps_size);
						const uint8_t * pps = _media_buffer_manager.get_pps(pps_size);

						if (sps_size>0 && pps_size>0)
						{
							_video_recv_keyframe = true;
							_media_buffer_manager.push_video(sps, sps_size, 0);
							_media_buffer_manager.push_video(pps, pps_size, 0);
							_media_buffer_manager.push_video(video_data, video_data_size + sizeof(start_code), 0);
						}
					}
					else if (_video_recv_keyframe)
					{
						_media_buffer_manager.push_video(video_data, video_data_size + sizeof(start_code), timestamp);
					}
#endif
					index += video_data_size;
				} 
			}
		}
		else if (packet.stream_index == _audio_stream_index)
		{
			
		}
		av_free_packet(&packet);
		::Sleep(actual_delay);
	}
#endif
	_run = false;
}

bool media_source_reader::is_vps(media_source_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_source_reader::vsubmedia_type_hevc && nal_unit_type == 32;
}

bool media_source_reader::is_sps(media_source_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_source_reader::vsubmedia_type_h264 ? nal_unit_type == 7 : nal_unit_type == 33;
}

bool media_source_reader::is_pps(media_source_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_source_reader::vsubmedia_type_h264 ? nal_unit_type == 8 : nal_unit_type == 34;
}

bool media_source_reader::is_idr(media_source_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_source_reader::vsubmedia_type_h264 ? nal_unit_type == 5 : nal_unit_type == 34;
}

bool media_source_reader::is_vlc(media_source_reader::vsubmedia_type smt, uint8_t nal_unit_type)
{
	return smt == media_source_reader::vsubmedia_type_h264 ? (nal_unit_type <= 5 && nal_unit_type > 0) : (nal_unit_type <= 31);
}

const int media_source_reader::next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end)
{
	int i;
	*nal_start = 0;
	*nal_end = 0;

	i = 0;
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0 || bitstream[i + 3] != 0x01))
	{
		i++;
		if (i + 4 >= size)
			return 0;
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
		i++;

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
		return 0;/* error, should never happen */

	i += 3;
	*nal_start = i;
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01))
	{
		i++;
		if (i + 3 >= size)
		{
			*nal_end = size;
			return -1;
		}
	}

	*nal_end = i;
	return (*nal_end - *nal_start);
}

#endif