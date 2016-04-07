#ifndef _DK_YUVSOURCE_READER_H_
#define _DK_YUVSOURCE_READER_H_

#if defined(WIN32)
#include <windows.h>
#if defined(EXPORT_LIB)
#define EXP_CLASS __declspec(dllexport)
#else
#define EXP_CLASS __declspec(dllimport)
#endif
#else
#include <pthreads.h>
#define EXP_CLASS
#endif

#include <cstdint>

class yuvsource_reader;
class EXP_CLASS dk_yuvsource_reader
{
public:
	typedef enum _error_code
	{
		error_code_success = 0,
		error_code_fail
	} error_code;


	dk_yuvsource_reader(void);
	virtual ~dk_yuvsource_reader(void);

	dk_yuvsource_reader::error_code initialize_reader(const char * filepath, int32_t width, int32_t height, int32_t fps);
	dk_yuvsource_reader::error_code initialize_reader(const char * filepath);
	dk_yuvsource_reader::error_code set_width(int32_t width);
	dk_yuvsource_reader::error_code set_height(int32_t height);
	dk_yuvsource_reader::error_code set_fps(int32_t fps);
	dk_yuvsource_reader::error_code release_reader(void);
	dk_yuvsource_reader::error_code read(uint8_t * yuv, int32_t stride);


private:
	yuvsource_reader * _reader;
};














#endif