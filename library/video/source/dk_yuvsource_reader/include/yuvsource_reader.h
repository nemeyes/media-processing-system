#ifndef _YUVSOURCE_READER_H_
#define _YUVSOURCE_READER_H_

#include "dk_yuvsource_reader.h"

class yuvsource_reader
{
public:
	yuvsource_reader(void);
	virtual ~yuvsource_reader(void);

	dk_yuvsource_reader::error_code initialize_reader(const char * filepath, int32_t width, int32_t height, int32_t fps);
	dk_yuvsource_reader::error_code release_reader(void);
	dk_yuvsource_reader::error_code read(uint8_t * yuv, int32_t stride);

private:
	HANDLE open_file(const char * filepath);
	void close_file(HANDLE file);
	unsigned long file_size(HANDLE file);
	void set_file_position(HANDLE file, uint32_t offset, uint32_t flag);
	void read_file(HANDLE file, void * buf, uint32_t bytes_to_read, uint32_t * bytes_read);

private:
	HANDLE _file;
	int32_t _width;
	int32_t _height;
	int32_t _fps;

	int32_t _frame_size;
	unsigned long _file_size;

	int32_t _current_frame_index;
	int32_t _max_frame_index;
};











#endif