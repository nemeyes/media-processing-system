#include "dk_recorder_service.h"
#include <dk_time_helper.h>
#include <dk_misc_helper.h>
#include "dk_rtsp_recorder.h"
#include <tinyxml.h>

#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <io.h>

debuggerking::recorder_service::_recorder_receiver_information_t::_recorder_receiver_information_t(void)
{
	memset(uuid, 0x00, sizeof(uuid));
	memset(url, 0x00, sizeof(url));
	memset(username, 0x00, sizeof(username));
	memset(password, 0x00, sizeof(password));
	recorder = nullptr;
}

debuggerking::recorder_service::_recorder_receiver_information_t::_recorder_receiver_information_t(const debuggerking::recorder_service::_recorder_receiver_information_t & clone)
{
	strncpy_s(uuid, clone.uuid, sizeof(uuid));
	strncpy_s(url, clone.url, sizeof(url));
	strncpy_s(username, clone.username, sizeof(username));
	strncpy_s(password, clone.password, sizeof(password));
	recorder = clone.recorder;
}

debuggerking::recorder_service::_recorder_receiver_information_t debuggerking::recorder_service::_recorder_receiver_information_t::operator=(const recorder_service::_recorder_receiver_information_t & clone)
{
	strncpy_s(uuid, clone.uuid, sizeof(uuid));
	strncpy_s(url, clone.url, sizeof(url));
	strncpy_s(username, clone.username, sizeof(username));
	strncpy_s(password, clone.password, sizeof(password));
	recorder = clone.recorder;
	return (*this);
}

debuggerking::recorder_service::recorder_service(void)
	: _backup_thread(INVALID_HANDLE_VALUE)
	, _backup_port_number(21)
	, _backup_enable(false)
	, _backup_delete_after_backup(false)
{
	memset(_storage_path, 0x00, sizeof(_storage_path));
	memset(_config_path, 0x00, sizeof(_config_path));
	memset(_backup_url, 0x00, sizeof(_backup_url));
	memset(_backup_username, 0x00, sizeof(_backup_username));
	memset(_backup_password, 0x00, sizeof(_backup_password));
	curl_global_init(CURL_GLOBAL_ALL);
}

debuggerking::recorder_service::~recorder_service(void)
{
	stop_recording();
	curl_global_cleanup();
}

debuggerking::recorder_service & debuggerking::recorder_service::instance(void)
{
	static debuggerking::recorder_service _instance;
	return _instance;
}

bool debuggerking::recorder_service::start_recording(void)
{
	memset(_backup_url, 0x00, sizeof(_backup_url));
	memset(_backup_username, 0x00, sizeof(_backup_username));
	memset(_backup_password, 0x00, sizeof(_backup_password));
	_backup_delete_after_backup = false;

	std::string config_path = retrieve_config_path();
	if (config_path.size() < 1)
		return false;
	if (::GetFileAttributesA(config_path.c_str()) == INVALID_FILE_ATTRIBUTES)
		return false;

	std::string config_filepath = config_path + "record_server.xml";
	if (::GetFileAttributesA(config_filepath.c_str()) == INVALID_FILE_ATTRIBUTES)
		return false;

	std::string storage_path = retrieve_storage_path();
	if (::GetFileAttributesA(storage_path.c_str()) == INVALID_FILE_ATTRIBUTES)
		CreateDirectoryA(storage_path.c_str(), NULL);

	TiXmlDocument document;
	document.LoadFile(config_filepath.c_str());

	TiXmlElement * root_elem = document.FirstChildElement("recorder");
	if (!root_elem)
		return false;

	const char * backup_url = nullptr;
	const char * str_backup_port_number = nullptr;
	const char * backup_username = nullptr;
	const char * backup_password = nullptr;
	TiXmlElement * backup_server_elem = root_elem->FirstChildElement("backup_server");
	if (!backup_server_elem)
		return false;

	const char * enable_backup = backup_server_elem->Attribute("enable");
	if (!enable_backup || strcmp(enable_backup, "true"))
		_backup_enable = false;
	else
		_backup_enable = true;

	if (_backup_enable)
	{
		const char * delete_after_backup = backup_server_elem->Attribute("delete_after_backup");
		if (!delete_after_backup || strcmp(delete_after_backup, "true"))
			_backup_delete_after_backup = false;
		else
			_backup_delete_after_backup = true;

		TiXmlElement * backup_url_elem = backup_server_elem->FirstChildElement("url");
		if (backup_url_elem)
			backup_url = backup_url_elem->GetText();

		TiXmlElement * backup_port_number_elem = backup_server_elem->FirstChildElement("port_number");
		if (backup_port_number_elem)
		{
			str_backup_port_number = backup_port_number_elem->GetText();
			if (str_backup_port_number && strlen(str_backup_port_number) > 0)
				_backup_port_number = atoi(str_backup_port_number);
		}

		TiXmlElement * backup_username_elem = backup_server_elem->FirstChildElement("username");
		if (backup_username_elem)
			backup_username = backup_username_elem->GetText();

		TiXmlElement * backup_password_elem = backup_server_elem->FirstChildElement("password");
		if (backup_password_elem)
			backup_password = backup_password_elem->GetText();

		if (backup_url && strlen(backup_url) > 0)
			strncpy_s(_backup_url, backup_url, sizeof(_backup_url));
		if (backup_username && strlen(backup_username) > 0)
			strncpy_s(_backup_username, backup_username, sizeof(_backup_username));
		if (backup_password && strlen(backup_password) > 0)
			strncpy_s(_backup_password, backup_password, sizeof(_backup_password));
	}

	TiXmlElement * media_sources_elem = root_elem->FirstChildElement("media_sources");
	if (!media_sources_elem)
		return false;

	const char * str_chunk_size = media_sources_elem->Attribute("chunk_size");
	int32_t chunk_size_in_mb = atoi(str_chunk_size);

	TiXmlElement * media_source_elem = media_sources_elem->FirstChildElement("media_source");
	while (media_source_elem)
	{
		TiXmlElement * url_elem = media_source_elem->FirstChildElement("url");
		TiXmlElement * username_elem = media_source_elem->FirstChildElement("username");
		TiXmlElement * password_elem = media_source_elem->FirstChildElement("password");

		const char * uuid = media_source_elem->Attribute("uuid");
		uuid = ::CharUpperA((char*)uuid);
		const char * str_recv_timeout = media_source_elem->Attribute("recv_timeout");
		int32_t recv_timeout = 0;
		if (str_recv_timeout && strlen(str_recv_timeout) > 0)
			recv_timeout = atoi(str_recv_timeout);

		const char * url = url_elem->GetText();
		const char * username = username_elem->GetText();
		const char * password = password_elem->GetText();


		recorder_service::recorder_receiver_information_t recv_info;
		strncpy_s(recv_info.uuid, uuid, sizeof(recv_info.uuid));
		strncpy_s(recv_info.url, url, sizeof(recv_info.url));
		if (username && strlen(username)>0)
			strncpy_s(recv_info.username, username, sizeof(recv_info.username));
		if (password && strlen(password)>0)
			strncpy_s(recv_info.password, password, sizeof(recv_info.password));
		recv_info.recorder = new rtsp_recorder(chunk_size_in_mb);
		recv_info.recorder->start_recording(recv_info.url, recv_info.username, recv_info.password, rtsp_recorder::rtp_over_tcp, rtsp_recorder::recv_option_t::video, recv_timeout, storage_path.c_str(), recv_info.uuid);
		_receivers.push_back(recv_info);

		media_source_elem = media_source_elem->NextSiblingElement();
	}

	if (_backup_enable)
		return start_backup_service();
	else
		return true;
}

bool debuggerking::recorder_service::stop_recording(void)
{
	if (_backup_enable)
		stop_backup_service();
	_backup_port_number = 21;
	_backup_enable = false;
	_backup_delete_after_backup = false;

	std::vector<recorder_service::recorder_receiver_information_t>::iterator iter;
	for (iter = _receivers.begin(); iter != _receivers.end(); iter++)
	{
		rtsp_recorder * recorder = (iter)->recorder;
		recorder->stop_recording();
		delete recorder;
		(iter)->recorder = nullptr;
	}
	_receivers.clear();
	return true;
}

const char * debuggerking::recorder_service::retrieve_storage_path(bool file_separator)
{
	char * module_path = nullptr;
	debuggerking::misc_helper::retrieve_absolute_module_path("ParallelRecordServer.exe", &module_path);
	if(module_path && strlen(module_path)>0)
	{
		if (file_separator)
			_snprintf_s(_storage_path, sizeof(_storage_path), "%s%s", module_path, "storage\\");
		else
			_snprintf_s(_storage_path, sizeof(_storage_path), "%s%s", module_path, "storage");
		free(module_path);
	}
	return _storage_path;
}

const char * debuggerking::recorder_service::retrieve_config_path(void)
{
	char * module_path = nullptr;
	debuggerking::misc_helper::retrieve_absolute_module_path("ParallelRecordServer.exe", &module_path);
	if (module_path && strlen(module_path)>0)
	{
		_snprintf_s(_config_path, sizeof(_config_path), "%s%s", module_path, "config\\");
		free(module_path);
	}
	return _config_path;
}

bool debuggerking::recorder_service::start_backup_service(void)
{
	unsigned int thrd_addr;
	_backup_thread = (HANDLE)::_beginthreadex(NULL, 0, recorder_service::backup_process_callback, this, 0, &thrd_addr);
	return true;
}

bool debuggerking::recorder_service::stop_backup_service(void)
{
	if (_backup_run)
	{
		_backup_run = false;
		::WaitForSingleObject(_backup_thread, INFINITE);
		::CloseHandle(_backup_thread);
		_backup_thread = INVALID_HANDLE_VALUE;
	}
	return true;
}

unsigned __stdcall debuggerking::recorder_service::backup_process_callback(void * param)
{
	debuggerking::recorder_service * self = static_cast<debuggerking::recorder_service*>(param);
	self->backup_process();
	return 0;
}

void debuggerking::recorder_service::backup_process(void)
{
	_backup_run = true;
	while (_backup_run)
	{
		if (!_backup_url || strlen(_backup_url) < 1)
			break;

		const char * storage_path = retrieve_storage_path(false);
		if (storage_path && strlen(storage_path)>0)
		{
			file_search_and_upload(storage_path);
		}
		::Sleep(1000);
	}
}


void debuggerking::recorder_service::file_search_and_upload(const char * path)
{
	WIN32_FIND_DATAA wfd;
	char search_path[260] = { 0 };

	_snprintf_s(search_path, sizeof(search_path), "%s\\*", path);
	HANDLE hFile = ::FindFirstFileA(search_path, &wfd);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			const char * file_name = &wfd.cFileName[0];

			// It could be a directory we are looking at
			// if so look into that dir
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (strcmp(file_name, ".") && strcmp(file_name, ".."))
				{
					//subDirs->Add(strPathToSearch + strTheNameOfTheFile);
					char sub_search_path[260] = { 0 };
					_snprintf_s(sub_search_path, sizeof(sub_search_path), "%s\\%s", path, file_name);
					file_search_and_upload(sub_search_path);
				}
			}
			else
			{
				char * slash = strrchr((char*)path, '\\');
				if (!slash)
					return;
				char * backup_folder_name = slash + 1;
				char recored_file_path[260] = { 0 };
				_snprintf_s(recored_file_path, sizeof(recored_file_path), "%s\\%s", path, file_name);


				HANDLE file2backup = ::CreateFileA(recored_file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file2backup != INVALID_HANDLE_VALUE)
				{
					long long stime;
					long long etime;

					void * buf = nullptr;
					unsigned long bytes_to_read = 0;
					unsigned long bytes_read = 0;

					buf = (void*)&stime;
					bytes_to_read = sizeof(long long);
					bytes_read = 0;
					::ReadFile(file2backup, buf, bytes_to_read, &bytes_read, NULL);

					buf = (void*)&etime;
					bytes_to_read = sizeof(long long);
					bytes_read = 0;
					::ReadFile(file2backup, buf, bytes_to_read, &bytes_read, NULL);

					::CloseHandle(file2backup);

					if (etime != 0 /*&& etime >= stime*/)
					{
						char backup_ftp_server[260] = { 0 };
						bool result = false;
						CURL * check_curl = NULL;
						check_curl = curl_easy_init();
						if (check_curl)
						{
				
							if (_backup_username && strlen(_backup_username) > 0 && _backup_password && strlen(_backup_password) > 0)
								_snprintf_s(backup_ftp_server, sizeof(backup_ftp_server), "ftp://%s:%s@%s:%d/%s/%s", _backup_username, _backup_password, _backup_url, _backup_port_number, backup_folder_name, file_name);
							else
								_snprintf_s(backup_ftp_server, sizeof(backup_ftp_server), "ftp://%s:%d/%s/%s", _backup_url, _backup_port_number, backup_folder_name, file_name);

							//_snprintf_s(backup_ftp_server, sizeof(backup_ftp_server), "ftp://%s:%d/%s/%s", _backup_url, _backup_port_number, backup_folder_name, file_name);
							result = backup_check_single_file(check_curl, backup_ftp_server, 0, 0, recored_file_path);
							if (result)
							{
								CURL * backup_curl = NULL;
								backup_curl = curl_easy_init();
								if (backup_curl)
								{
									result = backup_upload_single_file(backup_curl, backup_ftp_server, 0, 0, recored_file_path, 0, 3);
									if (result && _backup_delete_after_backup)
										::DeleteFileA(recored_file_path);
									curl_easy_cleanup(backup_curl);
								}
							}
							curl_easy_cleanup(check_curl);
						}
					}
				}
			}
		} while (FindNextFileA(hFile, &wfd));

		FindClose(hFile);
	}
}

size_t debuggerking::recorder_service::backup_get_content_length_callback2(void * ptr, size_t size, size_t nmemb, void * stream)
{
	(void)ptr;
	(void)stream;
	/* we are not interested in the headers itself,
	so we only return the size we would have saved ... */
	return (size_t)(size * nmemb);
}

/* parse headers for Content-Length */
size_t debuggerking::recorder_service::backup_get_content_length_callback(void * ptr, size_t size, size_t nmemb, void * stream)
{
	int r;
	long len = 0;

	/* _snscanf() is Win32 specific */
	r = _snscanf((char*)ptr, size * nmemb, "Content-Length: %ld\n", &len);

	if (r) /* Microsoft: we don't read the specs */
		*((long *)stream) = len;

	return size * nmemb;
}

/* discard downloaded data */
size_t debuggerking::recorder_service::backup_discard_callback(void * ptr, size_t size, size_t nmemb, void * stream)
{
	return size * nmemb;
}

/* read data to upload */
size_t debuggerking::recorder_service::backup_read_callback(void * ptr, size_t size, size_t nmemb, void * stream)
{
	FILE * f = (FILE*)stream;
	size_t n;

	if (ferror(f))
		return CURL_READFUNC_ABORT;

	n = fread(ptr, size, nmemb, f) * size;

	return n;
}

bool debuggerking::recorder_service::backup_check_single_file(CURL * curl, const char * remotepath, const char * username, const char * password, const char * localpath)
{
	CURLcode r = CURLE_GOT_NOTHING;

	if (!curl)
		return false;

	curl_off_t local_file_size;
	curl_off_t remote_file_size;

	/* get the file size of the local file */
	struct stat file_info;
	if (stat(localpath, &file_info))
		return false;
	local_file_size = (curl_off_t)file_info.st_size;

	curl_easy_setopt(curl, CURLOPT_URL, remotepath);
#if 1
	if (username && strlen(username)>0)
		curl_easy_setopt(curl, CURLOPT_USERNAME, username);
	if (password && strlen(password)>0)
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
#else
	if ((username && strlen(username)>0) && (password && strlen(password)>0))
	{
		char username_password[260] = { 0 };
		_snprintf_s(username_password, sizeof(username_password), "%s:%s", username, password);
		curl_easy_setopt(curl, CURLOPT_USERPWD, "username:password");
	}
#endif

	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
	curl_easy_setopt(curl, CURLOPT_FILETIME, 1);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &recorder_service::backup_get_content_length_callback2);
	curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

	r = curl_easy_perform(curl);
	if (r == CURLE_OK)
	{
		double file_size = 0.0;
		r = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &file_size);
		if ((r == CURLE_OK) && (file_size > 0.0))
		{
			remote_file_size = file_size;
			if (remote_file_size == local_file_size)
				return false;
			else
				return true;
		}
		else
			return true;
	}
	else
	{
		return true;
	}
}

/* read data to upload */
bool debuggerking::recorder_service::backup_upload_single_file(CURL * curl, const char * remotepath, const char * username, const char * password, const char * localpath, long timeout, long tries)
{
	FILE *f;
	long uploaded_len = 0;
	CURLcode r = CURLE_GOT_NOTHING;
	int c;

	if (!curl)
		return false;

	/* get the file size of the local file */
	struct stat file_info;
	curl_off_t fsize;
	if (stat(localpath, &file_info)) 
		return false;
	fsize = (curl_off_t)file_info.st_size;

	f = fopen(localpath, "rb");
	if (!f)
		return false;

	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, remotepath);
#if 1
	if (username && strlen(username)>0)
		curl_easy_setopt(curl, CURLOPT_USERNAME, username);
	if (password && strlen(password)>0)
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
#else
	if ((username && strlen(username)>0) && (password && strlen(password)>0))
	{
		char username_password[260] = { 0 };
		_snprintf_s(username_password, sizeof(username_password), "%s:%s", username, password);
		curl_easy_setopt(curl, CURLOPT_USERPWD, "username:password");
	}
#endif

	if (timeout)
		curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, timeout);

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &recorder_service::backup_get_content_length_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &uploaded_len);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &recorder_service::backup_discard_callback);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, &recorder_service::backup_read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, f);
	/* disable passive mode */
	curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	//r = curl_easy_perform(curl);
	for (c = 0; (r != CURLE_OK) && (c < tries); c++) 
	{
		// are we resuming?
		if (c) 
		{	// yes
			// determine the length of the file already written

			
			//With NOBODY and NOHEADER, libcurl will issue a SIZE
			//command, but the only way to retrieve the result is
			//to parse the returned Content-Length header. Thus,
			//getcontentlengthfunc(). We need discardfunc() above
			//because HEADER will dump the headers to stdout
			//without it.
			
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
			curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

			r = curl_easy_perform(curl);
			if (r != CURLE_OK)
				continue;

			curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
			curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

			fseek(f, uploaded_len, SEEK_SET);

			curl_easy_setopt(curl, CURLOPT_APPEND, 1L);
		}
		else 
		{
			curl_easy_setopt(curl, CURLOPT_APPEND, 0L);
		}
		
		r = curl_easy_perform(curl);
	}

	fclose(f);
	if (r == CURLE_OK)
		return true;
	else
		return false;
}