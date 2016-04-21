#ifndef _RECORD_MODULE_H_
#define _RECORD_MODULE_H_

#include "dk_record_module.h"
#include <dk_video_buffer.h>

class record_module
{
public:
	record_module(const char * storage, const char * uuid, long long timestamp);
	record_module(const char * filepath);
	virtual ~record_module(void);

	long long get_file_size(void);
	bool is_occupied(void);

	bool is_read_end(void);

	void get_start_end_time(long long & start_time, long long & end_time);
	long long get_start_time(void);
	long long get_end_time(void);

	void write(uint8_t * nalu, size_t nalu_size, long long timestamp);

	void seek(long long timestamp);
	void read(dk_record_module::nalu_type & type, uint8_t * data, size_t & data_size, long long & timestamp);

	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);

	const char * get_filename(void) { return _filename; }

private:
	void write_header_time(long long start_time, long long end_time);
	void write_bitstream(uint8_t * sps, size_t sps_size, uint8_t * pps, size_t pps_size, uint8_t * nalu, size_t nalu_size, long long timestamp);
	void write_bitstream(dk_record_module::nalu_type type, uint8_t * nalu, size_t nalu_size, long long timestamp);

	void read_header_time(long long & start_time, long long & end_time);
	void read_bitstream(dk_record_module::nalu_type & type, uint8_t * data, size_t & data_size, long long & timestamp);
	void read_next_bitstream_timestamp(long long & timestamp);

	static void set_file_position(HANDLE file, uint32_t offset, uint32_t flag);
	static long long get_elapsed_msec_from_epoch(void);
	static void get_time_from_elapsed_msec_from_epoch(long long elapsed_time, char * time_string, int time_string_size);
	static void get_time_from_elapsed_msec_from_epoch(long long elapsed_time, int & year, int & month, int & day, int & hour, int & minute, int & second);
	static bool is_sps(uint8_t nal_unit_type);
	static bool is_pps(uint8_t nal_unit_type);
	static bool is_idr(uint8_t nal_unit_type);
	static bool is_vcl(uint8_t nal_unit_type);

	void set_sps(uint8_t * sps, size_t sps_size);
	void set_pps(uint8_t * pps, size_t pps_size);
	void clear_sps(void);
	void clear_pps(void);

private:
	char _filename[MAX_PATH];

	uint8_t _sps[200];
	size_t _sps_size;
	uint8_t _pps[200];
	size_t _pps_size;
	bool _change_sps;
	bool _change_pps;
	bool _recv_idr;


	HANDLE _file;
	char * _write_buffer;
	uint32_t _write_index;
	uint32_t _read_index;

	long long _last_end_time;

};

#endif