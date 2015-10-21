#include "stdafx.h"
#include <dk_ff_mpeg2ts_muxer.h>
#include "dk_rtsp_receiver.h"


dk_rtsp_receiver::dk_rtsp_receiver(void)
	: _is_preview_enabled(false)
	, _is_recording_enabled(false)
{

}

dk_rtsp_receiver::~dk_rtsp_receiver(void)
{

}

void dk_rtsp_receiver::start_preview(const char * url, const char * username, const char * password, int transport_option, int recv_option, HANDLE handle)
{


	_is_preview_enabled = true;
}

void dk_rtsp_receiver::stop_preview(void)
{


	_is_preview_enabled = false;
}

void dk_rtsp_receiver::start_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option)
{
	_mpeg2ts_muxer = new dk_ff_mpeg2ts_muxer();
	_is_recording_enabled = true;
	dk_rtsp_client::play(url, username, password, transport_option, recv_option, true);
}

void dk_rtsp_receiver::stop_recording(void)
{
	dk_rtsp_client::stop();
	_is_recording_enabled = false;
	if (_mpeg2ts_muxer)
	{
		delete _mpeg2ts_muxer;
		_mpeg2ts_muxer = nullptr;
	}
}

void dk_rtsp_receiver::on_begin_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const unsigned char * data, unsigned data_size, struct timeval presentation_time)
{
	if (_is_recording_enabled)
	{
		dk_ff_mpeg2ts_muxer::configuration_t config;
		config.extra_data_size = data_size;
		memcpy(config.extra_data, data, data_size);
		config.width = 1280;
		config.height = 720;
		config.fps = 30;
		config.stream_index = 0;
		config.bitrate = 4000000;
		_mpeg2ts_muxer->initialize(config);
	}
	TRACE(_T("on_begin_media : received video data size is %d\n"), data_size);
}

void dk_rtsp_receiver::on_recv_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const unsigned char * data, unsigned data_size, struct timeval presentation_time)
{
	if (mt == dk_rtsp_client::MEDIA_TYPE_VIDEO)
	{
		if (_is_recording_enabled)
		{
			if ((data[3] & 0x1F)==0x05)
				_mpeg2ts_muxer->put_video_stream((unsigned char*)data, data_size, 0, true);
			else
				_mpeg2ts_muxer->put_video_stream((unsigned char*)data, data_size, 0, false);
		}
		TRACE(_T("on_recv_media : received video data size is %d\n"), data_size);
	}
	else if (mt == dk_rtsp_client::MEDIA_TYPE_AUDIO)
	{
		TRACE(_T("on_recv_media : received audio data size is %d\n"), data_size);
	}
}
