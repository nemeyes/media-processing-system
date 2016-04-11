#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef WIN32
# define snprintf _snprintf
# pragma warning(disable : 4996)        /* Remove snprintf Secure Warnings */
# define strcasecmp strcmp
# define strncasecmp _stricmp
# define strdup	_strdup
# define sleep_second(seconds) Sleep(seconds*1000)
# define sleep_millisecond(milliseconds) Sleep(milliseconds)
#else
# define strtok strtok_r
# define sleep_second(seconds) sleep(seconds)
# define sleep_millisecond(milliseconds) usleep(milliseconds*1000)
#endif

#define soap_new_float(soap) static_cast<float*>(soap_malloc(soap,sizeof(float*)))
#define soap_new_int(soap) static_cast<int*>(soap_malloc(soap,sizeof(int*)))
#define soap_new_int64(soap) static_cast<long long*>(soap_malloc(soap,sizeof(long long*)))
#define soap_new_char(soap) static_cast<char*>(soap_malloc(soap,sizeof(char*)))
#define soap_new_bool(soap) static_cast<bool*>(soap_malloc(soap,sizeof(bool*)))
#define soap_new_type(soap,type) static_cast<type*>(soap_malloc(soap,sizeof(type)))
#define soap_new_p_char(soap) static_cast<char**>(soap_malloc(soap,sizeof(char**)))
#define soap_new_p_type(soap,type,size) static_cast<type**>(soap_malloc(soap,sizeof(type*)*size))

#endif