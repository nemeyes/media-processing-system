#include "nvenc_encoder.h"
#include <vector_types.h>
#include <cuda.h>

#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024

#if defined(WITH_DYNAMIC_CUDA_LOAD)
extern void interleave_uv(void* driver_api, unsigned int width, unsigned int height, unsigned char * src, unsigned int src_pitch, CUdeviceptr dst, unsigned int dst_pitch, CUdeviceptr cb, CUdeviceptr cr);
#else
#pragma comment(lib, "cuda.lib")
extern void interleave_uv(unsigned int width, unsigned int height, unsigned char * src, unsigned int src_pitch, CUdeviceptr dst, unsigned int dst_pitch, CUdeviceptr cb, CUdeviceptr cr);
#endif

nvenc_encoder::nvenc_encoder(dk_nvenc_encoder * front)
	: _front(front)
	, _config(nullptr)
	, _encoder(nullptr)
	, _nvenc_api(nullptr)
	, _nvenc_instance(nullptr)
	, _cuda_context(nullptr)
	, _enc_buffer_count(0)
	, _state(dk_nvenc_encoder::ENCODER_STATE_NONE)
{
	memset(&_enc_buffer, 0, sizeof(_enc_buffer));
	memset(&_enc_eos_output_buffer, 0, sizeof(_enc_eos_output_buffer));

#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
	memset(_ptr_cu_chroma, 0, sizeof(_ptr_cu_chroma));
#endif
}

nvenc_encoder::~nvenc_encoder(void)
{
	// clean up encode API resources here
}

dk_nvenc_encoder::ENCODER_STATE nvenc_encoder::state(void)
{
	return _state;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::initialize_encoder(dk_nvenc_encoder::configuration_t * config)
{
	if ((_state != dk_nvenc_encoder::ENCODER_STATE_NONE) && (_state != dk_nvenc_encoder::ENCODER_STATE_RELEASED))
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	release_encoder();
	_state = dk_nvenc_encoder::ENCODER_STATE_INITIALIZING;

	_config = config;
	NVENCSTATUS status = NV_ENC_SUCCESS;
	dk_nvenc_encoder::ERR_CODE result = dk_nvenc_encoder::ERR_CODE_FAIL;

	result = initialize_cuda();
	if (result != dk_nvenc_encoder::ERR_CODE_SUCCESS)
	{
		release_cuda();
		return result;
	}

	status = initialize_nvenc_encoder(NV_ENC_DEVICE_TYPE_CUDA);
	if (status != NV_ENC_SUCCESS)
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;
	}

	if (dk_nvenc_encoder::ERR_CODE_SUCCESS != is_h264_supported())
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;
	}

	//if (dk_nvenc_encoder::ERR_CODE_SUCCESS != is_async_encode_supported())
	//	return dk_nvenc_encoder::ERR_CODE_FAIL;

	int max_width = 0;
	if (dk_nvenc_encoder::ERR_CODE_SUCCESS != get_max_width(max_width))
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	}

	int max_height = 0;
	if (dk_nvenc_encoder::ERR_CODE_SUCCESS != get_max_height(max_height))
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAIL;
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

	_config->max_width = max_width;
	_config->max_height = max_height;
	_config->width = (_config->width > _config->max_width ? _config->max_width : _config->width);
	_config->height = (_config->height > _config->max_height ? _config->max_height : _config->height);
	_config->bitstream_buffer_size = (_config->bitrate / 8)*(/*_enc_config.gop* / _enc_config.fps*/_config->keyframe_interval) * 2;

	NV_ENC_INITIALIZE_PARAMS nvenc_init_params;
	memset(&nvenc_init_params, 0x00, sizeof(nvenc_init_params));
	SET_VER(nvenc_init_params, NV_ENC_INITIALIZE_PARAMS);
	NV_ENC_CONFIG nvenc_config;
	memset(&nvenc_config, 0x00, sizeof(nvenc_config));
	SET_VER(nvenc_config, NV_ENC_CONFIG);
	nvenc_init_params.encodeConfig = &nvenc_config;
	nvenc_init_params.encodeGUID = NV_ENC_CODEC_H264_GUID;
	nvenc_init_params.presetGUID = NV_ENC_PRESET_HQ_GUID;
	nvenc_init_params.encodeWidth = _config->width;
	nvenc_init_params.encodeHeight = _config->height;
	nvenc_init_params.darWidth = _config->width;
	nvenc_init_params.darHeight = _config->height;
	nvenc_init_params.frameRateNum = _config->fps;
	nvenc_init_params.frameRateDen = 1;
	nvenc_init_params.maxEncodeWidth = _config->max_width;
	nvenc_init_params.maxEncodeHeight = _config->max_height;
	nvenc_init_params.enableEncodeAsync = 1;
	nvenc_init_params.enableEncodeAsync = 0;
	nvenc_init_params.enablePTD = 1;
	nvenc_init_params.encodeConfig->gopLength = _config->keyframe_interval*_config->fps;
	nvenc_init_params.encodeConfig->frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
	nvenc_init_params.encodeConfig->mvPrecision = NV_ENC_MV_PRECISION_DEFAULT;
	nvenc_init_params.encodeConfig->frameIntervalP = 1;
	nvenc_init_params.encodeConfig->rcParams.averageBitRate = _config->bitrate;
	nvenc_init_params.encodeConfig->rcParams.maxBitRate = _config->vbv_max_bitrate;
	nvenc_init_params.encodeConfig->rcParams.vbvBufferSize = _config->vbv_size;
	nvenc_init_params.encodeConfig->rcParams.vbvInitialDelay = _config->vbv_size * 9 / 10;
	nvenc_init_params.encodeConfig->rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)dk_nvenc_encoder::RC_MODE_CONSTQP;
	nvenc_init_params.encodeConfig->rcParams.constQP.qpInterP = nvenc_init_params.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : 28;
	nvenc_init_params.encodeConfig->rcParams.constQP.qpInterB = nvenc_init_params.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : 28;
	nvenc_init_params.encodeConfig->rcParams.constQP.qpIntra = nvenc_init_params.presetGUID == NV_ENC_PRESET_LOSSLESS_HP_GUID ? 0 : 28;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.chromaFormatIDC = 1;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.idrPeriod = nvenc_init_params.encodeConfig->gopLength;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.maxNumRefFrames = 16;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.bdirectMode = nvenc_init_params.encodeConfig->frameIntervalP > 1 ? NV_ENC_H264_BDIRECT_MODE_TEMPORAL : NV_ENC_H264_BDIRECT_MODE_DISABLE;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_AUTOSELECT;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.level = NV_ENC_LEVEL_AUTOSELECT;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.adaptiveTransformMode = NV_ENC_H264_ADAPTIVE_TRANSFORM_ENABLE;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.fmoMode = NV_ENC_H264_FMO_DISABLE;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.outputBufferingPeriodSEI = 1;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.outputPictureTimingSEI = 1;
	//nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.disableSPSPPS = 1;
	nvenc_init_params.encodeConfig->encodeCodecConfig.h264Config.enableVFR = 0;

	status = NvEncInitializeEncoder(&nvenc_init_params);

	_enc_buffer_count = _config->numb + 4;//min buffers is numb +1 +3 pipelining
	result = allocate_io_buffers(_config->width, _config->height);
	if (result != dk_nvenc_encoder::ERR_CODE_SUCCESS)
		return result;

	status = NvEncInitializeEncoder(&nvenc_init_params);
	if (status != NV_ENC_SUCCESS)
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	}

	_enc_buffer_count = _config->numb + 4;//min buffers is numb +1 +3 pipelining
	result = allocate_io_buffers(_config->width, _config->height);
	if (result != dk_nvenc_encoder::ERR_CODE_SUCCESS)
	{
		release_nvenc_encoder();
		release_cuda();
		return result;
	}

	///////////////////query pitch(stride)/////////////////
	//void * tmp_inbuffer;
	//unsigned char * tmp_inbuffer_surface;
	//status = _nvenc_encoder->NvEncCreateInputBuffer(_enc_config.width, _enc_config.height, (NV_ENC_BUFFER_FORMAT)_input_format, &tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAIL;
	//status = _nvenc_encoder->NvEncLockInputBuffer(tmp_inbuffer, (void**)&tmp_inbuffer_surface, &(*pitch));
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAIL;
	//status = _nvenc_encoder->NvEncUnlockInputBuffer(tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAIL;
	//_nvenc_encoder->NvEncDestroyInputBuffer(tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAIL;
	///////////////////query pitch(stride)/////////////////

	_encode_index = 0;


	_state = dk_nvenc_encoder::ENCODER_STATE_INITIALIZED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::release_encoder(void)
{
	if ((_state != dk_nvenc_encoder::ENCODER_STATE_NONE) && (_state != dk_nvenc_encoder::ENCODER_STATE_INITIALIZED) && (_state != dk_nvenc_encoder::ENCODER_STATE_ENCODED))
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	_state = dk_nvenc_encoder::ENCODER_STATE_RELEASING;

	NVENCSTATUS status = NV_ENC_SUCCESS;

	if (flush_encoder() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	if (release_io_buffers() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	if (release_nvenc_encoder() != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	if (release_cuda() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	_state = dk_nvenc_encoder::ENCODER_STATE_RELEASED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input, dk_nvenc_encoder::dk_video_entity_t * bitstream)
{

}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::encode(dk_nvenc_encoder::dk_video_entity_t * input)
{
	if ((_state != dk_nvenc_encoder::ENCODER_STATE_INITIALIZED) &&
		(_state != dk_nvenc_encoder::ENCODER_STATE_ENCODED))
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;

	_state = dk_nvenc_encoder::ENCODER_STATE_ENCODING;

	//입력은 프레임단위로 진행되는 것을 가정함
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int locked_pitch = 0;

	if (input->flush)
	{
		flush_encoder();
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;
	}

	nvenc_encoder::cu_enc_buffer_t * buffer = _enc_buffer_queue.get_available();
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
	cu_auto_lock lock(_cuda_driver_api, (CUcontext)_cuda_context);  //sometimes auto_lock doesn't return
#else
	cu_auto_lock lock((CUcontext)_cuda_context);  //sometimes auto_lock doesn't return
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
		return AFCCudaEncoder::ERR_CODE_FAIL;
#else
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	interleave_uv((void*)_cuda_driver_api, _config->width, _config->height, input->data, _config->width, buffer->in.device_ptr, buffer->in.stride, _ptr_cu_chroma[0], _ptr_cu_chroma[1]);
#else
	interleave_uv(_config->width, _config->height, input->data, _config->width, buffer->in.device_ptr, buffer->in.stride, _ptr_cu_chroma[0], _ptr_cu_chroma[1]);
#endif
#endif
	status = NvEncMapInputResource(buffer->in.registered_resource, &buffer->in.input_buffer);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
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
	NV_ENC_PIC_PARAMS nvenc_pic_params;
	memset(&nvenc_pic_params, 0x00, sizeof(nvenc_pic_params));
	SET_VER(nvenc_pic_params, NV_ENC_PIC_PARAMS);

	nvenc_pic_params.inputBuffer = buffer->in.input_buffer;
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)
	nvenc_pic_params.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12_PL;
#else
	nvenc_pic_params.bufferFmt = (NV_ENC_BUFFER_FORMAT)_input_format;
#endif
	nvenc_pic_params.inputWidth = _config->width;
	nvenc_pic_params.inputHeight = _config->height;
	nvenc_pic_params.outputBitstream = buffer->out.bitstream_buffer;
	nvenc_pic_params.completionEvent = buffer->out.output_evt;
	nvenc_pic_params.inputTimeStamp = _encode_index;
	nvenc_pic_params.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
	nvenc_pic_params.qpDeltaMap = 0;
	nvenc_pic_params.qpDeltaMapSize = 0;

	status = NvEncEncodePicture(&nvenc_pic_params);
	if (status != NV_ENC_SUCCESS && status != NV_ENC_ERR_NEED_MORE_INPUT)
	{
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	}

	_encode_index++;
#endif

	_state = dk_nvenc_encoder::ENCODER_STATE_ENCODED;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream)
{

}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::encode_async(dk_nvenc_encoder::dk_video_entity_t * input)
{

}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::check_encoding_flnish(void)
{

}


/*dk_nvenc_encoder::ERR_CODE nvenc_encoder::initialize(dk_nvenc_encoder::configuration_t config, unsigned int * pitch)
{
	if (_binit)
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;

	NVENCSTATUS status = NV_ENC_SUCCESS;
	dk_nvenc_encoder::ERR_CODE result = dk_nvenc_encoder::ERR_CODE_FAIL;
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
	//	return dk_nvenc_encoder::ERR_CODE_FAIL;
	
	int max_width = 0;
	if (dk_nvenc_encoder::ERR_CODE_SUCCESS != get_max_width(max_width))
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	}
		
	int max_height = 0;
	if (dk_nvenc_encoder::ERR_CODE_SUCCESS != get_max_height(max_height))
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAIL;
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
	_enc_config.bitstream_buffer_size = (_enc_config.bitrate / 8)*(_enc_config.keyframe_interval) * 2;


	NV_ENC_INITIALIZE_PARAMS init_params;
	memset(&init_params, 0x00, sizeof(init_params));
	SET_VER(init_params, NV_ENC_INITIALIZE_PARAMS);
	NV_ENC_CONFIG encode_config;
	memset(&encode_config, 0x00, sizeof(encode_config));
	SET_VER(encode_config, NV_ENC_CONFIG);
	init_params.encodeConfig = &encode_config;
	init_params.encodeGUID = NV_ENC_CODEC_H264_GUID;

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
	init_params.encodeConfig->rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)dk_nvenc_encoder::RC_MODE_CONSTQP;
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
	init_params.encodeConfig->rcParams.rateControlMode = (NV_ENC_PARAMS_RC_MODE)dk_nvenc_encoder::RC_MODE_CBR;
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
	
	status = NvEncInitializeEncoder(&init_params);

	_enc_buffer_count = _enc_config.numb + 4;//min buffers is numb +1 +3 pipelining
	result = allocate_io_buffers(_enc_config.width, _enc_config.height);
	if (result != dk_nvenc_encoder::ERR_CODE_SUCCESS)
		return result;

	status = NvEncInitializeEncoder(&init_params);
	if (status != NV_ENC_SUCCESS)
	{
		release_encoder();
		release_cuda();
		return dk_nvenc_encoder::ERR_CODE_FAIL;
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
	//status = _nvenc_encoder->NvEncCreateInputBuffer(_enc_config.width, _enc_config.height, (NV_ENC_BUFFER_FORMAT)_input_format, &tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAIL;
	//status = _nvenc_encoder->NvEncLockInputBuffer(tmp_inbuffer, (void**)&tmp_inbuffer_surface, &(*pitch));
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAIL;
	//status = _nvenc_encoder->NvEncUnlockInputBuffer(tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAIL;
	//_nvenc_encoder->NvEncDestroyInputBuffer(tmp_inbuffer);
	//if (status != NV_ENC_SUCCESS)
	//	return ERR_CODE_FAIL;
	///////////////////query pitch(stride)/////////////////

	_encode_index = 0;
	_binit = true;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::release(void)
{
	if (_binit)
	{
		NVENCSTATUS status = NV_ENC_SUCCESS;

		if (flush_encoder() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAIL;
		if (release_io_buffers() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAIL;
		if (release_encoder() != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAIL;
		if (release_cuda() != dk_nvenc_encoder::ERR_CODE_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAIL;

		_binit = false;
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, NV_ENC_PIC_TYPE & bs_pic_type, bool flush)
{
	//입력은 프레임단위로 진행되는 것을 가정함
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int locked_pitch = 0;

	if (flush)
	{
		flush_encoder();
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;
	}

	nvenc_encoder::cu_enc_buffer_t * buffer = _enc_buffer_queue.get_available();
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
	cu_auto_lock lock(_cuda_driver_api, (CUcontext)_cuda_context);  //sometimes auto_lock doesn't return
#else
	cu_auto_lock lock((CUcontext)_cuda_context);  //sometimes auto_lock doesn't return
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
		return AFCCudaEncoder::ERR_CODE_FAIL;
#else
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	interleave_uv((void*)_cuda_driver_api, _enc_config.width, _enc_config.height, input, _enc_config.width, buffer->in.device_ptr, buffer->in.stride, _ptr_cu_chroma[0], _ptr_cu_chroma[1]);
#else
	interleave_uv(_enc_config.width, _enc_config.height, input, _enc_config.width, buffer->in.device_ptr, buffer->in.stride, _ptr_cu_chroma[0], _ptr_cu_chroma[1]);
#endif
#endif
	status = NvEncMapInputResource(buffer->in.registered_resource, &buffer->in.input_buffer);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
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
	//status = _nvenc_api->nvEncEncodePicture(_encoder, &enc_pic_params);
	if (status != NV_ENC_SUCCESS && status != NV_ENC_ERR_NEED_MORE_INPUT)
	{
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	}

	_encode_index++;
#endif

	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}*/


dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_h264_supported(void)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	dk_nvenc_encoder::ERR_CODE result = dk_nvenc_encoder::ERR_CODE_FAIL;
	unsigned int codec_guid_count = 0;
	status = NvEncGetEncodeGUIDCount(&codec_guid_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	if (codec_guid_count == 0)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	GUID * codec_guids = new GUID[codec_guid_count];
	status = NvEncGetEncodeGUIDs(codec_guids, codec_guid_count, &codec_guid_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

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

/*dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_supported_codecs(std::vector<int> * const codec)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int codec_guid_count = 0;
	status = NvEncGetEncodeGUIDCount(&codec_guid_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	if (codec_guid_count == 0)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	GUID * codec_guids = new GUID[codec_guid_count];
	status = NvEncGetEncodeGUIDs(codec_guids, codec_guid_count, &codec_guid_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	for (unsigned int i = 0; i < codec_guid_count; i++)
	{
		if (codec_guids[i] == NV_ENC_CODEC_H264_GUID)
			codec->push_back(dk_nvenc_encoder::CODEC_TYPE_H264);
		else if (codec_guids[i] == NV_ENC_CODEC_HEVC_GUID)
			codec->push_back(dk_nvenc_encoder::CODEC_TYPE_HEVC);
	}
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}*/

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_supported_codec_profiles(std::vector<int> * const codec_profiles)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int codec_profile_count = 0;
	status = NvEncGetEncodeProfileGUIDCount(NV_ENC_CODEC_H264_GUID, &codec_profile_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	if (codec_profile_count == 0)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	GUID * codec_profile_guids = new GUID[codec_profile_count];
	status = NvEncGetEncodePresetGUIDs(NV_ENC_CODEC_H264_GUID, codec_profile_guids, codec_profile_count, &codec_profile_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

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


dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_supported_codec_presets(std::vector<int> * const codec_presets)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	unsigned int codec_preset_count = 0;
	status = NvEncGetEncodePresetCount(NV_ENC_CODEC_H264_GUID, &codec_preset_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	if (codec_preset_count == 0)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	GUID * codec_preset_guids = new GUID[codec_preset_count];
	status = NvEncGetEncodePresetGUIDs(NV_ENC_CODEC_H264_GUID, codec_preset_guids, codec_preset_count, &codec_preset_count);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

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

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_max_bframes(int & max_bframes)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_NUM_MAX_BFRAMES;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &max_bframes);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_supported_rc_mode(int & rc_mode)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_supported_field_encoding(int & field_encoding)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_monochrom_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_fmo_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_qpelmv_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_bdirect_mode_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_cabac_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_adaptive_transform_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_temporal_layers_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_hierarchical_pframes_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_hierarchical_bframes_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_max_encoding_level(int & enc_level)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_LEVEL_MAX;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &enc_level);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_min_encoding_level(int & enc_level)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_LEVEL_MIN;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &enc_level);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_separate_color_plane_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_max_width(int & max_width)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_WIDTH_MAX;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &max_width);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_max_height(int & max_height)
{
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_HEIGHT_MAX;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &max_height);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_temporal_svc_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_dynamic_resolution_change_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_dynamic_bitrate_change_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_dynamic_rc_mode_change_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_subframe_readback_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_constrained_encoding_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_intra_refresh_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_custom_vbv_buf_size_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_dynamic_slice_mode_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_ref_pic_invalidation_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_supported_preprocessing_flags(int *preproc_flag)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_async_encode_supported(void)
{
	int result = 0;
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_ASYNC_ENCODE_SUPPORT;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &result);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	if (result == 0)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	else
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::get_max_mb(int & max_mb)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_yuv444_encode_supported(void)
{
	return dk_nvenc_encoder::ERR_CODE_UNSUPPORTED_FUNCTION;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::is_lossless_encode_supported(void)
{
	int result = 0;
	NV_ENC_CAPS_PARAM caps_param;
	SET_VER(caps_param, NV_ENC_CAPS_PARAM);
	caps_param.capsToQuery = NV_ENC_CAPS_SUPPORT_LOSSLESS_ENCODE;
	NVENCSTATUS status = NvEncGetEncodeCaps(NV_ENC_CODEC_H264_GUID, &caps_param, &result);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	if (result == 0)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	else
		return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

////////////////////////////////////private function
dk_nvenc_encoder::ERR_CODE nvenc_encoder::initialize_cuda(void)
{
	CUresult result;
	CUdevice device;
	CUcontext cu_current_context;
	int  device_count = 0;
	int  SMminor = 0, SMmajor = 0;

#if defined(WITH_DYNAMIC_CUDA_LOAD)
	_cuda_driver_api = new dk_cuda_driver_api();
	if (!_cuda_driver_api || !_cuda_driver_api->load())
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;

	result = _cuda_driver_api->init(0);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;

	result = _cuda_driver_api->device_get_count(&device_count);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;

	if (device_count<1)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;

	size_t available_device_memory = 0;
	int device_id = -1;
	for (int index = 0; index < device_count; index++)
	{
		CUdevice tmp_device;
		size_t tmp_available_device_memory = 0;
		result = _cuda_driver_api->device_get(&tmp_device, index);
		if (result != CUDA_SUCCESS)
			continue;
		result = _cuda_driver_api->device_compute_capability(&SMmajor, &SMminor, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;
		if (((SMmajor << 4) + SMminor) < 0x30)
			continue;
		result = _cuda_driver_api->device_total_memory(&tmp_available_device_memory, tmp_device);
		if (result != CUDA_SUCCESS)
			continue;

		if (available_device_memory < tmp_available_device_memory)
			device_id = index;
	}

	if (device_id<0)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;
	result = _cuda_driver_api->device_get(&device, device_id);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;

	result = _cuda_driver_api->ctx_create((CUcontext*)(&_cuda_context), 0, device);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;
#else
	result = cuInit(0);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;

	result = cuDeviceGetCount(&device_count);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;

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
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;
	result = cuDeviceGet(&device, device_id);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;

	result = cuCtxCreate((CUcontext*)(&_cuda_context), 0, device);
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_INVALID_ENCODING_DEVICE;
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
		return dk_nvenc_encoder::ERR_CODE_FAIL;

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
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	delete[] jitOptions;
	delete[] jitOptVals;
	delete[] jitLogBuffer;

	result = cuModuleGetFunction(&_fn_interleave_uv, _cu_module, "interleave_uv");
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
#endif
#endif
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	result = _cuda_driver_api->ctx_pop_current(&cu_current_context);
#else
	result = cuCtxPopCurrent(&cu_current_context);
#endif
	if (result != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::release_cuda(void)
{
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	if (!_cuda_driver_api)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	CUresult status = _cuda_driver_api->ctx_destroy((CUcontext)_cuda_context);
	if (status != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	if (!_cuda_driver_api->free())
		return dk_nvenc_encoder::ERR_CODE_FAIL;
#else
	CUresult status = cuCtxDestroy((CUcontext)_cuda_context);
	if (status != CUDA_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;
#endif

	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::allocate_io_buffers(unsigned int input_width, unsigned int input_height)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	_enc_buffer_queue.initialize(_enc_buffer, _enc_buffer_count);
	
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	cu_auto_lock lock(_cuda_driver_api, (CUcontext)_cuda_context);  //sometimes auto_lock doesn't return
#else
	cu_auto_lock lock((CUcontext)_cuda_context);  //sometimes auto_lock doesn't return
#endif

#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	_cuda_driver_api->mem_alloc(&_ptr_cu_chroma[0], input_width*input_height / 4);
	_cuda_driver_api->mem_alloc(&_ptr_cu_chroma[1], input_width*input_height / 4);
#else
	cuMemAlloc(&_ptr_cu_chroma[0], input_width*input_height / 4);
	cuMemAlloc(&_ptr_cu_chroma[1], input_width*input_height / 4);
#endif
#endif

	for (uint32_t i = 0; i < _enc_buffer_count; i++)
	{
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
#if defined(WITH_DYNAMIC_CUDA_LOAD)
		_cuda_driver_api->mem_alloc_pitch(&_enc_buffer[i].in.device_ptr, (size_t *)&_enc_buffer[i].in.stride, input_width, input_height * 1.5, 16);
#else
		cuMemAllocPitch(&_enc_buffer[i].in.device_ptr, (size_t *)&_enc_buffer[i].in.stride, input_width, input_height * 1.5, 16);
#endif
		status = NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR, (void*)_enc_buffer[i].in.device_ptr,
									   input_width, input_height, _enc_buffer[i].in.stride, &_enc_buffer[i].in.registered_resource);
		if (status != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAIL;
#else
		status = NvEncCreateInputBuffer(input_width, input_height, (NV_ENC_BUFFER_FORMAT)_input_format, &_enc_buffer[i].in.input_buffer);
		if (status != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAIL;
#endif
		status = NvEncCreateBitstreamBuffer(BITSTREAM_BUFFER_SIZE, &_enc_buffer[i].out.bitstream_buffer);
		if (status != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAIL;
		_enc_buffer[i].out.bitstream_buffer_size = BITSTREAM_BUFFER_SIZE;

#if defined(_WIN32) && defined(WITH_ASYNC)
		status = NvEncRegisterAsyncEvent(&_enc_buffer[i].out.output_evt);
		if (status != NV_ENC_SUCCESS)
			return dk_nvenc_encoder::ERR_CODE_FAIL;

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
		return dk_nvenc_encoder::ERR_CODE_FAIL;
#else
	_enc_eos_output_buffer.output_evt = NULL;
#endif

	return dk_nvenc_encoder::ERR_CODE_SUCCESS;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::release_io_buffers(void)
{
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	cu_auto_lock lock(_cuda_driver_api, (CUcontext)_cuda_context); //sometimes auto_lock doesn't return
#else
	cu_auto_lock lock((CUcontext)_cuda_context); //sometimes auto_lock doesn't return
#endif
	for (uint32_t i = 0; i < _enc_buffer_count; i++)
	{
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)
		NvEncUnregisterResource(_enc_buffer[i].in.registered_resource);
#if defined(WITH_DYNAMIC_CUDA_LOAD)
		_cuda_driver_api->mem_free(_enc_buffer[i].in.device_ptr);
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
	_cuda_driver_api->mem_free(_ptr_cu_chroma[0]);
	_cuda_driver_api->mem_free(_ptr_cu_chroma[1]);
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

dk_nvenc_encoder::ERR_CODE nvenc_encoder::flush_encoder(void)
{
	dk_nvenc_encoder::ERR_CODE result = dk_nvenc_encoder::ERR_CODE_SUCCESS;
	NVENCSTATUS status = NvEncFlushEncoderQueue(_enc_eos_output_buffer.output_evt);
	if (status != NV_ENC_SUCCESS)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	nvenc_encoder::cu_enc_buffer_t * enc_buffer = _enc_buffer_queue.get_pending();
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
		result = dk_nvenc_encoder::ERR_CODE_FAIL;
#endif
	return result;
}

dk_nvenc_encoder::ERR_CODE nvenc_encoder::process(const nvenc_encoder::cu_enc_buffer_t * enc_buffer, unsigned char * encoded, unsigned int & encoded_size, NV_ENC_PIC_TYPE & bs_pic_type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	encoded_size = 0;
	if (enc_buffer->out.bitstream_buffer == NULL && enc_buffer->out.eos == FALSE)
		return dk_nvenc_encoder::ERR_CODE_FAIL;

	if (enc_buffer->out.wait_evt)
	{
		if (!enc_buffer->out.output_evt)
			return dk_nvenc_encoder::ERR_CODE_FAIL;
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
dk_nvenc_encoder::ERR_CODE nvenc_encoder::convert_yv12pitch_to_nv12(unsigned char * yv12_luma, unsigned char * yv12_cb, unsigned char * yv12_cr,
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

dk_nvenc_encoder::ERR_CODE nvenc_encoder::convert_yv12pitch_to_yv12(unsigned char * yv12_luma, unsigned char * yv12_cb, unsigned char * yv12_cr,
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

NVENCSTATUS nvenc_encoder::initialize_nvenc_encoder(NV_ENC_DEVICE_TYPE device_type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	MYPROC nvEncodeAPICreateInstance; // function pointer to create instance in nvEncodeAPI

#if defined(_WIN32)
#if defined (_WIN64)
	_nvenc_instance = LoadLibrary(TEXT("nvEncodeAPI64.dll"));
#else
	_nvenc_instance = LoadLibrary(TEXT("nvEncodeAPI.dll"));
#endif
#else
	_nvenc_instance = dlopen("libnvidia-encode.so.1", RTLD_LAZY);
#endif
	if (_nvenc_instance == 0)
		return NV_ENC_ERR_OUT_OF_MEMORY;

#if defined(_WIN32)
	nvEncodeAPICreateInstance = (MYPROC)GetProcAddress(_nvenc_instance, "NvEncodeAPICreateInstance");
#else
	nvEncodeAPICreateInstance = (MYPROC)dlsym(_nvenc_instance, "NvEncodeAPICreateInstance");
#endif

	if (nvEncodeAPICreateInstance == NULL)
		return NV_ENC_ERR_OUT_OF_MEMORY;

	_nvenc_api = new NV_ENCODE_API_FUNCTION_LIST;
	if (_nvenc_api == NULL)
		return NV_ENC_ERR_OUT_OF_MEMORY;

	memset(_nvenc_api, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
	_nvenc_api->version = NV_ENCODE_API_FUNCTION_LIST_VER;
	status = nvEncodeAPICreateInstance(_nvenc_api);
	if (status != NV_ENC_SUCCESS)
		return status;

	__try
	{
		_encoder = 0;
		status = NvEncOpenEncodeSessionEx(_cuda_context, device_type);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NV_ENC_ERR_GENERIC;
	}
	
	if (status != NV_ENC_SUCCESS)
		return status;

	return NV_ENC_SUCCESS;
}

NVENCSTATUS nvenc_encoder::release_nvenc_encoder(void)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	__try {
		status = NvEncDestroyEncoder();
	}__except (EXCEPTION_EXECUTE_HANDLER) {}
	_encoder = 0;

	if (_nvenc_api)
	{
		delete _nvenc_api;
		_nvenc_api = nullptr;
	}

	if (_nvenc_instance)
	{
#if defined (_WIN32)
		FreeLibrary(_nvenc_instance);
#else
		dlclose(_nvenc_instance);
#endif
		_nvenc_instance = nullptr;
	}
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncOpenEncodeSession(void * device, unsigned int device_type)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _nvenc_api->nvEncOpenEncodeSession(device, device_type, &_encoder);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncOpenEncodeSessionEx(void * device, NV_ENC_DEVICE_TYPE device_type)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS open_sessionex_params;

	memset(&open_sessionex_params, 0, sizeof(open_sessionex_params));
	SET_VER(open_sessionex_params, NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS);

	open_sessionex_params.device = device;
	open_sessionex_params.deviceType = device_type;
	open_sessionex_params.reserved = NULL;
	open_sessionex_params.apiVersion = NVENCAPI_VERSION;

	status = _nvenc_api->nvEncOpenEncodeSessionEx(&open_sessionex_params, &_encoder);
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncInitializeEncoder(NV_ENC_INITIALIZE_PARAMS * params)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _nvenc_api->nvEncInitializeEncoder(_encoder, params);
	return status;

}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeGUIDCount(unsigned int * encode_guid_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _nvenc_api->nvEncGetEncodeGUIDCount(_encoder, encode_guid_count);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeGUIDs(GUID * encode_guids, unsigned int encode_guids_size, unsigned int * encode_guids_count)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _nvenc_api->nvEncGetEncodeGUIDs(_encoder, encode_guids, encode_guids_size, encode_guids_count);
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeProfileGUIDCount(GUID encode_guid, unsigned int * encode_profile_guid_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _nvenc_api->nvEncGetEncodeProfileGUIDCount(_encoder, encode_guid, encode_profile_guid_count);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeProfileGUIDs(GUID encode_guid, GUID * encode_profiles_guid, unsigned int encode_profiles_guid_size, unsigned int * encode_profiles_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeProfileGUIDs(_encoder, encode_guid, encode_profiles_guid, encode_profiles_guid_size, encode_profiles_count);
    return status;
}


NVENCSTATUS nvenc_encoder::NvEncGetInputFormatCount(GUID encode_guid, unsigned int * input_fmt_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetInputFormatCount(_encoder, encode_guid, input_fmt_count);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetInputFormats(GUID encode_guid, NV_ENC_BUFFER_FORMAT * input_fmts, unsigned int input_fmts_size, unsigned int * input_fmt_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetInputFormats(_encoder, encode_guid, input_fmts, input_fmts_size, input_fmt_count);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeCaps(GUID encode_guid, NV_ENC_CAPS_PARAM * caps_param, int * caps_val)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeCaps(_encoder, encode_guid, caps_param, caps_val);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodePresetCount(GUID encode_guid, unsigned int * encode_presets_guid_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodePresetCount(_encoder, encode_guid, encode_presets_guid_count);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodePresetGUIDs(GUID encode_guid, GUID * encode_presets_guid, unsigned int encode_presets_size, unsigned int * encode_presets_count)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodePresetGUIDs(_encoder, encode_guid, encode_presets_guid, encode_presets_size, encode_presets_count);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodePresetConfig(GUID encode_guid, GUID encode_preset_guid, NV_ENC_PRESET_CONFIG * encode_preset_config)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodePresetConfig(_encoder, encode_guid, encode_preset_guid, encode_preset_config);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncCreateInputBuffer(unsigned int width, unsigned int height, NV_ENC_BUFFER_FORMAT intput_buffer_fmt, void ** input_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_CREATE_INPUT_BUFFER ct_input_buffer_params;

	memset(&ct_input_buffer_params, 0x00, sizeof(ct_input_buffer_params));
	SET_VER(ct_input_buffer_params, NV_ENC_CREATE_INPUT_BUFFER);

	ct_input_buffer_params.width = width;
	ct_input_buffer_params.height = height;
	ct_input_buffer_params.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;
	ct_input_buffer_params.bufferFmt = intput_buffer_fmt;

	status = _nvenc_api->nvEncCreateInputBuffer(_encoder, &ct_input_buffer_params);
	*input_buffer = ct_input_buffer_params.inputBuffer;
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR input_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	if (input_buffer)
    {
		status = _nvenc_api->nvEncDestroyInputBuffer(_encoder, input_buffer);
    }

    return status;
}


NVENCSTATUS nvenc_encoder::NvEncLockInputBuffer(void * input_buffer, void ** buffer_data, unsigned int * pitch)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_LOCK_INPUT_BUFFER lock_input_buffer_params;

	memset(&lock_input_buffer_params, 0, sizeof(lock_input_buffer_params));
	SET_VER(lock_input_buffer_params, NV_ENC_LOCK_INPUT_BUFFER);

	lock_input_buffer_params.inputBuffer = input_buffer;
	status = _nvenc_api->nvEncLockInputBuffer(_encoder, &lock_input_buffer_params);
	*buffer_data = lock_input_buffer_params.bufferDataPtr;
	*pitch = lock_input_buffer_params.pitch;

	return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnlockInputBuffer(NV_ENC_INPUT_PTR input_buffer)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncUnlockInputBuffer(_encoder, input_buffer);
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncCreateBitstreamBuffer(unsigned int size, void ** bitstream_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    NV_ENC_CREATE_BITSTREAM_BUFFER ct_bitstream_buffer_params;

	memset(&ct_bitstream_buffer_params, 0x00, sizeof(ct_bitstream_buffer_params));
	SET_VER(ct_bitstream_buffer_params, NV_ENC_CREATE_BITSTREAM_BUFFER);

	ct_bitstream_buffer_params.size = size;
	ct_bitstream_buffer_params.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;

	status = _nvenc_api->nvEncCreateBitstreamBuffer(_encoder, &ct_bitstream_buffer_params);
	*bitstream_buffer = ct_bitstream_buffer_params.bitstreamBuffer;
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstream_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	if (bitstream_buffer)
    {
		status = _nvenc_api->nvEncDestroyBitstreamBuffer(_encoder, bitstream_buffer);
    }
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM * lock_bitstream_buffer_params)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncLockBitstream(_encoder, lock_bitstream_buffer_params);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstream_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncUnlockBitstream(_encoder, bitstream_buffer);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetEncodeStats(NV_ENC_STAT* encode_stats)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetEncodeStats(_encoder, encode_stats);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncGetSequenceParams(NV_ENC_SEQUENCE_PARAM_PAYLOAD * sequence_param_payload)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncGetSequenceParams(_encoder, sequence_param_payload);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncRegisterAsyncEvent(void ** completion_event)
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
	status = _nvenc_api->nvEncRegisterAsyncEvent(_encoder, &eventParams);
	*completion_event = eventParams.completionEvent;

    return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnregisterAsyncEvent(void * completion_event)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    NV_ENC_EVENT_PARAMS event_params;

	if (completion_event)
    {
		memset(&event_params, 0x00, sizeof(event_params));
		SET_VER(event_params, NV_ENC_EVENT_PARAMS);

		event_params.completionEvent = completion_event;

		status = _nvenc_api->nvEncUnregisterAsyncEvent(_encoder, &event_params);
    }

    return status;
}

NVENCSTATUS nvenc_encoder::NvEncMapInputResource(void * registered_resource, void ** mapped_resource)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    NV_ENC_MAP_INPUT_RESOURCE map_input_resource_params;

	memset(&map_input_resource_params, 0, sizeof(map_input_resource_params));
	SET_VER(map_input_resource_params, NV_ENC_MAP_INPUT_RESOURCE);

	map_input_resource_params.registeredResource = registered_resource;

	status = _nvenc_api->nvEncMapInputResource(_encoder, &map_input_resource_params);
	*mapped_resource = map_input_resource_params.mappedResource;

    return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnmapInputResource(NV_ENC_INPUT_PTR mapped_input_buffer)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
    
	if (mapped_input_buffer)
    {
		status = _nvenc_api->nvEncUnmapInputResource(_encoder, mapped_input_buffer);
    }

    return status;
}

NVENCSTATUS nvenc_encoder::NvEncDestroyEncoder(void)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;
	if (_encoder)
		status = _nvenc_api->nvEncDestroyEncoder(_encoder);
    return status;
}

/*
NVENCSTATUS nvenc_encoder::NvEncInvalidateRefFrames(const NvEncPictureCommand * picture_command)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	for (unsigned int i = 0; i < picture_command->numRefFramesToInvalidate; i++)
    {
		status = _nvenc_api->nvEncInvalidateRefFrames(_encoder, picture_command->refFrameNumbers[i]);
    }

    return status;
}
*/

NVENCSTATUS nvenc_encoder::NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE resource_type, void * resource_to_register, unsigned int width, unsigned int height, unsigned int pitch, void ** registered_resource)
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

	status = _nvenc_api->nvEncRegisterResource(_encoder, &registerResParams);
	*registered_resource = registerResParams.registeredResource;

    return status;
}

NVENCSTATUS nvenc_encoder::NvEncUnregisterResource(NV_ENC_REGISTERED_PTR registered_resource)
{
    NVENCSTATUS status = NV_ENC_SUCCESS;

	status = _nvenc_api->nvEncUnregisterResource(_encoder, registered_resource);
    return status;
}

NVENCSTATUS nvenc_encoder::NvEncFlushEncoderQueue(void * eos_event)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	NV_ENC_PIC_PARAMS enc_pic_params;
	memset(&enc_pic_params, 0x00, sizeof(enc_pic_params));
	SET_VER(enc_pic_params, NV_ENC_PIC_PARAMS);
	enc_pic_params.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
	enc_pic_params.completionEvent = eos_event;
	status = _nvenc_api->nvEncEncodePicture(_encoder, &enc_pic_params);
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncEncodePicture(NV_ENC_PIC_PARAMS * pic_params)
{
	NVENCSTATUS status = NV_ENC_SUCCESS;
	status = _nvenc_api->nvEncEncodePicture(_encoder, pic_params);
	return status;
}

NVENCSTATUS nvenc_encoder::NvEncEncodeFrame(nvenc_encoder::cu_enc_buffer_t * buffer, uint32_t width, uint32_t height)
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