#include "record_module.h"
#include <boost/date_time/local_time/local_time.hpp>

record_module::record_module(const char * storage, const char * uuid)
	: _sps_size(0)
	, _pps_size(0)
	, _change_sps(false)
	, _change_pps(false)
	, _recv_idr(false)
	, _write_index(0)
	, _read_index(0)
	, _vbuffer(nullptr)
	, _last_end_time(0)
{
	memset(_sps, 0x00, sizeof(_sps));
	memset(_pps, 0x00, sizeof(_pps));

	char folder[MAX_PATH] = { 0 };
	char filepath[MAX_PATH] = { 0 };

	_snprintf_s(folder, MAX_PATH, "%s%s", storage, uuid);
	if (::GetFileAttributesA(folder) == INVALID_FILE_ATTRIBUTES)
		::CreateDirectoryA(folder, NULL);

	long long elapsed_millsec = record_module::get_elapsed_msec_from_epoch();
	_snprintf_s(filepath, MAX_PATH, "%s%s\\%lld.dat", storage, uuid, elapsed_millsec);
	_file = ::CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file == INVALID_HANDLE_VALUE)
	{
		DWORD err = ::GetLastError();
	}
}

record_module::record_module(const char * filepath)
	: _sps_size(0)
	, _pps_size(0)
	, _change_sps(false)
	, _change_pps(false)
	, _recv_idr(false)
	, _write_index(0)
	, _read_index(0)
	, _vbuffer(nullptr)
	, _last_end_time(0)
{
	_vbuffer = new dk_video_buffer();
	_file = ::CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file == INVALID_HANDLE_VALUE)
	{
		DWORD err = ::GetLastError();
	}


	uint8_t spspps[260] = { 0 };
	bool sps_found = false;
	bool pps_found = false;

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	uint32_t spspps_index = 2 * sizeof(long long);//skip file information header
	record_module::set_file_position(_file, spspps_index, FILE_BEGIN);
	do
	{
		uint8_t nalu_type = 0;
		long long nalu_timestamp = 0;
		uint32_t nalu_size = 0;


		/*
		[file information    ][nalu information                                    ]
		[8byte     ][8byte   ][1byte frame type][8byte    ][4byte    ][nalu-size byte]
		[start-time][end-time][keyframe        ][timestamp][nalu-size][nalu          ]
		*/

		buf = (void*)&nalu_type;
		bytes_to_read = sizeof(nalu_type);
		bytes_read = 0;
		::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
		spspps_index += bytes_read;

		buf = (void*)&nalu_timestamp;
		bytes_to_read = sizeof(nalu_timestamp);
		bytes_read = 0;
		::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
		spspps_index += bytes_read;

		buf = (void*)&nalu_size;
		bytes_to_read = sizeof(nalu_size);
		bytes_read = 0;
		::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
		spspps_index += bytes_read;

		if (dk_record_module::nalu_type(nalu_type) == dk_record_module::nalu_type_sps)
		{
			memset(spspps, 0x00, sizeof(spspps));
			buf = (void*)spspps;
			bytes_to_read = nalu_size;
			bytes_read = 0;
			::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
			spspps_index += bytes_read;
			set_sps(spspps, nalu_size);
			sps_found = true;
		}
		else if (dk_record_module::nalu_type(nalu_type) == dk_record_module::nalu_type_pps)
		{
			memset(spspps, 0x00, sizeof(spspps));
			buf = (void*)spspps;
			bytes_to_read = nalu_size;
			bytes_read = 0;
			::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
			spspps_index += bytes_read;
			set_pps(spspps, nalu_size);
			pps_found = true;
		}
		else
		{
			record_module::set_file_position(_file, spspps_index + nalu_size, FILE_BEGIN);
		}

		if (sps_found && pps_found)
			break;

	} while (1);
}

record_module::~record_module(void)
{
	//long long end_time = record_module::get_elapsed_msec_from_epoch();
	write_header_time(-1, _last_end_time);
	::CloseHandle(_file);

	if (_vbuffer)
		delete _vbuffer;
	_vbuffer = nullptr;
}

long long record_module::get_file_size(void)
{
	if (_file == NULL || _file == INVALID_HANDLE_VALUE)
		return 0;
	LARGE_INTEGER filesize = { 0 };
	::GetFileSizeEx(_file, &filesize);
	long long estimated_filesize = 0;
	estimated_filesize = filesize.HighPart;
	estimated_filesize <<= 32;
	estimated_filesize |= filesize.LowPart;
	return estimated_filesize;
}

bool record_module::is_occupied(void)
{
	long long stime = 0;
	long long etime = 0;
	read_header_time(stime, etime);
	if (etime == -1)
		return true;
	else
		return false;
}

bool record_module::is_read_end(void)
{
	long long file_size = get_file_size();
	return _read_index >= file_size;
}

void record_module::get_start_end_time(long long & start_time, long long & end_time)
{
	read_header_time(start_time, end_time);
}

long long record_module::get_start_time(void)
{
	long long stime = 0;
	long long etime = 0;
	read_header_time(stime, etime);
	return stime;
}

long long record_module::get_end_time(void)
{
	long long stime = 0;
	long long etime = 0;
	read_header_time(stime, etime);
	return etime;
}

void record_module::write(uint8_t * nalu, size_t nalu_size, long long timestamp)
{
	if (_file == NULL || _file == INVALID_HANDLE_VALUE)
		return;
	long long current_time = record_module::get_elapsed_msec_from_epoch();

	long long file_size = get_file_size();
	if (file_size == 0)
	{
		write_header_time(current_time, -1);
		_write_index += 2 * sizeof(long long);//skip file information header
	}

	uint8_t * data = nalu + 4;
	size_t data_size = nalu_size - 4;

	size_t saved_sps_size = 0;
	unsigned char * saved_sps = get_sps(saved_sps_size);
	size_t saved_pps_size = 0;
	unsigned char * saved_pps = get_pps(saved_pps_size);

	bool is_sps = record_module::is_sps(data[0] & 0x1F);
	bool is_pps = record_module::is_pps(data[0] & 0x1F);
	bool is_idr = record_module::is_idr(data[0] & 0x1F);
	if (is_sps)
	{
		if (saved_sps_size < 1 || !saved_sps)
		{
			//write_bitstream(dk_record_module::nalu_type_sps, data, data_size, current_time);
			set_sps(data, data_size);
			_recv_idr = false;
		}
		else
		{
			if (memcmp(saved_sps, data, saved_sps_size))
			{
				//write_bitstream(dk_record_module::nalu_type_sps, data, data_size, current_time);
				set_sps(data, data_size);
				_recv_idr = false;
			}
		}
	}
	else if (is_pps)
	{
		if (saved_pps_size < 1 || !saved_pps)
		{
			//write_bitstream(dk_record_module::nalu_type_pps, data, data_size, current_time);
			set_pps(data, data_size);
			_recv_idr = false;
		}
		else
		{
			if (memcmp(saved_pps, data, saved_pps_size))
			{
				//write_bitstream(dk_record_module::nalu_type_pps, data, data_size, current_time);
				set_pps(data, data_size);
				_recv_idr = false;
			}
		}
	}
	else if (is_idr)
	{
		if (!_recv_idr)
			_recv_idr = true;

		saved_sps = get_sps(saved_sps_size);
		saved_pps = get_pps(saved_pps_size);
		if((saved_sps && saved_sps_size>0) && (saved_pps && saved_pps_size>0))
		{
			write_bitstream(dk_record_module::nalu_type_sps, saved_sps, saved_sps_size, current_time);
			write_bitstream(dk_record_module::nalu_type_pps, saved_pps, saved_pps_size, current_time);
		}
		write_bitstream(dk_record_module::nalu_type_idr, data, data_size, current_time);
	}
	else if (_recv_idr)
	{
		write_bitstream(dk_record_module::nalu_type_vcl, data, data_size, current_time);
	}

	_last_end_time = current_time;
}

void record_module::seek(long long seek_timestamp)
{
	_read_index = 0;
	uint32_t seek_index = 2 * sizeof(long long);

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	uint32_t prev_idr_index = 0;
	long long prev_idr_timestamp = 0;

	uint32_t next_idr_index = 0;
	long long next_idr_timestamp = 0;

	do
	{
		long long file_size = get_file_size();
		if (seek_index >= file_size)
			break;

		record_module::set_file_position(_file, seek_index, FILE_BEGIN);

		uint8_t nalu_type = 0;
		long long nalu_timestamp = 0;
		uint32_t nalu_size = 0;

		//read frame type
		buf = (void*)&nalu_type;
		bytes_to_read = sizeof(uint8_t);
		bytes_read = 0;
		::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
		seek_index += bytes_read;

		//read timestramp
		buf = (void*)&nalu_timestamp;
		bytes_to_read = sizeof(long long);
		bytes_read = 0;
		::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
		seek_index += bytes_read;

		//read nalu size
		buf = (void*)&nalu_size;
		bytes_to_read = sizeof(uint32_t);
		bytes_read = 0;
		::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
		seek_index += bytes_read;
		seek_index += nalu_size;

		if (nalu_type == dk_record_module::nalu_type_sps)
		{
			if (nalu_timestamp >= seek_timestamp)
			{
				_read_index = seek_index - (sizeof(uint8_t) + sizeof(long long) + sizeof(uint32_t) + nalu_size);
				break;
			}
			/*else if (nalu_timestamp < seek_timestamp)
			{
				prev_idr_index = seek_index - (sizeof(uint8_t) + sizeof(long long) + sizeof(uint32_t) + nalu_size);
				prev_idr_timestamp = nalu_timestamp;
			}
			else if (nalu_timestamp > seek_timestamp)
			{
				next_idr_index = seek_index - (sizeof(uint8_t) + sizeof(long long) + sizeof(uint32_t) + nalu_size);
				next_idr_timestamp = nalu_timestamp;
				break;
			}*/
		}
	} while (1);

	/*
	if (_read_index == 0)
	{
		if ((seek_timestamp - prev_idr_timestamp) <= (next_idr_timestamp - seek_timestamp))
		{
			_read_index = prev_idr_index; 
		}
		else
		{
			_read_index = next_idr_index;
		}
	}
	*/
}

void record_module::read(dk_record_module::nalu_type & type, uint8_t * data, size_t & data_size, long long & timestamp)
{
	read_bitstream(type, data, data_size, timestamp);
	long long next_time_stamp = 0;
	read_next_bitstream_timestamp(next_time_stamp);
	timestamp = next_time_stamp - timestamp;
}

//make nalu information
void record_module::write_bitstream(dk_record_module::nalu_type naltype, uint8_t * nalu, size_t nalu_size, long long timestamp)
{
	void * buff = 0;
	unsigned long bytes_to_writes = 0;
	unsigned long bytes_writes = 0;

	record_module::set_file_position(_file, _write_index, FILE_BEGIN);
	uint8_t nalutype = naltype;

	//write nalu_type
	buff = &nalutype;
	bytes_to_writes = sizeof(uint8_t);
	bytes_writes = 0;
	::WriteFile(_file, buff, bytes_to_writes, &bytes_writes, NULL);
	_write_index += bytes_writes;

	//write timestamp
	buff = &timestamp;
	bytes_to_writes = sizeof(timestamp);
	bytes_writes = 0;
	::WriteFile(_file, buff, bytes_to_writes, &bytes_writes, NULL);
	_write_index += bytes_writes;

	//write nalu size
	uint32_t size = nalu_size;
	buff = &size;
	bytes_to_writes = sizeof(uint32_t);
	bytes_writes = 0;
	::WriteFile(_file, buff, bytes_to_writes, &bytes_writes, NULL);
	_write_index += bytes_writes;

	//write nalu
	buff = nalu;
	bytes_to_writes = nalu_size;
	bytes_writes = 0;
	::WriteFile(_file, buff, bytes_to_writes, &bytes_writes, NULL);
	_write_index += bytes_writes;
}

void record_module::write_header_time(long long start_time, long long end_time)
{
	if (_file == NULL || _file == INVALID_HANDLE_VALUE)
		return;

	record_module::set_file_position(_file, 0, FILE_BEGIN);
	if (start_time>0)
	{
		void * buff = &start_time;
		unsigned long bytes_to_writes = sizeof(long long);
		unsigned long bytes_writes = 0;
		record_module::set_file_position(_file, 0, FILE_BEGIN);
		::WriteFile(_file, buff, bytes_to_writes, &bytes_writes, NULL);
	}

	if (end_time > 0)
	{
		void * buff = &end_time;
		unsigned long bytes_to_writes = sizeof(long long);
		unsigned long bytes_writes = 0;
		record_module::set_file_position(_file, sizeof(long long), FILE_BEGIN);
		::WriteFile(_file, buff, bytes_to_writes, &bytes_writes, NULL);
	}
}

void record_module::read_header_time(long long & start_time, long long & end_time)
{
	if (_file == NULL || _file == INVALID_HANDLE_VALUE)
		return;

	record_module::set_file_position(_file, 0, FILE_BEGIN);

	long long stime;
	long long etime;

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	buf = (void*)&stime;
	bytes_to_read = sizeof(long long);
	bytes_read = 0;
	::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
	if (stime == 0)
		start_time = -1;
	else
		start_time = stime;

	buf = (void*)&etime;
	bytes_to_read = sizeof(long long);
	bytes_read = 0;
	::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
	if (etime == 0)
		end_time = -1;
	else
		end_time = etime;
}

void record_module::read_bitstream(dk_record_module::nalu_type & type, uint8_t * data, size_t & data_size, long long & timestamp)
{
	if (_read_index == 0)
		_read_index = 2 * sizeof(long long);

	record_module::set_file_position(_file, _read_index, FILE_BEGIN);

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	uint8_t nalu_type = 0;
	long long nalu_timestamp = 0;
	uint32_t nalu_size = 0;
	uint8_t * nalu = data;


	//read frame type
	buf = (void*)&nalu_type;
	bytes_to_read = sizeof(uint8_t);
	bytes_read = 0;
	::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
	_read_index += bytes_read;

	//read timestramp
	buf = (void*)&nalu_timestamp;
	bytes_to_read = sizeof(long long);
	bytes_read = 0;
	::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
	_read_index += bytes_read;

	//read nalu size
	buf = (void*)&nalu_size;
	bytes_to_read = sizeof(uint32_t);
	bytes_read = 0;
	::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
	_read_index += bytes_read;

	//read nalu size
	uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
	buf = (void*)(nalu+4);
	bytes_to_read = nalu_size;
	bytes_read = 0;
	::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
	_read_index += bytes_read;
	memmove(nalu, start_code, sizeof(start_code));

	type = dk_record_module::nalu_type(nalu_type);
	data_size = nalu_size + 4;
	timestamp = nalu_timestamp;
}

void record_module::read_next_bitstream_timestamp(long long & timestamp)
{
	record_module::set_file_position(_file, _read_index + 1, FILE_BEGIN);

	void * buf = nullptr;
	unsigned long bytes_to_read = 0;
	unsigned long bytes_read = 0;

	//read timestramp
	buf = (void*)&timestamp;
	bytes_to_read = sizeof(long long);
	bytes_read = 0;
	::ReadFile(_file, buf, bytes_to_read, &bytes_read, NULL);
}

uint8_t * record_module::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * record_module::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

long long record_module::get_elapsed_msec_from_epoch(void)
{
	boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
	boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration elapsed = current_time - epoch;
	long long elapsed_millsec = elapsed.total_milliseconds();
	return elapsed_millsec;
}

void record_module::get_time_from_elapsed_msec_from_epoch(long long elapsed_time, char * time_string, int time_string_size)
{
	boost::posix_time::time_duration elapsed = boost::posix_time::microseconds(elapsed_time);
	boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
	boost::posix_time::ptime current_time = epoch + elapsed;

	std::string tmp_time = boost::posix_time::to_simple_string(current_time);
	//strncpy_s(time_string, time_string_size, tmp_time.c_str(), (size_t)time_string_size);
	strcpy_s(time_string, time_string_size, tmp_time.c_str());
}

void record_module::set_file_position(HANDLE file, uint32_t offset, uint32_t flag)
{
	if (file != NULL && file != INVALID_HANDLE_VALUE)
	{
		::SetFilePointer(file, offset, NULL, flag);
	}
}

bool record_module::is_sps(uint8_t nal_unit_type)
{
	return nal_unit_type == 7;
}

bool record_module::is_pps(uint8_t nal_unit_type)
{
	return nal_unit_type == 8;
}

bool record_module::is_idr(uint8_t nal_unit_type)
{
	return nal_unit_type == 5;
}

bool record_module::is_vcl(uint8_t nal_unit_type)
{
	return (nal_unit_type <= 5 && nal_unit_type > 0);
}

void record_module::set_sps(uint8_t * sps, size_t sps_size)
{
	memset(_sps, 0x00, sizeof(_sps));
	memcpy(_sps, sps, sps_size);
	_sps_size = sps_size;
}

void record_module::set_pps(uint8_t * pps, size_t pps_size)
{
	memset(_pps, 0x00, sizeof(_pps));
	memcpy(_pps, pps, pps_size);
	_pps_size = pps_size;
}

void record_module::clear_sps(void)
{
	memset(_sps, 0x00, sizeof(_sps));
	_sps_size = 0;
}

void record_module::clear_pps(void)
{
	memset(_pps, 0x00, sizeof(_pps));
	_pps_size = 0;
}
