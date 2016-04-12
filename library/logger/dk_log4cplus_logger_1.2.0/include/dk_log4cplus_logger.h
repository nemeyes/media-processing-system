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

namespace log4cplus
{
	class ConfigureAndWatchThread;
};

class EXP_LOG4CPLUS_LOGGER_DLL dk_log4cplus_logger
{
public:
	static dk_log4cplus_logger & instance(void);

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
	static void printchar(char ** str, int c);
	static int prints(char ** out, const char * string, int width, int pad);
	static int printi(char ** out, int i, int b, int sg, int width, int pad, int letbase);
	static int print(char **out, const char *format, va_list args);
private:
	dk_log4cplus_logger(void);
	dk_log4cplus_logger(const dk_log4cplus_logger & clone);
	virtual ~dk_log4cplus_logger(void);

private:
	log4cplus::ConfigureAndWatchThread * _configure_thread;
};









#endif