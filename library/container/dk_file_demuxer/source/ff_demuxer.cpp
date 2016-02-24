#include "ff_demuxer.h"
#include "stream_parser.h"
#include <process.h>

#define VIDEO_BUFFER_SIZE 1920 * 1080 * 4
#define AUDIO_BUFFER_SIZE 48000 * 2 * 8 //48000hz * 16bitdetph * 8 channels ex) for 2channel 192000

ff_demuxer::ff_demuxer(dk_file_demuxer * front)
	: _front(front)
	, _format_ctx(nullptr)
	, _video_ctx(nullptr)
	, _video_stream_index(-1)
	, _video_stream(nullptr)
	, _video_pts(0)
	, _video_clock(0)
	, _change_sps(false)
	, _change_pps(false)
	, _is_first_idr_rcvd(false)
	, _video_extradata_size(0)
	, _audio_stream_index(-1)
	, _vsubmedia_type(dk_file_demuxer::VIDEO_SUBMEDIA_TYPE_T::UNKNOWN_VIDEO_TYPE)
	, _asubmedia_type(dk_file_demuxer::AUDIO_SUBMEDIA_TYPE_T::UNKNOWN_AUDIO_TYPE)
	, _run(false)
	, _thread(INVALID_HANDLE_VALUE)
{
	memset(_video_extradata, 0x00, sizeof(_video_extradata_size));
	_video_buffer = static_cast<uint8_t*>(malloc(VIDEO_BUFFER_SIZE));
	_audio_buffer = static_cast<uint8_t*>(malloc(AUDIO_BUFFER_SIZE)); //48000hz * 16bitdetph * 8 channels
}

ff_demuxer::~ff_demuxer(void)
{
	if (_video_buffer)
		free(_video_buffer);
	if (_audio_buffer)
		free(_audio_buffer);
}

dk_file_demuxer::ERR_CODE ff_demuxer::play(const char * filepath)
{
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
		_vsubmedia_type = dk_file_demuxer::SUBMEDIA_TYPE_JPEG;
		break;
	case AV_CODEC_ID_MPEG4 :
		_vsubmedia_type = dk_file_demuxer::SUBMEDIA_TYPE_MPEG4;
		break;
	case AV_CODEC_ID_H264 :
		_vsubmedia_type = dk_file_demuxer::SUBMEDIA_TYPE_H264;
		break;
	}

	_audio_stream = _format_ctx->streams[_audio_stream_index];
	AVCodecContext * acodec_ctx = _audio_stream->codec;
	switch (acodec_ctx->codec_id)
	{
	case AV_CODEC_ID_MP3:
		_asubmedia_type = dk_file_demuxer::SUBMEDIA_TYPE_MP3;
		break;
	case AV_CODEC_ID_AAC:
		_asubmedia_type = dk_file_demuxer::SUBMEDIA_TYPE_AAC;
		break;
	}

	unsigned thrdaddr = 0;
	_thread = (HANDLE)::_beginthreadex(NULL, 0, ff_demuxer::process_cb, this, 0, &thrdaddr);
	return dk_file_demuxer::ERR_CODE_SUCCESS;
}

dk_file_demuxer::ERR_CODE ff_demuxer::stop(void)
{
	_run = false;
	if (_thread != INVALID_HANDLE_VALUE)
	{
		::WaitForSingleObject(_thread, INFINITE);
		::CloseHandle(_thread);
		_thread = INVALID_HANDLE_VALUE;
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

void ff_demuxer::process(void)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;

	uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };

	_run = true;
	for (; _run && av_read_frame(_format_ctx, &packet) >= 0;)
	{
		if (packet.stream_index == _video_stream_index)
		{
			if (_vsubmedia_type == dk_file_demuxer::SUBMEDIA_TYPE_H264)
			{
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
					if (is_idr && !_is_first_idr_rcvd)
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
						_is_first_idr_rcvd = true;
						_front->on_begin_video(_vsubmedia_type, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size + _video_extradata_size, 0);
					}
					
					if (_is_first_idr_rcvd)
					{
						_front->on_recv_video(_vsubmedia_type, data, data_size, 0);
					}

					index += data_size;
				} while (index < packet_size);
			}
			
			/*
			double pts = 0;
			if (packet.pts != AV_NOPTS_VALUE)
				pts = packet.pts;
			else
				pts = 0;
			pts *= av_q2d(_video_stream->time_base);
			if (pts != 0)
				_video_clock = pts;
			else
				pts = _video_clock;

			double delay;
			delay = av_q2d(_video_ctx->time_base);

			//if we are repeating a frame, adjust clock accordingly
			//delay += src_frame->repeat_pict * (frame_delay * 0.5);
			_video_clock += delay;
			*/
		}

		if (packet.stream_index == _audio_stream_index)
		{

		}

		av_free_packet(&packet);
	}
}