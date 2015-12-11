#include "dk_mpeg2ts_segmenter.h"

#include "shlwapi.h"
#include "mpeg2ts_file.h"
#include "mpeg2ts_segment.h"

#include <vector>

#include <dk_fileio.h>
#include <dk_string_helper.h>

dk_mpeg2ts_segmenter::dk_mpeg2ts_segmenter(void)
{

}

dk_mpeg2ts_segmenter::~dk_mpeg2ts_segmenter(void)
{

}

dk_mpeg2ts_segmenter::ERR_CODE dk_mpeg2ts_segmenter::initialize(wchar_t * ifile_path, wchar_t * ofile_name, wchar_t * opath, wchar_t * stream_id, int32_t duration)
{
	if (!PathFileExists(ifile_path))
		return dk_mpeg2ts_segmenter::ERR_CODE_FAIL;

	//if (!ofile_name || wcslen(ofile_name)<1)
	//	return dk_mpeg2ts_segmenter::ERR_CODE_FAIL;

	if (!PathIsDirectory(opath))
		return dk_mpeg2ts_segmenter::ERR_CODE_FAIL;

	if (duration<1)
		return dk_mpeg2ts_segmenter::ERR_CODE_FAIL;

	wcscpy_s(_ifile_path, ifile_path);
	wcscpy_s(_ofile_name, ofile_name);
	wcscpy_s(_opath, opath);



	wcscpy_s(_stream_id, stream_id);
	_duration = duration;

	_ts_file = new mpeg2ts_file();
	_ts_file->initialize(ifile_path);

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_segmenter::ERR_CODE dk_mpeg2ts_segmenter::release(void)
{
	if (_ts_file)
	{
		_ts_file->release();
		delete _ts_file;
		_ts_file = nullptr;
	}
	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

dk_mpeg2ts_segmenter::ERR_CODE dk_mpeg2ts_segmenter::segment(void)
{
	int32_t file_count = 1;
	int32_t last_segment_copied = 0;
	int32_t last_association_table = 0;
	int32_t temp_last_association_table = 0;
	int32_t segment_count = 0;
	std::vector<int16_t> program_pids;
	double absolute_time = 0;
	double reference_time = 0;

	if (!_ts_file)
		return dk_mpeg2ts_segmenter::ERR_CODE_FAIL;

	mpeg2ts_segment * segment = nullptr;
	while (segment = _ts_file->next_segment())
	{
		if ((segment_count - 1) == temp_last_association_table)
		{
			std::vector<int16_t>::iterator iter;
			iter = std::find(program_pids.begin(), program_pids.end(), segment->ts_packet_header()->pid);
			if (iter != program_pids.end())
			{
				last_association_table = temp_last_association_table;
			}
		}

		if (segment->ts_packet_header()->pid == 0)
		{
			temp_last_association_table = segment_count;
			int32_t number_of_programs = (segment->program_association_section()->section_length - 9) / 4;
			for (int32_t index = 0; index < number_of_programs; index++)
			{
				if (segment->program_association_section()->program_numbers[index] != 0)
				{
					uint16_t program_map_pid = segment->program_association_section()->program_map_pids[index];
					program_pids.push_back(program_map_pid);
				}
			}
		}

		if (segment->pts_defined())
		{
			if (!reference_time)
				reference_time = segment->pts_to_second();

			absolute_time = segment->pts_to_second() - reference_time;
			if (absolute_time >= _duration)
			{
				reference_time = segment->pts_to_second();
				//m3u8file
				wchar_t ofile_path[500] = { 0 };
				_snwprintf_s(ofile_path, sizeof(ofile_path) / sizeof(wchar_t), L"%s\\%s_%d.ts", _opath, _ofile_name, file_count);
				copy_segments_to_file(_ifile_path, ofile_path, last_segment_copied, last_association_table);
				last_segment_copied = last_association_table;
				file_count++;
			}
		}

		segment_count++;
	}

	if (segment_count > last_segment_copied)
	{
		if (std::round(absolute_time) > 0)
		{
			//m3u9file
			wchar_t ofile_path[500] = { 0 };
			_snwprintf_s(ofile_path, sizeof(ofile_path) / sizeof(wchar_t), L"%s\\%s_%d.ts", _opath, _ofile_name, file_count);
			copy_segments_to_file(_ifile_path, ofile_path, last_segment_copied, last_association_table);
		}
		else
		{
			wchar_t ofile_path[500] = { 0 };
			_snwprintf_s(ofile_path, sizeof(ofile_path) / sizeof(wchar_t), L"%s\\%s_%d.ts", _opath, _ofile_name, file_count - 1);
			append_segments_to_file(_ifile_path, ofile_path, last_segment_copied, last_association_table);
		}
	}

	return dk_mpeg2ts_segmenter::ERR_CODE_SUCCESS;
}

void dk_mpeg2ts_segmenter::copy_segments_to_file(wchar_t * ifile_path, wchar_t * ofile_path, int64_t from, int64_t to)
{
	char * mb_ifile_path = 0;
	char * mb_ofile_path = 0;
	dk_string_helper::convert_wide2multibyte(ifile_path, &mb_ifile_path);
	dk_string_helper::convert_wide2multibyte(ofile_path, &mb_ofile_path);

	HANDLE ifile = open_file(mb_ifile_path);
	HANDLE ofile = open_file_write(mb_ofile_path);


	uint64_t offset = from;
	set_file_pointer64(ifile, offset, &offset, FILE_BEGIN);

	uint32_t bytes2read = to - from;
	uint32_t bytes_read = 0;
	uint8_t * buffer = (uint8_t*)malloc(bytes2read);
	do
	{
		uint32_t nb_read = 0;
		read_file(ifile, buffer, bytes2read, &nb_read, 0);
		bytes_read += nb_read;
		if (bytes2read == bytes_read)
			break;
	} while (1);

	uint32_t bytes2write = bytes2read;
	uint32_t bytes_written = 0;
	do
	{
		uint32_t nb_write = 0;
		write_file(ofile, buffer, bytes2write, &nb_write, NULL);
		bytes_written += nb_write;
		if (bytes2write == bytes_written)
			break;
	} while (1);

	if (buffer)
	{
		free(buffer);
		buffer = 0;
	}

	close_file(ifile);
	close_file(ofile);

	if (mb_ifile_path)
	{
		free(mb_ifile_path);
		mb_ifile_path = 0;
	}
	if (mb_ofile_path)
	{
		free(mb_ofile_path);
		mb_ofile_path = 0;
	}
}

void dk_mpeg2ts_segmenter::append_segments_to_file(wchar_t * ifile_path, wchar_t * ofile_path, int64_t from, int64_t to)
{
	char * mb_ifile_path = 0;
	char * mb_ofile_path = 0;
	dk_string_helper::convert_wide2multibyte(ifile_path, &mb_ifile_path);
	dk_string_helper::convert_wide2multibyte(ofile_path, &mb_ofile_path);

	HANDLE ifile = open_file(mb_ifile_path);
	HANDLE ofile = open_file_write(mb_ofile_path);

	uint64_t offset = from;
	set_file_pointer64(ifile, offset, &offset, FILE_BEGIN);

	DWORD file_size;
	get_file_size(ofile, &file_size);

	offset = file_size;
	set_file_pointer64(ofile, offset, &offset, FILE_BEGIN);

	uint32_t bytes2read = to - from;
	uint32_t bytes_read = 0;
	uint8_t * buffer = (uint8_t*)malloc(bytes2read);
	do
	{
		uint32_t nb_read = 0;
		read_file(ifile, buffer, bytes2read, &nb_read, 0);
		bytes_read += nb_read;
		if (bytes2read == bytes_read)
			break;
	} while (1);

	uint32_t bytes2write = bytes2read;
	uint32_t bytes_written = 0;
	do
	{
		uint32_t nb_write = 0;
		write_file(ofile, buffer, bytes2write, &nb_write, NULL);
		bytes_written += nb_write;
		if (bytes2write == bytes_written)
			break;
	} while (1);

	if (buffer)
	{
		free(buffer);
		buffer = 0;
	}

	close_file(ifile);
	close_file(ofile);

	if (mb_ifile_path)
	{
		free(mb_ifile_path);
		mb_ifile_path = 0;
	}
	if (mb_ofile_path)
	{
		free(mb_ofile_path);
		mb_ofile_path = 0;
	}
}