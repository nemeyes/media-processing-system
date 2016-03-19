#ifndef _DK_RECORDER_INFORMATION_MANAGER_H_
#define _DK_RECORDER_INFORMATION_MANAGER_H_

#include <vector>

class dk_rtsp_recorder;
class dk_recorder_service
{
public:
	typedef struct _recorder_receiver_information_t
	{
		char uuid[64];
		char url[260];
		char username[260];
		char password[260];
		dk_rtsp_recorder * recorder;
		_recorder_receiver_information_t(void);
		_recorder_receiver_information_t(const _recorder_receiver_information_t & clone);
		_recorder_receiver_information_t operator=(const _recorder_receiver_information_t & clone);
	} recorder_receiver_information_t;


	static dk_recorder_service & instance(void);

	bool start_recording(void);
	bool stop_recording(void);

	const char * retrieve_storage_path(void);
	const char * retrieve_config_path(void);
	//void retrieve_receivers(std::vector<dk_recorder_service::recorder_receiver_information_t> * receivers);

private:
	dk_recorder_service(void);
	virtual ~dk_recorder_service(void);

	char _storage_path[260];
	char _config_path[260];
	std::vector<dk_recorder_service::recorder_receiver_information_t> _receivers;
};















#endif