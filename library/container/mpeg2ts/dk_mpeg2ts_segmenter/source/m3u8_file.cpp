#include "m3u8_file.h"

#include <dk_platform.h>
#include <dk_string_helper.h>

m3u8_file::m3u8_file(void)
	: _m3u8_file(INVALID_HANDLE_VALUE)
{

}

m3u8_file::~m3u8_file(void)
{
	release();
}

dk_mpeg2ts_segmenter::ERR_CODE m3u8_file::initialize(wchar_t * ofile_name, wchar_t * opath, int32_t duration, int32_t sequence)
{
	release();

	wchar_t ofile_path[200] = { 0 };
	_snwprintf_s(ofile_path, sizeof(ofile_path) / sizeof(wchar_t), L"%s\\%s.m3u8", opath, ofile_name);

	char * mb_ofile_path = 0;
	dk_string_helper::convert_wide2multibyte(ofile_path, &mb_ofile_path);
	_m3u8_file = open_file_write(mb_ofile_path);

	puts(_m3u8_file, "#EXTM3U");

	char tmp[100] = { 0 };
	if (duration>0)
	{
		_snprintf_s(tmp, sizeof(tmp), "EXT-X-TARGETDURATION:%d", duration);
		puts(_m3u8_file, tmp);
	}
	if (sequence > 0)
	{
		memset(tmp, 0x00, sizeof(tmp));
		_snprintf_s(tmp, sizeof(tmp), "EXT-X-MEDIA-SEQUENCE:%d", sequence);
		puts(_m3u8_file, tmp);
	}

	_duration = duration;
	_sequence = sequence;
	_counter = 1;

	if (mb_ofile_path)
	{
		free(mb_ofile_path);
		mb_ofile_path = 0;
	}


	char * mb_ofile_name = 0;
	dk_string_helper::convert_wide2multibyte(ofile_name, &mb_ofile_name);
	strcpy_s(_ofile_name, mb_ofile_name);
	if (mb_ofile_name)
		free(mb_ofile_name);

	char * mb_opath = 0;
	dk_string_helper::convert_wide2multibyte(opath, &mb_opath);
	strcpy_s(_opath, mb_opath);
	if (mb_opath)
		free(mb_opath);

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_segmenter::ERR_CODE m3u8_file::release(void)
{
	if (_m3u8_file && _m3u8_file!=INVALID_HANDLE_VALUE)
	{
		puts(_m3u8_file, "#EXT-X-ENDLIST");
		close_file(_m3u8_file);
		_m3u8_file = INVALID_HANDLE_VALUE;
	}
	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}


dk_mpeg2ts_segmenter::ERR_CODE m3u8_file::insert_media(int32_t duration)
{
	if (duration < 1)
		duration = _duration;

	char tmp[500] = { 0 };
	_snprintf_s(tmp, sizeof(tmp), "%s_%d.ts", _ofile_name, _counter);
	_counter++;
	return insert(tmp, duration, "");
}

dk_mpeg2ts_segmenter::ERR_CODE m3u8_file::insert(char * resource, int32_t duration, char * description)
{
	if (!_m3u8_file || _m3u8_file == INVALID_HANDLE_VALUE)
		return dk_mpeg2ts_segmenter::ERR_CODE_FAIL;

	char tmp[100] = { 0 };
	if (description && strlen(description)>0)
		_snprintf_s(tmp, sizeof(tmp), "#EXTINF:%d, %s", duration, description);
	else
		_snprintf_s(tmp, sizeof(tmp), "#EXTINF:%d, %s", duration, "no desc");
	puts(_m3u8_file, tmp);
	puts(_m3u8_file, resource);

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

bool m3u8_file::puts(HANDLE file, char * b)
{
	uint32_t bytes2write = strlen(b);
	uint32_t bytes_written = 0;
	do
	{
		uint32_t nb_write = 0;
		write_file(file, b, bytes2write, &nb_write, NULL);
		bytes_written += nb_write;
		if (bytes2write == bytes_written)
			break;
	} while (1);


	bytes2write = 1;
	bytes_written = 0;
	write_file(file, "\n", bytes2write, &bytes_written, NULL);

	return true;
}