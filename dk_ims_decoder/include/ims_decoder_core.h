#ifndef _INTEL_MEDIA_SDK_DECODER_CORE_H_
#define _INTEL_MEDIA_SDK_DECODER_CORE_H_

#include "dk_ims_decoder.h"

#include <vector>
#include "intel_media_sdk\hw_device.h"
#include "intel_media_sdk\mfx_buffering.h"
#include <memory>

#include "intel_media_sdk\ms_utils.h"
#include "intel_media_sdk\ms_params.h"
#include "intel_media_sdk\base_allocator.h"

#include <mfxmvc.h>
#include <mfxjpeg.h>
#include <mfxplugin.h>
#include <mfxplugin++.h>
#include <mfxvideo.h>
#include <mfxvideo++.h>

#include "intel_media_sdk\plugin_loader.h"

class ims_decoder_core
{
public:
	typedef enum _MEMORY_TYPE
	{
		MEMORY_TYPE_SYSTEM = 0x00,
		MEMORY_TYPE_D3D9 = 0x01,
		MEMORY_TYPE_D3D11 = 0x02
	} MEMORY_TYPE;

	typedef struct _CONFIG_T
	{
		mfxU32 videoType;
		MEMORY_TYPE memType;
		bool    bUseHWLib; // true if application wants to use HW mfx library
		bool    bIsMVC; // true if Multi-View Codec is in use
		bool    bLowLat; // low latency mode
		bool    bCalLat; // latency calculation
		mfxU32  nMaxFPS; //rendering limited by certain fps
		mfxU32  nWallCell;
		mfxU32  nWallW; //number of windows located in each row
		mfxU32  nWallH; //number of windows located in each column
		mfxU32  nWallMonitor; //monitor id, 0,1,.. etc
		bool    bWallNoTitle; //whether to show title for each window with fps value
		mfxU32  nWallTimeout; //timeout for -wall option
		mfxU32  numViews; // number of views for Multi-View Codec
		mfxU32  nRotation; // rotation for Motion JPEG Codec
		mfxU16  nAsyncDepth; // asyncronous queue
		mfxU16  gpuCopy; // GPU Copy mode (three-state option)
		mfxU16  nThreadsNum;
		mfxI32  SchedulingType;
		mfxI32  Priority;

		mfxU16  width;
		mfxU16  height;
		mfxU32  fourcc;
		mfxU32  nFrames;

		msdk_char     strSrcFile[MSDK_MAX_FILENAME_LEN];
		msdk_char     strDstFile[MSDK_MAX_FILENAME_LEN];
		sPluginParams pluginParams;

		_CONFIG_T()
		{
			MSDK_ZERO_MEMORY(*this);
		}
	} CONFIG_T;


	ims_decoder_core(void);
	~ims_decoder_core(void);

	dk_ims_decoder::ERR_CODE initialize(unsigned int width, unsigned int height);
	dk_ims_decoder::ERR_CODE release(void);
	dk_ims_decoder::ERR_CODE decode(unsigned char * input, unsigned int isize, unsigned int stride, unsigned char * output, unsigned int & osize);




private:
	virtual mfxStatus init_mfx_params(CONFIG_T * config);


	template <typename Buffer> mfxStatus allocate_ext_buffer(void);
	virtual void delete_ext_buffers(void);

	virtual mfxStatus allocate_ext_mvc_buffers(void);
	virtual void deallocate_ext_mvc_buffers(void);

	virtual mfxStatus create_allocator();
	virtual void delete_allocator();

	virtual mfxStatus alloc_frames();
	virtual void delete_frames();

	virtual void AttachExtParam();



	virtual mfxStatus CreateHWDevice();











private:
	bool _binit;
	unsigned int _width;
	unsigned int _height;
	int _device_pitch;
	int _decoded_buffer_size;

	mfxBitstream _mfx_bitstream; // contains encoded data

	MFXVideoSession _mfx_session;
	MFXVideoDECODE * _mfx_decoder;
	mfxVideoParam _mfx_video_params;

	std::auto_ptr<MFXVideoUSER> _user_module;
	std::auto_ptr<MFXPlugin> _plugin;
	std::vector<mfxExtBuffer *> _ext_buffers;

	MFXFrameAllocator * _mfx_allocator;
	mfxAllocatorParams * _mfx_allocator_params;
	MEMORY_TYPE _mem_type;      // memory type of surfaces to use
	bool                    _use_external_alloc; // use memory allocator as external for Media SDK
	mfxFrameAllocResponse   _mfx_frame_alloc_response; // memory allocation response for decoder

	msdkFrameSurface * _current_free_surface; // surface detached from free surfaces array
	msdkOutputSurface * _current_free_output_surface; // surface detached from free output surfaces array
	msdkOutputSurface * _current_output_surface; // surface detached from output surfaces array

	MSDKSemaphore * _deliver_output_semaphore; // to access to DeliverOutput method
	MSDKEvent * _delivered_event; // to signal when output surfaces will be processed
	mfxStatus _error; // error returned by DeliverOutput method
	bool _stop_deliver_loop;

	bool _enable_mvc; // enables MVC mode (need to support several files as an output)
	bool _use_ext_buffers; // indicates if external buffers were allocated
	bool _use_video_wall; // indicates special mode: decoding will be done in a loop
	bool _complete_frame;
	bool _print_latency;

	mfxU32 _timeout; // enables timeout for video playback, measured in seconds
	mfxU32 _max_fps; // limit of fps, if isn't specified equal 0.
	mfxU32 _nframes; //limit number of output frames

	std::vector<msdk_tick> _vec_latency;

	CHWDevice * _device;
};

#endif

