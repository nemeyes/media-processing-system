#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdlib.h>
#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include <fstream> 
#include <iostream> 

#define banollim(x,dig) (floor(float(x)*pow(10.0f,float(dig))+0.5f)/pow(10.0f,float(dig)))

#ifdef WIN32
# define snprintf		_snprintf_s
# define sprintf		sprintf_s
# pragma warning(disable : 4996)        /* Remove snprintf Secure Warnings */
# define strcasecmp		strcmp
# define strncasecmp	_stricmp
# define strdup			_strdup
# define memicmp		_memicmp
# define sleep_second(seconds) Sleep(seconds*1000)
# define sleep_millisecond(milliseconds) Sleep(milliseconds)
#else
# define strtok strtok_r
# define sleep_second(seconds) sleep(seconds)
# define sleep_millisecond(milliseconds) usleep(milliseconds*1000)

#endif

#endif