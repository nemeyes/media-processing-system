#include "dk_ff_tsmuxer.h"
#include "ffmpeg_tsmuxer.h"

debuggerking::ff_tsmuxer::_configuration_t::_video_configuration_t::_video_configuration_t(void)
	: stream_index(0)
	, width(0)
	, height(0)
	, bitrate(0)
	, fps(30)
	, extradata_size(0)
{
	memset(extradata, 0x00, sizeof(extradata));
	memset(file_path, 0x00, sizeof(file_path));
}

debuggerking::ff_tsmuxer::_configuration_t::_video_configuration_t::_video_configuration_t(const debuggerking::ff_tsmuxer::_configuration_t::_video_configuration_t & clone)
{
	stream_index = clone.stream_index;
	width = clone.width;
	height = clone.height;
	bitrate = clone.bitrate;
	fps = clone.fps;
	bitrate = clone.bitrate;
	if (clone.extradata_size>0)
	{
		extradata_size = clone.extradata_size;
		memcpy(extradata, clone.extradata, sizeof(extradata_size));
	}
	else
	{
		extradata_size = 0;
		memset(extradata, 0x00, sizeof(extradata));
	}

	if (strlen(clone.file_path) > 0)
	{
		strncpy_s(file_path, clone.file_path, sizeof(file_path));
	}
}

debuggerking::ff_tsmuxer::_configuration_t::_video_configuration_t debuggerking::ff_tsmuxer::_configuration_t::_video_configuration_t::operator = (const debuggerking::ff_tsmuxer::_configuration_t::_video_configuration_t & clone)
{
	stream_index = clone.stream_index;
	width = clone.width;
	height = clone.height;
	bitrate = clone.bitrate;
	fps = clone.fps;
	bitrate = clone.bitrate;
	if (clone.extradata_size>0)
	{
		extradata_size = clone.extradata_size;
		memcpy(extradata, clone.extradata, sizeof(extradata_size));
	}
	else
	{
		extradata_size = 0;
		memset(extradata, 0x00, sizeof(extradata));
	}

	if (strlen(clone.file_path) > 0)
	{
		strncpy_s(file_path, clone.file_path, sizeof(file_path));
	}
	return (*this);
}


debuggerking::ff_tsmuxer::_configuration_t::_configuration_t(void)
{
}

debuggerking::ff_tsmuxer::_configuration_t::_configuration_t(const debuggerking::ff_tsmuxer::_configuration_t & clone)
{
	vconfig = clone.vconfig;
}

debuggerking::ff_tsmuxer::_configuration_t debuggerking::ff_tsmuxer::_configuration_t::operator = (const debuggerking::ff_tsmuxer::_configuration_t & clone)
{
	vconfig = clone.vconfig;
	return (*this);
}

debuggerking::ff_tsmuxer::ff_tsmuxer(void)
	: _ts_file(INVALID_HANDLE_VALUE)
{
	_core = new ffmpeg_tsmuxer(this);
}

debuggerking::ff_tsmuxer::~ff_tsmuxer(void)
{
	if (_core)
	{
		delete _core;
		_core = 0;
	}
}

int32_t debuggerking::ff_tsmuxer::initialize(configuration_t * config)
{
	if (strlen(config->vconfig.file_path) > 0)
	{
		_ts_file = ::CreateFileA(config->vconfig.file_path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	return _core->initialize(config);
}

int32_t debuggerking::ff_tsmuxer::release(void)
{
	int32_t status =  _core->release();
	if (_ts_file != NULL && _ts_file != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_ts_file);
		_ts_file = INVALID_HANDLE_VALUE;
	}
	return status;
}

debuggerking::ff_tsmuxer::tsmuxer_state debuggerking::ff_tsmuxer::state(void)
{
	return _core->state();
}

int32_t debuggerking::ff_tsmuxer::put_video_stream(const uint8_t * buffer, size_t nb, int64_t timestamp, bool keyframe)
{
	return _core->put_video_stream(buffer, nb, timestamp, keyframe);
}

int32_t debuggerking::ff_tsmuxer::after_demuxing_callback(uint8_t * ts, size_t stream_size)
{
	DWORD nbytes = 0;
	if (_ts_file != NULL && _ts_file != INVALID_HANDLE_VALUE && stream_size>0)
	{
		uint32_t bytes2write = stream_size;
		uint32_t bytes_written = 0;
		do
		{
			uint32_t nb_write = 0;
			::WriteFile(_ts_file, ts, bytes2write, (LPDWORD)&nb_write, 0);
			bytes_written += nb_write;
			if (bytes2write == bytes_written)
				break;
		} while (1);
	}
	return ff_tsmuxer::err_code_t::success;
}