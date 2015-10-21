#pragma once
#include <windows.h>
#include <dk_rtsp_client.h>

class dk_ff_mpeg2ts_muxer;
class dk_rtsp_receiver : public dk_rtsp_client
{
public:
	dk_rtsp_receiver(void);
	~dk_rtsp_receiver(void);

	void start_preview(const char * url, const char * username, const char * password, int transport_option, int recv_option, HANDLE handle);
	void stop_preview(void);

	void start_recording(const char * url, const char * username, const char * password, int transport_option, int recv_option);
	void stop_recording(void);

	void on_begin_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const unsigned char * data, unsigned data_size, struct timeval presentation_time);
	void on_recv_media(dk_rtsp_client::MEDIA_TYPE_T mt, dk_rtsp_client::SUBMEDIA_TYPE_T smt, const unsigned char * data, unsigned data_size, struct timeval presentation_time);


private:
	bool _is_preview_enabled;
	bool _is_recording_enabled;

	dk_ff_mpeg2ts_muxer * _mpeg2ts_muxer;

};

