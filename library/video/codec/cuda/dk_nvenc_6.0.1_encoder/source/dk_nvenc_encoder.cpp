#include "dk_nvenc_encoder.h"
#include "nvenc_encoder.h"

#define DEFAULT_I_QFACTOR -0.8f
#define DEFAULT_B_QFACTOR 1.25f
#define DEFAULT_I_QOFFSET 0.f
#define DEFAULT_B_QOFFSET 1.25f

debuggerking::nvenc_encoder::_configuration_t::_configuration_t(void)
	: device_id(0)
	, max_height(3840)
	, max_width(2160)
	, preset(nvenc_encoder::preset_high_performance)
	, rc_mode(nvenc_encoder::rate_control_cbr)
	, frame_field_mode(nvenc_encoder::frame_field_mode_frame)
	, motioin_vector_precision(nvenc_encoder::motion_vector_precision_quarter_pel)
	, encode_level(nvenc_encoder::encode_level_autoselect)
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

debuggerking::nvenc_encoder::_configuration_t::_configuration_t(const nvenc_encoder::_configuration_t & clone)
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

debuggerking::nvenc_encoder::_configuration_t & debuggerking::nvenc_encoder::_configuration_t::operator=(const nvenc_encoder::_configuration_t & clone)
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

debuggerking::nvenc_encoder::nvenc_encoder(void)
{
	_core = new nvenc_core(this);
}

debuggerking::nvenc_encoder::~nvenc_encoder(void)
{
	if (_core)
	{
		delete _core;
	}
	_core = nullptr;
}

debuggerking::nvenc_encoder::encoder_state debuggerking::nvenc_encoder::state(void)
{
	return _core->state();
}

int32_t debuggerking::nvenc_encoder::initialize_encoder(void * config)
{
	int32_t status = video_base::initialize(static_cast<video_base::configuration_t*>(config));
	if (status != nvenc_encoder::err_code_t::success)
		return status;
	return _core->initialize_encoder(static_cast<nvenc_encoder::configuration_t*>(config));
}

int32_t debuggerking::nvenc_encoder::release_encoder(void)
{
	int32_t status = _core->release_encoder();
	if (status != nvenc_encoder::err_code_t::success)
		return status;
	return video_base::release();
}

int32_t debuggerking::nvenc_encoder::encode(nvenc_encoder::entity_t * input, nvenc_encoder::entity_t * bitstream)
{
	return _core->encode(input, bitstream);
}

int32_t debuggerking::nvenc_encoder::encode(nvenc_encoder::entity_t * input)
{
	return _core->encode(input);
}

int32_t debuggerking::nvenc_encoder::get_queued_data(nvenc_encoder::entity_t * bitstream)
{
	return _core->get_qeueued_data(bitstream);
}

void debuggerking::nvenc_encoder::after_encoding_callback(uint8_t * bistream, size_t size)
{

}