#include "dk_nvenc_encoder.h"
#include "nvenc_encoder.h"

#define DEFAULT_I_QFACTOR -0.8f
#define DEFAULT_B_QFACTOR 1.25f
#define DEFAULT_I_QOFFSET 0.f
#define DEFAULT_B_QOFFSET 1.25f

dk_nvenc_encoder::_configuration_t::_configuration_t(void)
	: device_id(0)
	, max_height(3840)
	, max_width(2160)
	, preset(dk_nvenc_encoder::preset_high_performance)
	, rc_mode(dk_nvenc_encoder::rate_control_cbr)
	, frame_field_mode(dk_nvenc_encoder::frame_field_mode_frame)
	, motioin_vector_precision(dk_nvenc_encoder::motion_vector_precision_quarter_pel)
	, encode_level(dk_nvenc_encoder::encode_level_autoselect)
	, vbv_max_bitrate(10000000)
	, vbv_size(8000000)
	, qp(28)
	, i_quant_factor(DEFAULT_I_QFACTOR)
	, b_quant_factor(DEFAULT_B_QFACTOR)
	, i_quant_offset(DEFAULT_I_QOFFSET)
	, b_quant_offset(DEFAULT_B_QOFFSET)
	, invalidate_reference_frames_enable(false)
	, intra_refresh_enable(false)
	, intra_refresh_period(false)
	, intra_refresh_duration(false)
{
	
}

dk_nvenc_encoder::_configuration_t::_configuration_t(const dk_nvenc_encoder::_configuration_t & clone)
{
	device_id = clone.device_id;
	max_height = clone.max_height;
	max_width = clone.max_width;
	preset = clone.preset;
	rc_mode = clone.rc_mode;
	frame_field_mode = clone.frame_field_mode;
	motioin_vector_precision = clone.motioin_vector_precision;
	encode_level = clone.encode_level;
	vbv_max_bitrate = clone.vbv_max_bitrate;
	vbv_size = clone.vbv_size;
	qp = clone.qp;
	i_quant_factor = clone.i_quant_factor;
	b_quant_factor = clone.b_quant_factor;
	i_quant_offset = clone.i_quant_factor;
	b_quant_offset = clone.b_quant_factor;
	invalidate_reference_frames_enable = clone.invalidate_reference_frames_enable;
	intra_refresh_enable = clone.intra_refresh_enable;
	intra_refresh_period = clone.intra_refresh_period;
	intra_refresh_duration = clone.intra_refresh_duration;
}

dk_nvenc_encoder::_configuration_t & dk_nvenc_encoder::_configuration_t::operator=(const _configuration_t & clone)
{
	device_id = clone.device_id;
	max_height = clone.max_height;
	max_width = clone.max_width;
	preset = clone.preset;
	rc_mode = clone.rc_mode;
	frame_field_mode = clone.frame_field_mode;
	motioin_vector_precision = clone.motioin_vector_precision;
	encode_level = clone.encode_level;
	vbv_max_bitrate = clone.vbv_max_bitrate;
	vbv_size = clone.vbv_size;
	qp = clone.qp;
	i_quant_factor = clone.i_quant_factor;
	b_quant_factor = clone.b_quant_factor;
	i_quant_offset = clone.i_quant_factor;
	b_quant_offset = clone.b_quant_factor;
	invalidate_reference_frames_enable = clone.invalidate_reference_frames_enable;
	intra_refresh_enable = clone.intra_refresh_enable;
	intra_refresh_period = clone.intra_refresh_period;
	intra_refresh_duration = clone.intra_refresh_duration;
	return (*this);
}


dk_nvenc_encoder::dk_nvenc_encoder(void)
{
	_core = new nvenc_encoder(this);
}

dk_nvenc_encoder::~dk_nvenc_encoder(void)
{
	if (_core)
	{
		delete _core;
	}
	_core = nullptr;
}

dk_nvenc_encoder::encoder_state dk_nvenc_encoder::state(void)
{
	return _core->state();
}

dk_nvenc_encoder::err_code dk_nvenc_encoder::initialize_encoder(void * config)
{
	return _core->initialize_encoder(static_cast<dk_nvenc_encoder::configuration_t*>(config));
}

dk_nvenc_encoder::err_code dk_nvenc_encoder::release_encoder(void)
{
	return _core->release_encoder();
}

dk_nvenc_encoder::err_code dk_nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input, dk_nvenc_encoder::dk_video_entity_t * bitstream)
{
	return _core->encode(input, bitstream);
}

dk_nvenc_encoder::err_code dk_nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input)
{
	return _core->encode(input);
}

dk_nvenc_encoder::err_code dk_nvenc_encoder::get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream)
{
	return _core->get_qeueued_data(bitstream);
}

dk_nvenc_encoder::err_code dk_nvenc_encoder::encode_async(dk_nvenc_encoder::dk_video_entity_t * input)
{
	return _core->encode_async(input);
}

dk_nvenc_encoder::err_code dk_nvenc_encoder::check_encoding_flnish(void)
{
	return _core->check_encoding_flnish();
}

void dk_nvenc_encoder::on_acquire_bitstream(uint8_t * bistream, size_t size)
{

}