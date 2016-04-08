#ifndef _RECORD_MODULE_SEEKER_H_
#define _RECORD_MODULE_SEEKER_H_

#include <windows.h>
#include <process.h>
#include <cstdint>

class dk_record_module;
class record_module_seeker
{
public:
	record_module_seeker(void);
	virtual ~record_module_seeker(void);

	bool seek(const char * single_media_source_path, long long seek_time);
	void read(uint8_t * data, size_t & data_size, long long & timestamp);

	uint8_t * get_sps(size_t & sps_size);
	uint8_t * get_pps(size_t & pps_size);

private:
	dk_record_module * _record_module;

	char _single_media_source_path[260];
};












#endif