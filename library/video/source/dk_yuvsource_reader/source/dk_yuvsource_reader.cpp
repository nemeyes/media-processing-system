#include "dk_yuvsource_reader.h"
#include "yuvsource_reader.h"

dk_yuvsource_reader::dk_yuvsource_reader(void)
{
	_reader = new yuvsource_reader();
}

dk_yuvsource_reader::~dk_yuvsource_reader(void)
{
	if (_reader)
	{
		_reader->release_reader();
		delete _reader;
	}
	_reader = nullptr;
}


dk_yuvsource_reader::error_code dk_yuvsource_reader::initialize_reader(const char * filepath, int32_t width, int32_t height, int32_t fps)
{
	return _reader->initialize_reader(filepath, width, height, fps);
}

dk_yuvsource_reader::error_code dk_yuvsource_reader::initialize_reader(const char * filepath)
{
	return _reader->initialize_reader(filepath);
}

dk_yuvsource_reader::error_code dk_yuvsource_reader::set_width(int32_t width)
{
	return _reader->set_width(width);
}

dk_yuvsource_reader::error_code dk_yuvsource_reader::set_height(int32_t height)
{
	return _reader->set_height(height);
}

dk_yuvsource_reader::error_code dk_yuvsource_reader::set_fps(int32_t fps)
{
	return _reader->set_fps(fps);
}

dk_yuvsource_reader::error_code dk_yuvsource_reader::release_reader(void)
{
	return _reader->release_reader();
}

dk_yuvsource_reader::error_code dk_yuvsource_reader::read(uint8_t * yuv, int32_t stride)
{
	return _reader->read(yuv, stride);
}