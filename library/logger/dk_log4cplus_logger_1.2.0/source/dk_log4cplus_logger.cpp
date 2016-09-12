#include "dk_log4cplus_logger.h"

#if !defined(WITH_DISABLE)
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

#if defined(_DEBUG)
//#pragma comment(lib, "log4cplusSUD.lib")
//#pragma comment(lib, "log4cplusUD.lib")
#pragma comment(lib, "log4cplusU.lib")
#else
//#pragma comment(lib, "log4cplusSU.lib")
#pragma comment(lib, "log4cplusU.lib")
#endif
#endif

namespace debuggerking
{
	class log4cplus_lock
	{
	public:
		log4cplus_lock(void)
		{
			::InitializeCriticalSection(&_cs);
		}

		~log4cplus_lock(void)
		{
			::DeleteCriticalSection(&_cs);
		}

		void lock(void)
		{
			::EnterCriticalSection(&_cs);
		}

		void unlock(void)
		{
			::LeaveCriticalSection(&_cs);
		}

	private:
		CRITICAL_SECTION _cs;
	};

	class log4cplus_scopped_lock
	{
	public:
		log4cplus_scopped_lock(log4cplus_lock * lock)
			: _lock(lock)
		{
			_lock->lock();
		}

		~log4cplus_scopped_lock(void)
		{
			_lock->unlock();
		}

	private:
		log4cplus_lock * _lock;
	};
};

static debuggerking::log4cplus_lock g_log4cplus_lock;

debuggerking::log4cplus_logger * debuggerking::log4cplus_logger::_instance = nullptr;

void debuggerking::log4cplus_logger::create(const char * configuration_path)
{
	log4cplus_scopped_lock mutex(&g_log4cplus_lock);
	if (!_instance)
		_instance = new log4cplus_logger(configuration_path);
}

void debuggerking::log4cplus_logger::destroy(void)
{
	log4cplus_scopped_lock mutex(&g_log4cplus_lock);
	if (_instance)
	{
		debuggerking::log4cplus_logger * tmp_instance = _instance;
		_instance = nullptr;
		delete tmp_instance;
	}
}

debuggerking::log4cplus_logger::log4cplus_logger(const char * configuration_path)
{
#if !defined(WITH_DISABLE)
	std::locale::global(     // set global locale
		std::locale(         // using std::locale constructed from
		std::locale (),   // global locale
		// and codecvt_utf8 facet
		new std::codecvt_utf8<wchar_t, 0x10FFFF, static_cast<std::codecvt_mode>(std::consume_header | std::little_endian)>));

	log4cplus::initialize();
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
				_strset_s(pszModuleName, strlen(pszModuleName)+1, 0);
			}
			else
			{
				_strset_s(szModulePath, strlen(pszModuleName)+1, 0);
			}
		}

		_snprintf_s(szModulePath, sizeof(szModulePath), "%s%s", szModulePath, configuration_path);
		_configure_thread = new log4cplus::ConfigureAndWatchThread(LOG4CPLUS_STRING_TO_TSTRING(szModulePath), 5 * 1024);
	}
	catch (...)
	{
		_configure_thread = nullptr;
	}
#endif
}

debuggerking::log4cplus_logger::~log4cplus_logger(void)
{
#if !defined(WITH_DISABLE)
	if (_configure_thread)
	{
		delete _configure_thread;
		_configure_thread = nullptr;
	}
#endif
}

void debuggerking::log4cplus_logger::make_fatal_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "FATAL :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		char log[4096] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_FATAL(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void debuggerking::log4cplus_logger::make_error_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "ERROR :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		char log[4096] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_ERROR(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void debuggerking::log4cplus_logger::make_warn_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "WARN :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		char log[4096] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_WARN(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void debuggerking::log4cplus_logger::make_info_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "INFO :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		char log[4096] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_INFO(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void debuggerking::log4cplus_logger::make_debug_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "DEBUG :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		char log[4096] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_DEBUG(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}

void debuggerking::log4cplus_logger::make_trace_log(const char * secion, const char * fmt, ...)
{
	if (_instance)
	{
#if defined(WITH_DISABLE)
		char log[max_message_size] = { "TRACE :" };
		int index = strlen(log);
		char * rlog = &log[index];

		va_list args;
		va_start(args, fmt);
		vsnprintf_s(rlog, sizeof(log) - index, sizeof(log) - index, fmt, args);
		va_end(args);

		index = strlen(log);
		if (index > (sizeof(log) - 1))
		{
			log[max_message_size - 1] = 0;
			log[max_message_size - 2] = '\n';
			log[max_message_size - 3] = '\r';
		}
		else
		{
			log[index] = '\r';
			log[index + 1] = '\n';
			log[index + 2] = 0;
		}
		::OutputDebugStringA(log);
#else
		char log[4096] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(log, sizeof(log), fmt, args);
		va_end(args);
		LOG4CPLUS_TRACE(log4cplus::Logger::getInstance(LOG4CPLUS_STRING_TO_TSTRING(secion)), LOG4CPLUS_STRING_TO_TSTRING(log));
#endif
	}
}