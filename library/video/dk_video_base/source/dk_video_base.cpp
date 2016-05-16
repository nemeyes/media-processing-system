#include "dk_video_base.h"
#include <dk_circular_buffer.h>
#include <dk_auto_lock.h>

debuggerking::video_base::_configuration_t::_configuration_t(void)
	: mode(video_base::mode_t::none)
	, buffer_size(video_base::max_media_value_t::max_video_size)
{

}

debuggerking::video_base::_configuration_t::_configuration_t(const video_base::_configuration_t & clone)
{
	mode = clone.mode;
}

debuggerking::video_base::_configuration_t & debuggerking::video_base::_configuration_t::operator=(const video_base::_configuration_t & clone)
{
	mode = clone.mode;
	return (*this);
}


debuggerking::video_base::video_base(void)
	: _config(nullptr)
	, _extradata_size(0)
	, _vps_size(0)
	, _sps_size(0)
	, _pps_size(0)
{
	memset(_extradata, 0x00, sizeof(_extradata));
	memset(_vps, 0x00, sizeof(_vps));
	memset(_sps, 0x00, sizeof(_sps));
	memset(_pps, 0x00, sizeof(_pps));
}

debuggerking::video_base::~video_base(void)
{

}

int32_t debuggerking::video_base::initialize(video_base::configuration_t * config)
{
	_config = config;
	if (_config->mode == video_base::mode_t::sync)
	{
		::InitializeCriticalSection(&_mutex);
		_queue = circular_buffer_t::create(_config->buffer_size);
		_root = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
		init(_root);
	}
	return video_base::err_code_t::success;
}

int32_t debuggerking::video_base::release(void)
{
	if (_config->mode == video_base::mode_t::sync)
	{
		video_base::buffer_t * buffer = _root->next;
		while (buffer)
		{
			buffer->prev->next = buffer->next;
			free(buffer);
			buffer = nullptr;
		}
		free(_root);
		_root = nullptr;

		circular_buffer_t::destroy(_queue);
		::DeleteCriticalSection(&_mutex);
	}
	_config->mode = video_base::mode_t::none;
	return video_base::err_code_t::success;
}

int32_t debuggerking::video_base::push(uint8_t * bs, size_t size, long long timestamp)
{
	if (_config->mode != video_base::mode_t::sync)
		return video_base::err_code_t::unsupported_function;

	int32_t status = video_base::err_code_t::success;
	dk_auto_lock lock(&_mutex);
	if (bs && size > 0)
	{
		buffer_t * buffer = _root;
		buffer->amount = _config->buffer_size;
		//move to tail
		do
		{
			if (!buffer->next)
				break;
			buffer = buffer->next;
		} while (1);

		buffer->next = static_cast<buffer_t*>(malloc(sizeof(buffer_t)));
		init(buffer->next);
		buffer->next->prev = buffer;
		buffer = buffer->next;

		buffer->amount = size;
		buffer->timestamp = timestamp;
		int32_t result = circular_buffer_t::write(_queue, bs, buffer->amount);
		if (result == -1)
		{
			if (buffer->prev)
				buffer->prev->next = nullptr;
			free(buffer);
			buffer = nullptr;
			status = video_base::err_code_t::fail;
		}
	}
	return status;
}

int32_t debuggerking::video_base::pop(uint8_t * bs, size_t & size, long long & timestamp)
{
	if (_config->mode != video_base::mode_t::sync)
		return video_base::err_code_t::unsupported_function;

	int32_t status = video_base::err_code_t::success;
	size = 0;
	dk_auto_lock lock(&_mutex);
	buffer_t * buffer = _root->next;
	if (buffer)
	{
		buffer->prev->next = buffer->next;
		int32_t result = circular_buffer_t::read(_queue, bs, buffer->amount);
		if (result == -1)
		{
			status = video_base::err_code_t::fail;
		}
		else
		{
			size = buffer->amount;
			timestamp = buffer->timestamp;
		}
		free(buffer);
		buffer = nullptr;
	}
	return status;
}

int32_t debuggerking::video_base::init(video_base::buffer_t * buffer)
{
	buffer->timestamp = 0;
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return video_base::err_code_t::success;
}

void debuggerking::video_base::set_extradata(uint8_t * extradata, size_t extradata_size)
{
	_extradata_size = extradata_size;
	memmove(_extradata, extradata, _extradata_size);
}

void debuggerking::video_base::set_vps(uint8_t * vps, size_t vps_size)
{
	memmove(_vps, vps, vps_size);
}

void debuggerking::video_base::set_sps(uint8_t * sps, size_t sps_size)
{
	memmove(_sps, sps, sps_size);
}

void debuggerking::video_base::set_pps(uint8_t * pps, size_t pps_size)
{
	memmove(_pps, pps, pps_size);
}

uint8_t * debuggerking::video_base::get_extradata(size_t & extradata_size)
{
	extradata_size = _extradata_size;
	return _extradata;
}

uint8_t * debuggerking::video_base::get_vps(size_t & vps_size)
{
	vps_size = _vps_size;
	return _vps;
}

uint8_t * debuggerking::video_base::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * debuggerking::video_base::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

const int debuggerking::video_base::next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end)
{
	int i;
	*nal_start = 0;
	*nal_end = 0;

	i = 0;
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0 || bitstream[i + 3] != 0x01))
	{
		i++;
		if (i + 4 >= size)
			return 0;
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
		i++;

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
		return 0;/* error, should never happen */

	i += 3;
	*nal_start = i;
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01))
	{
		i++;
		if (i + 3 >= size)
		{
			*nal_end = size;
			return -1;
		}
	}

	*nal_end = i;
	return (*nal_end - *nal_start);
}

debuggerking::video_decoder::_configuration_t::_configuration_t(void)
	: mem_type(video_decoder::video_memory_type_t::host)
	, device(nullptr)
	, iwidth(0)
	, iheight(0)
	, istride(0)
	, owidth(0)
	, oheight(0)
	, ostride(0)
	, sarwidth(0)
	, sarheight(0)
	, codec(video_decoder::video_submedia_type_t::h264)
	, cs(video_decoder::video_submedia_type_t::yv12)
{
}

debuggerking::video_decoder::_configuration_t::_configuration_t(const video_decoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	device = clone.device;
	iwidth = clone.iwidth;
	iheight = clone.iheight;
	istride = clone.istride;
	owidth = clone.owidth;
	oheight = clone.oheight;
	ostride = clone.ostride;
	sarwidth = clone.sarwidth;
	sarheight = clone.sarheight;
	codec = clone.codec;
	cs = clone.cs;
}

debuggerking::video_decoder::_configuration_t & debuggerking::video_decoder::_configuration_t::operator=(const video_decoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	device = clone.device;
	iwidth = clone.iwidth;
	iheight = clone.iheight;
	istride = clone.istride;
	owidth = clone.owidth;
	oheight = clone.oheight;
	ostride = clone.ostride;
	sarwidth = clone.sarwidth;
	sarheight = clone.sarheight;
	codec = clone.codec;
	cs = clone.cs;
	return (*this);
}

debuggerking::video_decoder::video_decoder(void)
{

}

debuggerking::video_decoder::~video_decoder(void)
{

}

int32_t debuggerking::video_decoder::initialize_decoder(void * config)
{
	return video_base::initialize(static_cast<video_base::configuration_t*>(config));
}

int32_t debuggerking::video_decoder::release_decoder(void)
{
	return video_base::release();
}

int32_t debuggerking::video_decoder::decode(video_decoder::entity_t * bitstream, video_decoder::entity_t * decoded)
{
	return err_code_t::not_implemented;
}

int32_t debuggerking::video_decoder::decode(video_decoder::entity_t * bitstream)
{
	return err_code_t::not_implemented;
}

int32_t debuggerking::video_decoder::get_queued_data(video_decoder::entity_t * bitstream)
{
	return err_code_t::not_implemented;
}


debuggerking::video_encoder::_configuration_t::_configuration_t(void)
	: mem_type(video_encoder::video_memory_type_t::host)
	, device(nullptr)
	, cs(video_encoder::video_submedia_type_t::nv12)
	, width(3840)
	, height(2160)
	, codec(video_encoder::video_submedia_type_t::h264_hp)
	, bitrate(8000000)
	, fps(30)
	, keyframe_interval(2)
	, numb(0)
	, entropy_coding_mode(video_encoder::entropy_coding_mode_t::cabac)
{}

debuggerking::video_encoder::_configuration_t::_configuration_t(const video_encoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	device = clone.device;
	cs = clone.cs;
	width = clone.width;
	height = clone.height;
	codec = clone.codec;
	bitrate = clone.bitrate;
	fps = clone.fps;
	keyframe_interval = clone.keyframe_interval;
	numb = clone.numb;
	entropy_coding_mode = clone.entropy_coding_mode;
}

debuggerking::video_encoder::_configuration_t & debuggerking::video_encoder::_configuration_t::operator=(const video_encoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	device = clone.device;
	cs = clone.cs;
	width = clone.width;
	height = clone.height;
	codec = clone.codec;
	bitrate = clone.bitrate;
	fps = clone.fps;
	keyframe_interval = clone.keyframe_interval;
	numb = clone.numb;
	entropy_coding_mode = clone.entropy_coding_mode;
	return (*this);
}

debuggerking::video_encoder::video_encoder(void)
{}

debuggerking::video_encoder::~video_encoder(void)
{}

debuggerking::video_encoder::encoder_state debuggerking::video_encoder::state(void)
{
	return video_encoder::encoder_state_none;
}

int32_t debuggerking::video_encoder::initialize_encoder(void * config)
{
	return video_base::initialize(static_cast<video_encoder::configuration_t*>(config));
}

int32_t debuggerking::video_encoder::release_encoder(void)
{
	return video_encoder::release();
}

int32_t debuggerking::video_encoder::encode(video_encoder::entity_t * input, video_encoder::entity_t * bitstream)
{
	return video_encoder::err_code_t::not_implemented;
}

int32_t debuggerking::video_encoder::encode(video_encoder::entity_t * input)
{
	return video_encoder::err_code_t::not_implemented;
}

int32_t debuggerking::video_encoder::get_queued_data(video_encoder::entity_t * bitstream)
{
	return video_encoder::err_code_t::not_implemented;
}

debuggerking::video_renderer::_configuration_t::_configuration_t(void)
	: width(0)
	, height(0)
	, hwnd_full(NULL)
	, hwnd(NULL)
{}

debuggerking::video_renderer::_configuration_t::_configuration_t(const video_renderer::_configuration_t & clone)
{
	width = clone.width;
	height = clone.height;
	hwnd_full = clone.hwnd_full;
	hwnd = clone.hwnd;
}

debuggerking::video_renderer::_configuration_t & debuggerking::video_renderer::_configuration_t::operator = (const video_renderer::_configuration_t & clone)
{
	width = clone.width;
	height = clone.height;
	hwnd_full = clone.hwnd_full;
	hwnd = clone.hwnd;
	return (*this);
}

debuggerking::video_renderer::video_renderer(void)
{

}

debuggerking::video_renderer::~video_renderer(void)
{

}

int32_t debuggerking::video_renderer::initialize_renderer(void * config)
{
	return video_renderer::err_code_t::not_implemented;
}

int32_t debuggerking::video_renderer::release_renderer(void)
{
	return video_renderer::err_code_t::not_implemented;
}

int32_t debuggerking::video_renderer::render(video_renderer::entity_t * decoded)
{
	return video_renderer::err_code_t::not_implemented;
}
