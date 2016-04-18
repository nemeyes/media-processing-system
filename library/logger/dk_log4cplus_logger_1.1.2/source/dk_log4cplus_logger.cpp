#include "dk_log4cplus_logger.h"
#include <codecvt>
#include <locale.h>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/sleep.h>
#include <log4cplus/loggingmacros.h>
#include <tchar.h>
#include <process.h>
#include "userenv.h"
#include "wtsapi32.h"

dk_log4cplus_logger::dk_log4cplus_logger(void)
{
	try
	{
		HINSTANCE self;
		self = ::GetModuleHandleA("dk_log4cplus_logger_1.1.2.dll");
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
				_strset_s(pszModuleName, strlen(pszModuleName)+1, 0);
			}
			else
			{
				_strset_s(szModulePath, strlen(pszModuleName)+1, 0);
			}
		}

		_snprintf_s(szModulePath, sizeof(szModulePath), "%s%s", szModulePath, "config\\log.properties");
		_configure_thread = new log4cplus::ConfigureAndWatchThread(LOG4CPLUS_STRING_TO_TSTRING(szModulePath), 5 * 1024);
	}
	catch (...)
	{
		_configure_thread = nullptr;
	}
}

dk_log4cplus_logger::~dk_log4cplus_logger(void)
{
	if (_configure_thread)
	{
		delete _configure_thread;
		_configure_thread = nullptr;
	}
}

void dk_log4cplus_logger::make_system_info_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(log, sizeof(log), fmt, args);
	va_end(args);
	LOG4CPLUS_INFO(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_trace_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(log, sizeof(log), fmt, args);
	va_end(args);
	LOG4CPLUS_TRACE(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_debug_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(log, sizeof(log), fmt, args);
	va_end(args);
	LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_warn_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(log, sizeof(log), fmt, args);
	va_end(args);
	LOG4CPLUS_WARN(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_error_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(log, sizeof(log), fmt, args);
	va_end(args);
	LOG4CPLUS_ERROR(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
}

void dk_log4cplus_logger::make_system_fatal_log(const char * secion, const char * fmt, ...)
{
	char log[500] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(log, sizeof(log), fmt, args);
	va_end(args);
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