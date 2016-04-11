#if !defined(_HTTP_CLIENT_H_)
#define _HTTP_CLIENT_H_


#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(WIN32)
#include <unistd.h>
#endif
#include <curl/curl.h>
#include <pthread.h>

class url_encoder
{
public:
	static std::string		encode( std::string str );
private:
	static bool				is_ordinary_character( char c );
};

class url_decoder
{
public:
	static std::string		decode( std::string str );
private:
	static int				convert_to_decimal( const char *hex );
	static void				get_as_decimal( char *hex );
};


class http_client 
{
public:
	// path means uri, and it should be start with '/'.
	http_client( char *hostname, unsigned int portnumber, const char *path );
	~http_client( void );

	bool			put_variable( const char *key, const char *value );
	bool			get_variable( const char *key, char **value );
	bool			clear_variable( void );

	std::string		get_host( void ) const;
	std::string		get_access_path( void ) const;

	bool			send_request( char *user_id, char *user_password, char *recv_buffer=0, char *host=nullptr );
	unsigned long	get_bytes_read( void );

private:
	bool			is_duplicated_variable(const char* key);
	std::string		make_request_url( void );

private:
	std::map<std::string,std::string>	_request_map;
	std::string							_hostname;
	unsigned int						_portnumber;
	std::string							_access_path;
	unsigned long						_count_of_bytes_read;
	CURL								*_curl;

	pthread_mutex_t						_mutex;
};

#endif