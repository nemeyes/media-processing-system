#include "ff_demuxer.h"
#include <process.h>
#include "stream_parser.h"
#include "scoped_lock.h"


#define VIDEO_BUFFER_SIZE 1920 * 1080 * 2
#define AUDIO_BUFFER_SIZE 48000 * 2 * 8 //48000hz * 16bitdetph * 8 channels ex) for 2channel 192000

#define MAX_AUDIO_FRAME_SIZE 192000

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

ff_demuxer::ff_demuxer(dk_file_demuxer * front)
	: _front(front)
	, _format_ctx(nullptr)
	, _video_ctx(nullptr)
	, _video_stream_index(-1)
	, _video_stream(nullptr)
	, _video_timer(0)
	, _video_last_dts(0)
	, _video_last_delay(0)
	, _video_clock(0)
	, _video_recv_keyframe(false)
	, _video_extradata_size(0)
	, _audio_ctx(nullptr)
	, _audio_stream_index(-1)
	, _audio_stream(nullptr)
	, _audio_timer(0)
	, _audio_last_dts(0)
	, _audio_last_delay(0)
	, _audio_clock(0)
	, _vsubmedia_type(dk_file_demuxer::vsubmedia_type::unknown_video_type)
	, _asubmedia_type(dk_file_demuxer::asubmedia_type::unknown_audio_type)
	, _run(false)
	, _run_video(false)
	, _run_audio(false)
	, _thread(INVALID_HANDLE_VALUE)
	, _thread_video(INVALID_HANDLE_VALUE)
	, _thread_audio(INVALID_HANDLE_VALUE)
{
	_video_buffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));
	_audio_buffer = static_cast<uint8_t*>(malloc((MAX_AUDIO_FRAME_SIZE * 3) / 2)); //48000hz * 16bitdetph * 8 channels
}

ff_demuxer::~ff_demuxer(void)
{
	stop();
	if (_video_buffer)
		free(_video_buffer);
	if (_audio_buffer)
		free(_audio_buffer);
}

dk_file_demuxer::err_code ff_demuxer::play(const char * filepath)
{
	_format_ctx = nullptr;
	
	_video_ctx = nullptr;
	_video_stream_index = -1;
	_video_stream = nullptr;
	_video_timer = (double)av_gettime() / 1000000.0;
	_video_last_dts = 0;
	_video_last_delay = 40e-3;
	_video_current_dts = 0;
	_video_current_dts_time = av_gettime();
	_video_clock = 0;
	_video_recv_keyframe = false;
	memset(_video_extradata, 0x00, sizeof(_video_extradata));
	_video_extradata_size = 0;
	packet_queue_init(&_video_packet_queue);

	_audio_ctx = nullptr;
	_audio_stream_index = -1;
	_audio_stream = nullptr;
	_audio_timer = (double)av_gettime() / 1000000.0;
	_audio_last_dts = 0;
	_audio_last_delay = 40e-3;
	_audio_current_dts = 0;
	_audio_current_dts_time = av_gettime();
	_audio_clock = 0;
	_audio_recv_sample = false;
	memset(_audio_extradata, 0x00, sizeof(_audio_extradata));
	_audio_extradata_size = 0;
	packet_queue_init(&_audio_packet_queue);


	//open video file
	if (avformat_open_input(&_format_ctx, filepath, NULL, NULL) != 0)
		return dk_file_demuxer::ERR_CODE_FAIL; //couldn't open file

	//retrieve stream information
	if (avformat_find_stream_info(_format_ctx, NULL) < 0)
		return dk_file_demuxer::ERR_CODE_FAIL; //couldn't find stream information

	_video_stream_index = -1;
	_audio_stream_index = -1;
	for (int32_t index = 0; index<_format_ctx->nb_streams; index++)
	{
		if (_format_ctx->streams[index]->codec->codec_type == AVMEDIA_TYPE_VIDEO && _video_stream_index < 0)
			_video_stream_index = index;
		if (_format_ctx->streams[index]->codec->codec_type == AVMEDIA_TYPE_AUDIO && _audio_stream_index < 0)
			_audio_stream_index = index;
	}

	if (_video_stream_index == -1)
		return dk_file_demuxer::ERR_CODE_FAIL;
	if (_audio_stream_index == -1)
		return dk_file_demuxer::ERR_CODE_FAIL;

	_video_stream = _format_ctx->streams[_video_stream_index];
	_video_ctx = _video_stream->codec;
	switch (_video_ctx->codec_id)
	{
	case AV_CODEC_ID_MJPEG :
		_vsubmedia_type = dk_file_demuxer::vsubmedia_type_jpeg;
		break;
	case AV_CODEC_ID_MPEG4 :
		_vsubmedia_type = dk_file_demuxer::vsubmedia_type_mpeg4;
		break;
	case AV_CODEC_ID_H264 :
		_vsubmedia_type = dk_file_demuxer::vsubmedia_type_h264;
		break;
	default:
		_vsubmedia_type = dk_file_demuxer::unknown_video_type;
		break;
	}

	_audio_stream = _format_ctx->streams[_audio_stream_index];
	_audio_ctx = _audio_stream->codec;
	switch (_audio_ctx->codec_id)
	{
	case AV_CODEC_ID_MP3:
		_asubmedia_type = dk_file_demuxer::asubmedia_type_mp3;
		break;
	case AV_CODEC_ID_AAC:
		_asubmedia_type = dk_file_demuxer::asubmedia_type_aac;
		break;
	default :
		_asubmedia_type = dk_file_demuxer::unknown_audio_type;
		break;
	}

	unsigned thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, ff_demuxer::process_cb, this, 0, &thrdaddr);
	return dk_file_demuxer::ERR_CODE_SUCCESS;
}

dk_file_demuxer::err_code ff_demuxer::stop(void)
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
	return dk_file_demuxer::ERR_CODE_SUCCESS;
}

unsigned ff_demuxer::process_cb(void * param)
{
	ff_demuxer * self = static_cast<ff_demuxer*>(param);
	self->process();
	return 0;
}

unsigned ff_demuxer::process_video_cb(void * param)
{
	ff_demuxer * self = static_cast<ff_demuxer*>(param);
	self->process_video();
	return 0;
}

unsigned ff_demuxer::process_audio_cb(void * param)
{
	ff_demuxer * self = static_cast<ff_demuxer*>(param);
	self->process_audio();
	return 0;
}

void ff_demuxer::process(void)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;
	_run = true;

	unsigned thrdaddr = 0;
	if (_vsubmedia_type!=dk_file_demuxer::unknown_video_type)
		_thread_video = (HANDLE)::_beginthreadex(NULL, 0, ff_demuxer::process_video_cb, this, 0, &thrdaddr);
	if (_asubmedia_type != dk_file_demuxer::unknown_audio_type)
		_thread_audio = (HANDLE)::_beginthreadex(NULL, 0, ff_demuxer::process_audio_cb, this, 0, &thrdaddr);

	for (; _run && av_read_frame(_format_ctx, &packet) >= 0;)
	{
		if (packet.stream_index == _video_stream_index)
		{
			packet_queue_push(&_video_packet_queue, &packet);
		}
		else if (packet.stream_index == _audio_stream_index)
		{
			packet_queue_push(&_audio_packet_queue, &packet);
		}
		av_free_packet(&packet);
		::Sleep(10);
	}


	if (_vsubmedia_type != dk_file_demuxer::unknown_video_type)
	{
		_run_video = false;
		if (_thread_video != INVALID_HANDLE_VALUE)
		{
			::WaitForSingleObject(_thread_video, INFINITE);
			::CloseHandle(_thread_video);
			_thread_video = INVALID_HANDLE_VALUE;
		}
	}
	
	if (_asubmedia_type != dk_file_demuxer::unknown_audio_type)
	{
		_run_audio = false;
		if (_thread_audio != INVALID_HANDLE_VALUE)
		{
			::WaitForSingleObject(_thread_audio, INFINITE);
			::CloseHandle(_thread_audio);
			_thread_audio = INVALID_HANDLE_VALUE;
		}
	}
}

void ff_demuxer::process_video(void)
{
	AVPacket packet;
	double dts = 0.f;
	double delay = 0.f;
	double actual_delay = 0.f;
	_run_video = true;
	while (_run_video)
	{
		int32_t code = packet_queue_pop(&_video_packet_queue, &packet);
		if (code < 0)
			break;
		if (code == 0)
		{
			::Sleep(1);
			continue;
		}

		if (packet.dts != AV_NOPTS_VALUE)
			dts = packet.dts;
		dts *= av_q2d(_video_stream->time_base);
		if (dts != 0)
			_video_clock = dts;
		else
			dts = _video_clock;

		delay = av_q2d(_video_ctx->time_base);
		/* if we are repeating a frame, adjust clock accordingly */
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
			/* Really it should skip the picture instead */
			actual_delay = 0.010;
		}
		actual_delay = actual_delay * 1000 + 0.5;

		if (_vsubmedia_type == dk_file_demuxer::vsubmedia_type_h264)
		{
			uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
			uint8_t * data = _video_buffer;
			int32_t data_size = 0;
			int32_t index = 0;
			int32_t packet_size = packet.size;

			do
			{
				if (packet_size < (index + 4))
					break;
				data_size = (packet.data[index] << 24) + (packet.data[index + 1] << 16) + (packet.data[index + 2] << 8) + packet.data[index + 3];
				memmove(data, start_code, sizeof(start_code));
				index += sizeof(start_code);

				if (packet_size < (index + data_size) || (UINT32_MAX - index) < data_size)
					break;

				memcpy(data + index, &packet.data[index], data_size);

				bool is_idr = stream_parser::is_idr(_vsubmedia_type, data[4] & 0x1F);
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
						_front->set_sps(_video_extradata, sizeof(start_code) + sps_size);
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
						_front->set_pps(_video_extradata + sizeof(start_code) + sps_size, sizeof(start_code) + pps_size);
						_video_extradata_size = _video_extradata_size + sizeof(start_code) + pps_size;
					}

					uint8_t * saved_sps = nullptr;
					uint8_t * saved_pps = nullptr;
					size_t saved_sps_size = 0;
					size_t saved_pps_size = 0;
					saved_sps = _front->get_sps(saved_sps_size);
					saved_pps = _front->get_pps(saved_pps_size);

					memmove(data, _video_extradata, _video_extradata_size);
					memmove(data + _video_extradata_size, data, data_size);
					_video_recv_keyframe = true;
					_front->on_begin_video(_vsubmedia_type, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, sizeof(start_code) + data_size + _video_extradata_size, 0);
				}

				if (_video_recv_keyframe)
					_front->on_recv_video(_vsubmedia_type, data, sizeof(start_code) + data_size, 0);

				index += data_size;
			} while (index < packet_size);
		}
		::Sleep(actual_delay);
		av_free_packet(&packet);
	}
}

void ff_demuxer::process_audio(void)
{
	AVPacket packet;
	double dts = 0.f;
	double delay = 0.f;
	double actual_delay = 0.f;	_run_audio = true;
	while (_run_audio)
	{
		int32_t code = packet_queue_pop(&_audio_packet_queue, &packet);
		if (code < 0)
			break;
		if (code == 0)
		{
			::Sleep(1);
			continue;
		}

		if (packet.dts != AV_NOPTS_VALUE)
			dts = packet.dts;
		dts *= av_q2d(_audio_stream->time_base);
		if (dts != 0)
			_audio_clock = dts;
		else
			dts = _audio_clock;

		delay = av_q2d(_audio_ctx->time_base);
		/* if we are repeating a frame, adjust clock accordingly */
		//delay += src_frame->repeat_pict * (delay * 0.5);
		_audio_clock += delay;

		_audio_current_dts = dts;
		_audio_current_dts_time = av_gettime();
		delay = _audio_current_dts - _audio_last_dts;
		if (delay <= 0 || delay >= 1.0)
		{
			delay = _audio_last_delay;
		}
		_audio_last_delay = delay;
		_audio_last_dts = _audio_current_dts;

		_audio_timer += delay;
		actual_delay = _audio_timer - (av_gettime() / 1000000.0);
		if (actual_delay < 0.010)
		{
			/* Really it should skip the picture instead */
			actual_delay = 0.010;
		}
		actual_delay = actual_delay * 900;

		switch (_asubmedia_type)
		{
			case dk_file_demuxer::asubmedia_type_aac :
			{
				if (!_audio_recv_sample)
				{
					int32_t samplerate = _audio_ctx->sample_rate;
					int32_t channels = _audio_ctx->channels;
					int32_t bitdepth = 16;// _audio_ctx->bits_per_raw_sample;
					_audio_extradata_size = _audio_ctx->extradata_size;
					memmove(_audio_extradata, _audio_ctx->extradata, _audio_extradata_size);
					if (_front)
						_front->on_begin_audio(_asubmedia_type, _audio_extradata, _audio_extradata_size, samplerate, bitdepth, channels, packet.data, packet.size, 0);
					_audio_recv_sample = true;
				}
				else
				{
					_front->on_recv_audio(_asubmedia_type, packet.data, packet.size, 0);
				}
				break;
			}
			case dk_file_demuxer::asubmedia_type_mp3 :
			{

				break;
			}
		}


		Sleep(actual_delay);
		av_free_packet(&packet);
	}
}

void ff_demuxer::packet_queue_init(PACKET_QUEUE_T * q)
{
	memset(q, 0, sizeof(PACKET_QUEUE_T));
	q->lock = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	::SetEvent(q->lock);
}

int32_t ff_demuxer::packet_queue_push(PACKET_QUEUE_T * q, AVPacket * pkt)
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

int32_t ff_demuxer::packet_queue_pop(PACKET_QUEUE_T * q, AVPacket * pkt)
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

double ff_demuxer::get_audio_clock(void)
{
	double pts;
	int32_t hw_buf_size, bytes_per_sec, n;

	pts = _audio_clock; /* maintained in the audio thread */
	hw_buf_size = _audio_buffer_size - _audio_buffer_index;
	bytes_per_sec = 0;
	n = _audio_ctx->channels * 2;
	if (_audio_stream) 
	{
		bytes_per_sec = _audio_ctx->sample_rate * n;
	}
	if (bytes_per_sec) 
	{
		pts -= (double)hw_buf_size / bytes_per_sec;
	}
	return pts;
}
double ff_demuxer::get_video_clock(void)
{
	double delta;
	delta = (av_gettime() - _video_current_dts_time) / 1000000.0;
	return _video_current_dts + delta;
}

double ff_demuxer::get_master_clock(void)
{
	return get_audio_clock();
}