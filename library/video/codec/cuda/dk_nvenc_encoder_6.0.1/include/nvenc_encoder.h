#ifndef _NVENC_ENCODER_H_
#define _NVENC_ENCODER_H_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <d3d9.h>
#include <d3d10_1.h>
#include <d3d11.h>
#include <dxva2api.h>
#include "dynlink_cuda.h" // <cuda.h>
#include "nvEncodeAPI.h"
#include "dk_nvenc_encoder.h"

class nvenc_encoder
{
public:
	nvenc_encoder(void);
	virtual ~nvenc_encoder(void);

	dk_nvenc_encoder::encoder_state state(void);

	dk_nvenc_encoder::err_code initialize_encoder(dk_nvenc_encoder::configuration_t * config);
	dk_nvenc_encoder::err_code release_encoder(void);

	dk_nvenc_encoder::err_code encode(dk_nvenc_encoder::dk_video_entity_t * input, dk_nvenc_encoder::dk_video_entity_t * bitstream);
	dk_nvenc_encoder::err_code encode(dk_nvenc_encoder::dk_video_entity_t * input);
	dk_nvenc_encoder::err_code get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream);

	dk_nvenc_encoder::err_code encode_async(dk_nvenc_encoder::dk_video_entity_t * input);
	dk_nvenc_encoder::err_code check_encoding_flnish(void);

	
	
private:	
	NVENCSTATUS initialize_cuda(uint32_t device_id);
	NVENCSTATUS release_cuda(void);


	NVENCSTATUS initialize_nvenc_encoder(void * device, NV_ENC_DEVICE_TYPE type);
	NVENCSTATUS release_nvenc_encoder(void);

private:
	NVENCSTATUS NvEncInitializeEncoder(NV_ENC_INITIALIZE_PARAMS * params);
	NVENCSTATUS NvEncOpenEncodeSession(void* device, uint32_t device_type);
	NVENCSTATUS NvEncGetEncodeGUIDCount(uint32_t* encodeGUIDCount);
	NVENCSTATUS NvEncGetEncodeProfileGUIDCount(GUID encodeGUID, uint32_t* encodeProfileGUIDCount);
	NVENCSTATUS NvEncGetEncodeProfileGUIDs(GUID encodeGUID, GUID* profileGUIDs, uint32_t guidArraySize, uint32_t* GUIDCount);
	NVENCSTATUS NvEncGetEncodeGUIDs(GUID* GUIDs, uint32_t guidArraySize, uint32_t* GUIDCount);
	NVENCSTATUS NvEncGetInputFormatCount(GUID encodeGUID, uint32_t* inputFmtCount);
	NVENCSTATUS NvEncGetInputFormats(GUID encodeGUID, NV_ENC_BUFFER_FORMAT* inputFmts, uint32_t inputFmtArraySize, uint32_t* inputFmtCount);
	NVENCSTATUS NvEncGetEncodeCaps(GUID encodeGUID, NV_ENC_CAPS_PARAM* capsParam, int* capsVal);
	NVENCSTATUS NvEncGetEncodePresetCount(GUID encodeGUID, uint32_t* encodePresetGUIDCount);
	NVENCSTATUS NvEncGetEncodePresetGUIDs(GUID encodeGUID, GUID* presetGUIDs, uint32_t guidArraySize, uint32_t* encodePresetGUIDCount);
	NVENCSTATUS NvEncGetEncodePresetConfig(GUID encodeGUID, GUID  presetGUID, NV_ENC_PRESET_CONFIG* presetConfig);
	NVENCSTATUS NvEncCreateInputBuffer(uint32_t width, uint32_t height, void** inputBuffer, uint32_t isYuv444);
	NVENCSTATUS NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR inputBuffer);
	NVENCSTATUS NvEncCreateBitstreamBuffer(uint32_t size, void** bitstreamBuffer);
	NVENCSTATUS NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer);
	NVENCSTATUS NvEncCreateMVBuffer(uint32_t size, void** bitstreamBuffer);
	NVENCSTATUS NvEncDestroyMVBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer);
	NVENCSTATUS NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM* lockBitstreamBufferParams);
	NVENCSTATUS NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstreamBuffer);
	NVENCSTATUS NvEncLockInputBuffer(void* inputBuffer, void** bufferDataPtr, uint32_t* pitch);
	NVENCSTATUS NvEncUnlockInputBuffer(NV_ENC_INPUT_PTR inputBuffer);
	NVENCSTATUS NvEncGetEncodeStats(NV_ENC_STAT* encodeStats);
	NVENCSTATUS NvEncGetSequenceParams(NV_ENC_SEQUENCE_PARAM_PAYLOAD* sequenceParamPayload);
	NVENCSTATUS NvEncRegisterAsyncEvent(void** completionEvent);
	NVENCSTATUS NvEncUnregisterAsyncEvent(void* completionEvent);
	NVENCSTATUS NvEncMapInputResource(void* registeredResource, void** mappedResource);
	NVENCSTATUS NvEncUnmapInputResource(NV_ENC_INPUT_PTR mappedInputBuffer);
	NVENCSTATUS NvEncDestroyEncoder(void);
	NVENCSTATUS NvEncOpenEncodeSessionEx(void* device, NV_ENC_DEVICE_TYPE deviceType);
	NVENCSTATUS NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE resourceType, void* resourceToRegister, uint32_t width, uint32_t height, uint32_t pitch, void** registeredResource);
	NVENCSTATUS NvEncUnregisterResource(NV_ENC_REGISTERED_PTR registeredRes);
	NVENCSTATUS NvEncFlushEncoderQueue(void *hEOSEvent);

	//NVENCSTATUS NvRunMotionEstimationOnly(EncodeBuffer *pEncodeBuffer[2], MEOnlyConfig *pMEOnly);
	//NVENCSTATUS NvEncInvalidateRefFrames(const NvEncPictureCommand *pEncPicCommand);
	//NVENCSTATUS NvEncReconfigureEncoder(const NvEncPictureCommand *pEncPicCommand);


	//common
private:
	dk_nvenc_encoder::encoder_state _state;
	dk_nvenc_encoder::configuration_t * _config;


	//cuda and nvenc
private:
	void * _context;
#if defined(WIN32)
	HINSTANCE _nvenc_instance;
#else
	void * _nvenc_instance;
#endif
	NV_ENCODE_API_FUNCTION_LIST * _nvenc_api;
	void * _nvenc_encoder;



private:
	IDirectXVideoProcessorService * _dxva2_video_process_services;
	IDirectXVideoProcessor * _dxva2_video_processor;



};

typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);















#endif