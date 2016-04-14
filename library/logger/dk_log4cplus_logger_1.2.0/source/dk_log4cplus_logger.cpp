#include "dk_log4cplus_logger.h"
#include <log4cplus/config.hxx>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/sleep.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/win32debugappender.h>
#include <tchar.h>
#include <process.h>
#include "userenv.h"
#include "wtsapi32.h"

#define PAD_RIGHT		1
#define PAD_ZERO		2
#define PRINT_BUF_LEN	12 /* the following should be enough for 32 bit int */

dk_log4cplus_logger & dk_log4cplus_logger::instance(void)
{
	static dk_log4cplus_logger _instance;
	return _instance;
}

dk_log4cplus_logger::dk_log4cplus_logger(void)
{
	try
	{
		HINSTANCE self;
		self = ::GetModuleHandleA("dk_log4cplus_logger_1.2.0.dll");
		CHAR szModuleName[MAX_PATH] = { 0 };
		CHAR szModuleFindPath[MAX_PATH] = { 0 };
		CHAR szModulePath[MAX_PATH] = { 0 };
		CHAR *pszModuleName = szModulePath;
		pszModuleName += GetModuleFileNameA(self, pszModuleName, (sizeof(szModulePath) / sizeof(*szModulePath)) - (pszModuleName - szModulePath));
		if (pszModuleName != szModulePath)
		{
			CHAR *slash = strrchr(szModulePath, '\\');
			if (slash != NULL)
			{
				pszModuleName = slash + 1;
				_strset_s(pszModuleName, strlen(szModulePath), 0);
			}
			else
			{
				_strset_s(szModulePath, strlen(szModulePath), 0);
			}
		}
		_snprintf_s(szModulePath, sizeof(szModulePath), "%s%s", szModulePath, "config/log.properties");
#if defined(_UNICODE)
		_configure_thread = new log4cplus::ConfigureAndWatchThread(log4cplus::helpers::towstring(szModulePath), 5 * 1000);
#else
		_configure_thread = new log4cplus::ConfigureAndWatchThread(szModulePath, 5 * 1000);
#endif
	}
	catch (...)
	{
		_configure_thread = 0;
	}
}

dk_log4cplus_logger::~dk_log4cplus_logger(void)
{
	if (_configure_thread)
	{
		delete _configure_thread;
		_configure_thread = 0;
	}
}

void dk_log4cplus_logger::make_system_info_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
#if 1
	char * plog = log;
	print(&plog, fmt, args);
#else
	_snprintf_s(log, sizeof(log), fmt, args);
#endif
	LOG4CPLUS_INFO(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_trace_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
#if 1
	char * plog = log;
	print(&plog, fmt, args);
#else
	_snprintf_s(log, sizeof(log), fmt, args);
#endif
	LOG4CPLUS_TRACE(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_debug_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
#if 1
	print(&plog, fmt, args);
#else
	_snprintf_s(log, sizeof(log), fmt, args);
#endif
	LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_warn_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
#if 1
	char * plog = log;
	print(&plog, fmt, args);
#else
	_snprintf_s(log, sizeof(log), fmt, args);
#endif
	LOG4CPLUS_WARN(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_error_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
#if 1
	char * plog = log;
	print(&plog, fmt, args);
#else
	_snprintf_s(log, sizeof(log), fmt, args);
#endif
	LOG4CPLUS_ERROR(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_fatal_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
#if 1
	char * plog = log;
	print(&plog, fmt, args);
#else
	_snprintf_s(log, sizeof(log), fmt, args);
#endif
	LOG4CPLUS_FATAL(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

/*
void dk_log4cplus_logger::make_system_info_log(const wchar_t * secion, const wchar_t * log)
{
	LOG4CPLUS_INFO(log4cplus::Logger::getInstance(secion), log);
}

void dk_log4cplus_logger::make_system_trace_log(const wchar_t * secion, const wchar_t * log)
{
	LOG4CPLUS_TRACE(log4cplus::Logger::getInstance(secion), log);
}

void dk_log4cplus_logger::make_system_debug_log(const wchar_t * secion, const wchar_t * log)
{
	LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance(secion), log);
}

void dk_log4cplus_logger::make_system_warn_log(const wchar_t * secion, const wchar_t * log)
{
	LOG4CPLUS_WARN(log4cplus::Logger::getInstance(secion), log);
}

void dk_log4cplus_logger::make_system_error_log(const wchar_t * secion, const wchar_t * log)
{
	LOG4CPLUS_ERROR(log4cplus::Logger::getInstance(secion), log);
}

void dk_log4cplus_logger::make_system_fatal_log(const wchar_t * secion, const wchar_t * log)
{
	LOG4CPLUS_FATAL(log4cplus::Logger::getInstance(secion), log);
}
*/

void dk_log4cplus_logger::printchar(char ** str, int c)
{
	extern int putchar(int c);
	if (str) 
	{
		**str = c;
		++(*str);
	}
	else 
		(void)putchar(c);
}

int dk_log4cplus_logger::prints(char ** out, const char * string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) 
	{
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) 
			++len;
		if (len >= width) 
			width = 0;
		else 
			width -= len;
		if (pad & PAD_ZERO) 
			padchar = '0';
	}

	if (!(pad & PAD_RIGHT)) 
	{
		for (; width > 0; --width) 
		{
			printchar(out, padchar);
			++pc;
		}
	}
	for (; *string; ++string) 
	{
		printchar(out, *string);
		++pc;
	}
	for (; width > 0; --width) 
	{
		printchar(out, padchar);
		++pc;
	}

	return pc;
}

int dk_log4cplus_logger::printi(char ** out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) 
	{
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) 
	{
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';

	while (u) 
	{
		t = u % b;
		if (t >= 10)
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) 
	{
		if (width && (pad & PAD_ZERO)) 
		{
			printchar(out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}
	return pc + prints(out, s, width, pad);
}

int dk_log4cplus_logger::print(char ** out, const char * format, va_list args)
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) 
	{
		if (*format == '%') 
		{
			++format;
			width = pad = 0;
			if (*format == '\0') 
				break;
			if (*format == '%') 
				goto out;
			if (*format == '-') 
			{
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') 
			{
				++format;
				pad |= PAD_ZERO;
			}
			for (; *format >= '0' && *format <= '9'; ++format) 
			{
				width *= 10;
				width += *format - '0';
			}
			if (*format == 's') 
			{
				register char *s = (char *)va_arg(args, int);
				pc += prints(out, s ? s : "(null)", width, pad);
				continue;
			}
			if (*format == 'd') 
			{
				pc += printi(out, va_arg(args, int), 10, 1, width, pad, 'a');
				continue;
			}
			if (*format == 'x') 
			{
				pc += printi(out, va_arg(args, int), 16, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'X') 
			{
				pc += printi(out, va_arg(args, int), 16, 0, width, pad, 'A');
				continue;
			}
			if (*format == 'u') 
			{
				pc += printi(out, va_arg(args, int), 10, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'c') 
			{
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg(args, int);
				scr[1] = '\0';
				pc += prints(out, scr, width, pad);
				continue;
			}
		}
		else 
		{
		out:
			printchar(out, *format);
			++pc;
		}
	}
	if (out) 
		**out = '\0';
	va_end(args);
	return pc;
}
