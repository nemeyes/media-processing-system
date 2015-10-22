#include "x264_encoder_core.h"

x264_encoder_core::x264_encoder_core(void)
	: _dk_encoder(0)
	, _dk_frame_count(0)
{
}

x264_encoder_core::~x264_encoder_core(void)
{
}

dk_x264_encoder::ERR_CODE x264_encoder_core::initialize(dk_x264_encoder::configuration_t conf)
{
	_dk_frame_count = 0;
	_config = conf;
	memset(&_dk_param, 0x00, sizeof(_dk_param));

	if (x264_param_default_preset(&_dk_param, x264_preset_names[_config.preset], x264_tune_names[_config.tune])<0)
		goto fail;

	switch (_config.cs)
	{
	case dk_x264_encoder::COLOR_SPACE_I420:
		_dk_param.i_csp = X264_CSP_I420;
		break;
	case dk_x264_encoder::COLOR_SPACE_YV12 :
		_dk_param.i_csp = X264_CSP_YV12;
		break;
	case dk_x264_encoder::COLOR_SPACE_NV12 :
		_dk_param.i_csp = X264_CSP_NV12;
		break;
	case dk_x264_encoder::COLOR_SPACE_RGB24 :
		_dk_param.i_csp = X264_CSP_BGR;
		break;
	case dk_x264_encoder::COLOR_SPACE_RGB32 :
		_dk_param.i_csp = X264_CSP_BGRA;
		break;
	}
	_dk_param.i_width = _config.width;
	_dk_param.i_height = _config.height;
	_dk_param.i_fps_num = _config.fps;
	_dk_param.i_fps_den = 1;
	_dk_param.i_slice_count = 1;

	_dk_param.i_keyint_max = _config.keyframe_interval*_config.fps;
	_dk_param.i_keyint_min = _config.keyframe_interval*_config.fps;

	_dk_param.i_bframe = _config.numb;

	_dk_param.b_vfr_input = 0;
	_dk_param.b_repeat_headers = 1;
	_dk_param.b_annexb = 1;

	if (x264_param_apply_profile(&_dk_param, x264_profile_names[_config.profile])<0)
		goto fail;

	if (x264_picture_alloc(&_dk_pic_in, _dk_param.i_csp, _dk_param.i_width, _dk_param.i_height) < 0)
		goto fail;

	_dk_encoder = x264_encoder_open(&_dk_param);
	if (!_dk_encoder)
		goto fail2;

	return dk_x264_encoder::ERR_CODE_SUCCESS;

fail:
	return dk_x264_encoder::ERR_CODE_FAILED;
fail2:
	x264_picture_clean(&_dk_pic_in);
	return dk_x264_encoder::ERR_CODE_FAILED;
}

dk_x264_encoder::ERR_CODE x264_encoder_core::release(void)
{
	if (_dk_encoder)
	{
		//flush encoding buffer
		int frame_size = 0;
		x264_nal_t * nal = 0;
		int i_nal = 0;
		while (x264_encoder_delayed_frames(_dk_encoder))
		{
			frame_size = x264_encoder_encode(_dk_encoder, &nal, &i_nal, 0, &_dk_pic_out);
			if (frame_size < 0)
				goto done;
		}

done:
		_dk_frame_count = 0;
		x264_encoder_close(_dk_encoder);
		_dk_encoder = 0;
		x264_picture_clean(&_dk_pic_in);
		return dk_x264_encoder::ERR_CODE_SUCCESS;
	}

	return dk_x264_encoder::ERR_CODE_SUCCESS;
}

dk_x264_encoder::ERR_CODE x264_encoder_core::encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, dk_x264_encoder::PIC_TYPE & pic_type, bool flush)
{
	if (!_dk_encoder)
		return dk_x264_encoder::ERR_CODE_FAILED;

	unsigned char * y_plane = 0;
	unsigned char * u_plane = 0;
	unsigned char * v_plane = 0;
	unsigned char * uv_plane = 0;

	unsigned int luma_volume = _dk_param.i_width*_dk_param.i_height;
	unsigned int chroma_volume = luma_volume >> 1;
	unsigned int cbcr_volume = luma_volume >> 2;
	switch (_config.cs)
	{
	case dk_x264_encoder::COLOR_SPACE_I420:
		y_plane = input;
		u_plane = y_plane + luma_volume;
		v_plane = u_plane + cbcr_volume;

		memcpy(_dk_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_dk_pic_in.img.plane[1], u_plane, cbcr_volume);
		memcpy(_dk_pic_in.img.plane[2], v_plane, cbcr_volume);
		break;
	case dk_x264_encoder::COLOR_SPACE_YV12:
		y_plane = input;
		v_plane = y_plane + luma_volume;
		u_plane = v_plane + cbcr_volume;

		memcpy(_dk_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_dk_pic_in.img.plane[1], v_plane, cbcr_volume);
		memcpy(_dk_pic_in.img.plane[2], u_plane, cbcr_volume);
		break;
	case dk_x264_encoder::COLOR_SPACE_NV12:
		y_plane = input;
		uv_plane = y_plane + luma_volume;
		memcpy(_dk_pic_in.img.plane[0], y_plane, luma_volume);
		memcpy(_dk_pic_in.img.plane[0], uv_plane, chroma_volume);
		break;
	case dk_x264_encoder::COLOR_SPACE_RGB24:
		_dk_param.i_csp = X264_CSP_BGR;
		break;
	case dk_x264_encoder::COLOR_SPACE_RGB32:
		_dk_param.i_csp = X264_CSP_BGRA;
		break;
	}

	x264_nal_t * nal = 0;
	int nal_index = 0;

	_dk_pic_in.i_pts = _dk_frame_count;
	int frame_size = x264_encoder_encode(_dk_encoder, &nal, &nal_index, &_dk_pic_in, &_dk_pic_out);
	if (frame_size < 0)
	{
		osize = 0;
		goto fail;
	}
	else if (frame_size)
	{
		osize = frame_size;
		memcpy(output, nal->p_payload, osize);
	}
	else
	{
		osize = frame_size;
	}
	
	return dk_x264_encoder::ERR_CODE_SUCCESS;

fail :
	_dk_frame_count = 0;
	 x264_encoder_close(_dk_encoder);
	 _dk_encoder = 0;
	 x264_picture_clean(&_dk_pic_in);
	 return dk_x264_encoder::ERR_CODE_FAILED;
}