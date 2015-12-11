#include "mpeg2ts_file.h"

#include <dk_fileio.h>
#include <dk_string_helper.h>

mpeg2ts_file::mpeg2ts_file(void)
{

}

mpeg2ts_file::~mpeg2ts_file(void)
{

}

dk_mpeg2ts_segmenter::ERR_CODE mpeg2ts_file::initialize(wchar_t * filepath)
{
	char * mbfilepath = 0;
	dk_string_helper::convert_wide2multibyte(filepath, &mbfilepath);
	_ts_file = open_file(mbfilepath);
	if (!_ts_file || _ts_file==INVALID_HANDLE_VALUE)
		return dk_mpeg2ts_segmenter::ERR_CODE_FAIL;
	
	DWORD file_size;
	get_file_size(_ts_file, &file_size);
	_file_size = file_size;
	if ((_file_size%TS_PACKET_SIZE) != 0)
		return dk_mpeg2ts_segmenter::ERR_CODE_INCORRECT_TS_FILE_SIZE;

	_segment_count = _file_size / TS_PACKET_SIZE;
	_segment_number = 0;

	if (mbfilepath)
		free(mbfilepath);

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_segmenter::ERR_CODE mpeg2ts_file::release(void)
{
	if (_ts_file && (_ts_file != INVALID_HANDLE_VALUE))
	{
		close_file(_ts_file);
		_ts_file = NULL;
	}

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

mpeg2ts_segment * mpeg2ts_file::next_segment(void)
{
	mpeg2ts_segment * segment = nullptr;
	if (_segment_number == _segment_count)
		return nullptr;

	segment = get_segment(_segment_number);
	_segment_number++;

	return segment;
}

int64_t mpeg2ts_file::file_size(void)
{
	return _file_size;
}

int64_t mpeg2ts_file::segment_count(void)
{
	return _segment_count;
}

mpeg2ts_segment * mpeg2ts_file::get_segment(int64_t segment_number)
{
	if ((segment_number < 0) || (segment_number >= _segment_count))
		return nullptr;

	uint64_t offset = TS_PACKET_SIZE * segment_number;
	set_file_pointer64(_ts_file, offset, &offset, FILE_BEGIN);

	uint32_t bytes2read = TS_PACKET_SIZE;
	uint32_t bytes_read = 0;
	read_file(_ts_file, _ts_buffer, bytes2read, &bytes_read, NULL);

	mpeg2ts_segment * segment = new mpeg2ts_segment();
	segment->initialize(_ts_buffer, segment_number);
	return segment;
}