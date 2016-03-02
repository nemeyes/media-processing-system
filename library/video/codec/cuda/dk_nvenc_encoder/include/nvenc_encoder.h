#ifndef _NVENC_ENCODER_H_
#define _NVENC_ENCODER_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <cuda.h>
#include "dk_nvenc_encoder.h"
#include "dk_cuda_utils.h"
#include "cuda_device.h"
#include "dk_cuda_driver_api.h"
#include "nvEncodeAPI.h"

#define MAX_ENCODE_QUEUE 32
#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}

#if defined (_WIN32)
    #include "d3d9.h"
    #define NVENCAPI __stdcall
    #pragma warning(disable : 4996)
#else
    #include <dlfcn.h>
    #include <string.h>
    #define NVENCAPI
#endif



class cu_auto_lock
{
private:
	CUcontext _ctx;
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	dk_cuda_driver_api * _driver_api;
#endif

public:
#if defined(WITH_DYNAMIC_CUDA_LOAD)
	cu_auto_lock(dk_cuda_driver_api * driver_api, CUcontext ctx)
		: _driver_api(driver_api)
		, _ctx(ctx)
	{
		if (_driver_api)
			_driver_api->ctx_push_current(_ctx);
	};

	~cu_auto_lock(void)  
	{ 
		CUcontext cuLast = NULL; 
		if (_driver_api)
			_driver_api->ctx_pop_current(&cuLast);
	};
#else
	cu_auto_lock(CUcontext ctx)
		: _ctx(ctx)
	{
		cuCtxPushCurrent(_ctx);
	};

	~cu_auto_lock(void)
	{
		CUcontext cuLast = NULL;
		cuCtxPopCurrent(&cuLast);
	};
#endif
};

class nvenc_encoder
{
public:
	template<class T>
	class cu_enc_queue
	{
		T ** buffer;
		unsigned int size;
		unsigned int pending_count;
		unsigned int available_idx;
		unsigned int pending_idx;
	public:
		cu_enc_queue(void)
			: buffer(NULL)
			, size(0)
			, pending_count(0)
			, available_idx(0)
			, pending_idx(0)
		{
		}

		~cu_enc_queue(void)
		{
			delete[] buffer;
		}

		bool initialize(T * items, unsigned int size)
		{
			this->size = size;
			pending_count = 0;
			available_idx = 0;
			pending_idx = 0;
			buffer = new T *[size];
			for (unsigned int i = 0; i < size; i++)
			{
				buffer[i] = &items[i];
			}
			return true;
		}


		T * get_available(void)
		{
			if (pending_count == size)
				return NULL;
			T * item = buffer[available_idx];
			available_idx = (available_idx + 1) % size;
			pending_count += 1;
			return item;
		}

		T * get_pending(void)
		{
			if (pending_count == 0)
			{
				return NULL;
			}

			T * item = buffer[pending_idx];
			pending_idx = (pending_idx + 1) % size;
			pending_count -= 1;
			return item;
		}
	};

	typedef struct _enc_input_buffer_t
	{
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)
		NV_ENC_INPUT_PTR input_buffer;
		CUdeviceptr device_ptr;
		unsigned int stride;
		void * registered_resource;
#else
		NV_ENC_INPUT_PTR input_buffer;
#endif
	} enc_input_buffer_t;

	typedef struct _enc_output_buffer_t
	{
		unsigned int bitstream_buffer_size;
		NV_ENC_OUTPUT_PTR bitstream_buffer;
		HANDLE output_evt;
		bool wait_evt;
		bool eos;
	} enc_output_buffer_t;

	typedef struct _cu_enc_buffer_t
	{
		enc_input_buffer_t in;
		enc_output_buffer_t out;
	} cu_enc_buffer_t;


	nvenc_encoder(dk_nvenc_encoder * front);
	virtual ~nvenc_encoder(void);

	dk_nvenc_encoder::ENCODER_STATE state(void);

	dk_nvenc_encoder::ERR_CODE initialize_encoder(dk_nvenc_encoder::configuration_t * config);
	dk_nvenc_encoder::ERR_CODE release_encoder(void);

	dk_nvenc_encoder::ERR_CODE encode(dk_nvenc_encoder::dk_video_entity_t * rawstream, dk_nvenc_encoder::dk_video_entity_t * bitstream);
	dk_nvenc_encoder::ERR_CODE encode(dk_nvenc_encoder::dk_video_entity_t * rawstream);
	dk_nvenc_encoder::ERR_CODE get_queued_data(dk_nvenc_encoder::dk_video_entity_t * bitstream);

	dk_nvenc_encoder::ERR_CODE encode_async(dk_nvenc_encoder::dk_video_entity_t * input);
	dk_nvenc_encoder::ERR_CODE check_encoding_flnish(void);

	//codec support
	dk_nvenc_encoder::ERR_CODE is_h264_supported(void);
	//dk_nvenc_encoder::ERR_CODE get_supported_codecs(std::vector<int> * const codec);
	//codec profile support
	dk_nvenc_encoder::ERR_CODE get_supported_codec_profiles(std::vector<int> * const codec_profiles);
	//encoder preset support
	dk_nvenc_encoder::ERR_CODE get_supported_codec_presets(std::vector<int> * const codec_presets);

	//encoding capabilities
	dk_nvenc_encoder::ERR_CODE get_max_bframes(int & max_bframes);
	dk_nvenc_encoder::ERR_CODE get_supported_rc_mode(int & rc_mode);
	dk_nvenc_encoder::ERR_CODE get_supported_field_encoding(int & field_encoding);
	dk_nvenc_encoder::ERR_CODE is_monochrom_supported(void);
	dk_nvenc_encoder::ERR_CODE is_fmo_supported(void);
	dk_nvenc_encoder::ERR_CODE is_qpelmv_supported(void);
	dk_nvenc_encoder::ERR_CODE is_bdirect_mode_supported(void);
	dk_nvenc_encoder::ERR_CODE is_cabac_supported(void);
	dk_nvenc_encoder::ERR_CODE is_adaptive_transform_supported(void);
	dk_nvenc_encoder::ERR_CODE is_temporal_layers_supported(void);
	dk_nvenc_encoder::ERR_CODE is_hierarchical_pframes_supported(void);
	dk_nvenc_encoder::ERR_CODE is_hierarchical_bframes_supported(void);
	dk_nvenc_encoder::ERR_CODE get_max_encoding_level(int & enc_level);
	dk_nvenc_encoder::ERR_CODE get_min_encoding_level(int & enc_level);
	dk_nvenc_encoder::ERR_CODE is_separate_color_plane_supported(void);
	dk_nvenc_encoder::ERR_CODE get_max_width(int & max_width);
	dk_nvenc_encoder::ERR_CODE get_max_height(int & max_height);
	dk_nvenc_encoder::ERR_CODE is_temporal_svc_supported(void);
	dk_nvenc_encoder::ERR_CODE is_dynamic_resolution_change_supported(void);
	dk_nvenc_encoder::ERR_CODE is_dynamic_bitrate_change_supported(void);
	dk_nvenc_encoder::ERR_CODE is_dynamic_rc_mode_change_supported(void);
	dk_nvenc_encoder::ERR_CODE is_subframe_readback_supported(void);
	dk_nvenc_encoder::ERR_CODE is_constrained_encoding_supported(void);
	dk_nvenc_encoder::ERR_CODE is_intra_refresh_supported(void);
	dk_nvenc_encoder::ERR_CODE is_custom_vbv_buf_size_supported(void);
	dk_nvenc_encoder::ERR_CODE is_dynamic_slice_mode_supported(void);
	dk_nvenc_encoder::ERR_CODE is_ref_pic_invalidation_supported(void);
	dk_nvenc_encoder::ERR_CODE get_supported_preprocessing_flags(int *preproc_flag);
	dk_nvenc_encoder::ERR_CODE is_async_encode_supported(void);
	dk_nvenc_encoder::ERR_CODE get_max_mb(int & max_mb);
	dk_nvenc_encoder::ERR_CODE is_yuv444_encode_supported(void);
	dk_nvenc_encoder::ERR_CODE is_lossless_encode_supported(void);

private:
	dk_nvenc_encoder::ERR_CODE initialize_cuda(void);
	dk_nvenc_encoder::ERR_CODE release_cuda(void);

	dk_nvenc_encoder::ERR_CODE allocate_io_buffers(unsigned int input_width, unsigned int input_height);
	dk_nvenc_encoder::ERR_CODE release_io_buffers(void);
	dk_nvenc_encoder::ERR_CODE flush_encoder(void);
	dk_nvenc_encoder::ERR_CODE process(const nvenc_encoder::cu_enc_buffer_t * enc_buffer, unsigned char * encoded, unsigned int & encoded_size, NV_ENC_PIC_TYPE & bs_pic_type);


#if !defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
	dk_nvenc_encoder::ERR_CODE convert_yv12pitch_to_nv12(unsigned char * yv12_luma, unsigned char * yv12_cb, unsigned char * yv12_cr,
		unsigned char * nv12_luma, unsigned char * nv12_chroma, int width, int height, int src_stride, int dst_stride);
	dk_nvenc_encoder::ERR_CODE convert_yv12pitch_to_yv12(unsigned char * yv12_luma, unsigned char * yv12_cb, unsigned char * yv12_cr,
		unsigned char * yv12_luma1, unsigned char * yv12_chroma1, int width, int height, int src_stride, int dst_stride);
#endif


	NVENCSTATUS initialize_nvenc_encoder(NV_ENC_DEVICE_TYPE device_type);
	NVENCSTATUS release_nvenc_encoder(void);

    NVENCSTATUS NvEncOpenEncodeSession(void * device, unsigned int device_type);
	NVENCSTATUS NvEncOpenEncodeSessionEx(void * device, NV_ENC_DEVICE_TYPE device_type);
	NVENCSTATUS NvEncInitializeEncoder(NV_ENC_INITIALIZE_PARAMS * params);

	NVENCSTATUS NvEncGetEncodeGUIDCount(unsigned int * encode_guid_count);
	NVENCSTATUS NvEncGetEncodeGUIDs(GUID * encode_guids, unsigned int guidArraySize, unsigned int* GUIDCount);

	NVENCSTATUS NvEncGetEncodeProfileGUIDCount(GUID encode_guid, unsigned int * encode_profile_guid_count);
	NVENCSTATUS NvEncGetEncodeProfileGUIDs(GUID encode_guid, GUID * encode_profiles_guid, unsigned int encode_profiles_guid_size, unsigned int * encode_profiles_count);

	NVENCSTATUS NvEncGetInputFormatCount(GUID encode_guid, unsigned int * input_fmt_count);
	NVENCSTATUS NvEncGetInputFormats(GUID encode_guid, NV_ENC_BUFFER_FORMAT * input_fmts, unsigned int input_fmts_size, unsigned int * input_fmt_count);

	NVENCSTATUS NvEncGetEncodeCaps(GUID encode_guid, NV_ENC_CAPS_PARAM * caps_param, int * caps_val);

	NVENCSTATUS NvEncGetEncodePresetCount(GUID encode_guid, unsigned int * encode_presets_guid_count);
	NVENCSTATUS NvEncGetEncodePresetGUIDs(GUID encode_guid, GUID * encode_presets_guid, unsigned int encode_presets_size, unsigned int * encode_presets_count);
	NVENCSTATUS NvEncGetEncodePresetConfig(GUID encode_guid, GUID encode_preset_guid, NV_ENC_PRESET_CONFIG * encode_preset_config);

	NVENCSTATUS NvEncCreateInputBuffer(unsigned int width, unsigned int height, NV_ENC_BUFFER_FORMAT intput_buffer_fmt, void ** input_buffer);
	NVENCSTATUS NvEncDestroyInputBuffer(NV_ENC_INPUT_PTR input_buffer);
	NVENCSTATUS NvEncLockInputBuffer(void * input_buffer, void ** buffer_data, unsigned int * pitch);
	NVENCSTATUS NvEncUnlockInputBuffer(NV_ENC_INPUT_PTR input_buffer);

	NVENCSTATUS NvEncCreateBitstreamBuffer(unsigned int size, void ** bitstream_buffer);
	NVENCSTATUS NvEncDestroyBitstreamBuffer(NV_ENC_OUTPUT_PTR bitstream_buffer);
    NVENCSTATUS NvEncLockBitstream(NV_ENC_LOCK_BITSTREAM* lock_bitstream_buffer_params);
    NVENCSTATUS NvEncUnlockBitstream(NV_ENC_OUTPUT_PTR bitstream_buffer);

	NVENCSTATUS NvEncGetEncodeStats(NV_ENC_STAT* encode_stats);
	NVENCSTATUS NvEncGetSequenceParams(NV_ENC_SEQUENCE_PARAM_PAYLOAD* sequence_param_payload);

	NVENCSTATUS NvEncRegisterAsyncEvent(void ** completion_event);
	NVENCSTATUS NvEncUnregisterAsyncEvent(void * completion_event);

	NVENCSTATUS NvEncMapInputResource(void * registered_resource, void ** mapped_resource);
	NVENCSTATUS NvEncUnmapInputResource(NV_ENC_INPUT_PTR mapped_input_buffer);
    
	NVENCSTATUS NvEncDestroyEncoder(void);
    
	//NVENCSTATUS NvEncInvalidateRefFrames(const NvEncPictureCommand * picture_command);

    
	NVENCSTATUS NvEncRegisterResource(NV_ENC_INPUT_RESOURCE_TYPE resource_type, void * resource_to_register, unsigned int width, unsigned int height, unsigned int pitch, void ** registered_resource);
	NVENCSTATUS NvEncUnregisterResource(NV_ENC_REGISTERED_PTR registered_resource);
//    NVENCSTATUS NvEncReconfigureEncoder(const NvEncPictureCommand *pEncPicCommand);
    NVENCSTATUS NvEncFlushEncoderQueue(void * eos_event);

	NVENCSTATUS NvEncEncodePicture(NV_ENC_PIC_PARAMS * pic_params);

	NVENCSTATUS NvEncEncodeFrame(nvenc_encoder::cu_enc_buffer_t * buffer, uint32_t width, uint32_t height);

private:
	dk_nvenc_encoder::ENCODER_STATE _state;
	dk_nvenc_encoder::configuration_t * _config;
	dk_nvenc_encoder * _front;

	void * _encoder;
	unsigned int _enc_buffer_count;
	void * _cuda_context;
	NV_ENCODE_API_FUNCTION_LIST * _nvenc_api;

#if defined(WITH_DYNAMIC_CUDA_LOAD)
	dk_cuda_driver_api * _cuda_driver_api;
#endif

#if defined(WIN32)
	HINSTANCE _nvenc_instance;
#else
	void * _nvenc_instance;
#endif

	unsigned long long _encode_index;
	
#if defined(WITH_CUDA_BUFFER_MAPPED_TO_NVENC)	
	CUdeviceptr _ptr_cu_chroma[2];
#if defined(WITH_CUDA_PTX)
	CUmodule _cu_module;
	CUfunction _fn_interleave_uv;
#endif
#else
	int _input_format;
#endif

	nvenc_encoder::cu_enc_buffer_t _enc_buffer[MAX_ENCODE_QUEUE];
	nvenc_encoder::cu_enc_queue<nvenc_encoder::cu_enc_buffer_t> _enc_buffer_queue;
	nvenc_encoder::enc_output_buffer_t _enc_eos_output_buffer;

};

typedef NVENCSTATUS (NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*); 


#endif