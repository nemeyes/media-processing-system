#include "x264_encoder.h"

x264_encoder::x264_encoder(dk_x264_encoder * front)
	: _front(front)
	, _encoder(0)
	, _frame_count(0)
	, _state(dk_x264_encoder::encoder_state_none)
{
}

x264_encoder::~x264_encoder(void)
{

	_state = dk_x264_encoder::encoder_state_none;
}

dk_x264_encoder::encoder_state x264_encoder::state(void)
{
	return _state;
}

dk_x264_encoder::err_code x264_encoder::initialize_encoder(dk_x264_encoder::configuration_t * config)
{
	if ((_state != dk_x264_encoder::encoder_state_none) && (_state != dk_x264_encoder::encoder_state_released))
		return dk_x264_encoder::err_code_fail;

	release_encoder();
	_state = dk_x264_encoder::encoder_state_initializing;

	_frame_count = 0;
	_config = config;
	memset(&_param, 0x00, sizeof(_param));

	if (x264_param_default_preset(&_param, x264_preset_names[_config->preset], x264_tune_names[_config->tune])<0)
		goto fail;

	switch (_config->cs)
	{
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_i420:
		_param.i_csp = X264_CSP_I420;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_yv12:
		_param.i_csp = X264_CSP_YV12;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_nv12:
		_param.i_csp = X264_CSP_NV12;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_rgb24:
		_param.i_csp = X264_CSP_BGR;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_rgb32:
		_param.i_csp = X264_CSP_BGRA;
		break;
	}

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	size_t number_of_threads = si.dwNumberOfProcessors;
	_param.i_threads = number_of_threads;
	_param.i_lookahead_threads = X264_THREADS_AUTO;
	_param.i_width = _config->width;
	_param.i_height = _config->height;
	_param.i_fps_num = _config->fps;
	_param.i_fps_den = 1;
	_param.i_slice_count = 1;

	_param.i_keyint_max = _config->keyframe_interval*_config->fps;
	_param.i_keyint_min = _config->keyframe_interval*_config->fps;

	_param.i_bframe = _config->numb;

	_param.b_vfr_input = 0;
	_param.b_repeat_headers = 1;
	_param.b_annexb = 1;

	int32_t codec_profile = 0;
	switch (_config->codec)
	{
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_h264:
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_h264_bp:
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_h264_ep:
		codec_profile = 0;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_h264_hp:
		codec_profile = 2;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_h264_mp:
		codec_profile = 1;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_h264_h10p:
		codec_profile = 3;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_h264_h422p:
		codec_profile = 4;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_h264_h444p:
		codec_profile = 5;
		break;
	}

	if (x264_param_apply_profile(&_param, x264_profile_names[codec_profile])<0)
		goto fail;

	if (x264_picture_alloc(&_pic_in, _param.i_csp, _param.i_width, _param.i_height) < 0)
		goto fail;

	_encoder = x264_encoder_open(&_param);
	if (!_encoder)
		goto fail2;

	_state = dk_x264_encoder::encoder_state_initialized;
	return dk_x264_encoder::err_code_success;
fail:
	_state = dk_x264_encoder::encoder_state_none;
	return dk_x264_encoder::err_code_fail;
fail2:
	x264_picture_clean(&_pic_in);
	_state = dk_x264_encoder::encoder_state_none;
	return dk_x264_encoder::err_code_fail;
}

dk_x264_encoder::err_code x264_encoder::release_encoder(void)
{
	if ((_state != dk_x264_encoder::encoder_state_none) && (_state != dk_x264_encoder::encoder_state_initialized) && (_state != dk_x264_encoder::encoder_state_encoded))
		return dk_x264_encoder::err_code_fail;
	_state = dk_x264_encoder::encoder_state_releasing;

	if (_encoder)
	{
		//flush encoding buffer
		int frame_size = 0;
		x264_nal_t * nal = 0;
		int i_nal = 0;
		while (x264_encoder_delayed_frames(_encoder))
		{
			frame_size = x264_encoder_encode(_encoder, &nal, &i_nal, 0, &_pic_out);
			if (frame_size < 0)
				goto done;
		}

done:
		_frame_count = 0;
		x264_encoder_close(_encoder);
		_encoder = 0;
		x264_picture_clean(&_pic_in);
	}

	_state = dk_x264_encoder::encoder_state_released;
	return dk_x264_encoder::err_code_success;
}

dk_x264_encoder::err_code x264_encoder::encode(dk_video_encoder::dk_video_entity_t * input, dk_video_encoder::dk_video_entity_t * bitstream)
{
	if ((_state != dk_x264_encoder::encoder_state_initialized) && (_state != dk_x264_encoder::encoder_state_encoded))
		return dk_x264_encoder::err_code_success;

	_state = dk_x264_encoder::encoder_state_encoding;

	if (!_encoder)
		return dk_x264_encoder::err_code_fail;
	if (input->mem_type!=dk_x264_encoder::memory_type_host)
		return dk_x264_encoder::err_code_fail;

	unsigned char * y_plane = 0;
	unsigned char * u_plane = 0;
	unsigned char * v_plane = 0;
	unsigned char * uv_plane = 0;

	unsigned int luma_volume = _param.i_width*_param.i_height;
	unsigned int chroma_volume = luma_volume >> 1;
	unsigned int cbcr_volume = luma_volume >> 2;
	switch (_config->cs)
	{
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_i420:
		y_plane = input->data;
		u_plane = y_plane + luma_volume;
		v_plane = u_plane + cbcr_volume;

		memcpy(_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_pic_in.img.plane[1], u_plane, cbcr_volume);
		memcpy(_pic_in.img.plane[2], v_plane, cbcr_volume);
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_yv12:
		y_plane = input->data;
		v_plane = y_plane + luma_volume;
		u_plane = v_plane + cbcr_volume;

		memcpy(_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_pic_in.img.plane[1], v_plane, cbcr_volume);
		memcpy(_pic_in.img.plane[2], u_plane, cbcr_volume);
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_nv12:
		y_plane = input->data;
		uv_plane = y_plane + luma_volume;
		memcpy(_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_pic_in.img.plane[0], uv_plane, chroma_volume);
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_rgb24:
		_param.i_csp = X264_CSP_BGR;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_rgb32:
		_param.i_csp = X264_CSP_BGRA;
		break;
	}

	x264_nal_t * nal = 0;
	int nal_index = 0;

	_pic_in.i_pts = _frame_count;
	int frame_size = x264_encoder_encode(_encoder, &nal, &nal_index, &_pic_in, &_pic_out);
	if (frame_size < 0)
	{
		bitstream->data_size = 0;
		goto fail;
	}
	else if (frame_size)
	{
		bitstream->data_size = frame_size;
		memcpy(bitstream->data, nal->p_payload, bitstream->data_size);
	}
	else
	{
		bitstream->data_size = frame_size;
	}
	_state = dk_x264_encoder::encoder_state_encoded;
	return dk_x264_encoder::err_code_success;

fail:
	_frame_count = 0;
	x264_encoder_close(_encoder);
	_encoder = 0;
	x264_picture_clean(&_pic_in);

	_state = dk_x264_encoder::encoder_state_none;
	return dk_x264_encoder::err_code_fail;
}

dk_x264_encoder::err_code x264_encoder::encode(dk_video_encoder::dk_video_entity_t * input)
{
	if ((_state != dk_x264_encoder::encoder_state_initialized) && (_state != dk_x264_encoder::encoder_state_encoded))
		return dk_x264_encoder::err_code_success;

	_state = dk_x264_encoder::encoder_state_encoding;

	if (!_encoder)
		return dk_x264_encoder::err_code_fail;
	if (input->mem_type != dk_x264_encoder::memory_type_host)
		return dk_x264_encoder::err_code_fail;

	unsigned char * y_plane = 0;
	unsigned char * u_plane = 0;
	unsigned char * v_plane = 0;
	unsigned char * uv_plane = 0;

	unsigned int luma_volume = _param.i_width*_param.i_height;
	unsigned int chroma_volume = luma_volume >> 1;
	unsigned int cbcr_volume = luma_volume >> 2;
	switch (_config->cs)
	{
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_i420:
		y_plane = input->data;
		u_plane = y_plane + luma_volume;
		v_plane = u_plane + cbcr_volume;

		memcpy(_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_pic_in.img.plane[1], u_plane, cbcr_volume);
		memcpy(_pic_in.img.plane[2], v_plane, cbcr_volume);
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_yv12:
		y_plane = input->data;
		v_plane = y_plane + luma_volume;
		u_plane = v_plane + cbcr_volume;

		memcpy(_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_pic_in.img.plane[1], v_plane, cbcr_volume);
		memcpy(_pic_in.img.plane[2], u_plane, cbcr_volume);
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_nv12:
		y_plane = input->data;
		uv_plane = y_plane + luma_volume;
		memcpy(_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_pic_in.img.plane[0], uv_plane, chroma_volume);
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_rgb24:
		_param.i_csp = X264_CSP_BGR;
		break;
	case dk_x264_encoder::x264_submedia_type_t::submedia_type_rgb32:
		_param.i_csp = X264_CSP_BGRA;
		break;
	}

	x264_nal_t * nal = 0;
	int nal_index = 0;

	_pic_in.i_pts = _frame_count;
	int frame_size = x264_encoder_encode(_encoder, &nal, &nal_index, &_pic_in, &_pic_out);
	if (frame_size < 0)
	{
		goto fail;
	}
	else
	{
		if (frame_size > 0)
		{
			uint8_t * bs = nal->p_payload;
			uint8_t * begin = bs;
			uint8_t * end = bs;
			size_t bs_size = frame_size;
			while (begin < bs + bs_size)
			{
				int nalu_begin, nalu_end;
				int nalu_size = dk_x264_encoder::next_nalu(begin, (bs + bs_size) - begin, &nalu_begin, &nalu_end);
				if (nalu_size == 0)
				{
					break;
				}
				else if (nalu_size < 0)
				{
					begin += nalu_begin;
					if ((begin[0] & 0x1F) != 0x09) //exclude AUD
					{
						//_front->on_acquire_bitstream(begin - 4, (bs + bs_size) - begin + 4);
						_front->push(begin - 4, (bs + bs_size) - begin + 4, 0);
					}
					break;
				}
				else
				{
					begin += nalu_begin;
					end += nalu_end;
					if ((begin[0] & 0x1F) != 0x09) //exclude AUD
					{
						//_front->on_acquire_bitstream(begin - 4, nalu_end - nalu_begin + 4);
						_front->push(begin - 4, (bs + bs_size) - begin + 4, 0);
					}
				}
			}
		}
	}


	//x264_nal_t *nalOut;
	//int nalNum;
	//x264_encoder_headers(_encoder, &nalOut, &nalNum);

	_state = dk_x264_encoder::encoder_state_encoded;
	return dk_x264_encoder::err_code_success;

fail:
	_frame_count = 0;
	x264_encoder_close(_encoder);
	_encoder = 0;
	x264_picture_clean(&_pic_in);

	_state = dk_x264_encoder::encoder_state_none;
	return dk_x264_encoder::err_code_fail;
}

dk_x264_encoder::err_code x264_encoder::get_queued_data(dk_video_encoder::dk_video_entity_t * bitstream)
{
	if (_front)
	{
		bitstream->mem_type = dk_x264_encoder::memory_type_host;
		return _front->pop((uint8_t*)bitstream->data, bitstream->data_size, bitstream->timestamp);
	}
	else
	{
		return dk_x264_encoder::err_code_fail;
	}
}

dk_x264_encoder::err_code x264_encoder::encode_async(dk_video_encoder::dk_video_entity_t * input)
{
	return dk_x264_encoder::err_code_not_implemented;
}

dk_x264_encoder::err_code x264_encoder::check_encoding_finish(void)
{
	return dk_x264_encoder::err_code_not_implemented;
}