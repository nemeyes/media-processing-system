#ifndef _DK_LOG4CPLUS_LOGGER_H_
#define _DK_LOG4CPLUS_LOGGER_H_

#include <dk_basic_type.h>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LOG4CPLUS_LOGGER_LIB)
#  define EXP_LOG4CPLUS_LOGGER_DLL __declspec(dllexport)
# else
#  define EXP_LOG4CPLUS_LOGGER_DLL __declspec(dllimport)
# endif
#else
# define EXP_LOG4CPLUS_LOGGER_DLL
#endif
#include <string>

namespace log4cplus
{
	class ConfigureAndWatchThread;
}

namespace debuggerking
{
	class EXP_LOG4CPLUS_LOGGER_DLL log4cplus_logger : public foundation
	{
	public:
		log4cplus_logger(void);
		virtual ~log4cplus_logger(void);

		void make_system_info_log(const char * secion, const char * fmt, ...);
		void make_system_trace_log(const char * secion, const char * fmt, ...);
		void make_system_debug_log(const char * secion, const char * fmt, ...);
		void make_system_warn_log(const char * secion, const char * fmt, ...);
		void make_system_error_log(const char * secion, const char * fmt, ...);
		void make_system_fatal_log(const char * secion, const char * fmt, ...);

		/*
		void make_system_info_log(const wchar_t * secion, const wchar_t * log);
		void make_system_trace_log(const wchar_t * secion, const wchar_t * log);
		void make_system_debug_log(const wchar_t * secion, const wchar_t * log);
		void make_system_warn_log(const wchar_t * secion, const wchar_t * log);
		void make_system_error_log(const wchar_t * secion, const wchar_t * log);
		void make_system_fatal_log(const wchar_t * secion, const wchar_t * log);
		*/
	private:
		log4cplus_logger(const log4cplus_logger & clone);

	private:
		log4cplus::ConfigureAndWatchThread * _configure_thread;
	};
};








#endif