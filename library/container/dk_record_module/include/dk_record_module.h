#ifndef _DK_RECORD_MODULE_H_
#define _DK_RECORD_MODULE_H_

#include <cstdint>

#if defined(_WIN32)
# include <windows.h>
# if defined(EXPORT_LIB)
#  define EXP_DLL __declspec(dllexport)
# else
#  define EXP_DLL __declspec(dllimport)
# endif
#else
# define EXP_DLL
#endif

class record_module;
class EXP_DLL dk_record_module
{
public:
	typedef enum _nalu_type
	{
		nalu_type_sps = 0,
		nalu_type_pps,
		nalu_type_idr,
		nalu_type_vcl,
	} nalu_type;

	dk_record_module(const char * storage, const char * uuid); //constructor for writing
	dk_record_module(const char * filepath); //constructor for reading
	virtual ~dk_record_module(void);

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
private:
	record_module * _core;

	uint8_t _sps[200];
	uint8_t _pps[200];
	size_t _sps_size;
	size_t _pps_size;
};




/*
class dk_mpeg2ts_recorder : public dk_ff_mpeg2ts_muxer
{
public:
dk_mpeg2ts_recorder(const char * storage, const char * uuid);
virtual ~dk_mpeg2ts_recorder(void);

long long get_file_size(void);
dk_mpeg2ts_recorder::ERR_CODE recv_ts_stream_callback(uint8_t * ts, size_t stream_size);

//static unsigned long get_elapsed_utc_time(void);

private:
HANDLE _file;

};

*/








#endif