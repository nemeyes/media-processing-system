#ifndef _RECORDER_MODULE_H_
#define _RECORDER_MODULE_H_

#include "dk_recorder_module.h"

namespace debuggerking
{
	class module_core
	{
	public:
		module_core(recorder_module * front, long long chunk_size_bytes);
		virtual ~module_core(void);


		bool seek4w(const char * single_recorder_file_path, long long timestamp); //for writing
		bool seek4r(const char * single_recorder_file_path, long long seek_time); //for reading

		void write(uint8_t * data, size_t data_size, long long timestamp);
		void read(uint8_t * data, size_t & data_size, long long & interval, long long & timestamp);

		static void get_years(const char * content_path, const char * uuid, int years[], int capacity, int & size);
		static void get_months(const char * content_path, const char * uuid, int year, int months[], int capacity, int & size);
		static void get_days(const char * content_path, const char * uuid, int year, int month, int days[], int capacity, int & size);
		static void get_hours(const char * content_path, const char * uuid, int year, int month, int day, int hours[], int capacity, int & size);
		static void get_minutes(const char * content_path, const char * uuid, int year, int month, int day, int hour, int minutes[], int capacity, int & size);
		static void get_seconds(const char * content_path, const char * uuid, int year, int month, int day, int hour, int minute, int seconds[], int capacity, int & size);

		const uint8_t * get_vps(size_t & vps_size) const;
		const uint8_t * get_sps(size_t & sps_size) const;
		const uint8_t * get_pps(size_t & pps_size) const;

		void set_vps(const uint8_t * vps, size_t vps_size);
		void set_sps(const uint8_t * sps, size_t sps_size);
		void set_pps(const uint8_t * pps, size_t pps_size);

	private:
		HANDLE open_recorder_file(const char * recorder_file_path);
		HANDLE open_recorder_file(const char * storage, const char * uuid, long long timestamp);
		void close_recorder_file(HANDLE f);

		bool is_occupied(HANDLE f);
		bool is_read_end(HANDLE f);
		void get_start_end_time(HANDLE f, long long & start_time, long long & end_time);
		long long get_start_time(HANDLE f);
		long long get_end_time(HANDLE f);

		bool write(HANDLE f, uint8_t * nalu, size_t nalu_size, long long timestamp);
		bool seek(HANDLE f, long long seek_timestamp);
		void read(HANDLE f, uint8_t & nalu_type, uint8_t * data, size_t & data_size, long long & interval, long long & timestamp);

		void write_bitstream(HANDLE f, const uint8_t * sps, size_t sps_size, const uint8_t * pps, size_t pps_size, uint8_t * nalu, size_t size, long long timestamp);
		void write_bitstream(HANDLE f, uint8_t nalu_type, uint8_t * nalu, size_t nalu_size, long long timestamp);
		void write_header_time(HANDLE f, long long start_time, long long end_time);


		void read_header_time(HANDLE f, long long & start_time, long long & end_time);
		void read_bitstream(HANDLE f, uint8_t  & nalu_type, uint8_t * data, size_t & data_size, long long & timestamp);
		void read_next_bitstream_timestamp(HANDLE f, long long & timestamp);
	private:
		recorder_module * _front;
		char _single_recorder_file_path[MAX_PATH];
		long long _chunk_size_bytes;

		HANDLE _record_file;

		uint8_t _vps[200];
		uint8_t _sps[200];
		uint8_t _pps[200];
		size_t _vps_size;
		size_t _sps_size;
		size_t _pps_size;

		uint8_t * _wbuffer;
		size_t _wbuffer_size;
		bool _wrecv_idr;
		long long _windex;
		long long _last_w_timestamp;

		uint32_t _rindex;
		long long _last_r_timestamp;
		char _last_accessed_r_file_name[MAX_PATH];
	};
};

#endif