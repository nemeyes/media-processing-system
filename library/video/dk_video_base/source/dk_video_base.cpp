#include "dk_video_base.h"
#include <dk_circular_buffer.h>
#include <dk_auto_lock.h>

#define MAX_VIDEO_SIZE	1024*1024*2

dk_video_base::dk_video_base(bool use_builtin_queue)
	: _use_builtin_queue(use_builtin_queue)
	, _extradata_size(0)
	, _vps_size(0)
	, _sps_size(0)
	, _pps_size(0)
{
	memset(_extradata, 0x00, sizeof(_extradata));
	memset(_vps, 0x00, sizeof(_vps));
	memset(_sps, 0x00, sizeof(_sps));
	memset(_pps, 0x00, sizeof(_pps));

	if (_use_builtin_queue)
	{
		::InitializeCriticalSection(&_mutex);
		_vqueue = dk_circular_buffer_create(MAX_VIDEO_SIZE);
		_root = static_cast<vbuffer_t*>(malloc(sizeof(vbuffer_t)));
		init(_root);
	}
}

dk_video_base::~dk_video_base(void)
{
	if (_use_builtin_queue)
	{
		vbuffer_t * vbuffer = _root->next;
		while (vbuffer)
		{
			vbuffer->prev->next = vbuffer->next;
			free(vbuffer);
			vbuffer = nullptr;
		}
		free(_root);
		_root = nullptr;

		dk_circular_buffer_destroy(_vqueue);
		::DeleteCriticalSection(&_mutex);
	}
}

dk_video_base::err_code dk_video_base::push(uint8_t * bs, size_t size, long long timestamp)
{
	if (!_use_builtin_queue)
		return dk_video_base::err_code_unsupported_function;

	dk_video_base::err_code status = dk_video_base::err_code_success;
	dk_auto_lock lock(&_mutex);
	if (bs && size > 0)
	{
		vbuffer_t * vbuffer = _root;
		vbuffer->amount = MAX_VIDEO_SIZE;
		//move to tail
		do
		{
			if (!vbuffer->next)
				break;
			vbuffer = vbuffer->next;
		} while (1);

		vbuffer->next = static_cast<vbuffer_t*>(malloc(sizeof(vbuffer_t)));
		init(vbuffer->next);
		vbuffer->next->prev = vbuffer;
		vbuffer = vbuffer->next;

		vbuffer->amount = size;
		vbuffer->timestamp = timestamp;
		int32_t result = dk_circular_buffer_write(_vqueue, bs, vbuffer->amount);
		if (result == -1)
		{
			if (vbuffer->prev)
				vbuffer->prev->next = nullptr;
			free(vbuffer);
			vbuffer = nullptr;
			status = dk_video_base::err_code_fail;
		}
		//else
		//{
		//	wchar_t debug[500];
		//	_snwprintf_s(debug, sizeof(debug), L">>push audio data[%zu]\n", abuffer->amount);
		//	OutputDebugString(debug);
		//}
	}
	return status;
}

dk_video_base::err_code dk_video_base::pop(uint8_t * bs, size_t & size, long long & timestamp)
{
	if (!_use_builtin_queue)
		return dk_video_base::err_code_unsupported_function;

	dk_video_base::err_code status = dk_video_base::err_code_success;
	size = 0;
	dk_auto_lock lock(&_mutex);
	vbuffer_t * vbuffer = _root->next;
	if (vbuffer)
	{
		vbuffer->prev->next = vbuffer->next;
		int32_t result = dk_circular_buffer_read(_vqueue, bs, vbuffer->amount);
		if (result == -1)
		{
			status = dk_video_base::err_code_fail;
		}
		else
		{
			size = vbuffer->amount;
			timestamp = vbuffer->timestamp;
		}
		free(vbuffer);
		vbuffer = nullptr;
	}
	return status;
}

dk_video_base::err_code dk_video_base::init(vbuffer_t * buffer)
{
	buffer->timestamp = 0;
	buffer->amount = 0;
	buffer->next = nullptr;
	buffer->prev = nullptr;
	return dk_video_base::err_code_success;
}

void dk_video_base::set_extradata(uint8_t * extradata, size_t extradata_size)
{
	memmove(_extradata, extradata, extradata_size);
}

void dk_video_base::set_vps(uint8_t * vps, size_t vps_size)
{
	memmove(_vps, vps, vps_size);
}

void dk_video_base::set_sps(uint8_t * sps, size_t sps_size)
{
	memmove(_sps, sps, sps_size);
}

void dk_video_base::set_pps(uint8_t * pps, size_t pps_size)
{
	memmove(_pps, pps, pps_size);
}

uint8_t * dk_video_base::get_extradata(size_t & extradata_size)
{
	extradata_size = _extradata_size;
	return _extradata;
}

uint8_t * dk_video_base::get_vps(size_t & vps_size)
{
	vps_size = _vps_size;
	return _vps;
}

uint8_t * dk_video_base::get_sps(size_t & sps_size)
{
	sps_size = _sps_size;
	return _sps;
}

uint8_t * dk_video_base::get_pps(size_t & pps_size)
{
	pps_size = _pps_size;
	return _pps;
}

const int dk_video_base::next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end)
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
/*
typedef struct EXP_CLASS _configuration_t
{
	dk_video_encoder::memory_type mem_type;
	void * d3d_device;

	int32_t input_width;
	int32_t input_height;
	int32_t input_stride;
	int32_t output_width;
	int32_t output_height;
	int32_t output_stride;
	int32_t sar_width;
	int32_t sar_height;
	dk_video_decoder::submedia_type_t input_smt;
	dk_video_decoder::submedia_type_t output_smt;

} configuration_t;
*/

dk_video_decoder::_configuration_t::_configuration_t(void)
	: mem_type(dk_video_decoder::memory_type_host)
	, d3d_device(nullptr)
	, iwidth(0)
	, iheight(0)
	, istride(0)
	, owidth(0)
	, oheight(0)
	, ostride(0)
	, sarwidth(0)
	, sarheight(0)
	, codec(dk_video_decoder::submedia_type_t::submedia_type_h264)
	, cs(dk_video_decoder::submedia_type_t::submedia_type_yv12)
{

}

dk_video_decoder::_configuration_t::_configuration_t(const dk_video_decoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	d3d_device = clone.d3d_device;
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

dk_video_decoder::_configuration_t & dk_video_decoder::_configuration_t::operator=(const dk_video_decoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	d3d_device = clone.d3d_device;
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

dk_video_decoder::dk_video_decoder(void)
{

}

dk_video_decoder::~dk_video_decoder(void)
{

}

dk_video_decoder::err_code dk_video_decoder::initialize_decoder(void * config)
{
	return err_code_success;
}

dk_video_decoder::err_code dk_video_decoder::release_decoder(void)
{
	return err_code_success;
}

dk_video_decoder::err_code dk_video_decoder::decode(dk_video_entity_t * bitstream, dk_video_entity_t * decoded)
{
	return err_code_not_implemented;
}

dk_video_decoder::err_code dk_video_decoder::decode(dk_video_entity_t * bitstream)
{
	return err_code_not_implemented;
}

dk_video_decoder::err_code dk_video_decoder::get_queued_data(dk_video_entity_t * bitstream)
{
	return err_code_not_implemented;
}


dk_video_encoder::_configuration_t::_configuration_t(void)
	: mem_type(dk_video_encoder::memory_type_host)
	, d3d_device(nullptr)
	, cs(dk_video_encoder::submedia_type_t::submedia_type_nv12)
	, width(3840)
	, height(2160)
	, codec(dk_video_encoder::submedia_type_t::submedia_type_h264_hp)
	, bitrate(8000000)
	, fps(30)
	, keyframe_interval(2)
	, numb(0)
	, entropy_coding_mode(dk_video_encoder::entropy_coding_mode_t::entropy_coding_mode_cabac)
{}

dk_video_encoder::_configuration_t::_configuration_t(const dk_video_encoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	d3d_device = clone.d3d_device;
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

dk_video_encoder::_configuration_t & dk_video_encoder::_configuration_t::operator=(const dk_video_encoder::_configuration_t & clone)
{
	mem_type = clone.mem_type;
	d3d_device = clone.d3d_device;
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

dk_video_encoder::dk_video_encoder(bool use_builtin_queue)
	: dk_video_base(use_builtin_queue)
{

}

dk_video_encoder::~dk_video_encoder(void)
{

}

dk_video_encoder::encoder_state dk_video_encoder::state(void)
{
	return dk_video_encoder::encoder_state_none;
}

dk_video_encoder::err_code dk_video_encoder::initialize_encoder(void * config)
{
	return err_code_success;
}

dk_video_encoder::err_code dk_video_encoder::release_encoder(void)
{
	return err_code_success;
}

dk_video_encoder::err_code dk_video_encoder::encode(dk_video_entity_t * input, dk_video_entity_t * bitstream)
{
	return err_code_not_implemented;
}

dk_video_encoder::err_code dk_video_encoder::encode(dk_video_entity_t * input)
{
	return err_code_not_implemented;
}

dk_video_encoder::err_code dk_video_encoder::get_queued_data(dk_video_entity_t * bitstream)
{
	return err_code_not_implemented;
}

dk_video_encoder::err_code dk_video_encoder::encode_async(dk_video_encoder::dk_video_entity_t * input)
{
	return dk_video_encoder::err_code_not_implemented;
}

dk_video_encoder::err_code dk_video_encoder::check_encoding_finish(void)
{
	return dk_video_encoder::err_code_encoding_under_processing;
}

uint8_t * dk_video_encoder::spspps(uint32_t & spspps_size)
{
	spspps_size = _spspps_size;
	return &_spspps[0];
}

dk_video_renderer::dk_video_renderer(void)
{

}

dk_video_renderer::~dk_video_renderer(void)
{

}

dk_video_renderer::err_code dk_video_renderer::initialize_renderer(void * config)
{
	return err_code_not_implemented;
}

dk_video_renderer::err_code dk_video_renderer::release_renderer(void)
{
	return err_code_not_implemented;
}

dk_video_renderer::err_code dk_video_renderer::render(dk_video_entity_t * decoded)
{
	return err_code_not_implemented;
}
