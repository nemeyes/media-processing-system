#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef WIN32
# define snprintf _snprintf
# pragma warning(disable : 4996)        /* Remove snprintf Secure Warnings */
# define strcasecmp strcmp
# define strncasecmp _stricmp
# define strdup	_strdup
# define memicmp _memicmp
# define sleep_second(seconds) Sleep(seconds*1000)
# define sleep_millisecond(milliseconds) Sleep(milliseconds)
# define localtime_r(time,tm) localtime_s(tm,time)
# define gmtime_r(time,tm) gmtime_s(tm,time)
#else
# include <strings.h>
# include <sys/socket.h>
# define strtok strtok_r
# define sleep_second(seconds) sleep(seconds)
# define sleep_millisecond(milliseconds) usleep(milliseconds*1000)
# define localtime_r(time,tm) localtime_r(time, tm)
# define gmtime_r(time,tm) gmtime_r(time,tm)
#endif

#endif