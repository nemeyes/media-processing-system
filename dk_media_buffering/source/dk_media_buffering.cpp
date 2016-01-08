#include "dk_media_buffering.h"
#include "dk_video_buffer.h"
#include "dk_audio_buffer.h"

dk_media_buffering::dk_media_buffering(void)
{
	_vbuffer = new dk_video_buffer();
	_abuffer = new dk_audio_buffer();
}

dk_media_buffering::~dk_media_buffering(void)
{
	if (_vbuffer)
	{
		delete _vbuffer;
		_vbuffer = nullptr;
	}
	if (_abuffer)
	{
		delete _abuffer;
		_abuffer = nullptr;
	}
}

dk_media_buffering & dk_media_buffering::instance(void)
{
	static dk_media_buffering _instance;
	return _instance;
}

dk_media_buffering::ERR_CODE dk_media_buffering::push_video(const uint8_t * data, size_t size, long long timestamp)
{
	return _vbuffer->push(data, size, timestamp);
}

dk_media_buffering::ERR_CODE dk_media_buffering::pop_video(uint8_t * data, size_t & size, long long & timestamp)
{
	return _vbuffer->pop(data, size, timestamp);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_video_submedia_type(dk_media_buffering::VIDEO_SUBMEDIA_TYPE mt)
{
	return _vbuffer->set_submedia_type(mt);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_vps(uint8_t * vps, size_t size)
{
	return _vbuffer->set_vps(vps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_sps(uint8_t * sps, size_t size)
{
	return _vbuffer->set_sps(sps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_pps(uint8_t * pps, size_t size)
{
	return _vbuffer->set_pps(pps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_video_width(int32_t width)
{
	return _vbuffer->set_width(width);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_video_height(int32_t height)
{
	return _vbuffer->set_height(height);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_video_submedia_type(dk_media_buffering::VIDEO_SUBMEDIA_TYPE & mt)
{
	return _vbuffer->get_submedia_type(mt);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_vps(uint8_t * vps, size_t & size)
{
	return _vbuffer->get_vps(vps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_sps(uint8_t * sps, size_t & size)
{
	return _vbuffer->get_sps(sps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_pps(uint8_t * pps, size_t & size)
{
	return _vbuffer->get_pps(pps, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_video_width(int32_t & width)
{
	return _vbuffer->get_width(width);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_video_height(int32_t & height)
{
	return _vbuffer->get_height(height);
}

dk_media_buffering::ERR_CODE dk_media_buffering::push_audio(const uint8_t * data, size_t size, long long timestamp)
{
	return _abuffer->push(data, size, timestamp);
}

dk_media_buffering::ERR_CODE dk_media_buffering::pop_audio(uint8_t * data, size_t & size, long long & timestamp)
{
	return _abuffer->pop(data, size, timestamp);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_audio_submedia_type(dk_media_buffering::AUDIO_SUBMEDIA_TYPE mt)
{
	return _abuffer->set_submedia_type(mt);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_configstr(uint8_t * configstr, size_t size)
{
	return _abuffer->set_configstr(configstr, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_audio_samplerate(int32_t samplerate)
{
	return _abuffer->set_samplerate(samplerate);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_audio_bitdepth(int32_t bitdepth)
{
	return _abuffer->set_bitdepth(bitdepth);
}

dk_media_buffering::ERR_CODE dk_media_buffering::set_audio_channels(int32_t channels)
{
	return _abuffer->set_channels(channels);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_audio_submedia_type(dk_media_buffering::AUDIO_SUBMEDIA_TYPE & mt)
{
	return _abuffer->get_submedia_type(mt);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_configstr(uint8_t * configstr, size_t & size)
{
	return _abuffer->get_configstr(configstr, size);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_audio_samplerate(int32_t & samplerate)
{
	return _abuffer->get_samplerate(samplerate);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_audio_bitdepth(int32_t & bitdepth)
{
	return _abuffer->get_bitdepth(bitdepth);
}

dk_media_buffering::ERR_CODE dk_media_buffering::get_audio_channels(int32_t & channels)
{
	return _abuffer->get_channels(channels);
}