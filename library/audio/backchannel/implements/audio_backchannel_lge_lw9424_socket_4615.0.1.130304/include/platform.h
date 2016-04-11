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

#endif

#define MAX_PATH 260