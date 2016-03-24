#include "dk_record_module.h"
#include "record_module.h"

dk_record_module::dk_record_module(const char * storage, const char * uuid)
{
	_core = new record_module(storage, uuid);
}

dk_record_module::dk_record_module(const char * filepath)
{
	_core = new record_module(filepath);
}

dk_record_module::~dk_record_module(void)
{
	if (_core)
		delete _core;
	_core = nullptr;
}

long long dk_record_module::get_file_size(void)
{
	return _core->get_file_size();
}

bool dk_record_module::is_occupied(void)
{
	return _core->is_occupied();
}

bool dk_record_module::is_read_end(void)
{
	return _core->is_read_end();
}

void dk_record_module::get_start_end_time(long long & start_time, long long & end_time)
{
	return _core->get_start_end_time(start_time, end_time);
}

long long dk_record_module::get_start_time(void)
{
	return _core->get_start_time();
}

long long dk_record_module::get_end_time(void)
{
	return _core->get_end_time();
}

void dk_record_module::write(uint8_t * nalu, size_t nalu_size, long long timestamp)
{
	_core->write(nalu, nalu_size, timestamp);
}

void dk_record_module::read(dk_record_module::nalu_type & type, uint8_t * data, size_t & data_size, long long & timestamp)
{
	_core->read(type, data, data_size, timestamp);
}

uint8_t * dk_record_module::get_sps(size_t & sps_size)
{
	memset(_sps, 0x00, sizeof(_sps));
	uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
	uint8_t * sps_without_start_code = _core->get_sps(sps_size);
	memmove(_sps, start_code, sizeof(start_code));
	memmove(_sps + sizeof(start_code), sps_without_start_code, sps_size);
	_sps_size = sps_size + sizeof(start_code);
	sps_size = _sps_size;
	return _sps;
}

uint8_t * dk_record_module::get_pps(size_t & pps_size)
{
	memset(_pps, 0x00, sizeof(_pps));
	uint8_t start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
	uint8_t * pps_without_start_code = _core->get_pps(pps_size);
	memmove(_pps, start_code, sizeof(start_code));
	memmove(_pps + sizeof(start_code), pps_without_start_code, pps_size);
	_pps_size = pps_size + sizeof(start_code);
	pps_size = _pps_size;
	return _pps;
}