#include "ff_demuxer.h"
#include "stream_parser.h"
#include <process.h>

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
}

ff_demuxer::~ff_demuxer(void)
{

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

	_audio_stream = _format_ctx->streams[_video_stream_index];
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

	_run = true;
	for (; _run && av_read_frame(_format_ctx, &packet) >= 0;)
	{
		//is this a packet from the video stream?
		uint8_t * data = packet.data;
		size_t data_size = packet.size;
		if (packet.stream_index == _video_stream_index)
		{
			if (_front)
			{
				size_t saved_sps_size = 0;
				unsigned char * saved_sps = _front->get_sps(saved_sps_size);
				size_t saved_pps_size = 0;
				unsigned char * saved_pps = _front->get_pps(saved_pps_size);

				bool is_sps = stream_parser::is_sps(_vsubmedia_type, data[0] & 0x1F);
				if (is_sps)
				{
					if (saved_sps_size < 1 || !saved_sps)
					{
						_front->set_sps(data, data_size);
						_change_sps = true;
					}
					else
					{
						if (memcmp(saved_sps, data, saved_sps_size))
						{
							_front->set_sps(data, data_size);
							_change_sps = true;
						}
					}
				}

				bool is_pps = stream_parser::is_pps(_vsubmedia_type, data[0] & 0x1F);
				if (is_pps)
				{
					if (saved_pps_size < 1 || !saved_pps)
					{
						_front->set_pps(data, data_size);
						_change_pps = true;
					}
					else
					{
						if (memcmp(saved_pps, data, saved_pps_size))
						{
							_front->set_pps(data, data_size);
							_change_pps = true;
						}
					}
				}

				bool is_idr = stream_parser::is_idr(_vsubmedia_type, data[0] & 0x1F);

				if (_change_sps || _change_pps)
				{
					saved_sps = _front->get_sps(saved_sps_size);
					saved_pps = _front->get_pps(saved_pps_size);
					if ((saved_sps_size > 0) && (saved_pps_size > 0))
					{
						memcpy(_video_extradata, saved_sps, saved_sps_size);
						memcpy(_video_extradata + saved_sps_size, saved_pps, saved_pps_size);
						_video_extradata_size = saved_sps_size + saved_pps_size;

						if (is_idr && !_is_first_idr_rcvd)
						{
							memmove(data + _video_extradata_size, data, data_size);
							memmove(data, _video_extradata, _video_extradata_size);
							_is_first_idr_rcvd = true;

							_front->on_begin_video(_vsubmedia_type, saved_sps, saved_sps_size, saved_pps, saved_pps_size, data, data_size + _video_extradata_size, 0);
							_change_sps = false;
							_change_pps = false;
						}
					}
				}

				//if (_sps_not_changed && _pps_not_changed && is_idr && !_is_first_idr_rcvd)
				//	_is_first_idr_rcvd = true;

				if (!is_sps && !is_pps && _is_first_idr_rcvd)
				{
					saved_sps = _front->get_sps(saved_sps_size);
					saved_pps = _front->get_pps(saved_pps_size);
					if (saved_sps_size > 0 && saved_pps_size > 0)
					{
						_front->on_recv_video(_vsubmedia_type, data, data_size, 0);
					}
				}
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