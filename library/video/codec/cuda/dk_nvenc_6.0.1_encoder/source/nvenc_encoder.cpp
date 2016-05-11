#include "nvenc_encoder.h"
#include <tchar.h>

#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024
#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define FABS(a) ((a) >= 0 ? (a) : -(a))


template<class T>
debuggerking::nvenc_core::queue<T>::queue(void)
		: _buffer(NULL)
		, _size(0)
		, _pending_count(0)
		, _available_index(0)
		, _pending_index(0)
{}

template<class T>
debuggerking::nvenc_core::queue<T>::~queue(void)
{
	delete[] _buffer;
}

template<class T>
bool debuggerking::nvenc_core::queue<T>::initialize(T *pItems, uint32_t size)
{
	_size = size;
	_pending_count = 0;
	_available_index = 0;
	_pending_index = 0;
	_buffer = new T *[_size];
	for (uint32_t i = 0; i < _size; i++)
	{
		_buffer[i] = &pItems[i];
	}
	return true;
}

template<class T>
T * debuggerking::nvenc_core::queue<T>::get_available(void)
{
	T *pItem = NULL;
	if (_pending_count == _size)
	{
		return NULL;
	}
	pItem = _buffer[_available_index];
	_available_index = (_available_index + 1) % _size;
	_pending_count += 1;
	return pItem;
}

template<class T>
T * debuggerking::nvenc_core::queue<T>::get_pending(void)
{
	if (_pending_count == 0)
	{
		return NULL;
	}

	T *pItem = _buffer[_pending_index];
	_pending_index = (_pending_index + 1) % _size;
	_pending_count -= 1;
	return pItem;
}

debuggerking::nvenc_core::nvenc_core(nvenc_encoder * front)
	: _front(front)
	, _state(nvenc_encoder::encoder_state_none)
	, _config(NULL)
	, _context(NULL)
	, _instance(NULL)
	, _api(NULL)
	, _encoder(NULL)
	, _encode_index(0)
	, _buffer_count(0)
{
	//front->_spspps_size = 0;
	//memset(front->_spspps, 0x00, sizeof(front->_spspps));
}

debuggerking::nvenc_core::~nvenc_core(void)
{
}

debuggerking::nvenc_encoder::encoder_state debuggerking::nvenc_core::state(void)
{
	return _state;
}

int32_t debuggerking::nvenc_core::initialize_encoder(nvenc_encoder::configuration_t * config)
{
	if ((_state != nvenc_encoder::encoder_state_none) && (_state != nvenc_encoder::encoder_state_released))
		return nvenc_encoder::err_code_t::fail;

	release_encoder();
	_state = nvenc_encoder::encoder_state_initializing;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	do
	{
		_config = config;
		int32_t result = nvenc_encoder::err_code_t::fail;
		if (_config->mem_type == nvenc_encoder::video_memory_type_t::host)
		{
			status = initialize_cuda(_config->device_id);
			if (status != NV_ENC_SUCCESS)
			{
				release_cuda();
				break;
			}
			status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_CUDA);
			if (status != NV_ENC_SUCCESS)
			{
				release_nvenc_encoder();
				release_cuda();
				break;
			}
		}
		//else if (_config->mem_type == nvenc_encoder::video_memory_type_t::cuda)
		//{
		//	status = initialize_cuda(_config->device_id);
		//	if (status != NV_ENC_SUCCESS)
		//	{
		//		release_cuda();
		//		break;
		//	}
		//	status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_CUDA);
		//	if (status != NV_ENC_SUCCESS)
		//	{
		//		release_nvenc_encoder();
		//		release_cuda();
		//		break;
		//	}
		//}
		else if (_config->mem_type == nvenc_encoder::video_memory_type_t::d3d9)
		{
			status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_DIRECTX);
			if (status != NV_ENC_SUCCESS)
			{
				release_nvenc_encoder();
				break;
			}
		}
		else if (_config->mem_type == nvenc_encoder::video_memory_type_t::d3d10)
		{
			status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_DIRECTX);
			if (status != NV_ENC_SUCCESS)
			{
				release_nvenc_encoder();
				break;
			}
		}
		else if (_config->mem_type == nvenc_encoder::video_memory_type_t::d3d11)
		{
			status = initialize_d3d11();
			if (status != NV_ENC_SUCCESS)
				break;

			status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_DIRECTX);
			if (status != NV_ENC_SUCCESS)
			{
				release_nvenc_encoder();
				release_d3d11();
				break;
			}
		}

		NV_ENC_INITIALIZE_PARAMS nvenc_initialize_param;
		memset(&nvenc_initialize_param, 0x00, sizeof(nvenc_initialize_param));
		SET_VER(nvenc_initialize_param, NV_ENC_INITIALIZE_PARAMS);

		NV_ENC_CONFIG nvenc_config;
		memset(&nvenc_config, 0x00, sizeof(nvenc_config));
		SET_VER(nvenc_config, NV_ENC_CONFIG);

		if (((_config->codec >= nvenc_encoder::nvenc_submedia_type_t::h264) && (_config->codec <= nvenc_encoder::nvenc_submedia_type_t::h264_ep)) ||
			((_config->codec >= nvenc_encoder::nvenc_submedia_type_t::h264_sp) && (_config->codec <= nvenc_encoder::nvenc_submedia_type_t::h264_chp)))
		{
			nvenc_initialize_param.encodeGUID = NV_ENC_CODEC_H264_GUID;
			switch (_config->preset)
			{
			case nvenc_encoder::preset_default:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
				break;
			case nvenc_encoder::preset_high_performance:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_HP_GUID;
				break;
			case nvenc_encoder::preset_high_quality:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_HQ_GUID;
				break;
			case nvenc_encoder::preset_bluelay_disk:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_BD_GUID;
				break;
			case nvenc_encoder::preset_low_latency_default:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID;
				break;
			case nvenc_encoder::preset_low_latency_high_quality:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
				break;
			case nvenc_encoder::preset_low_latency_high_performance:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
				break;
			case nvenc_encoder::preset_lossless_default:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOSSLESS_DEFAULT_GUID;
				break;
			case nvenc_encoder::preset_lossless_high_performance:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOSSLESS_HP_GUID;
				break;
			}
		}
		else if ((_config->codec >= nvenc_encoder::video_submedia_type_t::hevc) && (_config->codec <= nvenc_encoder::video_submedia_type_t::hevc_mp))
		{
			nvenc_initialize_param.encodeGUID = NV_ENC_CODEC_HEVC_GUID;
			switch (_config->preset)
			{
			case nvenc_encoder::preset_default:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
				break;
			case nvenc_encoder::preset_high_performance:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_HP_GUID;
				break;
			case nvenc_encoder::preset_high_quality:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_HQ_GUID;
				break;
			case nvenc_encoder::preset_bluelay_disk:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_BD_GUID;
				break;
			case nvenc_encoder::preset_low_latency_default:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID;
				break;
			case nvenc_encoder::preset_low_latency_high_quality:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
				break;
			case nvenc_encoder::preset_low_latency_high_performance:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
				break;
			case nvenc_encoder::preset_lossless_default:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOSSLESS_DEFAULT_GUID;
				break;
			case nvenc_encoder::preset_lossless_high_performance:
				nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOSSLESS_HP_GUID;
				break;
			}
		}

		nvenc_initialize_param.encodeWidth = _config->width;
		nvenc_initialize_param.encodeHeight = _config->height;
		nvenc_initialize_param.darWidth = _config->width;
		nvenc_initialize_param.darHeight = _config->height;
		nvenc_initialize_param.maxEncodeWidth = _config->max_width;
		nvenc_initialize_param.maxEncodeHeight = _config->max_height;
		nvenc_initialize_param.frameRateNum = _config->fps;
		nvenc_initialize_param.frameRateDen = 1;
#if defined(WIN32) && defined(WITH_ASYNC)
		nvenc_initialize_param.enableEncodeAsync = 1;
#else
		nvenc_initialize_param.enableEncodeAsync = 0;
#endif
		nvenc_initialize_param.enablePTD = 1;
		nvenc_initialize_param.reportSliceOffsets = 0;
		nvenc_initialize_param.enableSubFrameWrite = 0;
		nvenc_initialize_param.encodeConfig = &nvenc_config;

		// apply preset
		NV_ENC_PRESET_CONFIG nvenc_preset_config;
		memset(&nvenc_preset_config, 0, sizeof(NV_ENC_PRESET_CONFIG));
		SET_VER(nvenc_preset_config, NV_ENC_PRESET_CONFIG);
		SET_VER(nvenc_preset_config.presetCfg, NV_ENC_CONFIG);

		status = NvEncGetEncodePresetConfig(nvenc_initialize_param.encodeGUID, nvenc_initialize_param.presetGUID, &nvenc_preset_config);
		if (status != NV_ENC_SUCCESS)
		{
			release_nvenc_encoder();
			if (_config->mem_type == nvenc_encoder::video_memory_type_t::host /*|| _config->mem_type == nvenc_encoder::video_memory_type_t::cuda*/)
				release_cuda();
			if (_config->mem_type == nvenc_encoder::video_memory_type_t::d3d11)
				release_d3d11();
			break;
		}
		memcpy(nvenc_initialize_param.encodeConfig, &nvenc_preset_config.presetCfg, sizeof(NV_ENC_CONFIG));

		switch (_config->codec)
		{
		case nvenc_encoder::nvenc_submedia_type_t::h264:
		case nvenc_encoder::nvenc_submedia_type_t::h264_ep:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::h264_bp:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_BASELINE_GUID;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::h264_hp:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::h264_mp:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_MAIN_GUID;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::h264_sp:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_STEREO_GUID;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::h264_stsp:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_SVC_TEMPORAL_SCALABILTY;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::h264_php:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_PROGRESSIVE_HIGH_GUID;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::h264_chp:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_CONSTRAINED_HIGH_GUID;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::hevc:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::hevc_mp:
			nvenc_initialize_param.encodeConfig->profileGUID = NV_ENC_HEVC_PROFILE_MAIN_GUID;
			break;
		}

		if (_config->keyframe_interval <= 0)
			nvenc_initialize_param.encodeConfig->gopLength = NVENC_INFINITE_GOPLENGTH;
		else
			nvenc_initialize_param.encodeConfig->gopLength = _config->keyframe_interval * _config->fps;

		nvenc_initialize_param.encodeConfig->frameIntervalP = _config->numb + 1;
		nvenc_initialize_param.encodeConfig->frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE(_config->frame_field_mode);
		nvenc_initialize_param.encodeConfig->mvPrecision = NV_ENC_MV_PRECISION(_config->motioin_vector_precision);

		nvenc_initialize_param.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_MODE(_config->rc_mode);
		nvenc_initialize_param.encodeConfig->rcParams.averageBitRate = _config->bitrate;
		nvenc_initialize_param.encodeConfig->rcParams.maxBitRate = _config->vbv_max_bitrate;
		nvenc_initialize_param.encodeConfig->rcParams.vbvBufferSize = _config->vbv_size;
		nvenc_initialize_param.encodeConfig->rcParams.vbvInitialDelay = _config->vbv_size * 9 / 10;

		/*if (_config->bitrate || _config->vbv_max_bitrate)
		{
			nvenc_initialize_param.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_MODE(_config->rc_mode);
			nvenc_initialize_param.encodeConfig->rcParams.averageBitRate = _config->bitrate;
			nvenc_initialize_param.encodeConfig->rcParams.maxBitRate = _config->bitrate;
			nvenc_initialize_param.encodeConfig->rcParams.vbvBufferSize = _config->vbv_size;
			nvenc_initialize_param.encodeConfig->rcParams.vbvInitialDelay = _config->vbv_size * 9 / 10;
		}
		else
		{
			nvenc_initialize_param.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CONSTQP;
		}
		*/
		if (_config->rc_mode == nvenc_encoder::rate_control_constant_qp)
		{
			nvenc_initialize_param.encodeConfig->rcParams.constQP.qpInterP = nvenc_initialize_param.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : _config->qp;
			nvenc_initialize_param.encodeConfig->rcParams.constQP.qpInterB = nvenc_initialize_param.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : _config->qp;
			nvenc_initialize_param.encodeConfig->rcParams.constQP.qpIntra = nvenc_initialize_param.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : _config->qp;
		}

		// set up initial QP value
		if (_config->rc_mode == nvenc_encoder::rate_control_vbr ||
			_config->rc_mode == nvenc_encoder::rate_control_vbr_min_qp ||
			_config->rc_mode == nvenc_encoder::rate_control_two_pass_vbr)
		{
			nvenc_initialize_param.encodeConfig->rcParams.enableInitialRCQP = 1;
			nvenc_initialize_param.encodeConfig->rcParams.initialRCQP.qpInterP = _config->qp;
			if (_config->i_quant_factor != 0.0 && _config->b_quant_factor != 0.0)
			{
				nvenc_initialize_param.encodeConfig->rcParams.initialRCQP.qpIntra = (int)(_config->qp * FABS(_config->i_quant_factor) + _config->i_quant_offset);
				nvenc_initialize_param.encodeConfig->rcParams.initialRCQP.qpInterB = (int)(_config->qp * FABS(_config->b_quant_factor) + _config->b_quant_offset);
			}
			else
			{
				nvenc_initialize_param.encodeConfig->rcParams.initialRCQP.qpIntra = _config->qp;
				nvenc_initialize_param.encodeConfig->rcParams.initialRCQP.qpInterB = _config->qp;
			}
		}

		if (nvenc_initialize_param.encodeGUID == NV_ENC_CODEC_H264_GUID)
		{
			//nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.enableIntraRefresh = 1;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.chromaFormatIDC = 1;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.idrPeriod = nvenc_initialize_param.encodeConfig->gopLength;
			//nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.maxNumRefFrames = 16;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.adaptiveTransformMode = NV_ENC_H264_ADAPTIVE_TRANSFORM_ENABLE;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.fmoMode = NV_ENC_H264_FMO_AUTOSELECT;// NV_ENC_H264_FMO_DISABLE;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.bdirectMode = NV_ENC_H264_BDIRECT_MODE_AUTOSELECT;//nvenc_initialize_param.encodeConfig->frameIntervalP > 1 ? NV_ENC_H264_BDIRECT_MODE_TEMPORAL : NV_ENC_H264_BDIRECT_MODE_DISABLE;
			switch (_config->entropy_coding_mode)
			{
			case nvenc_encoder::entropy_coding_mode_t::unknown:
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_AUTOSELECT;
				break;
			case nvenc_encoder::entropy_coding_mode_t::cabac:
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_CABAC;
				break;
			case nvenc_encoder::entropy_coding_mode_t::cavlc:
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_CAVLC;
				break;
			}
			//nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.disableSPSPPS = 1;
			//nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.repeatSPSPPS = 1;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.sliceMode = 0;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.sliceModeData = 0;
			//nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.enableVFR = 0;
			//nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.level = NV_ENC_LEVEL(_config->encode_level);
		}
		else if (nvenc_initialize_param.encodeGUID == NV_ENC_CODEC_HEVC_GUID)
		{
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.chromaFormatIDC = 1;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.idrPeriod = nvenc_initialize_param.encodeConfig->gopLength;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.sliceMode = 0;
			nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.sliceModeData = 0;
		}

		if (_config->intra_refresh_enable)
		{
			if (nvenc_initialize_param.encodeGUID == NV_ENC_CODEC_HEVC_GUID)
			{
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.enableIntraRefresh = 1;
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.intraRefreshPeriod = _config->intra_refresh_period;
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.intraRefreshCnt = _config->intra_refresh_duration;
			}
			else if (nvenc_initialize_param.encodeGUID == NV_ENC_CODEC_H264_GUID)
			{
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.enableIntraRefresh = 1;
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.intraRefreshPeriod = _config->intra_refresh_period;
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.intraRefreshCnt = _config->intra_refresh_duration;
			}
		}

		if (_config->invalidate_reference_frames_enable)
		{
			if (nvenc_initialize_param.encodeGUID == NV_ENC_CODEC_HEVC_GUID)
			{
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.maxNumRefFramesInDPB = 16;
			}
			else if (nvenc_initialize_param.encodeGUID == NV_ENC_CODEC_H264_GUID)
			{
				nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.maxNumRefFrames = 16;
			}
		}

		status = NvEncInitializeEncoder(&nvenc_initialize_param);
		if (status != NV_ENC_SUCCESS)
		{
			release_nvenc_encoder();
			if (_config->mem_type == nvenc_encoder::video_memory_type_t::host /*|| _config->mem_type == nvenc_encoder::video_memory_type_t::cuda*/)
				release_cuda();
			if (_config->mem_type == nvenc_encoder::video_memory_type_t::d3d11)
				release_d3d11();
			break;
		}

		memset(_front->_extradata, 0x00, sizeof(_front->_extradata));
		NV_ENC_SEQUENCE_PARAM_PAYLOAD extra_data;
		memset(&extra_data, 0x00, sizeof(NV_ENC_SEQUENCE_PARAM_PAYLOAD));
		SET_VER(extra_data, NV_ENC_SEQUENCE_PARAM_PAYLOAD);
		extra_data.inBufferSize = sizeof(_front->_extradata);
		extra_data.spsppsBuffer = _front->_extradata;
		extra_data.outSPSPPSPayloadSize = &_front->_extradata_size;
		status = NvEncGetSequenceParams(&extra_data);
		if (status != NV_ENC_SUCCESS)
		{
			release_nvenc_encoder();
			if (_config->mem_type == nvenc_encoder::video_memory_type_t::host)
				release_cuda();
			if (_config->mem_type == nvenc_encoder::video_memory_type_t::d3d11)
				release_d3d11();
			break;
		}

		if (_config->numb > 0)
		{
			_buffer_count = _config->numb + 4; // min buffers is numb + 1 + 3 pipelining
		}
		else
		{
			int32_t numMBs = ((_config->max_height + 15) >> 4) * ((_config->max_width + 15) >> 4);
			int32_t NumIOBuffers;
			if (numMBs >= 32768) //4kx2k
				NumIOBuffers = nvenc_core::max_encode_queue / 8;
			else if (numMBs >= 16384) // 2kx2k
				NumIOBuffers = nvenc_core::max_encode_queue / 4;
			else if (numMBs >= 8160) // 1920x1080
				NumIOBuffers = nvenc_core::max_encode_queue / 2;
			else
				NumIOBuffers = nvenc_core::max_encode_queue;
			_buffer_count = NumIOBuffers;
		}
		status = allocate_io_buffers(_config->width, _config->height);

	} while(0);


	if (status == NV_ENC_SUCCESS)
		return nvenc_encoder::err_code_t::success;
	else
		return nvenc_encoder::err_code_t::fail;
}

int32_t debuggerking::nvenc_core::release_encoder(void)
{
	if ((_state != nvenc_encoder::encoder_state_none) && (_state != nvenc_encoder::encoder_state_initialized) && (_state != nvenc_encoder::encoder_state_encoded))
		return nvenc_encoder::err_code_t::fail;
	_state = nvenc_encoder::encoder_state_releasing;

	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (flush_encoder() != NV_ENC_SUCCESS)
		return nvenc_encoder::err_code_t::fail;

	if (release_io_buffers() != NV_ENC_SUCCESS)
		return nvenc_encoder::err_code_t::fail;

	if (release_nvenc_encoder() != NV_ENC_SUCCESS)
		return nvenc_encoder::err_code_t::fail;

	if (_config)
	{
		if (_config->mem_type == nvenc_encoder::video_memory_type_t::host)
		{
			if (release_cuda() != NV_ENC_SUCCESS)
				return nvenc_encoder::err_code_t::fail;
		}
		if (_config->mem_type == nvenc_encoder::video_memory_type_t::d3d11)
		{
			if(release_d3d11() != NV_ENC_SUCCESS)
				return nvenc_encoder::err_code_t::fail;
		}
	}

	return nvenc_encoder::err_code_t::success;
}

int32_t debuggerking::nvenc_core::encode(nvenc_encoder::entity_t * input, nvenc_encoder::entity_t * bitstream)
{
	if (!input || !bitstream)
		return nvenc_encoder::err_code_t::fail;

	if (input->flush)
	{
		flush_encoder();
		return nvenc_encoder::err_code_t::success;
	}

	buffer_t * buffer = _queue.get_available();
	if (!buffer)
	{
		process_output(_queue.get_pending(), bitstream);
		buffer = _queue.get_available();
	}

	NVENCSTATUS status = encode_frame(buffer, input);
	if (status != NV_ENC_SUCCESS)
		return nvenc_encoder::err_code_t::fail;
	else
		return nvenc_encoder::err_code_t::success;
}

int32_t debuggerking::nvenc_core::encode(nvenc_encoder::entity_t * input)
{
	return encode(input, NULL);
}

int32_t debuggerking::nvenc_core::get_qeueued_data(nvenc_encoder::entity_t * bitstream)
{
	if (_front)
	{
		bitstream->mem_type = nvenc_encoder::video_memory_type_t::host;
		return _front->pop((uint8_t*)bitstream->data, bitstream->data_size, bitstream->timestamp);
	}
	else
	{
		return nvenc_encoder::err_code_t::fail;
	}
}

NVENCSTATUS debuggerking::nvenc_core::initialize_cuda(uint32_t device_id)
{
	CUresult result;
	CUdevice device;
	CUcontext current_context;
	int  deviceCount = 0;
	int  SMminor = 0, SMmajor = 0;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
		typedef HMODULE CUDADRIVER;
#else
		typedef void *CUDADRIVER;
#endif

	CUDADRIVER driver = 0;
	result = cuInit(0, __CUDA_API_VERSION, driver);
	if (result != CUDA_SUCCESS)
	{
		return NV_ENC_ERR_NO_ENCODE_DEVICE;
	}

	result = cuDeviceGetCount(&deviceCount);
	if (result != CUDA_SUCCESS)
	{
		return NV_ENC_ERR_NO_ENCODE_DEVICE;
	}

	// If dev is negative value, we clamp to 0
	if ((int)device_id < 0)
		device_id = 0;

	if (device_id >(unsigned int)deviceCount - 1)
	{
		return NV_ENC_ERR_INVALID_ENCODERDEVICE;
	}

	result = cuDeviceGet(&device, device_id);
	if (result != CUDA_SUCCESS)
	{
		return NV_ENC_ERR_NO_ENCODE_DEVICE;
	}

	result = cuDeviceComputeCapability(&SMmajor, &SMminor, device_id);
	if (result != CUDA_SUCCESS)
	{
		return NV_ENC_ERR_NO_ENCODE_DEVICE;
	}

	if (((SMmajor << 4) + SMminor) < 0x30)
	{
		return NV_ENC_ERR_NO_ENCODE_DEVICE;
	}

	result = cuCtxCreate((CUcontext*)(&_context), 0, device);
	if (result != CUDA_SUCCESS)
	{
		return NV_ENC_ERR_NO_ENCODE_DEVICE;
	}

	result = cuCtxPopCurrent(&current_context);
	if (result != CUDA_SUCCESS)
	{
		return NV_ENC_ERR_NO_ENCODE_DEVICE;
	}
	return NV_ENC_SUCCESS;
}

NVENCSTATUS debuggerking::nvenc_core::release_cuda(void)
{
	CUresult result = cuCtxDestroy((CUcontext)_context);
	if (result != CUDA_SUCCESS)
		return NV_ENC_ERR_GENERIC;
	return NV_ENC_SUCCESS;
}

NVENCSTATUS debuggerking::nvenc_core::initialize_d3d11(void)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	//DXVA2CreateVideoService()


	return status;
}

NVENCSTATUS debuggerking::nvenc_core::release_d3d11(void)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::initialize_nvenc_encoder(void * device, NV_ENC_DEVICE_TYPE type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	MYPROC nvenc_api_create_instance;

#if defined(WIN32)
#if defined(WIN64)
	_instance = ::LoadLibrary(_T("nvEncodeAPI64.dll"));
#else
	_instance = ::LoadLibrary(_T("nvEncodeAPI.dll"));
#endif
#else
	_instance = dlopen("libnvidia-encode.so.1", RTLD_LAZY);
#endif

	if (_instance == 0)
		return NV_ENC_ERR_OUT_OF_MEMORY;

#if defined(WIN32)
	nvenc_api_create_instance = (MYPROC)::GetProcAddress(_instance, "NvEncodeAPICreateInstance");
#else
	nvenc_api_create_instance = (MYPROC)dlsym(_instance, "NvEncodeAPICreateInstance");
#endif

	if (!nvenc_api_create_instance)
		return NV_ENC_ERR_OUT_OF_MEMORY;

	_api = new NV_ENCODE_API_FUNCTION_LIST;
	if (!_api)
		return NV_ENC_ERR_OUT_OF_MEMORY;

	memset(_api, 0x00, sizeof(NV_ENCODE_API_FUNCTION_LIST));
	_api->version = NV_ENCODE_API_FUNCTION_LIST_VER;
	status = nvenc_api_create_instance(_api);
	if (status != NV_ENC_SUCCESS)
		return status;

	__try
	{
		_encoder = NULL;
		status = NvEncOpenEncodeSessionEx(device, type);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return NV_ENC_ERR_GENERIC;
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::release_nvenc_encoder(void)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	__try
	{
		status = NvEncDestroyEncoder();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}

	if (_api)
	{
		delete _api;
		_api = NULL;
	}

	if (_instance)
	{
#if defined(WIN32)
		FreeLibrary(_instance);
#else
		dlclose(_instance);
#endif
		_instance = NULL;
	}
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::allocate_io_buffers(uint32_t width, uint32_t height)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	_queue.initialize(_buffer, _buffer_count);
	for (uint32_t i = 0; i < _buffer_count; i++)
	{
		switch (_config->cs)
		{
		case nvenc_encoder::nvenc_submedia_type_t::nv12 :
			_buffer[i].input.buffer_format = NV_ENC_BUFFER_FORMAT_NV12_PL;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::yv12 :
			_buffer[i].input.buffer_format = NV_ENC_BUFFER_FORMAT_YV12_PL;
			break;
		case nvenc_encoder::nvenc_submedia_type_t::i420 :
			_buffer[i].input.buffer_format = NV_ENC_BUFFER_FORMAT_IYUV_PL;
			break;
		}

		status = NvEncCreateInputBuffer(width, height, _buffer[i].input.buffer_format, &_buffer[i].input.input_surface);
		if (status != NV_ENC_SUCCESS)
			return status;
		
		_buffer[i].input.width = width;
		_buffer[i].input.height = height;
		status = NvEncCreateBitstreamBuffer(BITSTREAM_BUFFER_SIZE, &_buffer[i].output.bitstream_buffer);
		if (status != NV_ENC_SUCCESS)
			return status;
		_buffer[i].output.bitstream_buffer_size = BITSTREAM_BUFFER_SIZE;

#if defined(WIN32) && defined(WITH_ASYNC)
		status = NvEncRegisterAsyncEvent(&_buffer[i].output.output_event);
		if(status!=NV_ENC_SUCCESS)
			return status;
		_buffer[i].output.wait_event = true;
#else
		_buffer[i].output.output_event = NULL;
		_buffer[i].output.wait_event = false;
#endif
	}
	_eos_output_buffer.eos = true;

#if defined(WIN32) && defined(WITH_ASYNC)
	status = NvEncRegisterAsyncEvent(&_eos_output_buffer.output_event);
	if(status!=NV_ENC_SUCCESS)
		return status;
#else
	_nvenc_eos_output_buffer.output_event = NULL;
#endif
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::release_io_buffers(void)
{
	for (uint32_t i = 0; i < _buffer_count; i++)
	{
		NvEncDestroyInputBuffer(_buffer[i].input.input_surface);
		_buffer[i].input.input_surface = NULL;

		NvEncDestroyBitstreamBuffer(_buffer[i].output.bitstream_buffer);
		_buffer[i].output.bitstream_buffer = NULL;

#if defined(WIN32) && defined(WITH_ASYNC)
		NvEncUnregisterAsyncEvent(_buffer[i].output.output_event);
		::CloseHandle(_buffer[i].output.output_event);
		_buffer[i].output.output_event = NULL;
#endif
	}

	if (_eos_output_buffer.output_event)
	{
#if defined(WIN32) && defined(WITH_ASYNC)
		NvEncUnregisterAsyncEvent(_eos_output_buffer.output_event);
		::CloseHandle(_eos_output_buffer.output_event);
		_eos_output_buffer.output_event = NULL;
#endif
	}
	return NV_ENC_SUCCESS;
}

NVENCSTATUS debuggerking::nvenc_core::flush_encoder(void)
{
	NVENCSTATUS status = NvEncFlushEncoderQueue(_eos_output_buffer.output_event);
	if (status != NV_ENC_SUCCESS)
		return status;

	buffer_t * buffer = _queue.get_pending();
	while (buffer)
	{
		process_output(buffer, NULL, true);
		buffer = _queue.get_pending();
	}

#if defined(WIN32) && defined(WITH_ASYNC)
	if (::WaitForSingleObject(_eos_output_buffer.output_event, 500) != WAIT_OBJECT_0)
		status = NV_ENC_ERR_GENERIC;
#endif
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::encode_frame(nvenc_core::buffer_t * nvenc_buffer, nvenc_encoder::entity_t * input)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	uint8_t * input_surface = NULL;
	uint32_t locked_pitch = 0;
	status = NvEncLockInputBuffer(nvenc_buffer->input.input_surface, (void**)&input_surface, &locked_pitch);
	if (status != NV_ENC_SUCCESS)
		return status;

	uint8_t * origin_y_plane = input->data;
	uint8_t * origin_u_plane = origin_y_plane + _config->width * _config->height;
	uint8_t * origin_v_plane = origin_u_plane + ((_config->width * _config->height) >> 2);
	uint8_t * input_surface_uv = input_surface + locked_pitch * _config->height;

#if 0
	if (_config->cs==nvenc_encoder::nvenc_submedia_type_t::submedia_type_t::nv12)
		status = convert_yv12pitch_to_nv12(origin_y_plane, origin_u_plane, origin_v_plane, input_surface, input_surface_uv, _config->width, _config->height, _config->width, locked_pitch);
	else if (_config->cs == nvenc_encoder::nvenc_submedia_type_t::submedia_type_t::yv12)
		status = convert_yv12pitch_to_yv12(origin_y_plane, origin_u_plane, origin_v_plane, input_surface, input_surface_uv, _config->width, _config->height, _config->width, locked_pitch);
#else
	if (_config->cs == nvenc_encoder::nvenc_submedia_type_t::yv12)
		status = convert_yv12pitch_to_yv12(origin_y_plane, origin_u_plane, origin_v_plane, input_surface, input_surface_uv, _config->width, _config->height, _config->width, locked_pitch);
#endif

	if (status != NV_ENC_SUCCESS)
		return status;

	status = NvEncUnlockInputBuffer(nvenc_buffer->input.input_surface);
	if (status != NV_ENC_SUCCESS)
		return status;

	NV_ENC_PIC_PARAMS nvenc_pic_param;
	memset(&nvenc_pic_param, 0x00, sizeof(nvenc_pic_param));
	SET_VER(nvenc_pic_param, NV_ENC_PIC_PARAMS);

	nvenc_pic_param.inputBuffer = nvenc_buffer->input.input_surface;
	nvenc_pic_param.bufferFmt = nvenc_buffer->input.buffer_format;
	if (!input->width)
		nvenc_pic_param.inputWidth = _config->width;
	else
		nvenc_pic_param.inputWidth = input->width;

	if (!input->height)
		nvenc_pic_param.inputHeight = _config->height;
	else
		nvenc_pic_param.inputHeight = input->height;

	nvenc_pic_param.outputBitstream = nvenc_buffer->output.bitstream_buffer;
	nvenc_pic_param.completionEvent = nvenc_buffer->output.output_event;
	if (!input->timestamp)
		nvenc_pic_param.inputTimeStamp = _encode_index;
	else
		nvenc_pic_param.inputTimeStamp = input->timestamp;

	if (_config->frame_field_mode == nvenc_encoder::frame_field_mode_frame)
		nvenc_pic_param.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
	else
		nvenc_pic_param.pictureStruct = NV_ENC_PIC_STRUCT_FIELD_TOP_BOTTOM; //NV_ENC_PIC_STRUCT_FIELD_BOTTOM_TOP

	nvenc_pic_param.qpDeltaMap = 0;
	nvenc_pic_param.qpDeltaMapSize = 0;

	if (input->gen_spspps)
		nvenc_pic_param.encodePicFlags |= NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;
	if (input->gen_idr)
		nvenc_pic_param.encodePicFlags |= NV_ENC_PIC_FLAG_FORCEIDR;


	status = NvEncEncodePicture(&nvenc_pic_param);
	if ((status != NV_ENC_SUCCESS) && (status != NV_ENC_ERR_NEED_MORE_INPUT))
		return status;
	_encode_index++;
	return NV_ENC_SUCCESS;
}

NVENCSTATUS debuggerking::nvenc_core::process_output(const nvenc_core::buffer_t * nvenc_buffer, nvenc_encoder::entity_t * bitstream, bool flush)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	if (!nvenc_buffer || !nvenc_buffer->output.bitstream_buffer || nvenc_buffer->output.eos)
		return NV_ENC_ERR_INVALID_PARAM;

#if defined(WITH_ASYNC)
	if (nvenc_buffer->output.wait_event)
	{
		if (!nvenc_buffer->output.output_event)
			return NV_ENC_ERR_INVALID_PARAM;
#if defined(WIN32)
		//::WaitForSingleObject(nvenc_buffer->output.output_event, 500);
		if (::WaitForSingleObject(nvenc_buffer->output.output_event, 500) != WAIT_OBJECT_0)
			return NV_ENC_ERR_GENERIC;
#endif
	}
#endif

	if (nvenc_buffer->output.eos)
		return NV_ENC_SUCCESS;

	status = NV_ENC_SUCCESS;
	NV_ENC_LOCK_BITSTREAM lock_bitstream;
	memset(&lock_bitstream, 0x00, sizeof(lock_bitstream));
	SET_VER(lock_bitstream, NV_ENC_LOCK_BITSTREAM);
	lock_bitstream.outputBitstream = nvenc_buffer->output.bitstream_buffer;
#if defined(WIN32) && defined(WITH_ASYNC)
	lock_bitstream.doNotWait = 1;
#else
	lock_bitstream.doNotWait = 0;
#endif

	status = NvEncLockBitstream(&lock_bitstream);
	if (status == NV_ENC_SUCCESS)
	{
		if (!flush)
		{
			if (bitstream)
			{
				if (bitstream->mem_type != nvenc_encoder::video_memory_type_t::host)
					assert(0);
				if (lock_bitstream.bitstreamSizeInBytes > bitstream->data_capacity)
					bitstream->data_size = bitstream->data_capacity;
				else
					bitstream->data_size = lock_bitstream.bitstreamSizeInBytes;
				bitstream->timestamp = lock_bitstream.outputTimeStamp;
				memmove(bitstream->data, lock_bitstream.bitstreamBufferPtr, bitstream->data_size);
			}
			else
			{
				if (_front)
				{
					_front->push((uint8_t*)lock_bitstream.bitstreamBufferPtr, lock_bitstream.bitstreamSizeInBytes, lock_bitstream.outputTimeStamp);
				}
			}
		}
		status = NvEncUnlockBitstream(nvenc_buffer->output.bitstream_buffer);
	}
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::convert_yv12pitch_to_nv12(uint8_t * src_y, uint8_t * src_u, uint8_t * src_v,
													 uint8_t * dst_y, uint8_t * dst_u,
													 int32_t width, int32_t height, uint32_t src_stride, uint32_t dst_stride)
{
	int32_t y;
	int32_t x;
	if (src_stride == 0)
		src_stride = width;
	if (dst_stride == 0)
		dst_stride = width;

	for (y = 0; y < height; y++)
	{
		memcpy(dst_y+ (dst_stride*y), src_y + (src_stride*y), width);
		if (y < height / 2)
		{
			for (x = 0; x < width; x = x + 2)
			{
				dst_u[(y*dst_stride) + x] = src_u[((src_stride / 2)*y) + (x >> 1)];
				dst_u[(y*dst_stride) + (x + 1)] = src_v[((src_stride / 2)*y) + (x >> 1)];
			}
		}
	}
	return NV_ENC_SUCCESS;
}

NVENCSTATUS debuggerking::nvenc_core::convert_yv12pitch_to_yv12(uint8_t * src_y, uint8_t * src_u, uint8_t * src_v,
													 uint8_t * dst_y, uint8_t * dst_u,
													 int32_t width, int32_t height, uint32_t src_stride, uint32_t dst_stride)
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
	return NV_ENC_SUCCESS;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncInitializeEncoder(NV_ENC_INITIALIZE_PARAMS * params)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _api->nvEncInitializeEncoder(_encoder, params);
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncOpenEncodeSession(void * device, uint32_t device_type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncOpenEncodeSession(device, device_type, &_encoder);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodeGUIDCount(uint32_t * encodeGUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodeGUIDCount(_encoder, encodeGUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodeProfileGUIDCount(GUID encodeGUID, uint32_t* encodeProfileGUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodeProfileGUIDCount(_encoder, encodeGUID, encodeProfileGUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodeProfileGUIDs(GUID encodeGUID, GUID* profileGUIDs, uint32_t guidArraySize, uint32_t* GUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodeProfileGUIDs(_encoder, encodeGUID, profileGUIDs, guidArraySize, GUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodeGUIDs(GUID* GUIDs, uint32_t guidArraySize, uint32_t* GUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodeGUIDs(_encoder, GUIDs, guidArraySize, GUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetInputFormatCount(GUID encodeGUID, uint32_t* inputFmtCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetInputFormatCount(_encoder, encodeGUID, inputFmtCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetInputFormats(GUID encodeGUID, NV_ENC_BUFFER_FORMAT* inputFmts, uint32_t inputFmtArraySize, uint32_t* inputFmtCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetInputFormats(_encoder, encodeGUID, inputFmts, inputFmtArraySize, inputFmtCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodeCaps(GUID encodeGUID, NV_ENC_CAPS_PARAM* capsParam, int* capsVal)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodeCaps(_encoder, encodeGUID, capsParam, capsVal);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodePresetCount(GUID encodeGUID, uint32_t* encodePresetGUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodePresetCount(_encoder, encodeGUID, encodePresetGUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodePresetGUIDs(GUID encodeGUID, GUID* presetGUIDs, uint32_t guidArraySize, uint32_t* encodePresetGUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodePresetGUIDs(_encoder, encodeGUID, presetGUIDs, guidArraySize, encodePresetGUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodePresetConfig(GUID encodeGUID, GUID  presetGUID, NV_ENC_PRESET_CONFIG* presetConfig)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodePresetConfig(_encoder, encodeGUID, presetGUID, presetConfig);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncCreateInputBuffer(uint32_t width, uint32_t height, NV_ENC_BUFFER_FORMAT fmt, void ** input_buffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_CREATE_INPUT_BUFFER createInputBufferParams;

	memset(&createInputBufferParams, 0, sizeof(createInputBufferParams));
	SET_VER(createInputBufferParams, NV_ENC_CREATE_INPUT_BUFFER);

	createInputBufferParams.width = width;
	createInputBufferParams.height = height;
	createInputBufferParams.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;
	createInputBufferParams.bufferFmt = fmt;


	status = _api->nvEncCreateInputBuffer(_encoder, &createInputBufferParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	(*input_buffer) = createInputBufferParams.inputBuffer;

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR inputBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (inputBuffer)
	{
		status = _api->nvEncDestroyInputBuffer(_encoder, inputBuffer);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncCreateMVBuffer(uint32_t size, void** bitstreamBuffer)
{
	NVENCSTATUS status;
	NV_ENC_CREATE_MV_BUFFER stAllocMVBuffer;
	memset(&stAllocMVBuffer, 0, sizeof(stAllocMVBuffer));
	SET_VER(stAllocMVBuffer, NV_ENC_CREATE_MV_BUFFER);
	status = _api->nvEncCreateMVBuffer(_encoder, &stAllocMVBuffer);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}
	*bitstreamBuffer = stAllocMVBuffer.MVBuffer;
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncDestroyMVBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
	NVENCSTATUS status;
	NV_ENC_CREATE_MV_BUFFER stAllocMVBuffer;
	memset(&stAllocMVBuffer, 0, sizeof(stAllocMVBuffer));
	SET_VER(stAllocMVBuffer, NV_ENC_CREATE_MV_BUFFER);
	status = _api->nvEncDestroyMVBuffer(_encoder, bitstreamBuffer);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}
	bitstreamBuffer = NULL;
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncCreateBitstreamBuffer(uint32_t size, void** bitstreamBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_CREATE_BITSTREAM_BUFFER createBitstreamBufferParams;

	memset(&createBitstreamBufferParams, 0, sizeof(createBitstreamBufferParams));
	SET_VER(createBitstreamBufferParams, NV_ENC_CREATE_BITSTREAM_BUFFER);

	createBitstreamBufferParams.size = size;
	createBitstreamBufferParams.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;

	status = _api->nvEncCreateBitstreamBuffer(_encoder, &createBitstreamBufferParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*bitstreamBuffer = createBitstreamBufferParams.bitstreamBuffer;

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (bitstreamBuffer)
	{
		status = _api->nvEncDestroyBitstreamBuffer(_encoder, bitstreamBuffer);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM* lockBitstreamBufferParams)
{
	if (!_api || !_encoder)
		return NV_ENC_ERR_GENERIC;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _api->nvEncLockBitstream(_encoder, lockBitstreamBufferParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncUnlockBitstream(_encoder, bitstreamBuffer);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncLockInputBuffer(void * input_buffer, void ** buffer_data, uint32_t * pitch)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_LOCK_INPUT_BUFFER lock_input_buffer_params;

	memset(&lock_input_buffer_params, 0, sizeof(lock_input_buffer_params));
	SET_VER(lock_input_buffer_params, NV_ENC_LOCK_INPUT_BUFFER);

	lock_input_buffer_params.inputBuffer = input_buffer;
	status = _api->nvEncLockInputBuffer(_encoder, &lock_input_buffer_params);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*buffer_data = lock_input_buffer_params.bufferDataPtr;
	*pitch = lock_input_buffer_params.pitch;

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncUnlockInputBuffer(NV_ENC_INPUT_PTR inputBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncUnlockInputBuffer(_encoder, inputBuffer);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetEncodeStats(NV_ENC_STAT* encodeStats)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetEncodeStats(_encoder, encodeStats);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncGetSequenceParams(NV_ENC_SEQUENCE_PARAM_PAYLOAD* sequenceParamPayload)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _api->nvEncGetSequenceParams(_encoder, sequenceParamPayload);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncRegisterAsyncEvent(void ** completionEvent)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_EVENT_PARAMS eventParams;

	memset(&eventParams, 0, sizeof(eventParams));
	SET_VER(eventParams, NV_ENC_EVENT_PARAMS);

#if defined (WIN32) && defined(WITH_ASYNC)
	eventParams.completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	eventParams.completionEvent = NULL;
#endif
	status = _api->nvEncRegisterAsyncEvent(_encoder, &eventParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*completionEvent = eventParams.completionEvent;

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncUnregisterAsyncEvent(void* completionEvent)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_EVENT_PARAMS eventParams;

	if (completionEvent)
	{
		memset(&eventParams, 0, sizeof(eventParams));
		SET_VER(eventParams, NV_ENC_EVENT_PARAMS);

		eventParams.completionEvent = completionEvent;

		status = _api->nvEncUnregisterAsyncEvent(_encoder, &eventParams);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncMapInputResource(void* registeredResource, void** mappedResource)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_MAP_INPUT_RESOURCE mapInputResParams;

	memset(&mapInputResParams, 0, sizeof(mapInputResParams));
	SET_VER(mapInputResParams, NV_ENC_MAP_INPUT_RESOURCE);

	mapInputResParams.registeredResource = registeredResource;

	status = _api->nvEncMapInputResource(_encoder, &mapInputResParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*mappedResource = mapInputResParams.mappedResource;

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncUnmapInputResource(NV_ENC_INPUT_PTR mappedInputBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (mappedInputBuffer)
	{
		status = _api->nvEncUnmapInputResource(_encoder, mappedInputBuffer);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncDestroyEncoder(void)
{
	if (!_api || !_encoder)
		return NV_ENC_ERR_GENERIC;
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _api->nvEncDestroyEncoder(_encoder);
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncOpenEncodeSessionEx(void* device, NV_ENC_DEVICE_TYPE deviceType)
{
	if (!_api)
		return NV_ENC_ERR_GENERIC;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS open_sessionex_param;

	memset(&open_sessionex_param, 0, sizeof(open_sessionex_param));
	SET_VER(open_sessionex_param, NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS);

	open_sessionex_param.device = device;
	open_sessionex_param.deviceType = deviceType;
	open_sessionex_param.reserved = NULL;
	open_sessionex_param.apiVersion = NVENCAPI_VERSION;

	status = _api->nvEncOpenEncodeSessionEx(&open_sessionex_param, &_encoder);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE resourceType, void* resourceToRegister, uint32_t width, uint32_t height, uint32_t pitch, void** registeredResource)
{
	if (!_api || !_encoder)
		return NV_ENC_ERR_GENERIC;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_REGISTER_RESOURCE register_resource_param;
	memset(&register_resource_param, 0, sizeof(register_resource_param));
	SET_VER(register_resource_param, NV_ENC_REGISTER_RESOURCE);

	register_resource_param.resourceType = resourceType;
	register_resource_param.resourceToRegister = resourceToRegister;
	register_resource_param.width = width;
	register_resource_param.height = height;
	register_resource_param.pitch = pitch;
	register_resource_param.bufferFormat = NV_ENC_BUFFER_FORMAT_NV12_PL;

	status = _api->nvEncRegisterResource(_encoder, &register_resource_param);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*registeredResource = register_resource_param.registeredResource;

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncUnregisterResource(NV_ENC_REGISTERED_PTR registeredRes)
{
	if (!_api || !_encoder)
		return NV_ENC_ERR_GENERIC;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _api->nvEncUnregisterResource(_encoder, registeredRes);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncFlushEncoderQueue(void * eos_event)
{
	if (!_api || !_encoder)
		return NV_ENC_ERR_GENERIC;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_PIC_PARAMS nvenc_pic_param;
	memset(&nvenc_pic_param, 0, sizeof(nvenc_pic_param));
	SET_VER(nvenc_pic_param, NV_ENC_PIC_PARAMS);
	nvenc_pic_param.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
	nvenc_pic_param.completionEvent = eos_event;
	status = _api->nvEncEncodePicture(_encoder, &nvenc_pic_param);
	return status;
}

NVENCSTATUS debuggerking::nvenc_core::NvEncEncodePicture(NV_ENC_PIC_PARAMS * pic_params)
{
	if (!_api || !_encoder)
		return NV_ENC_ERR_GENERIC;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _api->nvEncEncodePicture(_encoder, pic_params);
	return status;
}
/*
NVENCSTATUS nvenc_core::NvRunMotionEstimationOnly(EncodeBuffer *pEncodeBuffer[2], MEOnlyConfig *pMEOnly)
{
	NVENCSTATUS status;
	NV_ENC_MEONLY_PARAMS stMEOnlyParams;
	SET_VER(stMEOnlyParams, NV_ENC_MEONLY_PARAMS);
	stMEOnlyParams.referenceFrame = pEncodeBuffer[0]->stInputBfr.hInputSurface;
	stMEOnlyParams.inputBuffer = pEncodeBuffer[1]->stInputBfr.hInputSurface;
	stMEOnlyParams.bufferFmt = pEncodeBuffer[1]->stInputBfr.bufferFmt;
	stMEOnlyParams.inputWidth = pEncodeBuffer[1]->stInputBfr.dwWidth;
	stMEOnlyParams.inputHeight = pEncodeBuffer[1]->stInputBfr.dwHeight;
	stMEOnlyParams.outputMV = pEncodeBuffer[0]->stOutputBfr.hBitstreamBuffer;
	status = _api->nvEncRunMotionEstimationOnly(_nvenc_encoder, &stMEOnlyParams);

	if (m_fOutput)
	{
		unsigned int numMBs = ((m_uMaxWidth + 15) >> 4) * ((m_uMaxHeight + 15) >> 4);
		fprintf(m_fOutput, "Motion Vectors for input frame = %d, reference frame = %d\n", pMEOnly->inputFrameIndex, pMEOnly->referenceFrameIndex);
		NV_ENC_H264_MV_DATA *outputMV = (NV_ENC_H264_MV_DATA *)stMEOnlyParams.outputMV;
		for (unsigned int i = 0; i < numMBs; i++)
		{
			fprintf(m_fOutput, "block = %d, mb_type = %d, partitionType = %d, MV[0].x = %d, MV[0].y = %d, MV[1].x = %d, MV[1].y = %d, MV[2].x = %d, MV[2].y = %d, MV[3].x = %d, MV[3].y = %d, cost=%d ", \
				i, outputMV[i].mb_type, outputMV[i].partitionType, outputMV[i].MV[0].mvx, outputMV[i].MV[0].mvy, outputMV[i].MV[1].mvx, outputMV[i].MV[1].mvy, \
				outputMV[i].MV[2].mvx, outputMV[i].MV[2].mvy, outputMV[i].MV[3].mvx, outputMV[i].MV[3].mvy, outputMV[i].MBCost);
			fprintf(m_fOutput, "\n");
		}
		fprintf(m_fOutput, "\n");
	}
	return status;
}

NVENCSTATUS nvenc_core::NvEncInvalidateRefFrames(const NvEncPictureCommand *pEncPicCommand)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	for (uint32_t i = 0; i < pEncPicCommand->numRefFramesToInvalidate; i++)
	{
		status = _api->nvEncInvalidateRefFrames(_nvenc_encoder, pEncPicCommand->refFrameNumbers[i]);
	}

	return status;
}
NVENCSTATUS nvenc_core::NvEncReconfigureEncoder(const NvEncPictureCommand *pEncPicCommand)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (pEncPicCommand->bBitrateChangePending || pEncPicCommand->bResolutionChangePending)
	{
		if (pEncPicCommand->bResolutionChangePending)
		{
			m_uCurWidth = pEncPicCommand->newWidth;
			m_uCurHeight = pEncPicCommand->newHeight;
			if ((m_uCurWidth > m_uMaxWidth) || (m_uCurHeight > m_uMaxHeight))
			{
				return NV_ENC_ERR_INVALID_PARAM;
			}
			m_stCreateEncodeParams.encodeWidth = m_uCurWidth;
			m_stCreateEncodeParams.encodeHeight = m_uCurHeight;
			m_stCreateEncodeParams.darWidth = m_uCurWidth;
			m_stCreateEncodeParams.darHeight = m_uCurHeight;
		}

		if (pEncPicCommand->bBitrateChangePending)
		{
			m_stEncodeConfig.rcParams.averageBitRate = pEncPicCommand->newBitrate;
			m_stEncodeConfig.rcParams.maxBitRate = pEncPicCommand->newBitrate;
			m_stEncodeConfig.rcParams.vbvBufferSize = pEncPicCommand->newVBVSize != 0 ? pEncPicCommand->newVBVSize : (pEncPicCommand->newBitrate * m_stCreateEncodeParams.frameRateDen) / m_stCreateEncodeParams.frameRateNum;
			m_stEncodeConfig.rcParams.vbvInitialDelay = m_stEncodeConfig.rcParams.vbvBufferSize;
		}

		NV_ENC_RECONFIGURE_PARAMS stReconfigParams;
		memset(&stReconfigParams, 0, sizeof(stReconfigParams));
		memcpy(&stReconfigParams.reInitEncodeParams, &m_stCreateEncodeParams, sizeof(m_stCreateEncodeParams));
		stReconfigParams.version = NV_ENC_RECONFIGURE_PARAMS_VER;
		stReconfigParams.forceIDR = pEncPicCommand->bResolutionChangePending ? 1 : 0;

		status = _api->nvEncReconfigureEncoder(_nvenc_encoder, &stReconfigParams);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}
*/