#include "dk_recorder_service.h"
#include <dk_time_helper.h>
#include <dk_misc_helper.h>
#include "dk_rtsp_recorder.h"
#include <tinyxml.h>

#include <curl/curl.h>

dk_recorder_service::_recorder_receiver_information_t::_recorder_receiver_information_t(void)
{
	memset(uuid, 0x00, sizeof(uuid));
	memset(url, 0x00, sizeof(url));
	memset(username, 0x00, sizeof(username));
	memset(password, 0x00, sizeof(password));
	recorder = nullptr;
}

dk_recorder_service::_recorder_receiver_information_t::_recorder_receiver_information_t(const dk_recorder_service::_recorder_receiver_information_t & clone)
{
	strncpy_s(uuid, clone.uuid, sizeof(uuid));
	strncpy_s(url, clone.url, sizeof(url));
	strncpy_s(username, clone.username, sizeof(username));
	strncpy_s(password, clone.password, sizeof(password));
	recorder = clone.recorder;
}

dk_recorder_service::_recorder_receiver_information_t dk_recorder_service::_recorder_receiver_information_t::operator=(const dk_recorder_service::_recorder_receiver_information_t & clone)
{
	strncpy_s(uuid, clone.uuid, sizeof(uuid));
	strncpy_s(url, clone.url, sizeof(url));
	strncpy_s(username, clone.username, sizeof(username));
	strncpy_s(password, clone.password, sizeof(password));
	recorder = clone.recorder;
	return (*this);
}

dk_recorder_service::dk_recorder_service(void)
	: _backup_thread(INVALID_HANDLE_VALUE)
	, _backup_port_number(21)
{
	memset(_storage_path, 0x00, sizeof(_storage_path));
	memset(_config_path, 0x00, sizeof(_config_path));
	memset(_backup_url, 0x00, sizeof(_backup_url));
	memset(_backup_username, 0x00, sizeof(_backup_username));
	memset(_backup_password, 0x00, sizeof(_backup_password));
}

dk_recorder_service::~dk_recorder_service(void)
{
	stop_recording();
}

dk_recorder_service & dk_recorder_service::instance(void)
{
	static dk_recorder_service _instance;
	return _instance;
}

bool dk_recorder_service::start_recording(void)
{
	memset(_backup_url, 0x00, sizeof(_backup_url));
	memset(_backup_username, 0x00, sizeof(_backup_username));
	memset(_backup_password, 0x00, sizeof(_backup_password));

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

	TiXmlElement * root_elem = document.FirstChildElement("record_server");
	if (!root_elem)
		return false;

	const char * backup_url = nullptr;
	const char * str_backup_port_number = nullptr;
	const char * backup_username = nullptr;
	const char * backup_password = nullptr;
	TiXmlElement * backup_server_elem = root_elem->FirstChildElement("backup_server");
	if (!backup_server_elem)
		return false;
	TiXmlElement * backup_url_elem = backup_server_elem->FirstChildElement("url");
	if (backup_url_elem)
		backup_url = backup_url_elem->GetText();

	TiXmlElement * backup_port_number_elem = backup_server_elem->FirstChildElement("port_number");
	if (backup_port_number_elem)
	{
		str_backup_port_number = backup_port_number_elem->GetText();
		if (str_backup_port_number && strlen(str_backup_port_number)>0)
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
		const char * url = url_elem->GetText();
		const char * username = username_elem->GetText();
		const char * password = password_elem->GetText();


		dk_recorder_service::recorder_receiver_information_t recv_info;
		strncpy_s(recv_info.uuid, uuid, sizeof(recv_info.uuid));
		strncpy_s(recv_info.url, url, sizeof(recv_info.url));
		if (username && strlen(username)>0)
			strncpy_s(recv_info.username, username, sizeof(recv_info.username));
		if (password && strlen(password)>0)
			strncpy_s(recv_info.password, password, sizeof(recv_info.password));
		recv_info.recorder = new dk_rtsp_recorder(chunk_size_in_mb);
		recv_info.recorder->start_recording(recv_info.url, recv_info.username, recv_info.password, dk_rtsp_recorder::rtp_over_tcp, dk_rtsp_recorder::recv_video, storage_path.c_str(), recv_info.uuid);
		_receivers.push_back(recv_info);

		media_source_elem = media_source_elem->NextSiblingElement();
	}

	//return start_backup_service();
	return true;
}

bool dk_recorder_service::stop_recording(void)
{
	//stop_backup_service();

	std::vector<dk_recorder_service::recorder_receiver_information_t>::iterator iter;
	for (iter = _receivers.begin(); iter != _receivers.end(); iter++)
	{
		dk_rtsp_recorder * recorder = (iter)->recorder;
		recorder->stop_recording();
		delete recorder;
		(iter)->recorder = nullptr;
	}
	_receivers.clear();
	return true;
}

const char * dk_recorder_service::retrieve_storage_path(void)
{
	char * module_path = nullptr;
	dk_misc_helper::retrieve_absolute_module_path("ParallelRecordServer.exe", &module_path);
	if(module_path && strlen(module_path)>0)
	{
		_snprintf_s(_storage_path, sizeof(_storage_path), "%s%s", module_path, "storage\\");
		free(module_path);
	}
	return _storage_path;
}

const char * dk_recorder_service::retrieve_config_path(void)
{
	char * module_path = nullptr;
	dk_misc_helper::retrieve_absolute_module_path("ParallelRecordServer.exe", &module_path);
	if (module_path && strlen(module_path)>0)
	{
		_snprintf_s(_config_path, sizeof(_config_path), "%s%s", module_path, "config\\");
		free(module_path);
	}
	return _config_path;
}

bool dk_recorder_service::start_backup_service(void)
{
	unsigned int thrd_addr;
	_backup_thread = (HANDLE)::_beginthreadex(NULL, 0, dk_recorder_service::backup_process_callback, this, 0, &thrd_addr);
	return true;
}

bool dk_recorder_service::stop_backup_service(void)
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

unsigned __stdcall dk_recorder_service::backup_process_callback(void * param)
{
	dk_recorder_service * self = static_cast<dk_recorder_service*>(param);
	self->backup_process();
	return 0;
}

void dk_recorder_service::backup_process(void)
{
	_backup_run = true;
	while (_backup_run)
	{
		if (!_backup_url || strlen(_backup_url) < 1)
			break;

		const char * storage_path = retrieve_storage_path();
		if (storage_path && strlen(storage_path)>0)
		{
			file_search_and_upload(storage_path);
		}
		::Sleep(1000);
	}
}


void dk_recorder_service::file_search_and_upload(const char * path)
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
						CURL * curl = NULL;
						curl_global_init(CURL_GLOBAL_ALL);
						curl = curl_easy_init();
						bool result = false;

						char backup_ftp_server[260] = { 0 };
						if (_backup_username && strlen(_backup_username) > 0 && _backup_password && strlen(_backup_password) > 0)
							_snprintf_s(backup_ftp_server, sizeof(backup_ftp_server), "ftp://%s:%s@%s:%d/%s/%s", _backup_username, _backup_password, _backup_url, _backup_port_number, "", file_name);
						else
							_snprintf_s(backup_ftp_server, sizeof(backup_ftp_server), "ftp://%s:%d/%s/%s", _backup_url, _backup_port_number, "", file_name);

						result = backup_upload_single_file(curl, backup_ftp_server, recored_file_path, 0, 3);

						curl_easy_cleanup(curl);
						curl_global_cleanup();

						if (result)
							::DeleteFileA(recored_file_path);
					}
				}
			}
		} while (FindNextFileA(hFile, &wfd));

		FindClose(hFile);
	}
}

/* parse headers for Content-Length */
size_t __stdcall dk_recorder_service::backup_get_content_length_callback(void * ptr, size_t size, size_t nmemb, void * stream)
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
size_t __stdcall dk_recorder_service::backup_discard_callback(void * ptr, size_t size, size_t nmemb, void * stream)
{
	return size * nmemb;
}

/* read data to upload */
size_t __stdcall dk_recorder_service::backup_read_callback(void * ptr, size_t size, size_t nmemb, void * stream)
{
	FILE * f = (FILE*)stream;
	size_t n;

	if (ferror(f))
		return CURL_READFUNC_ABORT;

	n = fread(ptr, size, nmemb, f) * size;

	return n;
}

/* read data to upload */
bool dk_recorder_service::backup_upload_single_file(CURL * curl, const char * remotepath, const char * localpath, long timeout, long tries)
{
	FILE *f;
	long uploaded_len = 0;
	CURLcode r = CURLE_GOT_NOTHING;
	int c;

	f = fopen(localpath, "rb");
	if (!f)
		return false;

	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, remotepath);
	if (timeout)
		curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, timeout);

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, backup_get_content_length_callback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &uploaded_len);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, backup_discard_callback);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, backup_read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, f);
	/* disable passive mode */
	curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	for (c = 0; (r != CURLE_OK) && (c < tries); c++) 
	{
		/* are we resuming? */
		if (c) { /* yes */
			/* determine the length of the file already written */

			/*
			* With NOBODY and NOHEADER, libcurl will issue a SIZE
			* command, but the only way to retrieve the result is
			* to parse the returned Content-Length header. Thus,
			* getcontentlengthfunc(). We need discardfunc() above
			* because HEADER will dump the headers to stdout
			* without it.
			*/
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
		{ /* no */
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