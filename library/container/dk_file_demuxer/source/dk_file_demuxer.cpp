#include "dk_file_demuxer.h"
#include "ff_demuxer.h"

dk_file_demuxer::dk_file_demuxer(void)
{
	_core = new ff_demuxer(this);
}

dk_file_demuxer::~dk_file_demuxer(void)
{
	if (_core)
	{
		delete _core;
		_core = nullptr;
	}
}

dk_file_demuxer::err_code dk_file_demuxer::play(const char * filepath)
{
	return _core->play(filepath);
}

dk_file_demuxer::err_code dk_file_demuxer::stop(void)
{
	return _core->stop();
}

uint8_t * dk_file_demuxer::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * dk_file_demuxer::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

void dk_file_demuxer::set_sps(uint8_t * sps, size_t sps_size)
{
	memset(_sps, 0x00, sizeof(_sps));
	memcpy(_sps, sps, sps_size);
	_sps_size = sps_size;
}

void dk_file_demuxer::set_pps(uint8_t * pps, size_t pps_size)
{
	memset(_pps, 0x00, sizeof(_pps));
	memcpy(_pps, pps, pps_size);
	_pps_size = pps_size;
}