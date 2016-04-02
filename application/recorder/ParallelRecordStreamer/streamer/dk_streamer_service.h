#ifndef _DK_STREAMER_SERVICE_H_
#define _DK_STREAMER_SERVICE_H_


class dk_vod_rtsp_server;
class dk_streamer_service
{
public:
	static dk_streamer_service & instance(void);

	bool start_streaming(void);
	bool stop_streaming(void);

	//const char * retrieve_storage_path(void);
	const char * retrieve_config_path(void);

private:
	dk_streamer_service(void);
	virtual ~dk_streamer_service(void);

	char _config_path[260];

	dk_vod_rtsp_server * _rtsp_server;
};


#endif