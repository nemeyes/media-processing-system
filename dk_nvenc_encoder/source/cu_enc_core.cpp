#include "cu_enc_core.h"
#include <vector_types.h>
#include <cuda.h>

#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024

#if defined(WITH_DYNAMIC_CUDA_LOAD)
extern void interleave_uv(void* driver_api, unsigned int width, unsigned int height, unsigned char * src, unsigned int src_pitch, CUdeviceptr dst, unsigned int dst_pitch, CUdeviceptr cb, CUdeviceptr cr);
#else
#pragma comment(lib, "cuda.lib")
extern void interleave_uv(unsigned int width, unsigned int height, unsigned char * src, unsigned int src_pitch, CUdeviceptr dst, unsigned int dst_pitch, CUdeviceptr cb, CUdeviceptr cr);
#endif

cu_enc_core::cu_enc_core(void)
	: _binit(false)
	, _encoder(NULL)
	, _encoder_api(NULL)
	, _cu_encoder_inst(NULL)
	, _cu_context(NULL)
	, _enc_buffer_count(0)
{
	memset(&_enc_config, 0, sizeof(_enc_config));
	memset(&_enc_buffer, 0, sizeof(_enc_buffer));
	memset(&_enc_eos_output_buffer, 0, sizeof(_enc_eos_output_buffer));

#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
	memset(_ptr_cu_chroma, 0, sizeof(_ptr_cu_chroma));
#endif
}

cu_enc_core::~cu_enc_core(void)
{
	// clean up encode API resources here
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::initialize(dk_nvenc_encoder::configuration_t config, unsigned int * pitch)
{
	if (_binit)
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	dk_nvenc_encoder::ERR_CODE result = dk_nvenc_encoder::ERR_CODE_FAILED;
	_enc_config = config;
	result = initialize_cuda();
	if (result != dk_nvenc_encoder::ERR_CODE_SUCCESS)
	{
		release_cuda();
		return result;
	}

	status = initialize_encoder(NV_ENC_DEVICE_TYPE_CUDA);
	if (status != NV_ENC_SUCCESS)
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;
	}

	if (dk_nvenc_encoder::ERR_CODE_SUCCESS != is_h264_supported())
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;
	}

	//if (dk_nvenc_encoder::ERR_CODE_SUCCESS != is_async_encode_supported())
	//	return dk_nvenc_encoder::ERR_CODE_FAILED;
	
	int max_width = 0;
	if (dk_nvenc_encoder::ERR_CODE_SUCCESS != get_max_width(max_width))
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	}
		
	int max_height = 0;
	if (dk_nvenc_encoder::ERR_CODE_SUCCESS != get_max_height(max_height))
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	}

#if !defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)
	switch (_enc_config.cs)
	{
	case dk_nvenc_encoder::COLOR_SPACE_YUY2:
		_input_format = NV_ENC_BUFFER_FORMAT_NV12_PL;
		break;
	case dk_nvenc_encoder::COLOR_SPACE_YV12:
		_input_format = NV_ENC_BUFFER_FORMAT_YV12_PL;
		break;
	case dk_nvenc_encoder::COLOR_SPACE_NV12:
		_input_format = NV_ENC_BUFFER_FORMAT_NV12_PL;
		break;
	case dk_nvenc_encoder::COLOR_SPACE_RGB24:
		_input_format = NV_ENC_BUFFER_FORMAT_NV12_PL;
		break;
	case dk_nvenc_encoder::COLOR_SPACE_RGB32:
		_input_format = NV_ENC_BUFFER_FORMAT_NV12_PL;
		break;
	}
#endif

	_enc_config.max_width = max_width;
	_enc_config.max_height = max_height;

	_enc_config.width = (_enc_config.width > _enc_config.max_width ? _enc_config.max_width : _enc_config.width);
	_enc_config.height = (_enc_config.height > _enc_config.max_height ? _enc_config.max_height : _enc_config.height);
	_enc_config.bitstream_buffer_size = (_enc_config.bitrate / 8)*(/*_enc_config.gop* / _enc_config.fps*/_enc_config.keyframe_interval) * 2;


	NV_ENC_INITIALIZE_PARAMS init_params;
	memset(&init_params, 0x00, sizeof(init_params));
	SET_VER(init_params, NV_ENC_INITIALIZE_PARAMS);
	NV_ENC_CONFIG encode_config;
	memset(&encode_config, 0x00, sizeof(encode_config));
	SET_VER(encode_config, NV_ENC_CONFIG);
	init_params.encodeConfig = &encode_config;
	init_params.encodeGUID = NV_ENC_CODEC_H264_GUID;

	switch (_enc_config.auto_preset)
	{
		case dk_nvenc_encoder::CU_ENC_AUTO_PRESET_NOT_APPLICABLE:
		{
			if (_enc_config.auto_preset == dk_nvenc_encoder::CU_ENC_AUTO_PRESET_NOT_APPLICABLE)
			{
				switch (_enc_config.profile)
				{
				case dk_nvenc_encoder::CODEC_PROFILE_TYPE_AUTOSELECT:
					init_params.encodeConfig->profileGUID = NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID;
					break;
				case dk_nvenc_encoder::CODEC_PROFILE_TYPE_BASELINE:
					init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_BASELINE_GUID;
					break;
				case dk_nvenc_encoder::CODEC_PROFILE_TYPE_MAIN:
					init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_MAIN_GUID;
					break;
				case dk_nvenc_encoder::CODEC_PROFILE_TYPE_HIGH:
					init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
					break;
				}
			}

#if 0//defined(CHOOSE_AUTOMATIC)
			if (_enc_config.height*_enc_config.width >= 1280 * 1024)
			{
				init_params.presetGUID = NV_ENC_PRESET_HQ_GUID;
			}
			else if ((_enc_config.height*_enc_config.width > 1280 * 720) || ((_enc_config.height*_enc_config.width == 1280 * 720) && _enc_config.fps > 30))
			{
				init_params.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
			}
			else
			{
				init_params.presetGUID = NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID;
			}
#else
			switch (_enc_config.preset)
			{
			case dk_nvenc_encoder::PRESET_TYPE_DEFAULT:
				init_params.presetGUID = NV_ENC_PRESET_DEFAULT_GUID;
				break;
			case dk_nvenc_encoder::PRESET_TYPE_HP:
				init_params.presetGUID = NV_ENC_PRESET_HP_GUID;
				break;
			case dk_nvenc_encoder::PRESET_TYPE_HQ:
				init_params.presetGUID = NV_ENC_PRESET_HQ_GUID;
				break;
			case dk_nvenc_encoder::PRESET_TYPE_BD:
				init_params.presetGUID = NV_ENC_PRESET_BD_GUID;
				break;
			case dk_nvenc_encoder::PRESET_TYPE_LOW_LATENCY_DEFAULT:
				init_params.presetGUID = NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID;
				break;
			case dk_nvenc_encoder::PRESET_TYPE_LOW_LATENCY_HQ:
				init_params.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
				break;
			case dk_nvenc_encoder::PRESET_TYPE_LOW_LATENCY_HP:
				init_params.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
				break;
			case dk_nvenc_encoder::PRESET_TYPE_LOSSLESS_DEFAULT:
				init_params.presetGUID = NV_ENC_PRESET_LOSSLESS_DEFAULT_GUID;
				break;
			case dk_nvenc_encoder::PRESET_TYPE_LOSSLESS_HP:
				init_params.presetGUID = NV_ENC_PRESET_LOSSLESS_HP_GUID;
				break;
			}
#endif
			NV_ENC_PRESET_CONFIG preset_config;
			memset(&preset_config, 0, sizeof(NV_ENC_PRESET_CONFIG));
			SET_VER(preset_config, NV_ENC_PRESET_CONFIG);
			SET_VER(preset_config.presetCfg, NV_ENC_CONFIG);

			init_params.encodeWidth = _enc_config.width;
			init_params.encodeHeight = _enc_config.height;
			init_params.darWidth = _enc_config.width;
			init_params.darHeight = _enc_config.height;
			init_params.frameRateNum = _enc_config.fps;
			init_params.frameRateDen = 1;
#if defined(_WIN32) && defined(WITH_ASYNC)
			init_params.enableEncodeAsync = 1;
#else
			init_params.enableEncodeAsync = 0;
#endif
			init_params.enablePTD = 1;
			init_params.reportSliceOffsets = 0;
			init_params.enableSubFrameWrite = 0;
			init_params.maxEncodeWidth = _enc_config.max_width;
			init_params.maxEncodeHeight = _enc_config.max_height;

			init_params.encodeConfig->gopLength = _enc_config.keyframe_interval*_enc_config.fps;
			init_params.encodeConfig->frameIntervalP = _enc_config.numb + 1;
			init_params.encodeConfig->frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
			init_params.encodeConfig->mvPrecision = NV_ENC_MV_PRECISION_DEFAULT;
			init_params.encodeConfig->rcParams.averageBitRate = _enc_config.bitrate;
			init_params.encodeConfig->rcParams.maxBitRate = _enc_config.vbv_max_bitrate;
			init_params.encodeConfig->rcParams.vbvBufferSize = _enc_config.vbv_size;
			init_params.encodeConfig->rcParams.vbvInitialDelay = _enc_config.vbv_size * 9 / 10;//_enc_config.vbv_size;//;
			init_params.encodeConfig->encodeCodecConfig.h264Config.chromaFormatIDC = 1;
			init_params.encodeConfig->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_AUTOSELECT;// NV_ENC_H264_ENTROPY_CODING_MODE_AUTOSELECT;
			init_params.encodeConfig->encodeCodecConfig.h264Config.level = NV_ENC_LEVEL_AUTOSELECT;
			init_params.encodeConfig->encodeCodecConfig.h264Config.repeatSPSPPS = 1;

			if (_enc_config.rc_mode == dk_nvenc_encoder::RC_MODE_CBR || _enc_config.rc_mode == dk_nvenc_encoder::RC_MODE_VBR ||
				_enc_config.rc_mode == dk_nvenc_encoder::RC_MODE_2_PASS_FRAMESIZE_CAP || _enc_config.rc_mode == dk_nvenc_encoder::RC_MODE_2_PASS_VBR)
			{
				init_params.encodeConfig->rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)_enc_config.rc_mode;
				init_params.encodeConfig->frameIntervalP = 1;
				init_params.encodeConfig->encodeCodecConfig.h264Config.adaptiveTransformMode = NV_ENC_H264_ADAPTIVE_TRANSFORM_ENABLE;
				init_params.encodeConfig->encodeCodecConfig.h264Config.fmoMode = NV_ENC_H264_FMO_DISABLE;
			}
			else
			{
				init_params.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_VBR_MINQP;
				init_params.encodeConfig->rcParams.enableMinQP = 1;
				//m_stEncodeConfig.rcParams.minQP.qpInterB = 32 - quality;
				//m_stEncodeConfig.rcParams.minQP.qpInterP = 32 - quality;
				//m_stEncodeConfig.rcParams.minQP.qpIntra = 32 - quality;
				init_params.encodeConfig->frameIntervalP = 3;
			}

			init_params.encodeConfig->encodeCodecConfig.h264Config.outputBufferingPeriodSEI = 1;
			init_params.encodeConfig->encodeCodecConfig.h264Config.outputPictureTimingSEI = 1;
			//init_params.encodeConfig->encodeCodecConfig.h264Config.disableSPSPPS = 1;

			init_params.encodeConfig->encodeCodecConfig.h264Config.enableVFR = 0;
			init_params.encodeConfig->encodeCodecConfig.h264Config.bdirectMode = init_params.encodeConfig->frameIntervalP > 1 ? NV_ENC_H264_BDIRECT_MODE_TEMPORAL : NV_ENC_H264_BDIRECT_MODE_DISABLE;


			//init_params.encodeConfig->encodeCodecConfig.h264Config.maxNumRefFrames = 16;
			init_params.encodeConfig->encodeCodecConfig.h264Config.idrPeriod = init_params.encodeConfig->gopLength;


			break;
		}
		case dk_nvenc_encoder::CU_ENC_AUTO_PRESET_LOWEST:
		{
			init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
			break;
		}
		case dk_nvenc_encoder::CU_ENC_AUTO_PRESET_LOW:
		{
			init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
			break;
		}
		case dk_nvenc_encoder::CU_ENC_AUTO_PRESET_RECOMMEND:
		{
			init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
			break;
		}
		case dk_nvenc_encoder::CU_ENC_AUTO_PRESET_HIGH:
		{
			init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
			init_params.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;

			break;
		}
		case dk_nvenc_encoder::CU_ENC_AUTO_PRESET_HIGHEST:
		{
			init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
			init_params.presetGUID = NV_ENC_PRESET_HQ_GUID;
			init_params.encodeWidth = _enc_config.width;
			init_params.encodeHeight = _enc_config.height;
			init_params.darWidth = _enc_config.width;
			init_params.darHeight = _enc_config.height;
			init_params.frameRateNum = _enc_config.fps;
			init_params.frameRateDen = 1;
#if defined(_WIN32) && defined(WITH_ASYNC)
			init_params.enableEncodeAsync = 1;
#else
			init_params.enableEncodeAsync = 0;
#endif
			init_params.enablePTD = 1;
			init_params.reportSliceOffsets = 0;
			init_params.enableSubFrameWrite = 0;
			init_params.maxEncodeWidth = _enc_config.max_width;
			init_params.maxEncodeHeight = _enc_config.max_height;

			init_params.encodeConfig->gopLength = _enc_config.keyframe_interval*_enc_config.fps;
			init_params.encodeConfig->frameIntervalP = 1;
			init_params.encodeConfig->frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
			init_params.encodeConfig->mvPrecision = NV_ENC_MV_PRECISION_DEFAULT;

			init_params.encodeConfig->rcParams.averageBitRate = _enc_config.bitrate;
			init_params.encodeConfig->rcParams.maxBitRate = _enc_config.vbv_max_bitrate;
			init_params.encodeConfig->rcParams.vbvBufferSize = _enc_config.vbv_size;
			init_params.encodeConfig->rcParams.vbvInitialDelay = _enc_config.vbv_size * 9 / 10;
			init_params.encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_2_PASS_FRAMESIZE_CAP;

			init_params.encodeConfig->encodeCodecConfig.h264Config.adaptiveTransformMode = NV_ENC_H264_ADAPTIVE_TRANSFORM_ENABLE;
			init_params.encodeConfig->encodeCodecConfig.h264Config.fmoMode = NV_ENC_H264_FMO_DISABLE;
			init_params.encodeConfig->encodeCodecConfig.h264Config.outputBufferingPeriodSEI = 1;
			init_params.encodeConfig->encodeCodecConfig.h264Config.outputPictureTimingSEI = 1;
			init_params.encodeConfig->encodeCodecConfig.h264Config.chromaFormatIDC = 1;
			init_params.encodeConfig->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_AUTOSELECT;// NV_ENC_H264_ENTROPY_CODING_MODE_AUTOSELECT;
			init_params.encodeConfig->encodeCodecConfig.h264Config.level = NV_ENC_LEVEL_AUTOSELECT;
			init_params.encodeConfig->encodeCodecConfig.h264Config.repeatSPSPPS = 1;
			init_params.encodeConfig->encodeCodecConfig.h264Config.enableVFR = 0;
			init_params.encodeConfig->encodeCodecConfig.h264Config.bdirectMode = NV_ENC_H264_BDIRECT_MODE_DISABLE;
			//init_params.encodeConfig->encodeCodecConfig.h264Config.maxNumRefFrames = 16;
			init_params.encodeConfig->encodeCodecConfig.h264Config.idrPeriod = init_params.encodeConfig->gopLength;
			break;
		}
	};







	/*
	init_params.presetGUID = NV_ENC_PRESET_HQ_GUID;
	init_params.encodeWidth = _enc_config.width;
	init_params.encodeHeight = _enc_config.height;
	init_params.darWidth = _enc_config.width;
	init_params.darHeight = _enc_config.height;
	init_params.frameRateNum = _enc_config.fps;
	init_params.frameRateDen = 1;
	init_params.maxEncodeWidth = _enc_config.max_width;
	init_params.maxEncodeHeight = _enc_config.max_height;
	init_params.enableEncodeAsync = 1;
	init_params.enableEncodeAsync = 0;
	init_params.enablePTD = 1;
	init_params.encodeConfig->gopLength = _enc_config.keyframe_interval*_enc_config.fps;
	init_params.encodeConfig->frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
	init_params.encodeConfig->mvPrecision = NV_ENC_MV_PRECISION_QUARTER_PEL;
	init_params.encodeConfig->frameIntervalP = 1;
	//init_params.encodeConfig->rcParams.averageBitRate = _enc_config.bitrate;
	//init_params.encodeConfig->rcParams.maxBitRate = _enc_config.vbv_max_bitrate;
	//init_params.encodeConfig->rcParams.vbvBufferSize = _enc_config.vbv_size;
	//init_params.encodeConfig->rcParams.vbvInitialDelay = _enc_config.vbv_size * 9 / 10;
	init_params.encodeConfig->rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)RC_MODE_CONSTQP;
	init_params.encodeConfig->rcParams.constQP.qpInterP = init_params.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : 28;
	init_params.encodeConfig->rcParams.constQP.qpInterB = init_params.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : 28;
	init_params.encodeConfig->rcParams.constQP.qpIntra = init_params.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : 28;
	init_params.encodeConfig->encodeCodecConfig.h264Config.chromaFormatIDC = 1;
	init_params.encodeConfig->encodeCodecConfig.h264Config.idrPeriod = init_params.encodeConfig->gopLength;
	init_params.encodeConfig->encodeCodecConfig.h264Config.maxNumRefFrames = 16;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.adaptiveTransformMode = NV_ENC_H264_ADAPTIVE_TRANSFORM_ENABLE;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.fmoMode = NV_ENC_H264_FMO_DISABLE;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.outputBufferingPeriodSEI = 1;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.outputPictureTimingSEI = 1;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.disableSPSPPS = 1;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.enableVFR = 0;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.bdirectMode = init_params.encodeConfig->frameIntervalP > 1 ? NV_ENC_H264_BDIRECT_MODE_TEMPORAL : NV_ENC_H264_BDIRECT_MODE_DISABLE;

	init_params.presetGUID = NV_ENC_PRESET_HQ_GUID;
	init_params.encodeWidth = _enc_config.width;
	init_params.encodeHeight = _enc_config.height;
	init_params.darWidth = _enc_config.width;
	init_params.darHeight = _enc_config.height;
	init_params.frameRateNum = _enc_config.fps;
	init_params.frameRateDen = 1;
	init_params.maxEncodeWidth = _enc_config.max_width;
	init_params.maxEncodeHeight = _enc_config.max_height;
	init_params.enableEncodeAsync = 1;
	init_params.enableEncodeAsync = 0;
	init_params.enablePTD = 1;
	init_params.encodeConfig->gopLength = _enc_config.keyframe_interval*_enc_config.fps;
	init_params.encodeConfig->frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
	init_params.encodeConfig->mvPrecision = NV_ENC_MV_PRECISION_DEFAULT;
	init_params.encodeConfig->frameIntervalP = 1;
	init_params.encodeConfig->rcParams.averageBitRate = _enc_config.bitrate;
	init_params.encodeConfig->rcParams.maxBitRate = _enc_config.vbv_max_bitrate;
	init_params.encodeConfig->rcParams.vbvBufferSize = _enc_config.vbv_size;
	init_params.encodeConfig->rcParams.vbvInitialDelay = _enc_config.vbv_size * 9 / 10;
	init_params.encodeConfig->rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)RC_MODE_CBR;
	init_params.encodeConfig->encodeCodecConfig.h264Config.chromaFormatIDC = 1;
	init_params.encodeConfig->encodeCodecConfig.h264Config.idrPeriod = init_params.encodeConfig->gopLength;
	init_params.encodeConfig->encodeCodecConfig.h264Config.maxNumRefFrames = 16;
	init_params.encodeConfig->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_AUTOSELECT;
	init_params.encodeConfig->encodeCodecConfig.h264Config.level = NV_ENC_LEVEL_AUTOSELECT;
	init_params.encodeConfig->encodeCodecConfig.h264Config.adaptiveTransformMode = NV_ENC_H264_ADAPTIVE_TRANSFORM_ENABLE;
	init_params.encodeConfig->encodeCodecConfig.h264Config.fmoMode = NV_ENC_H264_FMO_DISABLE;
	init_params.encodeConfig->encodeCodecConfig.h264Config.outputBufferingPeriodSEI = 1;
	init_params.encodeConfig->encodeCodecConfig.h264Config.outputPictureTimingSEI = 1;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.disableSPSPPS = 1;
	init_params.encodeConfig->encodeCodecConfig.h264Config.enableVFR = 0;
	//init_params.encodeConfig->encodeCodecConfig.h264Config.bdirectMode = init_params.encodeConfig->frameIntervalP > 1 ? NV_ENC_H264_BDIRECT_MODE_TEMPORAL : NV_ENC_H264_BDIRECT_MODE_DISABLE;

	status = _cu_enc_core->NvEncInitializeEncoder(&init_params);

	_enc_buffer_count = _enc_config.numb + 4;//min buffers is numb +1 +3 pipelining
	result = allocate_io_buffers(_enc_config.width, _enc_config.height);
	if (result != ERR_CODE_SUCCESS)
	return result;
	*/


	status = NvEncInitializeEncoder(&init_params);
	if (status != NV_ENC_SUCCESS)
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	}

	_enc_buffer_count = _enc_config.numb + 4;//min buffers is numb +1 +3 pipelining
	result = allocate_io_buffers(_enc_config.width, _enc_config.height);
	if (result != dk_nvenc_encoder::ERR_CODE_SUCCESS)
	{
		release_encoder();
		release_cuda();
		return result;
	}

	///////////////////query pitch(stride)/////////////////
	//void * tmp_inbuffer;
	//unsigned char * tmp_inbuffer_surface;
	//status = _cu_enc_core->NvEncCreateInputBuffer(_enc_config.width, _enc_config.height, (NV_ENC_BUFFER_FORMAT)_input_format, &tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAILED;
	//status = _cu_enc_core->NvEncLockInputBuffer(tmp_inbuffer, (void**)&tmp_inbuffer_surface, &(*pitch));
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAILED;
	//status = _cu_enc_core->NvEncUnlockInputBuffer(tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAILED;
	//_cu_enc_core->NvEncDestroyInputBuffer(tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAILED;
	///////////////////query pitch(stride)/////////////////

	_encode_index = 0;
	_binit = true;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::release(void)
{
	if (_binit)
	{
		NVENCSTATUS status = NV_ENC_SUCCESS;

		if (flush_encoder() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAILED;
		if (release_io_buffers() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAILED;
		if (release_encoder() != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAILED;
		if (release_cuda() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAILED;

		_binit = false;
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, NV_ENC_PIC_TYPE & bs_pic_type, bool flush)
{
	//입력은 프레임단위로 진행되는 것을 가정함
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int locked_pitch = 0;

	if (flush)
	{
		flush_encoder();
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;
	}

	cu_enc_core::cu_enc_buffer_t * buffer = _enc_buffer_queue.get_available();
	if (!buffer)
	{
		buffer = _enc_buffer_queue.get_pending();
		process(buffer, output, osize, bs_pic_type);
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)
		if (buffer->in.input_buffer)
		{
			status = NvEncUnmapInputResource(buffer->in.input_buffer);
			buffer->in.input_buffer = NULL;
		}
#endif
		buffer = _enc_buffer_queue.get_available();
	}

#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)

#if defined(WITH_DYNAMIC_CUDA_LOAD)
	cu_auto_lock lock(_cu_driver_api, (CUcontext)_cu_context);  //sometimes auto_lock doesn't return
#else
	cu_auto_lock lock((CUcontext)_cu_context);  //sometimes auto_lock doesn't return
#endif

#if defined(WITH_CUDA_PTX) && !defined(WITH_DYNAMIC_CUDA_LOAD)
	unsigned char * origin_yv12_y_plane = input;
	unsigned char * origin_yv12_v_plane = origin_yv12_y_plane + _enc_config.width*_enc_config.height;
	unsigned char * origin_yv12_u_plane = origin_yv12_v_plane + _enc_config.width*_enc_config.height / 4;

	// copy luma
	CUDA_MEMCPY2D copy_param;
	memset(&copy_param, 0, sizeof(copy_param));
	copy_param.dstMemoryType = CU_MEMORYTYPE_DEVICE;
	copy_param.dstDevice = buffer->in.device_ptr;
	copy_param.dstPitch = buffer->in.stride;
	copy_param.srcMemoryType = CU_MEMORYTYPE_HOST;
	copy_param.srcHost = origin_yv12_y_plane;
	copy_param.srcPitch = _enc_config.width;
	copy_param.WidthInBytes = _enc_config.width;
	copy_param.Height = _enc_config.height;
	cuMemcpy2D(&copy_param);

	// copy chroma
	cuMemcpyHtoD(_ptr_cu_chroma[0], origin_yv12_u_plane, _enc_config.width*_enc_config.height / 4);
	cuMemcpyHtoD(_ptr_cu_chroma[1], origin_yv12_v_plane, _enc_config.width*_enc_config.height / 4);

#define BLOCK_X 32
#define BLOCK_Y 16
	int chroma_height = _enc_config.height / 2;
	int chroma_width = _enc_config.width / 2;
	dim3 block(BLOCK_X, BLOCK_Y, 1);
	dim3 grid((chroma_width + BLOCK_X - 1) / BLOCK_X, (chroma_height + BLOCK_Y - 1) / BLOCK_Y, 1);
#undef BLOCK_Y
#undef BLOCK_X
	CUdeviceptr dNV12Chroma = (CUdeviceptr)((unsigned char*)buffer->in.device_ptr + buffer->in.stride*_enc_config.height);
	void * args[8] = { &_ptr_cu_chroma[0], &_ptr_cu_chroma[1], &dNV12Chroma, &chroma_width, &chroma_height, &chroma_width, &chroma_width, &buffer->in.stride };
	cuLaunchKernel(_fn_interleave_uv, grid.x, grid.y, grid.z, block.x, block.y, block.z, 0, NULL, args, NULL);
	CUresult cuResult = cuStreamQuery(NULL);
	if (!((cuResult == CUDA_SUCCESS) || (cuResult == CUDA_ERROR_NOT_READY)))
		return AFCCudaEncoder::ERR_CODE_FAILED;
#else
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	interleave_uv((void*)_cu_driver_api, _enc_config.width, _enc_config.height, input, _enc_config.width, buffer->in.device_ptr, buffer->in.stride, _ptr_cu_chroma[0], _ptr_cu_chroma[1]);
#else
	interleave_uv(_enc_config.width, _enc_config.height, input, _enc_config.width, buffer->in.device_ptr, buffer->in.stride, _ptr_cu_chroma[0], _ptr_cu_chroma[1]);
#endif
#endif
	status = NvEncMapInputResource(buffer->in.registered_resource, &buffer->in.input_buffer);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
#else
	unsigned char * input_buffer_surface;
	status = NvEncLockInputBuffer(buffer->in.input_buffer, (void**)&input_buffer_surface, &locked_pitch);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;

	unsigned char *origin_yv12_y_plane = input;
	unsigned char *origin_yv12_v_plane = origin_yv12_y_plane + _enc_config.width*_enc_config.height;
	unsigned char *origin_yv12_u_plane = origin_yv12_v_plane + _enc_config.width*_enc_config.height / 4;
	unsigned char *input_buffer_surface_chroma = input_buffer_surface + (_enc_config.height*locked_pitch);
	convert_yv12pitch_to_yv12(origin_yv12_y_plane, origin_yv12_u_plane, origin_yv12_v_plane,
		input_buffer_surface, input_buffer_surface_chroma, _enc_config.width, _enc_config.height, _enc_config.width, locked_pitch);
	status = NvEncUnlockInputBuffer(buffer->in.input_buffer);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;
#endif

#if 0
	NvEncEncodeFrame(buffer, _enc_config.width, _enc_config.height);
#else

	NV_ENC_PIC_PARAMS enc_pic_params;
	memset(&enc_pic_params, 0x00, sizeof(enc_pic_params));
	SET_VER(enc_pic_params, NV_ENC_PIC_PARAMS);

	enc_pic_params.inputBuffer = buffer->in.input_buffer;
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)
	enc_pic_params.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12_PL;
#else
	enc_pic_params.bufferFmt = (NV_ENC_BUFFER_FORMAT)_input_format;
#endif
	enc_pic_params.inputWidth = _enc_config.width;
	enc_pic_params.inputHeight = _enc_config.height;
	enc_pic_params.outputBitstream = buffer->out.bitstream_buffer;
	enc_pic_params.completionEvent = buffer->out.output_evt;
	enc_pic_params.inputTimeStamp = _encode_index;
	enc_pic_params.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
	enc_pic_params.qpDeltaMap = 0;
	enc_pic_params.qpDeltaMapSize = 0;

	status = NvEncEncodePicture(&enc_pic_params);
	//status = _encoder_api->nvEncEncodePicture(_encoder, &enc_pic_params);
	if (status != NV_ENC_SUCCESS && status != NV_ENC_ERR_NEED_MORE_INPUT)
	{
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	}

	_encode_index++;
#endif

	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}


dk_nvenc_encoder::ERR_CODE cu_enc_core::is_h264_supported(void)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	dk_nvenc_encoder::ERR_CODE result = dk_nvenc_encoder::ERR_CODE_FAILED;
	unsigned int codec_guid_count = 0;
	status = NvEncGetEncodeGUIDCount(&codec_guid_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	if (codec_guid_count == 0)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	GUID * codec_guids = new GUID[codec_guid_count];
	status = NvEncGetEncodeGUIDs(codec_guids, codec_guid_count, &codec_guid_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	for (unsigned int i = 0; i < codec_guid_count; i++)
	{
		if (codec_guids[i] == NV_ENC_CODEC_H264_GUID)
		{
			result = dk_nvenc_encoder::ERR_CODE_SUCCESS;
			break;
		}
	}

	delete[] codec_guids;
	return result;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_supported_codecs(std::vector<int> * const codec)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int codec_guid_count = 0;
	status = NvEncGetEncodeGUIDCount(&codec_guid_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	if (codec_guid_count == 0)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	GUID * codec_guids = new GUID[codec_guid_count];
	status = NvEncGetEncodeGUIDs(codec_guids, codec_guid_count, &codec_guid_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	for (unsigned int i = 0; i < codec_guid_count; i++)
	{
		if (codec_guids[i] == NV_ENC_CODEC_H264_GUID)
			codec->push_back(dk_nvenc_encoder::CODEC_TYPE_H264);
		else if (codec_guids[i] == NV_ENC_CODEC_HEVC_GUID)
			codec->push_back(dk_nvenc_encoder::CODEC_TYPE_HEVC);
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_supported_codec_profiles(std::vector<int> * const codec_profiles)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int codec_profile_count = 0;
	status = NvEncGetEncodeProfileGUIDCount(NV_ENC_CODEC_H264_GUID, &codec_profile_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	if (codec_profile_count == 0)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	GUID * codec_profile_guids = new GUID[codec_profile_count];
	status = NvEncGetEncodePresetGUIDs(NV_ENC_CODEC_H264_GUID, codec_profile_guids, codec_profile_count, &codec_profile_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	for (unsigned int i = 0; i < codec_profile_count; i++)
	{
		if (codec_profile_guids[i] == NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID)
			codec_profiles->push_back(dk_nvenc_encoder::CODEC_PROFILE_TYPE_AUTOSELECT);
		else if (codec_profile_guids[i] == NV_ENC_H264_PROFILE_BASELINE_GUID)
			codec_profiles->push_back(dk_nvenc_encoder::CODEC_PROFILE_TYPE_BASELINE);
		else if (codec_profile_guids[i] == NV_ENC_H264_PROFILE_MAIN_GUID)
			codec_profiles->push_back(dk_nvenc_encoder::CODEC_PROFILE_TYPE_MAIN);
		else if (codec_profile_guids[i] == NV_ENC_H264_PROFILE_HIGH_GUID)
			codec_profiles->push_back(dk_nvenc_encoder::CODEC_PROFILE_TYPE_HIGH);
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}


dk_nvenc_encoder::ERR_CODE cu_enc_core::get_supported_codec_presets(std::vector<int> * const codec_presets)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int codec_preset_count = 0;
	status = NvEncGetEncodePresetCount(NV_ENC_CODEC_H264_GUID, &codec_preset_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	if (codec_preset_count == 0)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	GUID * codec_preset_guids = new GUID[codec_preset_count];
	status = NvEncGetEncodePresetGUIDs(NV_ENC_CODEC_H264_GUID, codec_preset_guids, codec_preset_count, &codec_preset_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	for (unsigned int i = 0; i < codec_preset_count; i++)
	{
		if (codec_preset_guids[i] == NV_ENC_PRESET_DEFAULT_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_DEFAULT);
		else if (codec_preset_guids[i] == NV_ENC_PRESET_HP_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_HP);
		else if (codec_preset_guids[i] == NV_ENC_PRESET_HQ_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_HQ);
		else if (codec_preset_guids[i] == NV_ENC_PRESET_BD_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_BD);
		else if (codec_preset_guids[i] == NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_LOW_LATENCY_DEFAULT);
		else if (codec_preset_guids[i] == NV_ENC_PRESET_LOW_LATENCY_HQ_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_LOW_LATENCY_HQ);
		else if (codec_preset_guids[i] == NV_ENC_PRESET_LOW_LATENCY_HP_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_LOW_LATENCY_HP);
		else if (codec_preset_guids[i] == NV_ENC_PRESET_LOSSLESS_DEFAULT_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_LOSSLESS_DEFAULT);
		else if (codec_preset_guids[i] == NV_ENC_PRESET_LOSSLESS_HP_GUID)
			codec_presets->push_back(dk_nvenc_encoder::PRESET_TYPE_LOSSLESS_HP);
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_max_bframes(int & max_bframes)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_NUM_MAX_BFRAMES;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &max_bframes);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_supported_rc_mode(int & rc_mode)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_supported_field_encoding(int & field_encoding)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_monochrom_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_fmo_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_qpelmv_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_bdirect_mode_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_cabac_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_adaptive_transform_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_temporal_layers_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_hierarchical_pframes_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_hierarchical_bframes_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_max_encoding_level(int & enc_level)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_LEVEL_MAX;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &enc_level);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_min_encoding_level(int & enc_level)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_LEVEL_MIN;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &enc_level);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_separate_color_plane_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_max_width(int & max_width)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_WIDTH_MAX;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &max_width);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_max_height(int & max_height)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_HEIGHT_MAX;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &max_height);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_temporal_svc_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_dynamic_resolution_change_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_dynamic_bitrate_change_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_dynamic_rc_mode_change_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_subframe_readback_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_constrained_encoding_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_intra_refresh_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_custom_vbv_buf_size_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_dynamic_slice_mode_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_ref_pic_invalidation_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_supported_preprocessing_flags(int *preproc_flag)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_async_encode_supported(void)
{
	int result = 0;
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_ASYNC_ENCODE_SUPPORT;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &result);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	if (result == 0)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	else
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::get_max_mb(int & max_mb)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_yuv444_encode_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::is_lossless_encode_supported(void)
{
	int result = 0;
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_SUPPORT_LOSSLESS_ENCODE;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &result);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	if (result == 0)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	else
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

////////////////////////////////////private function
dk_nvenc_encoder::ERR_CODE cu_enc_core::initialize_cuda(void)
{
	CUresult result;
	CUdevice device;
	CUcontext cu_current_context;
	int  device_count = 0;
	int  SMminor = 0, SMmajor = 0;

#if defined(WITH_DYNAMIC_CUDA_LOAD)
	_cu_driver_api = new dk_cuda_driver_api();
	if (!_cu_driver_api || !_cu_driver_api->load())
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;

	result = _cu_driver_api->init(0);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;

	result = _cu_driver_api->device_get_count(&device_count);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;

	if (device_count<1)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;

	size_t available_device_memory = 0;
	int device_id = -1;
	for (int index = 0; index < device_count; index++)
	{
		CUdevice tmp_device;
		size_t tmp_available_device_memory = 0;
		result = _cu_driver_api->device_get(&tmp_device, index);
		if (result != CUDA_SUCCESS)
			continue;
		result = _cu_driver_api->device_compute_capability(&SMmajor, &SMminor, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;
		if (((SMmajor << 4) + SMminor) < 0x30)
			continue;
		result = _cu_driver_api->device_total_memory(&tmp_available_device_memory, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;

		if (available_device_memory < tmp_available_device_memory)
			device_id = index;
	}

	if (device_id<0)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;
	result = _cu_driver_api->device_get(&device, device_id);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;

	result = _cu_driver_api->ctx_create((CUcontext*)(&_cu_context), 0, device);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;
#else
	result = cuInit(0);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;

	result = cuDeviceGetCount(&device_count);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;

	size_t available_device_memory = 0;
	int device_id = -1;
	for (int index = 0; index < device_count; index++)
	{
		CUdevice tmp_device;
		size_t tmp_available_device_memory = 0;
		result = cuDeviceGet(&tmp_device, index);
		if (result != CUDA_SUCCESS)
			continue;
		result = cuDeviceComputeCapability(&SMmajor, &SMminor, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;
		if (((SMmajor << 4) + SMminor) < 0x30)
			continue;
		result = cuDeviceTotalMem(&tmp_available_device_memory, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;

		if (available_device_memory < tmp_available_device_memory)
			device_id = index;
	}

	if (device_id<0)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;
	result = cuDeviceGet(&device, device_id);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;

	result = cuCtxCreate((CUcontext*)(&_cu_context), 0, device);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_CUDA_ENCODING_DEVICE;
#endif

#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
#if defined(WITH_CUDA_PTX) && !defined(WITH_DYNAMIC_CUDA_LOAD)
	// in this branch we use compilation with parameters
	const unsigned int jitNumOptions = 3;
	CUjit_option *jitOptions = new CUjit_option[jitNumOptions];
	void **jitOptVals = new void *[jitNumOptions];

	// set up size of compilation log buffer
	jitOptions[0] = CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES;
	int jitLogBufferSize = 1024;
	jitOptVals[0] = (void *)(size_t)jitLogBufferSize;

	// set up pointer to the compilation log buffer
	jitOptions[1] = CU_JIT_INFO_LOG_BUFFER;
	char *jitLogBuffer = new char[jitLogBufferSize];
	jitOptVals[1] = jitLogBuffer;

	// set up pointer to set the Maximum # of registers for a particular kernel
	jitOptions[2] = CU_JIT_MAX_REGISTERS;
	int jitRegCount = 32;
	jitOptVals[2] = (void *)(size_t)jitRegCount;

	std::string ptx_source;
	FILE *fp = fopen("dk_nvenc_encoder.ptx", "rb");
	if (!fp)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	char *buf = new char[file_size + 1];
	fseek(fp, 0, SEEK_SET);
	fread(buf, sizeof(char), file_size, fp);
	fclose(fp);
	buf[file_size] = '\0';
	ptx_source = buf;
	delete[] buf;

	result = cuModuleLoadDataEx(&_cu_module, ptx_source.c_str(), jitNumOptions, jitOptions, (void **)jitOptVals);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	delete[] jitOptions;
	delete[] jitOptVals;
	delete[] jitLogBuffer;

	result = cuModuleGetFunction(&_fn_interleave_uv, _cu_module, "interleave_uv");
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
#endif
#endif
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	result = _cu_driver_api->ctx_pop_current(&cu_current_context);
#else
	result = cuCtxPopCurrent(&cu_current_context);
#endif
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::release_cuda(void)
{
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	if (!_cu_driver_api)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	CUresult status = _cu_driver_api->ctx_destroy((CUcontext)_cu_context);
	if (status != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	if (!_cu_driver_api->free())
		return dk_nvenc_encoder::ERR_CODE_FAILED;
#else
	CUresult status = cuCtxDestroy((CUcontext)_cu_context);
	if (status != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
#endif

	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::allocate_io_buffers(unsigned int input_width, unsigned int input_height)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	_enc_buffer_queue.initialize(_enc_buffer, _enc_buffer_count);
	
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	cu_auto_lock lock(_cu_driver_api, (CUcontext)_cu_context);  //sometimes auto_lock doesn't return
#else
	cu_auto_lock lock((CUcontext)_cu_context);  //sometimes auto_lock doesn't return
#endif

#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	_cu_driver_api->mem_alloc(&_ptr_cu_chroma[0], input_width*input_height / 4);
	_cu_driver_api->mem_alloc(&_ptr_cu_chroma[1], input_width*input_height / 4);
#else
	cuMemAlloc(&_ptr_cu_chroma[0], input_width*input_height / 4);
	cuMemAlloc(&_ptr_cu_chroma[1], input_width*input_height / 4);
#endif
#endif

	for (uint32_t i = 0; i < _enc_buffer_count; i++)
	{
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
#if defined(WITH_DYNAMIC_CUDA_LOAD)
		_cu_driver_api->mem_alloc_pitch(&_enc_buffer[i].in.device_ptr, (size_t *)&_enc_buffer[i].in.stride, input_width, input_height * 1.5, 16);
#else
		cuMemAllocPitch(&_enc_buffer[i].in.device_ptr, (size_t *)&_enc_buffer[i].in.stride, input_width, input_height * 1.5, 16);
#endif
		status = NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR, (void*)_enc_buffer[i].in.device_ptr,
									   input_width, input_height, _enc_buffer[i].in.stride, &_enc_buffer[i].in.registered_resource);
		if (status != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAILED;
#else
		status = NvEncCreateInputBuffer(input_width, input_height, (NV_ENC_BUFFER_FORMAT)_input_format, &_enc_buffer[i].in.input_buffer);
		if (status != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAILED;
#endif
		status = NvEncCreateBitstreamBuffer(BITSTREAM_BUFFER_SIZE, &_enc_buffer[i].out.bitstream_buffer);
		if (status != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAILED;
		_enc_buffer[i].out.bitstream_buffer_size = BITSTREAM_BUFFER_SIZE;

#if defined(_WIN32) && defined(WITH_ASYNC)
		status = NvEncRegisterAsyncEvent(&_enc_buffer[i].out.output_evt);
		if (status != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAILED;

		_enc_buffer[i].out.wait_evt = true;
#else
		_enc_buffer[i].out.wait_evt = false;
		_enc_buffer[i].out.output_evt = NULL;
#endif
	}
	_enc_eos_output_buffer.eos = true;
#if defined(_WIN32) && defined(WITH_ASYNC)
	status = NvEncRegisterAsyncEvent(&_enc_eos_output_buffer.output_evt);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;
#else
	_enc_eos_output_buffer.output_evt = NULL;
#endif

	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::release_io_buffers(void)
{
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	cu_auto_lock lock(_cu_driver_api, (CUcontext)_cu_context); //sometimes auto_lock doesn't return
#else
	cu_auto_lock lock((CUcontext)_cu_context); //sometimes auto_lock doesn't return
#endif
	for (uint32_t i = 0; i < _enc_buffer_count; i++)
	{
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)
		NvEncUnregisterResource(_enc_buffer[i].in.registered_resource);
#if defined(WITH_DYNAMIC_CUDA_LOAD)
		_cu_driver_api->mem_free(_enc_buffer[i].in.device_ptr);
#else
		cuMemFree(_enc_buffer[i].in.device_ptr);
#endif
#else
		NvEncDestroyInputBuffer(_enc_buffer[i].in.input_buffer);
		_enc_buffer[i].in.input_buffer = NULL;
#endif

		NvEncDestroyBitstreamBuffer(_enc_buffer[i].out.bitstream_buffer);
		_enc_buffer[i].out.bitstream_buffer = NULL;

#if defined(_WIN32) && defined(WITH_ASYNC)
		NvEncUnregisterAsyncEvent(_enc_buffer[i].out.output_evt);
		::CloseHandle(_enc_buffer[i].out.output_evt);
		_enc_buffer[i].out.output_evt = NULL;
#endif
	}

#if defined(WITH_DYNAMIC_CUDA_LOAD)
	_cu_driver_api->mem_free(_ptr_cu_chroma[0]);
	_cu_driver_api->mem_free(_ptr_cu_chroma[1]);
#else
	cuMemFree(_ptr_cu_chroma[0]);
	cuMemFree(_ptr_cu_chroma[1]);
#endif

	if (_enc_eos_output_buffer.output_evt)
	{
#if defined(_WIN32) && defined(WITH_ASYNC)
		NvEncUnregisterAsyncEvent(_enc_eos_output_buffer.output_evt);
		::CloseHandle(_enc_eos_output_buffer.output_evt);
		_enc_eos_output_buffer.output_evt = NULL;
#endif
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::flush_encoder(void)
{
	dk_nvenc_encoder::ERR_CODE result = dk_nvenc_encoder::ERR_CODE_SUCCESS;
	NVENCSTATUS status = NvEncFlushEncoderQueue(_enc_eos_output_buffer.output_evt);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	cu_enc_core::cu_enc_buffer_t * enc_buffer = _enc_buffer_queue.get_pending();
	while (enc_buffer)
	{
		unsigned int size = 0;
		NV_ENC_PIC_TYPE bs_pic_type;
		process(enc_buffer, NULL, size, bs_pic_type);
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)
		if (enc_buffer->in.input_buffer)
		{
			status = NvEncUnmapInputResource(enc_buffer->in.input_buffer);
			enc_buffer->in.input_buffer = NULL;
		}
#endif
		enc_buffer = _enc_buffer_queue.get_pending();
	}

#if defined(_WIN32) && defined(WITH_ASYNC)
	if (WaitForSingleObject(_enc_eos_output_buffer.output_evt, 500) != WAIT_OBJECT_0)
		result = dk_nvenc_encoder::ERR_CODE_FAILED;
#endif
	return result;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::process(const cu_enc_core::cu_enc_buffer_t * enc_buffer, unsigned char * encoded, unsigned int & encoded_size, NV_ENC_PIC_TYPE & bs_pic_type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	encoded_size = 0;
	if (enc_buffer->out.bitstream_buffer == NULL && enc_buffer->out.eos == FALSE)
		return dk_nvenc_encoder::ERR_CODE_FAILED;

	if (enc_buffer->out.wait_evt)
	{
		if (!enc_buffer->out.output_evt)
			return dk_nvenc_encoder::ERR_CODE_FAILED;
#if defined(_WIN32) && defined(WITH_ASYNC)
		WaitForSingleObject(enc_buffer->out.output_evt, INFINITE);
#endif
	}

	if (enc_buffer->out.eos)
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;

	status = NV_ENC_SUCCESS;
	NV_ENC_LOCK_BITSTREAM lock_bitstream_data;
	memset(&lock_bitstream_data, 0x00, sizeof(lock_bitstream_data));
	SET_VER(lock_bitstream_data, NV_ENC_LOCK_BITSTREAM);
	lock_bitstream_data.outputBitstream = enc_buffer->out.bitstream_buffer;
#if defined(_WIN32) && defined(WITH_ASYNC)
	lock_bitstream_data.doNotWait = true;
#else
	lock_bitstream_data.doNotWait = false;
#endif

	status = NvEncLockBitstream(&lock_bitstream_data);
	if (status == NV_ENC_SUCCESS)
	{
		if (encoded)
		{
			encoded_size = lock_bitstream_data.bitstreamSizeInBytes;
			memcpy(encoded, lock_bitstream_data.bitstreamBufferPtr, encoded_size);
		}
		bs_pic_type = lock_bitstream_data.pictureType;
		status = NvEncUnlockBitstream(enc_buffer->out.bitstream_buffer);
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

#if !defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
dk_nvenc_encoder::ERR_CODE cu_enc_core::convert_yv12pitch_to_nv12(unsigned char * yv12_luma, unsigned char * yv12_cb, unsigned char * yv12_cr,
	unsigned char * nv12_luma, unsigned char * nv12_chroma, int width, int height, int src_stride, int dst_stride)
{
	int y;
	int x;
	if (src_stride == 0)
		src_stride = width;
	if (dst_stride == 0)
		dst_stride = width;

	for (y = 0; y < height; y++)
	{
		memcpy(nv12_luma + (dst_stride*y), yv12_luma + (src_stride*y), width);
		if (y < height / 2)
		{
			for (x = 0; x < width; x = x + 2)
			{
				nv12_chroma[(y*dst_stride) + x] = yv12_cb[((src_stride / 2)*y) + (x >> 1)];
				nv12_chroma[(y*dst_stride) + (x + 1)] = yv12_cr[((src_stride / 2)*y) + (x >> 1)];
			}
		}
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE cu_enc_core::convert_yv12pitch_to_yv12(unsigned char * yv12_luma, unsigned char * yv12_cb, unsigned char * yv12_cr,
	unsigned char * yv12_luma1, unsigned char * yv12_chroma1, int width, int height, int src_stride, int dst_stride)
{
	int y;
	if (src_stride == 0)
		src_stride = width;
	if (dst_stride == 0)
		dst_stride = width;

	for (y = 0; y < height; y++)
	{
		memcpy(yv12_luma1 + (dst_stride*y), yv12_luma + (src_stride*y), width);
		if (y < height / 2)
		{
			memcpy(yv12_chroma1 + ((height*dst_stride) >> 2) + y*(dst_stride >> 1), yv12_cb + y*(src_stride >> 1), width >> 1);
			memcpy(yv12_chroma1 + y*(dst_stride >> 1), yv12_cr + y*(src_stride >> 1), width >> 1);
		}
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}
#endif

NVENCSTATUS cu_enc_core::initialize_encoder(NV_ENC_DEVICE_TYPE device_type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	MYPROC nvEncodeAPICreateInstance; // function pointer to create instance in nvEncodeAPI

#if defined(_WIN32)
#if defined (_WIN64)
	_cu_encoder_inst = LoadLibrary(TEXT("nvEncodeAPI64.dll"));
#else
	_cu_encoder_inst = LoadLibrary(TEXT("nvEncodeAPI.dll"));
#endif
#else
	_cu_encoder_inst = dlopen("libnvidia-encode.so.1", RTLD_LAZY);
#endif
	if (_cu_encoder_inst == 0)
		return NV_ENC_ERR_OUT_OF_MEMORY;

#if defined(_WIN32)
	nvEncodeAPICreateInstance = (MYPROC)GetProcAddress(_cu_encoder_inst, "NvEncodeAPICreateInstance");
#else
	nvEncodeAPICreateInstance = (MYPROC)dlsym(_cu_encoder_inst, "NvEncodeAPICreateInstance");
#endif

	if (nvEncodeAPICreateInstance == NULL)
		return NV_ENC_ERR_OUT_OF_MEMORY;

	_encoder_api = new NV_ENCODE_API_FUNCTION_LIST;
	if (_encoder_api == NULL)
		return NV_ENC_ERR_OUT_OF_MEMORY;

	memset(_encoder_api, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
	_encoder_api->version = NV_ENCODE_API_FUNCTION_LIST_VER;
	status = nvEncodeAPICreateInstance(_encoder_api);
	if (status != NV_ENC_SUCCESS)
		return status;

	__try
	{
		_encoder = 0;
		status = NvEncOpenEncodeSessionEx(_cu_context, device_type);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NV_ENC_ERR_GENERIC;
	}
	
	if (status != NV_ENC_SUCCESS)
		return status;

	return NV_ENC_SUCCESS;
}

NVENCSTATUS cu_enc_core::release_encoder(void)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	__try {
		status = NvEncDestroyEncoder();
	}__except (EXCEPTION_EXECUTE_HANDLER) {}
	_encoder = 0;

	if (_encoder_api)
	{
		delete _encoder_api;
		_encoder_api = 0;
	}

	if (_cu_encoder_inst)
	{
#if defined (_WIN32)
		FreeLibrary(_cu_encoder_inst);
#else
		dlclose(_cu_encoder_inst);
#endif
		_cu_encoder_inst = 0;
	}
	return status;
}

NVENCSTATUS cu_enc_core::NvEncOpenEncodeSession(void * device, unsigned int device_type)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _encoder_api->nvEncOpenEncodeSession(device, device_type, &_encoder);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncOpenEncodeSessionEx(void * device, NV_ENC_DEVICE_TYPE device_type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS open_sessionex_params;

	memset(&open_sessionex_params, 0, sizeof(open_sessionex_params));
	SET_VER(open_sessionex_params, NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS);

	open_sessionex_params.device = device;
	open_sessionex_params.deviceType = device_type;
	open_sessionex_params.reserved = NULL;
	open_sessionex_params.apiVersion = NVENCAPI_VERSION;

	status = _encoder_api->nvEncOpenEncodeSessionEx(&open_sessionex_params, &_encoder);
	return status;
}

NVENCSTATUS cu_enc_core::NvEncInitializeEncoder(NV_ENC_INITIALIZE_PARAMS * params)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _encoder_api->nvEncInitializeEncoder(_encoder, params);
	return status;

}

NVENCSTATUS cu_enc_core::NvEncGetEncodeGUIDCount(unsigned int * encode_guid_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _encoder_api->nvEncGetEncodeGUIDCount(_encoder, encode_guid_count);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetEncodeGUIDs(GUID * encode_guids, unsigned int encode_guids_size, unsigned int * encode_guids_count)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _encoder_api->nvEncGetEncodeGUIDs(_encoder, encode_guids, encode_guids_size, encode_guids_count);
	return status;
}

NVENCSTATUS cu_enc_core::NvEncGetEncodeProfileGUIDCount(GUID encode_guid, unsigned int * encode_profile_guid_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _encoder_api->nvEncGetEncodeProfileGUIDCount(_encoder, encode_guid, encode_profile_guid_count);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetEncodeProfileGUIDs(GUID encode_guid, GUID * encode_profiles_guid, unsigned int encode_profiles_guid_size, unsigned int * encode_profiles_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetEncodeProfileGUIDs(_encoder, encode_guid, encode_profiles_guid, encode_profiles_guid_size, encode_profiles_count);
    return status;
}


NVENCSTATUS cu_enc_core::NvEncGetInputFormatCount(GUID encode_guid, unsigned int * input_fmt_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetInputFormatCount(_encoder, encode_guid, input_fmt_count);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetInputFormats(GUID encode_guid, NV_ENC_BUFFER_FORMAT * input_fmts, unsigned int input_fmts_size, unsigned int * input_fmt_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetInputFormats(_encoder, encode_guid, input_fmts, input_fmts_size, input_fmt_count);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetEncodeCaps(GUID encode_guid, NV_ENC_CAPS_PARAM * caps_param, int * caps_val)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetEncodeCaps(_encoder, encode_guid, caps_param, caps_val);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetEncodePresetCount(GUID encode_guid, unsigned int * encode_presets_guid_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetEncodePresetCount(_encoder, encode_guid, encode_presets_guid_count);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetEncodePresetGUIDs(GUID encode_guid, GUID * encode_presets_guid, unsigned int encode_presets_size, unsigned int * encode_presets_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetEncodePresetGUIDs(_encoder, encode_guid, encode_presets_guid, encode_presets_size, encode_presets_count);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetEncodePresetConfig(GUID encode_guid, GUID encode_preset_guid, NV_ENC_PRESET_CONFIG * encode_preset_config)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetEncodePresetConfig(_encoder, encode_guid, encode_preset_guid, encode_preset_config);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncCreateInputBuffer(unsigned int width, unsigned int height, NV_ENC_BUFFER_FORMAT intput_buffer_fmt, void ** input_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_CREATE_INPUT_BUFFER ct_input_buffer_params;

	memset(&ct_input_buffer_params, 0x00, sizeof(ct_input_buffer_params));
	SET_VER(ct_input_buffer_params, NV_ENC_CREATE_INPUT_BUFFER);

	ct_input_buffer_params.width = width;
	ct_input_buffer_params.height = height;
	ct_input_buffer_params.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;
	ct_input_buffer_params.bufferFmt = intput_buffer_fmt;

	status = _encoder_api->nvEncCreateInputBuffer(_encoder, &ct_input_buffer_params);
	*input_buffer = ct_input_buffer_params.inputBuffer;
    return status;
}

NVENCSTATUS cu_enc_core::NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR input_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	if (input_buffer)
    {
		status = _encoder_api->nvEncDestroyInputBuffer(_encoder, input_buffer);
    }

    return status;
}


NVENCSTATUS cu_enc_core::NvEncLockInputBuffer(void * input_buffer, void ** buffer_data, unsigned int * pitch)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_LOCK_INPUT_BUFFER lock_input_buffer_params;

	memset(&lock_input_buffer_params, 0, sizeof(lock_input_buffer_params));
	SET_VER(lock_input_buffer_params, NV_ENC_LOCK_INPUT_BUFFER);

	lock_input_buffer_params.inputBuffer = input_buffer;
	status = _encoder_api->nvEncLockInputBuffer(_encoder, &lock_input_buffer_params);
	*buffer_data = lock_input_buffer_params.bufferDataPtr;
	*pitch = lock_input_buffer_params.pitch;

	return status;
}

NVENCSTATUS cu_enc_core::NvEncUnlockInputBuffer(NV_ENC_INPUT_PTR input_buffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncUnlockInputBuffer(_encoder, input_buffer);
	return status;
}

NVENCSTATUS cu_enc_core::NvEncCreateBitstreamBuffer(unsigned int size, void ** bitstream_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    NV_ENC_CREATE_BITSTREAM_BUFFER ct_bitstream_buffer_params;

	memset(&ct_bitstream_buffer_params, 0x00, sizeof(ct_bitstream_buffer_params));
	SET_VER(ct_bitstream_buffer_params, NV_ENC_CREATE_BITSTREAM_BUFFER);

	ct_bitstream_buffer_params.size = size;
	ct_bitstream_buffer_params.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;

	status = _encoder_api->nvEncCreateBitstreamBuffer(_encoder, &ct_bitstream_buffer_params);
	*bitstream_buffer = ct_bitstream_buffer_params.bitstreamBuffer;
    return status;
}

NVENCSTATUS cu_enc_core::NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstream_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	if (bitstream_buffer)
    {
		status = _encoder_api->nvEncDestroyBitstreamBuffer(_encoder, bitstream_buffer);
    }
    return status;
}

NVENCSTATUS cu_enc_core::NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM * lock_bitstream_buffer_params)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncLockBitstream(_encoder, lock_bitstream_buffer_params);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstream_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncUnlockBitstream(_encoder, bitstream_buffer);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetEncodeStats(NV_ENC_STAT* encode_stats)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetEncodeStats(_encoder, encode_stats);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncGetSequenceParams(NV_ENC_SEQUENCE_PARAM_PAYLOAD * sequence_param_payload)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncGetSequenceParams(_encoder, sequence_param_payload);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncRegisterAsyncEvent(void ** completion_event)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    NV_ENC_EVENT_PARAMS eventParams;

    memset(&eventParams, 0, sizeof(eventParams));
    SET_VER(eventParams, NV_ENC_EVENT_PARAMS);

#if defined (_WIN32) && defined(WITH_ASYNC)
    eventParams.completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    eventParams.completionEvent = NULL;
#endif
    status = _encoder_api->nvEncRegisterAsyncEvent(_encoder, &eventParams);
	*completion_event = eventParams.completionEvent;

    return status;
}

NVENCSTATUS cu_enc_core::NvEncUnregisterAsyncEvent(void * completion_event)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    NV_ENC_EVENT_PARAMS event_params;

	if (completion_event)
    {
		memset(&event_params, 0x00, sizeof(event_params));
		SET_VER(event_params, NV_ENC_EVENT_PARAMS);

		event_params.completionEvent = completion_event;

		status = _encoder_api->nvEncUnregisterAsyncEvent(_encoder, &event_params);
    }

    return status;
}

NVENCSTATUS cu_enc_core::NvEncMapInputResource(void * registered_resource, void ** mapped_resource)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    NV_ENC_MAP_INPUT_RESOURCE map_input_resource_params;

	memset(&map_input_resource_params, 0, sizeof(map_input_resource_params));
	SET_VER(map_input_resource_params, NV_ENC_MAP_INPUT_RESOURCE);

	map_input_resource_params.registeredResource = registered_resource;

	status = _encoder_api->nvEncMapInputResource(_encoder, &map_input_resource_params);
	*mapped_resource = map_input_resource_params.mappedResource;

    return status;
}

NVENCSTATUS cu_enc_core::NvEncUnmapInputResource(NV_ENC_INPUT_PTR mapped_input_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    
	if (mapped_input_buffer)
    {
		status = _encoder_api->nvEncUnmapInputResource(_encoder, mapped_input_buffer);
    }

    return status;
}

NVENCSTATUS cu_enc_core::NvEncDestroyEncoder(void)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	if (_encoder)
		status = _encoder_api->nvEncDestroyEncoder(_encoder);
    return status;
}

/*
NVENCSTATUS cu_enc_core::NvEncInvalidateRefFrames(const NvEncPictureCommand * picture_command)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	for (unsigned int i = 0; i < picture_command->numRefFramesToInvalidate; i++)
    {
		status = _encoder_api->nvEncInvalidateRefFrames(_encoder, picture_command->refFrameNumbers[i]);
    }

    return status;
}
*/

NVENCSTATUS cu_enc_core::NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE resource_type, void * resource_to_register, unsigned int width, unsigned int height, unsigned int pitch, void ** registered_resource)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    NV_ENC_REGISTER_RESOURCE registerResParams;

    memset(&registerResParams, 0, sizeof(registerResParams));
    SET_VER(registerResParams, NV_ENC_REGISTER_RESOURCE);

	registerResParams.resourceType = resource_type;
	registerResParams.resourceToRegister = resource_to_register;
    registerResParams.width = width;
    registerResParams.height = height;
    registerResParams.pitch = pitch;

    status = _encoder_api->nvEncRegisterResource(_encoder, &registerResParams);
	*registered_resource = registerResParams.registeredResource;

    return status;
}

NVENCSTATUS cu_enc_core::NvEncUnregisterResource(NV_ENC_REGISTERED_PTR registered_resource)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _encoder_api->nvEncUnregisterResource(_encoder, registered_resource);
    return status;
}

NVENCSTATUS cu_enc_core::NvEncFlushEncoderQueue(void * eos_event)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_PIC_PARAMS enc_pic_params;
	memset(&enc_pic_params, 0x00, sizeof(enc_pic_params));
	SET_VER(enc_pic_params, NV_ENC_PIC_PARAMS);
	enc_pic_params.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
	enc_pic_params.completionEvent = eos_event;
	status = _encoder_api->nvEncEncodePicture(_encoder, &enc_pic_params);
	return status;
}

NVENCSTATUS cu_enc_core::NvEncEncodePicture(NV_ENC_PIC_PARAMS * pic_params)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _encoder_api->nvEncEncodePicture(_encoder, pic_params);
	return status;
}


/*
NVENCSTATUS cu_enc_core::NvEncReconfigureEncoder(const NvEncPictureCommand *pEncPicCommand)
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

        status = _encoder_api->nvEncReconfigureEncoder(_encoder, &stReconfigParams);
        if (status != NV_ENC_SUCCESS)
        {
            assert(0);
        }
    }

    return status;
}
*/


NVENCSTATUS cu_enc_core::NvEncEncodeFrame(cu_enc_core::cu_enc_buffer_t * buffer, uint32_t width, uint32_t height)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_PIC_PARAMS enc_pic_params;

	memset(&enc_pic_params, 0x00, sizeof(enc_pic_params));
	SET_VER(enc_pic_params, NV_ENC_PIC_PARAMS);

	static int index = 0;

	enc_pic_params.inputBuffer = buffer->in.input_buffer;
	enc_pic_params.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12_PL;// pEncodeBuffer->stInputBfr.bufferFmt;
	enc_pic_params.inputWidth = width;
	enc_pic_params.inputHeight = height;
	enc_pic_params.outputBitstream = buffer->out.bitstream_buffer;
	enc_pic_params.completionEvent = buffer->out.output_evt;
	enc_pic_params.inputTimeStamp = index;
	enc_pic_params.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
	enc_pic_params.qpDeltaMap = 0;
	enc_pic_params.qpDeltaMapSize = 0;

	status = NvEncEncodePicture(&enc_pic_params);
	if (status != NV_ENC_SUCCESS && status != NV_ENC_ERR_NEED_MORE_INPUT)
		return status;

	index++;
	return NV_ENC_SUCCESS;
}