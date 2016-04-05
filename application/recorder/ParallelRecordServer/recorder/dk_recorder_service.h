#ifndef _DK_RECORDER_INFORMATION_MANAGER_H_
#define _DK_RECORDER_INFORMATION_MANAGER_H_

#include <windows.h>
#include <vector>

typedef void CURL;
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

	const char * retrieve_storage_path(bool file_separator=true);
	const char * retrieve_config_path(void);
	
private:
	dk_recorder_service(void);
	virtual ~dk_recorder_service(void);

	bool start_backup_service(void);
	bool stop_backup_service(void);
	static unsigned __stdcall backup_process_callback(void * param);
	void backup_process(void);

	void file_search_and_upload(const char * path);
	/* parse headers for size*/
	static size_t backup_get_content_length_callback2(void * ptr, size_t size, size_t nmemb, void * stream);
	/* parse headers for Content-Length */
	static size_t backup_get_content_length_callback(void * ptr, size_t size, size_t nmemb, void * stream);
	/* discard downloaded data */
	static size_t backup_discard_callback(void * ptr, size_t size, size_t nmemb, void * stream);
	/* read data to upload */
	static size_t backup_read_callback(void * ptr, size_t size, size_t nmemb, void * stream);

	bool backup_check_single_file(CURL * curl, const char * remotepath, const char * localpath);
	bool backup_upload_single_file(CURL * curl, const char * remotepath, const char * localpath, long timeout, long tries);

private:
	char _storage_path[260];
	char _config_path[260];
	std::vector<dk_recorder_service::recorder_receiver_information_t> _receivers;

	char _backup_url[260];
	unsigned int _backup_port_number;
	char _backup_username[260];
	char _backup_password[260];
	bool _backup_delete_after_backup;
	bool _backup_run;
	HANDLE _backup_thread;
};















#endif