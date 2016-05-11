#ifndef _NVENC_ENCODER_H_
#define _NVENC_ENCODER_H_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
//#include <d3d9.h>
//#include <d3d10_1.h>
//#include <d3d11.h>
//#include <dxgi1_2.h>
//#include <d3d11_1.h>
#pragma warning(disable : 4996)
//#include <dxva2api.h>

#include <atlbase.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <d3d11.h>

#include "dynlink_cuda.h" // <cuda.h>
#include "nvEncodeAPI.h"
#include "dk_nvenc_encoder.h"

namespace debuggerking
{
	class nvenc_core
	{
		static const int32_t max_encode_queue = 32;
	public:
		typedef struct _input_buffer_t
		{
			int32_t					width;
			int32_t					height;
			void *					nv12_surface;
			CUdeviceptr				nv12_device_ptr;
			uint32_t				nv12_stride;
			CUdeviceptr				nv12_temp_device_ptr;
			uint32_t				nv12_temp_stride;
			void *					nv_registered_resource;
			NV_ENC_INPUT_PTR		input_surface;
			NV_ENC_BUFFER_FORMAT	buffer_format;
			_input_buffer_t(void)
				: width(0)
				, height(0)
				, nv12_surface(NULL)
				, nv12_device_ptr(NULL)
				, nv12_stride(0)
				, nv12_temp_device_ptr(NULL)
				, nv12_temp_stride(NULL)
				, nv_registered_resource(NULL)
				, input_surface(NULL)
				, buffer_format(NV_ENC_BUFFER_FORMAT_YV12)
			{}
		} input_buffer_t;

		typedef struct _output_buffer_t
		{
			uint32_t				bitstream_buffer_size;
			NV_ENC_OUTPUT_PTR		bitstream_buffer;
			HANDLE					output_event;
			bool					wait_event;
			bool					eos;
			_output_buffer_t(void)
				: bitstream_buffer_size(0)
				, bitstream_buffer(NULL)
				, output_event(INVALID_HANDLE_VALUE)
				, wait_event(false)
				, eos(false)
			{}

		} output_buffer_t;

		typedef struct _buffer_t
		{
			input_buffer_t	input;
			output_buffer_t	output;
		} buffer_t;

		template<class T>
		class queue
		{
		public:
			queue(void);
			virtual ~queue(void);

			bool initialize(T *pItems, uint32_t size);
			T * get_available(void);
			T* get_pending(void);

		protected:
			T** _buffer;
			uint32_t _size;
			uint32_t _pending_count;
			uint32_t _available_index;
			uint32_t _pending_index;
		};

		nvenc_core(nvenc_encoder * front);
		virtual ~nvenc_core(void);

		nvenc_encoder::encoder_state state(void);

		int32_t initialize_encoder(nvenc_encoder::configuration_t * config);
		int32_t release_encoder(void);

		int32_t encode(nvenc_encoder::entity_t * input, nvenc_encoder::entity_t * bitstream);
		int32_t encode(nvenc_encoder::entity_t * input);
		int32_t get_qeueued_data(nvenc_encoder::entity_t * bitstream);

	private:
		NVENCSTATUS initialize_cuda(uint32_t device_id);
		NVENCSTATUS release_cuda(void);

		NVENCSTATUS initialize_d3d11(void);
		NVENCSTATUS release_d3d11(void);

		NVENCSTATUS initialize_nvenc_encoder(void * device, NV_ENC_DEVICE_TYPE type);
		NVENCSTATUS release_nvenc_encoder(void);

		NVENCSTATUS allocate_io_buffers(uint32_t width, uint32_t height);
		NVENCSTATUS release_io_buffers(void);
		NVENCSTATUS flush_encoder(void);

		NVENCSTATUS encode_frame(buffer_t * buffer, nvenc_encoder::entity_t * input);
		NVENCSTATUS process_output(const buffer_t * buffer, nvenc_encoder::entity_t * bitstream, bool flush = false);

		NVENCSTATUS convert_yv12pitch_to_nv12(uint8_t * src_y, uint8_t * src_u, uint8_t * src_v, uint8_t * dst_y, uint8_t * dst_u, int32_t width, int32_t height, uint32_t src_stride, uint32_t dst_stride);
		NVENCSTATUS convert_yv12pitch_to_yv12(uint8_t * src_y, uint8_t * src_u, uint8_t * src_v, uint8_t * dst_y, uint8_t * dst_u, int32_t width, int32_t height, uint32_t src_stride, uint32_t dst_stride);

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
		NVENCSTATUS NvEncCreateInputBuffer(uint32_t width, uint32_t height, NV_ENC_BUFFER_FORMAT fmt, void ** input_buffer);
		NVENCSTATUS NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR inputBuffer);
		NVENCSTATUS NvEncCreateBitstreamBuffer(uint32_t size, void** bitstreamBuffer);
		NVENCSTATUS NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer);
		NVENCSTATUS NvEncCreateMVBuffer(uint32_t size, void** bitstreamBuffer);
		NVENCSTATUS NvEncDestroyMVBuffer(NV_ENC_OUTPUT_PTR bitstreamBuffer);
		NVENCSTATUS NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM* lockBitstreamBufferParams);
		NVENCSTATUS NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstreamBuffer);
		NVENCSTATUS NvEncLockInputBuffer(void * input_buffer, void ** buffer_data, uint32_t * pitch);
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
		NVENCSTATUS NvEncFlushEncoderQueue(void * eos_event);
		NVENCSTATUS NvEncEncodePicture(NV_ENC_PIC_PARAMS * pic_params);

		//NVENCSTATUS NvRunMotionEstimationOnly(EncodeBuffer *pEncodeBuffer[2], MEOnlyConfig *pMEOnly);
		//NVENCSTATUS NvEncInvalidateRefFrames(const NvEncPictureCommand *pEncPicCommand);
		//NVENCSTATUS NvEncReconfigureEncoder(const NvEncPictureCommand *pEncPicCommand);
	private:
		nvenc_encoder * _front;
		nvenc_encoder::encoder_state _state;
		nvenc_encoder::configuration_t * _config;

	private:
		void * _context;
#if defined(WIN32)
		HINSTANCE _instance;
#else
		void * _instance;
#endif
		NV_ENCODE_API_FUNCTION_LIST * _api;
		void * _encoder;

		uint32_t _encode_index;
		uint32_t _buffer_count;
		buffer_t _buffer[max_encode_queue];
		queue<buffer_t> _queue;
		output_buffer_t _eos_output_buffer;

	private:
		ID3D11VideoProcessorEnumerator * _video_processor_enum;
		ID3D11VideoProcessor * _video_processor;



	};
};

typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);


#endif