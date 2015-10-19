#include "stdafx.h"
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

void dk_rtsp_receiver::stop_preview(const char * url, const char * username, const char * password, int transport_option, int recv_option, HANDLE handle)
{


	_is_preview_enabled = false;
}

void dk_rtsp_receiver::start_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option)
{


	_is_recording_enabled = true;
}

void dk_rtsp_receiver::stop_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option)
{


	_is_recording_enabled = false;
}

void dk_rtsp_receiver::on_begin_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const unsigned char * data, unsigned data_size, struct timeval presentation_time)
{
	TRACE(_T("on_begin_media : received video data size is %d\n"), data_size);
}

void dk_rtsp_receiver::on_recv_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const unsigned char * data, unsigned data_size, struct timeval presentation_time)
{
	if (mt == dk_rtsp_client::MEDIA_TYPE_VIDEO)
	{
		TRACE(_T("on_recv_media : received video data size is %d\n"), data_size);
	}
	else if (mt == dk_rtsp_client::MEDIA_TYPE_AUDIO)
	{
		TRACE(_T("on_recv_media : received audio data size is %d\n"), data_size);
	}
}
