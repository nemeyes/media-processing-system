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

/*
debuggerking::video_base::_entity_t::_entity_t(void)
	: timestamp(0)
	, mem_type(video_base::video_memory_type_t::host)
	, surface(nullptr)
	, data(nullptr)
	, data_size(0)
	, data_capacity(0)
	, pic_type(video_base::video_picture_type_t::unknown)
	, width(0)
	, height(0)
	, gen_spspps(false)
	, gen_idr(false)
	, gen_intra(false)
	, flush(false)
{
}

debuggerking::video_base::_entity_t::_entity_t(const video_base::_entity_t & clone)
{
	timestamp = clone.timestamp;
	mem_type = clone.mem_type;
	surface = clone.surface;
	data = clone.data;
	data_size = clone.data_size;
	data_capacity = clone.data_capacity;
	pic_type = clone.pic_type;
	width = clone.width;
	height = clone.height;
	gen_spspps = clone.gen_spspps;
	gen_idr = clone.gen_idr;
	gen_intra = clone.gen_intra;
	flush = clone.flush;
}

debuggerking::video_base::_entity_t debuggerking::video_base::_entity_t::operator = (const video_base::_entity_t & clone)
{
	timestamp = clone.timestamp;
	mem_type = clone.mem_type;
	surface = clone.surface;
	data = clone.data;
	data_size = clone.data_size;
	data_capacity = clone.data_capacity;
	pic_type = clone.pic_type;
	width = clone.width;
	height = clone.height;
	gen_spspps = clone.gen_spspps;
	gen_idr = clone.gen_idr;
	gen_intra = clone.gen_intra;
	flush = clone.flush;
	return (*this);
}

debuggerking::video_base::_entity_t::~_entity_t(void)
{

}
*/

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

#if 0
int32_t debuggerking::video_base::initialize_d3d11(ID3D11Device * d3d11_device, int32_t iwidth, int32_t iheight, int32_t ifps, int32_t owidth, int32_t oheight, int32_t ofps)
{
	HRESULT hr = S_OK;
	int32_t status = video_base::err_code_t::fail;

	ATL::CComPtr<ID3D11VideoContext> d3d11_video_context = NULL;
	do
	{
		hr = d3d11_device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&_d3d11_video_device);
		if (FAILED(hr))
			break;

		d3d11_device->GetImmediateContext(&_d3d11_device_context);
		hr = _d3d11_device_context->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&d3d11_video_context);
		if (FAILED(hr))
			break;

		D3D11_VIDEO_PROCESSOR_CONTENT_DESC content_desc;
		content_desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
		content_desc.InputWidth = (DWORD)iwidth;
		content_desc.InputHeight = (DWORD)iheight;
		content_desc.OutputWidth = (DWORD)owidth;
		content_desc.OutputHeight = (DWORD)oheight;
		content_desc.InputFrameRate.Numerator = ifps;
		content_desc.InputFrameRate.Denominator = 1;
		content_desc.OutputFrameRate.Numerator = ofps;
		content_desc.OutputFrameRate.Denominator = 1;
		content_desc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

		hr = _d3d11_video_device->CreateVideoProcessorEnumerator(&content_desc, &_d3d11_video_processor_enum);
		if (FAILED(hr))
			break;

		UINT flags;
		DXGI_FORMAT output_format = DXGI_FORMAT_NV12;
		hr = _d3d11_video_processor_enum->CheckVideoProcessorFormat(output_format, &flags);
		if (FAILED(hr) || (flags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT) == 0)
			break;

		DWORD index = 0;
		D3D11_VIDEO_PROCESSOR_CAPS caps = {};
		D3D11_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS conv_caps = {};

		hr = _d3d11_video_processor_enum->GetVideoProcessorCaps(&caps);
		if (FAILED(hr))
			break;

		for (DWORD i = 0; i < caps.RateConversionCapsCount; i++)
		{
			hr = _d3d11_video_processor_enum->GetVideoProcessorRateConversionCaps(i, &conv_caps);
			if (FAILED(hr))
				break;

			if ((conv_caps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BOB) != 0)
				index = i;
		}

		if (FAILED(hr))
			break;

		hr = _d3d11_video_device->CreateVideoProcessor(_d3d11_video_processor_enum, index, &_d3d11_video_processor);
		if (FAILED(hr))
			break;

		status = video_base::err_code_t::success;
	} while (0);

	return status;
}

int32_t debuggerking::video_base::release_d3d11(void)
{
	int32_t status = video_base::err_code_t::success;

	return status;
}

int32_t debuggerking::video_base::convert_d3d11_rgb32_to_nv12(ID3D11Texture2D * rgb32, ID3D11Texture2D * nv12, int32_t iwidth, int32_t iheight, int32_t owidth, int32_t oheight)
{
	int32_t status = video_base::err_code_t::fail;
	HRESULT hr = E_FAIL;

	ATL::CComPtr<ID3D11VideoProcessorInputView> input_view = NULL;
	ATL::CComPtr<ID3D11VideoProcessorOutputView> output_view = NULL;
	ATL::CComPtr<ID3D11VideoContext> video_context = NULL;
	do
	{
		hr = _d3d11_device_context->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&video_context);
		if (FAILED(hr))
			break;

#if defined(_DEBUG)
		D3D11_TEXTURE2D_DESC rgb32_desc;
		rgb32->GetDesc(&rgb32_desc);

		D3D11_TEXTURE2D_DESC nv12_desc;
		nv12->GetDesc(&nv12_desc);
#endif

		D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC output_view_desc;
		memset(&output_view_desc, 0x00, sizeof(D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC));
		output_view_desc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
		output_view_desc.Texture2D.MipSlice = 0;
		output_view_desc.Texture2DArray.MipSlice = 0;
		output_view_desc.Texture2DArray.FirstArraySlice = 0;
		hr = _d3d11_video_device->CreateVideoProcessorOutputView(nv12, _d3d11_video_processor_enum, &output_view_desc, &output_view);
		if (FAILED(hr))
			break;

		D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC input_view_desc;
		memset(&input_view_desc, 0x00, sizeof(D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC));
		input_view_desc.FourCC = 0;
		input_view_desc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
		input_view_desc.Texture2D.MipSlice = 0;
		input_view_desc.Texture2D.ArraySlice = 0;
		hr = _d3d11_video_device->CreateVideoProcessorInputView(rgb32, _d3d11_video_processor_enum, &input_view_desc, &input_view);
		if (FAILED(hr))
			break;

		video_context->VideoProcessorSetStreamFrameFormat(_d3d11_video_processor, 0, D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
		video_context->VideoProcessorSetStreamOutputRate(_d3d11_video_processor, 0, D3D11_VIDEO_PROCESSOR_OUTPUT_RATE_NORMAL, TRUE, NULL); // Output rate (repeat frames)

		RECT SRect = { 0, 0, iwidth, iheight };
		RECT DRect = { 0, 0, owidth, oheight };
		video_context->VideoProcessorSetStreamSourceRect(_d3d11_video_processor, 0, TRUE, &SRect); // Source rect
		video_context->VideoProcessorSetStreamDestRect(_d3d11_video_processor, 0, TRUE, &SRect); // Stream dest rect
		video_context->VideoProcessorSetOutputTargetRect(_d3d11_video_processor, TRUE, &DRect);

		D3D11_VIDEO_PROCESSOR_COLOR_SPACE cs = {};
		cs.YCbCr_xvYCC = 1;
		video_context->VideoProcessorSetStreamColorSpace(_d3d11_video_processor, 0, &cs);
		video_context->VideoProcessorSetOutputColorSpace(_d3d11_video_processor, &cs); // Output color space

		D3D11_VIDEO_COLOR bgcolor = {};
		bgcolor.RGBA.A = 1.0F;
		bgcolor.RGBA.R = 1.0F * static_cast<float>(GetRValue(0)) / 255.0F;
		bgcolor.RGBA.G = 1.0F * static_cast<float>(GetGValue(0)) / 255.0F;
		bgcolor.RGBA.B = 1.0F * static_cast<float>(GetBValue(0)) / 255.0F;
		video_context->VideoProcessorSetOutputBackgroundColor(_d3d11_video_processor, TRUE, &bgcolor);

		D3D11_VIDEO_PROCESSOR_STREAM d3d11_stream_data;
		ZeroMemory(&d3d11_stream_data, sizeof(D3D11_VIDEO_PROCESSOR_STREAM));
		d3d11_stream_data.Enable = TRUE;
		d3d11_stream_data.OutputIndex = 0;
		d3d11_stream_data.InputFrameOrField = 0;
		d3d11_stream_data.PastFrames = 0;
		d3d11_stream_data.FutureFrames = 0;
		d3d11_stream_data.ppPastSurfaces = NULL;
		d3d11_stream_data.ppFutureSurfaces = NULL;
		d3d11_stream_data.pInputSurface = input_view;
		d3d11_stream_data.ppPastSurfacesRight = NULL;
		d3d11_stream_data.ppFutureSurfacesRight = NULL;

		hr = video_context->VideoProcessorBlt(_d3d11_video_processor, output_view, 0, 1, &d3d11_stream_data);
		if (FAILED(hr))
			break;

		status = video_base::err_code_t::success;

	} while (0);

	return status;
}
#endif

int32_t debuggerking::video_base::convert_yv12pitch_to_nv12(uint8_t * src_y, uint8_t * src_u, uint8_t * src_v, uint8_t * dst_y, uint8_t * dst_u, int32_t width, int32_t height, uint32_t src_stride, uint32_t dst_stride)
{
	int32_t y;
	int32_t x;
	if (src_stride == 0)
		src_stride = width;
	if (dst_stride == 0)
		dst_stride = width;

	for (y = 0; y < height; y++)
	{
		memcpy(dst_y + (dst_stride*y), src_y + (src_stride*y), width);
		if (y < height / 2)
		{
			for (x = 0; x < width; x = x + 2)
			{
				dst_u[(y*dst_stride) + x] = src_u[((src_stride / 2)*y) + (x >> 1)];
				dst_u[(y*dst_stride) + (x + 1)] = src_v[((src_stride / 2)*y) + (x >> 1)];
			}
		}
	}
	return video_base::err_code_t::success;
}

int32_t debuggerking::video_base::convert_yv12pitch_to_yv12(uint8_t * src_y, uint8_t * src_u, uint8_t * src_v, uint8_t * dst_y, uint8_t * dst_u, int32_t width, int32_t height, uint32_t src_stride, uint32_t dst_stride)
{
	int32_t y;
	if (src_stride == 0)
		src_stride = width;
	if (dst_stride == 0)
		dst_stride = width;

	for (y = 0; y < height; y++)
	{
		memcpy(dst_y + (dst_stride*y), src_y + (src_stride*y), width);
		if (y < height / 2)
		{
#if 1
			memcpy(dst_u + y*(dst_stride >> 1), src_u + y*(src_stride >> 1), width >> 1);
			memcpy(dst_u + ((height*dst_stride) >> 2) + y*(dst_stride >> 1), src_v + y*(src_stride >> 1), width >> 1);
#else
			memcpy(dst_u + ((height*dst_stride) >> 2) + y*(dst_stride >> 1), src_u + y*(src_stride >> 1), width >> 1);
			memcpy(dst_u + y*(dst_stride >> 1), src_v + y*(src_stride >> 1), width >> 1);
#endif
		}
	}
	return video_base::err_code_t::success;
}

const int32_t debuggerking::video_base::next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end)
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
