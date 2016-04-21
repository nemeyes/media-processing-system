#ifndef _DK_LOG4CPLUS_LOGGER_H_
#define _DK_LOG4CPLUS_LOGGER_H_

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

class EXP_LOG4CPLUS_LOGGER_DLL dk_log4cplus_logger
{
public:
	static void create(const char * configuration_path);
	static void destroy(void);
	static dk_log4cplus_logger * instance(void);

	static void make_fatal_log(const char * secion, const char * fmt, ...);
	static void make_error_log(const char * secion, const char * fmt, ...);
	static void make_warn_log(const char * secion, const char * fmt, ...);
	static void make_info_log(const char * secion, const char * fmt, ...);
	static void make_debug_log(const char * secion, const char * fmt, ...);
	static void make_trace_log(const char * secion, const char * fmt, ...);


	/*
	void make_system_info_log(const char * secion, const char * fmt, ...);
	void make_system_trace_log(const char * secion, const char * fmt, ...);
	void make_system_debug_log(const char * secion, const char * fmt, ...);
	void make_system_warn_log(const char * secion, const char * fmt, ...);
	void make_system_error_log(const char * secion, const char * fmt, ...);
	void make_system_fatal_log(const char * secion, const char * fmt, ...);

	void make_system_info_log(const wchar_t * secion, const wchar_t * log);
	void make_system_trace_log(const wchar_t * secion, const wchar_t * log);
	void make_system_debug_log(const wchar_t * secion, const wchar_t * log);
	void make_system_warn_log(const wchar_t * secion, const wchar_t * log);
	void make_system_error_log(const wchar_t * secion, const wchar_t * log);
	void make_system_fatal_log(const wchar_t * secion, const wchar_t * log);
	*/

private:
	dk_log4cplus_logger(const char * configuration_path);
	dk_log4cplus_logger(const dk_log4cplus_logger & clone);
	virtual ~dk_log4cplus_logger(void);
	
private:
	static dk_log4cplus_logger * _instance;

private:
	log4cplus::ConfigureAndWatchThread * _configure_thread;
};









#endif