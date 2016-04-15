#include "yuvsource_reader.h"

yuvsource_reader::yuvsource_reader(void)
	: _file(INVALID_HANDLE_VALUE)
	, _width(0)
	, _height(0)
	, _fps(0)
{

}

yuvsource_reader::~yuvsource_reader(void)
{

}

dk_yuvsource_reader::error_code yuvsource_reader::initialize_reader(const char * filepath, int32_t width, int32_t height, int32_t fps)
{
	_file = open_file(filepath);
	if (_file == NULL || _file == INVALID_HANDLE_VALUE)
		return dk_yuvsource_reader::error_code_fail;

	if (width < 1 || height < 1 || fps < 1)
		return dk_yuvsource_reader::error_code_fail;

	_width = width;
	_height = height;
	_fps = fps;
	_frame_size = _width * _height * 1.5;
	_file_size = file_size(_file);
	_current_frame_index = 0;
	_max_frame_index = _file_size / (long)_frame_size;
	return dk_yuvsource_reader::error_code_success;
}

dk_yuvsource_reader::error_code yuvsource_reader::initialize_reader(const char * filepath)
{
	_file = open_file(filepath);
	if (_file == NULL || _file == INVALID_HANDLE_VALUE)
		return dk_yuvsource_reader::error_code_fail;

	if (_width < 1 || _height < 1 || _fps < 1)
		return dk_yuvsource_reader::error_code_fail;

	_frame_size = _width * _height * 1.5;
	_file_size = file_size(_file);
	_current_frame_index = 0;
	_max_frame_index = _file_size / (long)_frame_size;
	return dk_yuvsource_reader::error_code_success;
}

dk_yuvsource_reader::error_code yuvsource_reader::set_width(int32_t width)
{
	_width = width;
	return dk_yuvsource_reader::error_code_success;
}

dk_yuvsource_reader::error_code yuvsource_reader::set_height(int32_t height)
{
	_height = height;
	return dk_yuvsource_reader::error_code_success;
}

dk_yuvsource_reader::error_code yuvsource_reader::set_fps(int32_t fps)
{
	_fps = fps;
	return dk_yuvsource_reader::error_code_success;
}

dk_yuvsource_reader::error_code yuvsource_reader::release_reader(void)
{
	close_file(_file);
	_file = INVALID_HANDLE_VALUE;
	return dk_yuvsource_reader::error_code_success;
}

dk_yuvsource_reader::error_code yuvsource_reader::read(uint8_t * yuv, int32_t stride)
{
	uint8_t * y = yuv;
	uint8_t * v = y + _height * stride;
	uint8_t * u = v + ((_height * stride) >> 2);

	uint64_t file_offset = _frame_size * _current_frame_index;
	uint32_t bytes_read = 0;
	set_file_position(_file, file_offset, FILE_BEGIN);
	read_file(_file, y, _height * _width, &bytes_read); //copy luma plane
	bytes_read = 0;
	//set_file_position(_file, file_offset + _height * _width, FILE_BEGIN);
	read_file(_file, u, (_height * _width) >> 2, &bytes_read); //copy chroma cb plane
	bytes_read = 0;
	//set_file_position(_file, file_offset + _height * _width + (_height * _width)>>2, FILE_BEGIN);
	read_file(_file, v, (_height * _width) >> 2, &bytes_read); //copy chroma cr plane

	if (_current_frame_index >= _max_frame_index)
		_current_frame_index = 0;
	else
		_current_frame_index++;
	return dk_yuvsource_reader::error_code_success;
}

HANDLE yuvsource_reader::open_file(const char * filepath)
{
	HANDLE file = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	return file;
}

void yuvsource_reader::close_file(HANDLE file)
{
	if (file != NULL && file != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(file);
	}
}

int64_t yuvsource_reader::file_size(HANDLE file)
{
	int64_t size = 0;
	if (file != NULL && file != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER filesize = { 0 };
		::GetFileSizeEx(_file, &filesize);

		size = filesize.HighPart;
		size <<= 32;
		size |= filesize.LowPart;
	}
	return size;
}

void yuvsource_reader::set_file_position(HANDLE file, uint32_t offset, uint32_t flag)
{
	if (file != NULL && file != INVALID_HANDLE_VALUE)
	{
		::SetFilePointer(file, offset, NULL, flag);
	}
}

void yuvsource_reader::set_file_position(HANDLE file, uint64_t offset, uint32_t flag)
{
	if (file != NULL && file != INVALID_HANDLE_VALUE)
	{
		::SetFilePointer(file, ((uint32_t*)&offset)[0], (PLONG)&((uint32_t*)&offset)[1], flag);
	}
}

void yuvsource_reader::read_file(HANDLE file, void * buf, uint32_t bytes_to_read, uint32_t * bytes_read)
{
	if (file != NULL && file != INVALID_HANDLE_VALUE)
	{
		::ReadFile(file, buf, bytes_to_read, (LPDWORD)bytes_read, NULL);
	}
}