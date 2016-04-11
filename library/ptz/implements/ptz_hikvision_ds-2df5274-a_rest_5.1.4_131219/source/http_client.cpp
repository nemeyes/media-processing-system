#include "platform.h"
#include "http_client.h"

typedef struct _MEMORY_STRUCT_T 
{
	char		*memory;
	size_t		size;
} MEMORY_STRUCT_T;

size_t write_data( void *buffer, size_t size, size_t nmemb, void *stream ) 
{
	size_t real_size = size * nmemb;

	MEMORY_STRUCT_T *mem = static_cast<MEMORY_STRUCT_T*>( stream );
	mem->memory = (char*)realloc( mem->memory, mem->size + real_size+1 );

	if( mem->memory==NULL ) 
		return 0;

	memcpy( &(mem->memory[mem->size]), buffer, real_size );
	mem->size += real_size;
	mem->memory[mem->size] = 0;
	return real_size;
}

std::string url_encoder::encode( std::string str )
{
	int len = str.length();
	char* buff = new char[len + 1];
	strcpy( buff, str.c_str() );
	std::string ret("");
	for(int i=0;i<len;i++)
	{
		if( is_ordinary_character(buff[i]) )
		{
			ret = ret + buff[i];
		}
		else if(buff[i] == ' ')
		{
			ret = ret + "+";
		}
		else
		{
			char tmp[6];
			sprintf( tmp, "%%%x", buff[i]);
			ret = ret + tmp;
		}
	}
	delete[] buff;
	return ret;
}

bool url_encoder::is_ordinary_character( char c )
{
	char ch = tolower(c);
	if(ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' || ch == 'e'
		|| ch == 'f' || ch == 'g' || ch == 'h' || ch == 'i' || ch == 'j'
		|| ch == 'k' || ch == 'l' || ch == 'm' || ch == 'n' || ch == 'o'
		|| ch == 'p' || ch == 'q' || ch == 'r' || ch == 's' || ch == 't'
		|| ch == 'u' || ch == 'v' || ch == 'w' || ch == 'x' || ch == 'y'
		|| ch == 'z' || ch == '0' || ch == '1' || ch == '2' || ch == '3'
		|| ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8'
		|| ch == '9') {
		return true;
	}
	return false;
}

std::string url_decoder::decode( std::string str )
{
	int len = str.length();
	char* buff = new char[len + 1];
	strcpy( buff, str.c_str() );
	std::string ret("");
	for( int i=0;i<len;i++ )
	{
		if( buff[i]=='+' )
		{
			ret = ret + " ";
		}
		else if( buff[i]=='%' )
		{
			char tmp[4];
			char hex[4];
			hex[0] = buff[++i];
			hex[1] = buff[++i];
			hex[2] = '\0';
			//int hex_i = atoi(hex);
			sprintf( tmp, "%c", convert_to_decimal(hex));
			ret = ret + tmp;
		}
		else
		{
			ret = ret + buff[i];
		}
	}
	delete[] buff;
	return ret;
}

int url_decoder::convert_to_decimal( const char* hex )
{
	char buff[12];
	sprintf( buff, "%s", hex );
	int ret = 0;
	int len = strlen(buff);
	for( int i=0;i<len;i++ )
	{
		char tmp[4];
		tmp[0] = buff[i];
		tmp[1] = '\0';
		get_as_decimal(tmp);
		int tmp_i = atoi( tmp );
		int rs = 1;
		for( int j=i; j<(len-1); j++ )
		{
			rs *= 16;
		}
		ret += (rs * tmp_i);
	}
	return ret;
}

void url_decoder::get_as_decimal( char* hex )
{
	char tmp = tolower(hex[0]);
	if( tmp=='a' )
	{
		strcpy( hex, "10" );
	}
	else if( tmp=='b' )
	{
		strcpy( hex, "11" );
	}
	else if( tmp=='c' )
	{
		strcpy( hex, "12" );
	}
	else if( tmp=='d' )
	{
		strcpy( hex, "13" );
	}
	else if( tmp=='e' )
	{
		strcpy( hex, "14" );
	}
	else if( tmp=='f' )
	{
		strcpy( hex, "15" );
	}
	else if( tmp=='g' )
	{
		strcpy( hex, "16" );
	}
}

http_client::http_client( char *hostname, unsigned int portnumber, const char *path ) 
	: _hostname(hostname)
	, _portnumber(portnumber)
	, _count_of_bytes_read(0)
	, _mutex(PTHREAD_MUTEX_INITIALIZER)
{
	pthread_mutex_init( &_mutex, 0 );
	if (path == NULL)
		_access_path.assign("");
	_access_path = path;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	_curl = curl_easy_init();
}

http_client::~http_client( void ) 
{
	curl_easy_cleanup(_curl);
	curl_global_cleanup();
	pthread_mutex_destroy( &_mutex );
}

bool http_client::send_request( char *user_id, char *user_password, char *method, char *recv_buffer ) 
{
	bool result = false;
	CURLcode curl_result;

	pthread_mutex_lock( &_mutex );

	std::string request_url = make_request_url();

	// wirte buffer contents to MEMCPYed block
	MEMORY_STRUCT_T chunk;
	chunk.memory = static_cast<char*>( malloc(1) ); // will be grown as needed by the realloc above
	chunk.size = 0; // no data at this point

	curl_easy_setopt( _curl, CURLOPT_URL, request_url.c_str() );
	curl_easy_setopt( _curl, CURLOPT_WRITEFUNCTION, write_data );
	curl_easy_setopt( _curl, CURLOPT_WRITEDATA, &chunk );
	curl_easy_setopt( _curl, CURLOPT_USERNAME, user_id );
	curl_easy_setopt( _curl, CURLOPT_PASSWORD, user_password );
	curl_easy_setopt( _curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC );
	curl_easy_setopt( _curl, CURLOPT_CONNECTTIMEOUT, 1 );
	if(strcmp(method,"PUT")==0) curl_easy_setopt( _curl, CURLOPT_CUSTOMREQUEST, "PUT");
	if(strcmp(method,"DELETE")==0) curl_easy_setopt( _curl, CURLOPT_CUSTOMREQUEST, "DELETE");


	int tries_left = 3;
	do 
	{
		curl_result = curl_easy_perform( _curl );
		tries_left--;
	} while( curl_result!=CURLE_OK && tries_left>0 );

	if( curl_result==CURLE_OK )
		result = true;

	if( recv_buffer!=NULL )
		strcpy( recv_buffer, chunk.memory );

	if( chunk.memory )
		free( chunk.memory );

	pthread_mutex_unlock( &_mutex );
	return result;
}

bool http_client::send_request_with_data( char *user_id, char *user_password, char* method, char *send_buffer, char **recv_buffer )
{
	bool result = false;
	CURLcode curl_result;

	pthread_mutex_lock( &_mutex );

	char portnumber_character[10];
	sprintf( portnumber_character, "%d", _portnumber );
	std::string portnumber_string( portnumber_character );
	std::string request_url = "http://" + _hostname + ":" + portnumber_string + _access_path;

	char *post = send_buffer;

	// wirte buffer contents to MEMCPYed block
	MEMORY_STRUCT_T chunk;
	chunk.memory = static_cast<char*>( malloc(1) ); // will be grown as needed by the realloc above
	chunk.size = 0; // no data at this point

	struct curl_slist *slist = NULL;
	slist = curl_slist_append( slist, "Accept: */*" );
	// slist = curl_slist_append(slist, "Content-Type: application/x-www-form-urlencoded");
	slist = curl_slist_append( slist, "Content-Type: text/xml" );
	curl_easy_setopt( _curl, CURLOPT_HTTPHEADER, slist);
	curl_easy_setopt( _curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt( _curl, CURLOPT_HEADER, 0);
	curl_easy_setopt( _curl, CURLOPT_USERAGENT,  "LG CNS IntelliVMS");
	curl_easy_setopt( _curl, CURLOPT_URL, request_url.c_str() );
	curl_easy_setopt( _curl, CURLOPT_WRITEFUNCTION, write_data );
	curl_easy_setopt( _curl, CURLOPT_WRITEDATA, &chunk );
	curl_easy_setopt( _curl, CURLOPT_USERNAME, user_id );
	curl_easy_setopt( _curl, CURLOPT_PASSWORD, user_password );
	curl_easy_setopt( _curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	if(strcmp(method,"PUT")==0) curl_easy_setopt( _curl, CURLOPT_CUSTOMREQUEST, "PUT");
	curl_easy_setopt( _curl, CURLOPT_POSTFIELDS, post );

	curl_result = curl_easy_perform( _curl );
	if( curl_result==CURLE_OK )
		result = true;

	if( chunk.size>0 )
	{
		(*recv_buffer) = static_cast<char*>( malloc(chunk.size) );
		memcpy( (*recv_buffer), chunk.memory, chunk.size );
	}

	if( chunk.memory )
		free( chunk.memory );

	curl_slist_free_all(slist);
	pthread_mutex_unlock( &_mutex );
	return result;
}

std::string http_client::make_request_url( void ) 
{
	std::string complete_url;

	char portnumber_character[10];
	sprintf( portnumber_character, "%d", _portnumber );
	std::string portnumber_string( portnumber_character );

	complete_url = "http://" + _hostname + ":" + portnumber_string + _access_path;

	if( _request_map.size()>0 ) 
	{
		std::map<std::string, std::string>::iterator iter;
		complete_url += "?";
		for( iter=_request_map.begin(); iter!=_request_map.end(); iter++ ) 
		{
			std::string key				= (*iter).first;
			std::string value			= (*iter).second;
			std::string encoded_key		= url_encoder::encode( key );
			std::string encoded_value	= url_encoder::encode( value );
			complete_url += encoded_key + "=" + encoded_value + "&";
		}
		complete_url = complete_url.substr(0, complete_url.size() - 1);
	}
	return complete_url;
}


bool http_client::is_duplicated_variable( const char *key ) 
{
	std::map<std::string, std::string>::iterator iter;
	for( iter=_request_map.begin(); iter!=_request_map.end(); iter++ ) 
	{
		if( strcmp(key, (*iter).first.c_str())==0 )
			return true;
	}
	return false;
}

bool http_client::put_variable(const char* key, const char* value) 
{
	pthread_mutex_lock( &_mutex );
	if( is_duplicated_variable(key) )
	{
		pthread_mutex_unlock( &_mutex );
		return false;
	}
	else 
	{
		pthread_mutex_unlock( &_mutex );
		_request_map.insert( std::make_pair(key, value) );
		return true;
	}
}

bool http_client::get_variable(const char* key, char** value ) 
{
	pthread_mutex_lock( &_mutex );
	std::map<std::string,std::string>::iterator iter;
	for( iter=_request_map.begin(); iter!=_request_map.end(); iter++ )
	{
		if( strcmp(key, ((*iter).first.c_str()))==0 )
		{
			(*value) = (char*) (*iter).second.c_str();
			pthread_mutex_unlock( &_mutex );
			return true;
		}
	}
	pthread_mutex_unlock( &_mutex );
	return false;
}


bool http_client::clear_variable( void )
{
	pthread_mutex_lock( &_mutex );
	_request_map.clear();
	pthread_mutex_unlock( &_mutex );
	return true;
}

unsigned long	http_client::get_bytes_read( void ) 
{ 
	return _count_of_bytes_read; 
}

std::string	http_client::get_host( void ) const 
{
	return _hostname;
}

std::string http_client::get_access_path( void ) const 
{
	return _access_path;
}

