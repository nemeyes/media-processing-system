#include "nvenc_encoder.h"
#include <tchar.h>
#include <vector_types.h>
#include <cuda.h>

#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define FABS(a) ((a) >= 0 ? (a) : -(a))

nvenc_encoder::nvenc_encoder(void)
	: _state(dk_nvenc_encoder::encoder_state_none)
{

}

nvenc_encoder::~nvenc_encoder(void)
{

}

dk_nvenc_encoder::encoder_state nvenc_encoder::state(void)
{
	return _state;
}

dk_nvenc_encoder::err_code nvenc_encoder::initialize_encoder(dk_nvenc_encoder::configuration_t * config)
{
	if ((_state != dk_nvenc_encoder::encoder_state_none) && (_state != dk_nvenc_encoder::encoder_state_released))
		return dk_nvenc_encoder::err_code_fail;

	release_encoder();
	_state = dk_nvenc_encoder::encoder_state_initializing;

	_config = config;
	NVENCSTATUS status = NV_ENC_SUCCESS;
	dk_nvenc_encoder::err_code result = dk_nvenc_encoder::err_code_fail;

	if (_config->mem_type == dk_nvenc_encoder::memory_type_cuda)
	{
		status = initialize_cuda(_config->device_id);
		if (status != NV_ENC_SUCCESS)
		{
			release_cuda();
			return result;
		}
		status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_CUDA);
	}
	else if (_config->mem_type == dk_nvenc_encoder::memory_type_dx10)
	{
		status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_DIRECTX);
	}
	else if (_config->mem_type == dk_nvenc_encoder::memory_type_dx10)
	{
		status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_DIRECTX);
	}
	else if (_config->mem_type == dk_nvenc_encoder::memory_type_dx11)
	{
		status = initialize_nvenc_encoder(_context, NV_ENC_DEVICE_TYPE_DIRECTX);
	}

	NV_ENC_INITIALIZE_PARAMS nvenc_initialize_param;
	memset(&nvenc_initialize_param, 0x00, sizeof(nvenc_initialize_param));
	SET_VER(nvenc_initialize_param, NV_ENC_INITIALIZE_PARAMS);

	NV_ENC_CONFIG nvenc_config;
	memset(&nvenc_config, 0x00, sizeof(nvenc_config));
	SET_VER(nvenc_config, NV_ENC_CONFIG);

	nvenc_initialize_param.encodeConfig = &nvenc_config;

	if ((_config->codec >= dk_nvenc_encoder::submedia_type_h264) && (_config->codec <= dk_nvenc_encoder::submedia_type_h264_ep))
	{
		nvenc_initialize_param.encodeGUID = NV_ENC_CODEC_H264_GUID;
		switch (_config->preset)
		{
		case dk_nvenc_encoder::preset_default :
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
			break;
		case dk_nvenc_encoder::preset_high_performance:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_HP_GUID;
			break;
		case dk_nvenc_encoder::preset_high_quality:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_HQ_GUID;
			break;
		case dk_nvenc_encoder::preset_bluelay_disk:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_BD_GUID;
			break;
		case dk_nvenc_encoder::preset_low_latency_default:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID;
			break;
		case dk_nvenc_encoder::preset_low_latency_high_quality:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
			break;
		case dk_nvenc_encoder::preset_low_latency_high_performance:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
			break;
		case dk_nvenc_encoder::preset_lossless_default:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOSSLESS_DEFAULT_GUID;
			break;
		case dk_nvenc_encoder::preset_lossless_high_performance:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOSSLESS_HP_GUID;
			break;
		}
	}
	else if ((_config->codec >= dk_nvenc_encoder::submedia_type_hevc) && (_config->codec <= dk_nvenc_encoder::submedia_type_hevc_mp))
	{
		nvenc_initialize_param.encodeGUID = NV_ENC_CODEC_HEVC_GUID;
		switch (_config->preset)
		{
		case dk_nvenc_encoder::preset_default:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
			break;
		case dk_nvenc_encoder::preset_high_performance:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_HP_GUID;
			break;
		case dk_nvenc_encoder::preset_high_quality:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_HQ_GUID;
			break;
		case dk_nvenc_encoder::preset_bluelay_disk:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_BD_GUID;
			break;
		case dk_nvenc_encoder::preset_low_latency_default:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID;
			break;
		case dk_nvenc_encoder::preset_low_latency_high_quality:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
			break;
		case dk_nvenc_encoder::preset_low_latency_high_performance:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
			break;
		case dk_nvenc_encoder::preset_lossless_default:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOSSLESS_DEFAULT_GUID;
			break;
		case dk_nvenc_encoder::preset_lossless_high_performance:
			nvenc_initialize_param.presetGUID = NV_ENC_PRESET_LOSSLESS_HP_GUID;
			break;
		}

	}

	nvenc_initialize_param.encodeWidth = _config->width;
	nvenc_initialize_param.encodeHeight = _config->height;
	nvenc_initialize_param.darWidth = _config->width;
	nvenc_initialize_param.darHeight = _config->height;
	nvenc_initialize_param.maxEncodeWidth = _config->width;
	nvenc_initialize_param.maxEncodeHeight = _config->height;
	nvenc_initialize_param.frameRateNum = _config->fps;
	nvenc_initialize_param.frameRateDen = 1;
#if defined(_WIN32)
	nvenc_initialize_param.enableEncodeAsync = 1;
#else
	nvenc_initialize_param.enableEncodeAsync = 0;
#endif
	nvenc_initialize_param.enablePTD = 1;
	nvenc_initialize_param.reportSliceOffsets = 0;
	nvenc_initialize_param.enableSubFrameWrite = 0;

	// apply preset
	NV_ENC_PRESET_CONFIG nvenc_preset_config;
	memset(&nvenc_preset_config, 0, sizeof(NV_ENC_PRESET_CONFIG));
	SET_VER(nvenc_preset_config, NV_ENC_PRESET_CONFIG);
	SET_VER(nvenc_preset_config.presetCfg, NV_ENC_CONFIG);

	status = NvEncGetEncodePresetConfig(nvenc_initialize_param.encodeGUID, nvenc_initialize_param.presetGUID, &nvenc_preset_config);
	if (status != NV_ENC_SUCCESS)
	{
		return dk_nvenc_encoder::err_code_fail;
	}
	memcpy(nvenc_initialize_param.encodeConfig, &nvenc_preset_config.presetCfg, sizeof(NV_ENC_CONFIG));
	if (_config->keyframe_interval <= 0)
		nvenc_initialize_param.encodeConfig->gopLength = NVENC_INFINITE_GOPLENGTH;
	else
		nvenc_initialize_param.encodeConfig->gopLength = _config->keyframe_interval * _config->fps;

	nvenc_initialize_param.encodeConfig->frameIntervalP = _config->numb + 1;
	nvenc_initialize_param.encodeConfig->frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE(_config->frame_field_mode);
	_config->motioin_vector_precision = NV_ENC_MV_PRECISION(_config->motioin_vector_precision);
	if (_config->bitrate || _config->vbv_max_bitrate)
	{
		nvenc_initialize_param.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_MODE(_config->rc_mode);
		nvenc_initialize_param.encodeConfig->rcParams.averageBitRate = _config->bitrate;
		nvenc_initialize_param.encodeConfig->rcParams.maxBitRate = _config->vbv_max_bitrate;
		nvenc_initialize_param.encodeConfig->rcParams.vbvBufferSize = _config->vbv_size;
		nvenc_initialize_param.encodeConfig->rcParams.vbvInitialDelay = _config->vbv_size * 9 / 10;
	}
	else
	{
		nvenc_initialize_param.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CONSTQP;
	}
	if (_config->rc_mode == dk_nvenc_encoder::rate_control_constant_qp)
	{
		nvenc_initialize_param.encodeConfig->rcParams.constQP.qpInterP = nvenc_initialize_param.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : _config->qp;
		nvenc_initialize_param.encodeConfig->rcParams.constQP.qpInterB = nvenc_initialize_param.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : _config->qp;
		nvenc_initialize_param.encodeConfig->rcParams.constQP.qpIntra = nvenc_initialize_param.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : _config->qp;
	}

	// set up initial QP value
	if (_config->rc_mode == dk_nvenc_encoder::rate_control_vbr || 
		_config->rc_mode == dk_nvenc_encoder::rate_control_vbr_min_qp ||
		_config->rc_mode == dk_nvenc_encoder::rate_control_two_pass_vbr) 
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
		nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.chromaFormatIDC = 1;
		nvenc_initialize_param.encodeConfig->encodeCodecConfig.h264Config.idrPeriod = nvenc_initialize_param.encodeConfig->gopLength;
	}
	else if (nvenc_initialize_param.encodeGUID == NV_ENC_CODEC_HEVC_GUID)
	{
		nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.chromaFormatIDC = 1;
		nvenc_initialize_param.encodeConfig->encodeCodecConfig.hevcConfig.idrPeriod = nvenc_initialize_param.encodeConfig->gopLength;
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

	if (status == NV_ENC_SUCCESS)
		return dk_nvenc_encoder::err_code_success;
	else
		return dk_nvenc_encoder::err_code_fail;
}

dk_nvenc_encoder::err_code nvenc_encoder::release_encoder(void)
{
	dk_nvenc_encoder::err_code_success;
}

dk_nvenc_encoder::err_code nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input, dk_nvenc_encoder::dk_video_entity_t * bitstream)
{
	dk_nvenc_encoder::err_code_success;
}

dk_nvenc_encoder::err_code nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input)
{
	dk_nvenc_encoder::err_code_success;
}

dk_nvenc_encoder::err_code nvenc_encoder::get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream)
{
	dk_nvenc_encoder::err_code_success;
}

dk_nvenc_encoder::err_code nvenc_encoder::encode_async(dk_nvenc_encoder::dk_video_entity_t * input)
{
	dk_nvenc_encoder::err_code_success;
}

dk_nvenc_encoder::err_code nvenc_encoder::check_encoding_flnish(void)
{
	dk_nvenc_encoder::err_code_success;
}

NVENCSTATUS nvenc_encoder::initialize_cuda(uint32_t device_id)
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
	CUDADRIVER hHandleDriver = 0;

	result = cuInit(0, __CUDA_API_VERSION, hHandleDriver);
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

NVENCSTATUS nvenc_encoder::release_cuda(void)
{
	CUresult result = cuCtxDestroy((CUcontext)_context);
	if (result != CUDA_SUCCESS)
		return NV_ENC_ERR_GENERIC;
	return NV_ENC_SUCCESS;
}


NVENCSTATUS nvenc_encoder::initialize_nvenc_encoder(void * device, NV_ENC_DEVICE_TYPE type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	MYPROC nvenc_api_create_instance;

#if defined(_WIN32)
#if defined(_WIN64)
	_nvenc_instance = ::LoadLibrary(_T("nvEncodeAPI64.dll"));
#else
	_nvenc_instance = ::LoadLibrary(_T("nvEncodeAPI.dll"));
#endif
#else
	_nvenc_instance = dlopen("libnvidia-encode.so.1", RTLD_LAZY);
#endif

	if (_nvenc_instance == 0)
		return NV_ENC_ERR_OUT_OF_MEMORY;

#if defined(_WIN32)
	nvenc_api_create_instance = (MYPROC)::GetProcAddress(_nvenc_instance, "NvEncodeAPICreateInstance");
#else
	nvenc_api_create_instance = (MYPROC)dlsym(_nvenc_instance, "NvEncodeAPICreateInstance");
#endif

	if (!nvenc_api_create_instance)
		return NV_ENC_ERR_OUT_OF_MEMORY;

	_nvenc_api = new NV_ENCODE_API_FUNCTION_LIST;
	if (!_nvenc_api)
		return NV_ENC_ERR_OUT_OF_MEMORY;

	memset(_nvenc_api, 0x00, sizeof(NV_ENCODE_API_FUNCTION_LIST));
	_nvenc_api->version = NV_ENCODE_API_FUNCTION_LIST_VER;
	status = nvenc_api_create_instance(_nvenc_api);
	if (status != NV_ENC_SUCCESS)
		return status;

	__try
	{
		_nvenc_encoder = nullptr;
		status = NvEncOpenEncodeSessionEx(device, type);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return NV_ENC_ERR_GENERIC;
	}

	return status;
}

NVENCSTATUS nvenc_encoder::release_nvenc_encoder(void)
{

}





NVENCSTATUS nvenc_encoder::NvEncInitializeEncoder(NV_ENC_INITIALIZE_PARAMS * params)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _nvenc_api->nvEncInitializeEncoder(_nvenc_encoder, params);
	return status;

}

NVENCSTATUS nvenc_encoder::NvEncOpenEncodeSession(void* device, uint32_t deviceType)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncOpenEncodeSession(device, deviceType, &_nvenc_encoder);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeGUIDCount(uint32_t* encodeGUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeGUIDCount(_nvenc_encoder, encodeGUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeProfileGUIDCount(GUID encodeGUID, uint32_t* encodeProfileGUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeProfileGUIDCount(_nvenc_encoder, encodeGUID, encodeProfileGUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeProfileGUIDs(GUID encodeGUID, GUID* profileGUIDs, uint32_t guidArraySize, uint32_t* GUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeProfileGUIDs(_nvenc_encoder, encodeGUID, profileGUIDs, guidArraySize, GUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeGUIDs(GUID* GUIDs, uint32_t guidArraySize, uint32_t* GUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeGUIDs(_nvenc_encoder, GUIDs, guidArraySize, GUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetInputFormatCount(GUID encodeGUID, uint32_t* inputFmtCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetInputFormatCount(_nvenc_encoder, encodeGUID, inputFmtCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetInputFormats(GUID encodeGUID, NV_ENC_BUFFER_FORMAT* inputFmts, uint32_t inputFmtArraySize, uint32_t* inputFmtCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetInputFormats(_nvenc_encoder, encodeGUID, inputFmts, inputFmtArraySize, inputFmtCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeCaps(GUID encodeGUID, NV_ENC_CAPS_PARAM* capsParam, int* capsVal)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeCaps(_nvenc_encoder, encodeGUID, capsParam, capsVal);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodePresetCount(GUID encodeGUID, uint32_t* encodePresetGUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodePresetCount(_nvenc_encoder, encodeGUID, encodePresetGUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodePresetGUIDs(GUID encodeGUID, GUID* presetGUIDs, uint32_t guidArraySize, uint32_t* encodePresetGUIDCount)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodePresetGUIDs(_nvenc_encoder, encodeGUID, presetGUIDs, guidArraySize, encodePresetGUIDCount);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodePresetConfig(GUID encodeGUID, GUID  presetGUID, NV_ENC_PRESET_CONFIG* presetConfig)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodePresetConfig(_nvenc_encoder, encodeGUID, presetGUID, presetConfig);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncCreateInputBuffer(uint32_t width, uint32_t height, void** inputBuffer, uint32_t isYuv444)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_CREATE_INPUT_BUFFER createInputBufferParams;

	memset(&createInputBufferParams, 0, sizeof(createInputBufferParams));
	SET_VER(createInputBufferParams, NV_ENC_CREATE_INPUT_BUFFER);

	createInputBufferParams.width = width;
	createInputBufferParams.height = height;
	createInputBufferParams.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;
	createInputBufferParams.bufferFmt = isYuv444 ? NV_ENC_BUFFER_FORMAT_YUV444_PL : NV_ENC_BUFFER_FORMAT_NV12_PL;

	status = _nvenc_api->nvEncCreateInputBuffer(_nvenc_encoder, &createInputBufferParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*inputBuffer = createInputBufferParams.inputBuffer;

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR inputBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (inputBuffer)
	{
		status = _nvenc_api->nvEncDestroyInputBuffer(_nvenc_encoder, inputBuffer);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncCreateMVBuffer(uint32_t size, void** bitstreamBuffer)
{
	NVENCSTATUS status;
	NV_ENC_CREATE_MV_BUFFER stAllocMVBuffer;
	memset(&stAllocMVBuffer, 0, sizeof(stAllocMVBuffer));
	SET_VER(stAllocMVBuffer, NV_ENC_CREATE_MV_BUFFER);
	status = _nvenc_api->nvEncCreateMVBuffer(_nvenc_encoder, &stAllocMVBuffer);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}
	*bitstreamBuffer = stAllocMVBuffer.MVBuffer;
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncDestroyMVBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
	NVENCSTATUS status;
	NV_ENC_CREATE_MV_BUFFER stAllocMVBuffer;
	memset(&stAllocMVBuffer, 0, sizeof(stAllocMVBuffer));
	SET_VER(stAllocMVBuffer, NV_ENC_CREATE_MV_BUFFER);
	status = _nvenc_api->nvEncDestroyMVBuffer(_nvenc_encoder, bitstreamBuffer);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}
	bitstreamBuffer = NULL;
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncCreateBitstreamBuffer(uint32_t size, void** bitstreamBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_CREATE_BITSTREAM_BUFFER createBitstreamBufferParams;

	memset(&createBitstreamBufferParams, 0, sizeof(createBitstreamBufferParams));
	SET_VER(createBitstreamBufferParams, NV_ENC_CREATE_BITSTREAM_BUFFER);

	createBitstreamBufferParams.size = size;
	createBitstreamBufferParams.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;

	status = _nvenc_api->nvEncCreateBitstreamBuffer(_nvenc_encoder, &createBitstreamBufferParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*bitstreamBuffer = createBitstreamBufferParams.bitstreamBuffer;

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (bitstreamBuffer)
	{
		status = _nvenc_api->nvEncDestroyBitstreamBuffer(_nvenc_encoder, bitstreamBuffer);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM* lockBitstreamBufferParams)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncLockBitstream(_nvenc_encoder, lockBitstreamBufferParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstreamBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncUnlockBitstream(_nvenc_encoder, bitstreamBuffer);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncLockInputBuffer(void* inputBuffer, void** bufferDataPtr, uint32_t* pitch)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_LOCK_INPUT_BUFFER lockInputBufferParams;

	memset(&lockInputBufferParams, 0, sizeof(lockInputBufferParams));
	SET_VER(lockInputBufferParams, NV_ENC_LOCK_INPUT_BUFFER);

	lockInputBufferParams.inputBuffer = inputBuffer;
	status = _nvenc_api->nvEncLockInputBuffer(_nvenc_encoder, &lockInputBufferParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*bufferDataPtr = lockInputBufferParams.bufferDataPtr;
	*pitch = lockInputBufferParams.pitch;

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnlockInputBuffer(NV_ENC_INPUT_PTR inputBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncUnlockInputBuffer(_nvenc_encoder, inputBuffer);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeStats(NV_ENC_STAT* encodeStats)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeStats(_nvenc_encoder, encodeStats);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetSequenceParams(NV_ENC_SEQUENCE_PARAM_PAYLOAD* sequenceParamPayload)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetSequenceParams(_nvenc_encoder, sequenceParamPayload);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncRegisterAsyncEvent(void** completionEvent)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_EVENT_PARAMS eventParams;

	memset(&eventParams, 0, sizeof(eventParams));
	SET_VER(eventParams, NV_ENC_EVENT_PARAMS);

#if defined (NV_WINDOWS)
	eventParams.completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	eventParams.completionEvent = NULL;
#endif
	status = _nvenc_api->nvEncRegisterAsyncEvent(_nvenc_encoder, &eventParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*completionEvent = eventParams.completionEvent;

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnregisterAsyncEvent(void* completionEvent)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_EVENT_PARAMS eventParams;

	if (completionEvent)
	{
		memset(&eventParams, 0, sizeof(eventParams));
		SET_VER(eventParams, NV_ENC_EVENT_PARAMS);

		eventParams.completionEvent = completionEvent;

		status = _nvenc_api->nvEncUnregisterAsyncEvent(_nvenc_encoder, &eventParams);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncMapInputResource(void* registeredResource, void** mappedResource)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_MAP_INPUT_RESOURCE mapInputResParams;

	memset(&mapInputResParams, 0, sizeof(mapInputResParams));
	SET_VER(mapInputResParams, NV_ENC_MAP_INPUT_RESOURCE);

	mapInputResParams.registeredResource = registeredResource;

	status = _nvenc_api->nvEncMapInputResource(_nvenc_encoder, &mapInputResParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*mappedResource = mapInputResParams.mappedResource;

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnmapInputResource(NV_ENC_INPUT_PTR mappedInputBuffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (mappedInputBuffer)
	{
		status = _nvenc_api->nvEncUnmapInputResource(_nvenc_encoder, mappedInputBuffer);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncDestroyEncoder(void)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _nvenc_api->nvEncDestroyEncoder(_nvenc_encoder);
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncOpenEncodeSessionEx(void* device, NV_ENC_DEVICE_TYPE deviceType)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS openSessionExParams;

	memset(&openSessionExParams, 0, sizeof(openSessionExParams));
	SET_VER(openSessionExParams, NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS);

	openSessionExParams.device = device;
	openSessionExParams.deviceType = deviceType;
	openSessionExParams.reserved = NULL;
	openSessionExParams.apiVersion = NVENCAPI_VERSION;

	status = _nvenc_api->nvEncOpenEncodeSessionEx(&openSessionExParams, &_nvenc_encoder);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE resourceType, void* resourceToRegister, uint32_t width, uint32_t height, uint32_t pitch, void** registeredResource)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_REGISTER_RESOURCE registerResParams;

	memset(&registerResParams, 0, sizeof(registerResParams));
	SET_VER(registerResParams, NV_ENC_REGISTER_RESOURCE);

	registerResParams.resourceType = resourceType;
	registerResParams.resourceToRegister = resourceToRegister;
	registerResParams.width = width;
	registerResParams.height = height;
	registerResParams.pitch = pitch;
	registerResParams.bufferFormat = NV_ENC_BUFFER_FORMAT_NV12_PL;

	status = _nvenc_api->nvEncRegisterResource(_nvenc_encoder, &registerResParams);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	*registeredResource = registerResParams.registeredResource;

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnregisterResource(NV_ENC_REGISTERED_PTR registeredRes)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncUnregisterResource(_nvenc_encoder, registeredRes);
	if (status != NV_ENC_SUCCESS)
	{
		assert(0);
	}

	return status;
}
/*
NVENCSTATUS nvenc_encoder::NvRunMotionEstimationOnly(EncodeBuffer *pEncodeBuffer[2], MEOnlyConfig *pMEOnly)
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
	status = _nvenc_api->nvEncRunMotionEstimationOnly(_nvenc_encoder, &stMEOnlyParams);

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

NVENCSTATUS nvenc_encoder::NvEncInvalidateRefFrames(const NvEncPictureCommand *pEncPicCommand)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	for (uint32_t i = 0; i < pEncPicCommand->numRefFramesToInvalidate; i++)
	{
		status = _nvenc_api->nvEncInvalidateRefFrames(_nvenc_encoder, pEncPicCommand->refFrameNumbers[i]);
	}

	return status;
}
NVENCSTATUS nvenc_encoder::NvEncReconfigureEncoder(const NvEncPictureCommand *pEncPicCommand)
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

		status = _nvenc_api->nvEncReconfigureEncoder(_nvenc_encoder, &stReconfigParams);
		if (status != NV_ENC_SUCCESS)
		{
			assert(0);
		}
	}

	return status;
}
*/