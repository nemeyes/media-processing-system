#include "dk_multiple_media_buffering.h"
#include "dk_video_buffer.h"
#include "dk_audio_buffer.h"
#include <dk_auto_lock.h>
#include <map>

std::map<std::string, debuggerking::video_buffer*> _vbuffers;
std::map<std::string, debuggerking::audio_buffer*> _abuffers;

debuggerking::multiple_media_buffering::multiple_media_buffering(void)
{
	::InitializeCriticalSection(&_vmutex);
	::InitializeCriticalSection(&_amutex);
}

debuggerking::multiple_media_buffering::~multiple_media_buffering(void)
{
	{
		auto_lock lock(&_vmutex);
		std::map<std::string, video_buffer*>::iterator iter;
		for (iter = _vbuffers.begin(); iter != _vbuffers.end(); iter++)
		{
			video_buffer * vbuffer = iter->second;
			delete vbuffer;
		}
		_vbuffers.clear();
	}

	{
		auto_lock lock(&_amutex);
		std::map<std::string, audio_buffer*>::iterator iter;
		for (iter = _abuffers.begin(); iter != _abuffers.end(); iter++)
		{
			audio_buffer * abuffer = iter->second;
			delete abuffer;
		}
		_abuffers.clear();
	}

	::DeleteCriticalSection(&_vmutex);
	::DeleteCriticalSection(&_amutex);
}

#if defined(WITH_SINGLETON)
debuggerking::multiple_media_buffering & debuggerking::multiple_media_buffering::instance(void)
{
	static multiple_media_buffering _instance;
	return _instance;
}
#endif

int32_t debuggerking::multiple_media_buffering::create(const char * id)
{
	if (!id || strlen(id) < 1)
		return multiple_media_buffering::err_code_t::fail;

	{
		auto_lock lock(&_vmutex);
		video_buffer * vbuffer = new video_buffer();
		_vbuffers.insert(std::make_pair(id, vbuffer));
	}

	{
		auto_lock lock(&_amutex);
		audio_buffer * abuffer = new audio_buffer();
		_abuffers.insert(std::make_pair(id, abuffer));
	}

	return multiple_media_buffering::err_code_t::success;
}

int32_t debuggerking::multiple_media_buffering::destroy(const char * id)
{
	if (!id || strlen(id) < 1)
		return multiple_media_buffering::err_code_t::fail;

	{
		video_buffer * vbuffer = nullptr;
		auto_lock lock(&_vmutex);
		std::map<std::string, video_buffer*>::iterator iter;
		iter = _vbuffers.find(id);
		if (iter != _vbuffers.end())
		{
			vbuffer = iter->second;
			delete vbuffer;
			_vbuffers.erase(iter);
		}
	}

	{
		audio_buffer * abuffer = nullptr;
		auto_lock lock(&_amutex);
		std::map<std::string, audio_buffer*>::iterator iter;
		iter = _abuffers.find(id);
		if (iter != _abuffers.end())
		{
			abuffer = iter->second;
			delete abuffer;
			_abuffers.erase(iter);
		}
	}

	return multiple_media_buffering::err_code_t::success;
}

int32_t debuggerking::multiple_media_buffering::push_video(const char * id, const uint8_t * data, size_t size, long long timestamp)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->push(data, size, timestamp);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::pop_video(const char * id, uint8_t * data, size_t & size, long long & timestamp)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->pop(data, size, timestamp);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_video_submedia_type(const char * id, int32_t mt)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->set_submedia_type(mt);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_vps(const char * id, uint8_t * vps, size_t size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->set_vps(vps, size);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_sps(const char * id, uint8_t * sps, size_t size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->set_sps(sps, size);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_pps(const char * id, uint8_t * pps, size_t size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->set_pps(pps, size);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_video_width(const char * id, int32_t width)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->set_width(width);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_video_height(const char * id, int32_t height)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->set_height(height);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_video_submedia_type(const char * id, int32_t & mt)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_submedia_type(mt);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_vps(const char * id, uint8_t * vps, size_t & size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_vps(vps, size);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_sps(const char * id, uint8_t * sps, size_t & size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_sps(sps, size);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_pps(const char * id, uint8_t * pps, size_t & size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_pps(pps, size);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_video_width(const char * id, int32_t & width)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_width(width);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_video_height(const char * id, int32_t & height)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_height(height);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

const uint8_t * debuggerking::multiple_media_buffering::get_vps(const char * id, size_t & size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_vps(size);
	}
	else
	{
		return nullptr;
	}
}

const uint8_t * debuggerking::multiple_media_buffering::get_sps(const char * id, size_t & size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_sps(size);
	}
	else
	{
		return nullptr;
	}
}

const uint8_t * debuggerking::multiple_media_buffering::get_pps(const char * id, size_t & size)
{
	video_buffer * vbuffer = get_video_buffer(id);
	if (vbuffer)
	{
		return vbuffer->get_pps(size);
	}
	else
	{
		return nullptr;
	}
}

int32_t debuggerking::multiple_media_buffering::push_audio(const char * id, const uint8_t * data, size_t size, long long timestamp)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->push(data, size, timestamp);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::pop_audio(const char * id, uint8_t * data, size_t & size, long long & timestamp)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->pop(data, size, timestamp);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_audio_submedia_type(const char * id, int32_t mt)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->set_submedia_type(mt);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_configstr(const char * id, uint8_t * configstr, size_t size)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->set_configstr(configstr, size);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_audio_samplerate(const char * id, int32_t samplerate)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->set_samplerate(samplerate);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_audio_bitdepth(const char * id, int32_t bitdepth)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->set_bitdepth(bitdepth);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::set_audio_channels(const char * id, int32_t channels)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->set_channels(channels);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_audio_submedia_type(const char * id, int32_t & mt)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->get_submedia_type(mt);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_configstr(const char * id, uint8_t * configstr, size_t & size)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->get_configstr(configstr, size);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_audio_samplerate(const char * id, int32_t & samplerate)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->get_samplerate(samplerate);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_audio_bitdepth(const char * id, int32_t & bitdepth)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->get_bitdepth(bitdepth);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

int32_t debuggerking::multiple_media_buffering::get_audio_channels(const char * id, int32_t & channels)
{
	audio_buffer * abuffer = get_audio_buffer(id);
	if (abuffer)
	{
		return abuffer->get_channels(channels);
	}
	else
	{
		return multiple_media_buffering::err_code_t::fail;
	}
}

debuggerking::video_buffer * debuggerking::multiple_media_buffering::get_video_buffer(const char * id)
{
	if (!id || strlen(id) < 1)
		return nullptr;

	auto_lock lock(&_vmutex);
	video_buffer * buffer = nullptr;
	std::map<std::string, video_buffer*>::iterator iter;
	iter = _vbuffers.find(id);
	if (iter != _vbuffers.end())
	{
		buffer = iter->second;
	}
	return buffer;
}

debuggerking::audio_buffer * debuggerking::multiple_media_buffering::get_audio_buffer(const char * id)
{
	if (!id || strlen(id) < 1)
		return nullptr;

	auto_lock lock(&_amutex);
	audio_buffer * buffer = nullptr;
	std::map<std::string, audio_buffer*>::iterator iter;
	iter = _abuffers.find(id);
	if (iter != _abuffers.end())
	{
		buffer = iter->second;
	}
	return buffer;
}