#include "intel_media_sdk\mfx_config.h"
#include "intel_media_sdk\ms_defs.h"

#if defined(_WIN32) || defined(_WIN64)
#include <tchar.h>
#include <windows.h>
#endif

#include <ctime>
#include <algorithm>
#include "media_sdk_decoder_core.h"
#include "intel_media_sdk\sysmem_allocator.h"

#if defined(_WIN32) || defined(_WIN64)
#include "intel_media_sdk\d3d_allocator.h"
#include "intel_media_sdk\d3d11_allocator.h"
#include "intel_media_sdk\d3d_device.h"
#include "intel_media_sdk\d3d11_device.h"
#endif

#if defined LIBVA_SUPPORT
#include "intel_media_sdk\vaapi_allocator.h"
#include "intel_media_sdk\vaapi_device.h"
#endif

#pragma warning(disable : 4100)

#define __SYNC_WA // avoid sync issue on Media SDK side

media_sdk_decoder_core::media_sdk_decoder_core(void)
{

	MSDK_ZERO_MEMORY(_mfx_bitstream);

	_mfx_decoder = NULL;
	MSDK_ZERO_MEMORY(_mfx_video_params);

	_mfx_allocator = NULL;
	_mfx_allocator_params = NULL;
	_mem_type = MEMORY_TYPE_SYSTEM;
	_use_external_alloc = false;
	MSDK_ZERO_MEMORY(_mfx_frame_alloc_response);

	_current_free_surface = NULL;
	_current_free_output_surface = NULL;
	_current_output_surface = NULL;

//	_deliver_output_semaphore = NULL;
//	_delivered_event = NULL;
	_error = MFX_ERR_NONE;
	_stop_deliver_loop = false;

	//_enable_mvc = false;
	_use_ext_buffers = false;
	_use_video_wall = false;
	//_complete_frame = false;
	_print_latency = false;

	_timeout = 0;
	_max_fps = 0;

	//_vec_latency.reserve(1000); // reserve some space to reduce dynamic reallocation impact on pipeline execution

	_device = NULL;
}

media_sdk_decoder_core::~media_sdk_decoder_core(void)
{

}

dk_media_sdk_decoder::ERR_CODE media_sdk_decoder_core::initialize(unsigned int width, unsigned int height)
{
	_config.bUseHWLib = true;
	_config.videoType = MFX_CODEC_AVC;
	_config.memType = MEMORY_TYPE_D3D9;
	_config.bLowLat = true;
	_config.nMaxFPS = 60;
	_config.width = width;
	_config.height = height;
	_config.fourcc = MFX_FOURCC_YV12;
	_config.nAsyncDepth = 4; //default

	_binit = false;


	mfxStatus sts = MFX_ERR_NONE;
	_mem_type = _config.memType;
	_max_fps = _config.nMaxFPS;
	_nframes = _config.nFrames ? _config.nFrames : MFX_INFINITE;


	mfxInitParam mfx_init_param;
	mfxExtThreadsParam mfx_threads_param;
	mfxExtBuffer * mfx_ext_buffers[1];
	mfxVersion mfx_version;     // real API version with which library is initialized

	MSDK_ZERO_MEMORY(mfx_init_param);
	MSDK_ZERO_MEMORY(mfx_threads_param);

	// we set version to 1.0 and later we will query actual version of the library which will got leaded
	mfx_init_param.Version.Major = 1;
	mfx_init_param.Version.Minor = 0;

	//initPar.GPUCopy = config.gpuCopy;

	init_ext_buffer(mfx_threads_param);

	bool need_init_mfx_ext_param = false;

	if (_config.nThreadsNum)
	{
		mfx_threads_param.NumThread = _config.nThreadsNum;
		need_init_mfx_ext_param = true;
	}
	if (_config.SchedulingType)
	{
		mfx_threads_param.SchedulingType = _config.SchedulingType;
		need_init_mfx_ext_param = true;
	}
	if (_config.Priority)
	{
		mfx_threads_param.Priority = _config.Priority;
		need_init_mfx_ext_param = true;
	}
	if (need_init_mfx_ext_param) 
	{
		mfx_ext_buffers[0] = (mfxExtBuffer*)&mfx_threads_param;
		mfx_init_param.ExtParam = mfx_ext_buffers;
		mfx_init_param.NumExtParam = 1;
	}

	// Init session
	if (_config.bUseHWLib)
	{
		// try searching on all display adapters
		mfx_init_param.Implementation = MFX_IMPL_HARDWARE_ANY;
		// if d3d11 surfaces are used ask the library to run acceleration through D3D11
		// feature may be unsupported due to OS or MSDK API version
		if (MEMORY_TYPE_D3D11 == _config.memType)
			mfx_init_param.Implementation |= MFX_IMPL_VIA_D3D11;

		sts = _mfx_session.InitEx(mfx_init_param);
		// MSDK API version may not support multiple adapters - then try initialize on the default
		if (MFX_ERR_NONE != sts)
		{
			mfx_init_param.Implementation = (mfx_init_param.Implementation & !MFX_IMPL_HARDWARE_ANY) | MFX_IMPL_HARDWARE;
			sts = _mfx_session.InitEx(mfx_init_param);
		}
	}
	else
	{
		mfx_init_param.Implementation = MFX_IMPL_SOFTWARE;
		sts = _mfx_session.InitEx(mfx_init_param);
	}

	if (sts != MFX_ERR_NONE)
		return dk_media_sdk_decoder::ERR_CODE_FAILED;

	sts = MFXQueryVersion(_mfx_session, &mfx_version); // get real API version of the loaded library
	if (sts != MFX_ERR_NONE)
		return dk_media_sdk_decoder::ERR_CODE_FAILED;

	/*if (config.bIsMVC && !CheckVersion(&mfx_version, MSDK_FEATURE_MVC)) 
	{
		//msdk_printf(MSDK_STRING("error: MVC is not supported in the %d.%d API version\n"), version.Major, version.Minor);
		return dk_media_sdk_decoder::ERR_CODE_FAILED;
	}*/
	
	if (_config.bLowLat && !CheckVersion(&mfx_version, MSDK_FEATURE_LOW_LATENCY))
	{
		//msdk_printf(MSDK_STRING("error: Low Latency mode is not supported in the %d.%d API version\n"), version.Major, version.Minor);
		return dk_media_sdk_decoder::ERR_CODE_FAILED;
	}

	// create decoder
	_mfx_decoder = new MFXVideoDECODE(_mfx_session);
	// set video type in parameters
	_mfx_video_params.mfx.CodecId = _config.videoType;
	// prepare bit stream
	sts = InitMfxBitstream(&_mfx_bitstream, 1024 * 1024);
	if (sts != MFX_ERR_NONE)
		return dk_media_sdk_decoder::ERR_CODE_FAILED;

	if (CheckVersion(&mfx_version, MSDK_FEATURE_PLUGIN_API))
	{
		/* Here we actually define the following codec initialization scheme:
		*  1. If plugin path or guid is specified: we load user-defined plugin (example: VP8 sample decoder plugin)
		*  2. If plugin path not specified:
		*    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
		*    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
		*/
		// Load user plug-in, should go after CreateAllocator function (when all callbacks were initialized)
		if (_config.pluginParams.type == MFX_PLUGINLOAD_TYPE_FILE && strlen(_config.pluginParams.strPluginPath))
		{
			_user_module.reset(new MFXVideoUSER(_mfx_session));
			if (_config.videoType == CODEC_VP8 || _config.videoType == MFX_CODEC_HEVC)
			{
				_plugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, _mfx_session, _config.pluginParams.pluginGuid, 1, _config.pluginParams.strPluginPath, (mfxU32)strlen(_config.pluginParams.strPluginPath)));
			}
			if (_plugin.get() == NULL) 
				sts = MFX_ERR_UNSUPPORTED;
		}
		else
		{
			if (AreGuidsEqual(_config.pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
			{
				mfxIMPL impl = _config.bUseHWLib ? MFX_IMPL_HARDWARE : MFX_IMPL_SOFTWARE;
				_config.pluginParams.pluginGuid = msdkGetPluginUID(impl, MSDK_VDECODE, _config.videoType);
			}
			if (!AreGuidsEqual(_config.pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
			{
				_plugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, _mfx_session, _config.pluginParams.pluginGuid, 1));
				if (_plugin.get() == NULL) 
					sts = MFX_ERR_UNSUPPORTED;
			}
		}
		if (sts != MFX_ERR_NONE)
			return dk_media_sdk_decoder::ERR_CODE_FAILED;
	}


	/*
	// Populate parameters. Involves DecodeHeader call
	sts = init_mfx_params(&_config);
	if (sts != MFX_ERR_NONE)
		return dk_media_sdk_decoder::ERR_CODE_FAILED;

	// create device and allocator
	sts = create_allocator();
	if (sts != MFX_ERR_NONE)
		return dk_media_sdk_decoder::ERR_CODE_FAILED;

	// in case of HW accelerated decode frames must be allocated prior to decoder initialization
	sts = alloc_frames();
	if (sts != MFX_ERR_NONE)
		return dk_media_sdk_decoder::ERR_CODE_FAILED;

	sts = _mfx_decoder->Init(&_mfx_video_params);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	if (sts != MFX_ERR_NONE)
		return dk_media_sdk_decoder::ERR_CODE_FAILED;
	*/

	return dk_media_sdk_decoder::ERR_CODE_SUCCESS;
}

dk_media_sdk_decoder::ERR_CODE media_sdk_decoder_core::release(void)
{
	WipeMfxBitstream(&_mfx_bitstream);
	MSDK_SAFE_DELETE(_mfx_decoder);

	delete_frames();

	if (_use_ext_buffers)
	{
		deallocate_ext_mvc_buffers();
		delete_ext_buffers();
	}

	_plugin.reset();
	_mfx_session.Close();

	// allocator if used as external for MediaSDK must be deleted after decoder
	delete_allocator();

	return dk_media_sdk_decoder::ERR_CODE_SUCCESS;
}

dk_media_sdk_decoder::ERR_CODE media_sdk_decoder_core::decode(unsigned char * input, size_t isize, unsigned int stride, unsigned char * output, size_t & osize)
{
	mfxStatus status = MFX_ERR_NONE;

	mfxBitstream * bitstream = &_mfx_bitstream;
	bitstream->DataOffset = 0;
	bitstream->DataLength = isize;
	bitstream->MaxLength = isize;
	memcpy(bitstream->Data, input, bitstream->DataLength);


	if (!_binit)
	{
		// Populate parameters. Involves DecodeHeader call
		status = init_mfx_params(&_config, input, isize);
		if ((status != MFX_ERR_NONE) && (status != MFX_ERR_MORE_DATA))
			return dk_media_sdk_decoder::ERR_CODE_FAILED;

		// create device and allocator
		status = create_allocator();
		if (status != MFX_ERR_NONE)
			return dk_media_sdk_decoder::ERR_CODE_FAILED;

		// in case of HW accelerated decode frames must be allocated prior to decoder initialization
		status = alloc_frames();
		if (status != MFX_ERR_NONE)
			return dk_media_sdk_decoder::ERR_CODE_FAILED;

		status = _mfx_decoder->Init(&_mfx_video_params);
		if (status==MFX_WRN_PARTIAL_ACCELERATION)
		{
			msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
			MSDK_IGNORE_MFX_STS(status, MFX_WRN_PARTIAL_ACCELERATION);
		}
		if (status != MFX_ERR_NONE)
			return dk_media_sdk_decoder::ERR_CODE_FAILED;

		_binit = true;
		return dk_media_sdk_decoder::ERR_CODE_FAILED;
	}
	


#ifndef __SYNC_WA
	status = sync_output_surface(0, output, osize);
	if (status == MFX_ERR_UNKNOWN)
		return dk_media_sdk_decoder::ERR_CODE_FAILED;
#endif
	if ((status == MFX_ERR_NONE) || (status == MFX_ERR_MORE_DATA) || (status == MFX_ERR_MORE_SURFACE))
	{
		SyncFrameSurfaces();
		if (_current_free_surface)
			_current_free_surface = m_FreeSurfacesPool.GetSurface();
#ifndef __SYNC_WA
		if(!_current_free_surface)
#else
		if (!_current_free_surface || (m_OutputSurfacesPool.GetSurfaceCount() == _mfx_video_params.AsyncDepth))
#endif
		{
			status = sync_output_surface(MSDK_DEC_WAIT_INTERVAL, output, osize);
			if (status == MFX_ERR_MORE_DATA)
				status = MFX_ERR_NOT_FOUND;

			if (status==MFX_ERR_NOT_FOUND)
				return dk_media_sdk_decoder::ERR_CODE_SUCCESS;
		}
		if (_current_free_output_surface)
			_current_free_output_surface = GetFreeOutputSurface();
		if (_current_free_output_surface)
			return dk_media_sdk_decoder::ERR_CODE_SUCCESS;
	}
	_current_free_surface->submit = m_timer_overall.Sync();

	mfxFrameSurface1 * out_surface = NULL;
	do
	{
		status = _mfx_decoder->DecodeFrameAsync(bitstream, &(_current_free_surface->frame), &out_surface, &(_current_free_output_surface->syncp));
		if (status == MFX_WRN_DEVICE_BUSY)
		{
			//in low latency mode device busy leads to increasing of latency
			//msdk_printf(MSDK_STRING("Warning : latency increased due to MFX_WRN_DEVICE_BUSY\n"));

			mfxStatus sts = sync_output_surface(MSDK_DEC_WAIT_INTERVAL, output, osize);
			// note: everything except MFX_ERR_NONE are errors at this point
			if (sts == MFX_ERR_NONE)
				status = MFX_WRN_DEVICE_BUSY;
			else
			{
				status = sts;
				if (status == MFX_ERR_MORE_DATA)
				{
					// we can't receive MFX_ERR_MORE_DATA and have no output - that's a bug
					status = MFX_WRN_DEVICE_BUSY;
				}
			}
		}
	} while (status == MFX_WRN_DEVICE_BUSY);

	if (status > MFX_ERR_NONE)
	{
		if (_current_free_output_surface->syncp)
		{
			MSDK_SELF_CHECK(out_surface);
			status = MFX_ERR_NONE;
		}
		else
		{
			status = MFX_ERR_MORE_SURFACE;
		}
	}
	else if ((status == MFX_ERR_MORE_DATA) && bitstream)
	{
		if (bitstream->DataLength)
		{
			// In low_latency mode decoder have to process bitstream completely
			msdk_printf(MSDK_STRING("error: Incorrect decoder behavior in low latency mode (bitstream length is not equal to 0 after decoding)\n"));
			status = MFX_ERR_UNDEFINED_BEHAVIOR;
			return dk_media_sdk_decoder::ERR_CODE_FAILED;
		}
	}
	else if ((status == MFX_ERR_MORE_DATA) && !bitstream)
	{
		do 
		{
			status = sync_output_surface(MSDK_DEC_WAIT_INTERVAL, output, osize);
		} 
		while (status == MFX_ERR_NONE);

		if (status==MFX_ERR_MORE_DATA) 
			status = MFX_ERR_NONE;
	}
	else if (status == MFX_ERR_INCOMPATIBLE_VIDEO_PARAM)
	{
		bitstream = 0;
		status = MFX_ERR_NONE;
		return dk_media_sdk_decoder::ERR_CODE_FAILED;
	}

	if ((status == MFX_ERR_NONE) || (status == MFX_ERR_MORE_DATA) || (status == MFX_ERR_MORE_SURFACE))
	{
		// if current free surface is locked we are moving it to the used surfaces array
		/*if (m_pCurrentFreeSurface->frame.Data.Locked)*/ 
		{
			m_UsedSurfacesPool.AddSurface(_current_free_surface);
			_current_free_surface = 0;
		}
	}

	if (status == MFX_ERR_NONE)
	{
		msdkFrameSurface * surface = FindUsedSurface(out_surface);
		msdk_atomic_inc16(&(surface->render_lock));

		_current_free_output_surface->surface = surface;
		m_OutputSurfacesPool.AddSurface(_current_free_output_surface);
		_current_free_output_surface = 0;
	}

	if (status == MFX_ERR_NONE)
		return dk_media_sdk_decoder::ERR_CODE_SUCCESS;
	else
		return dk_media_sdk_decoder::ERR_CODE_FAILED;
}

mfxStatus media_sdk_decoder_core::init_mfx_params(CONFIG_T * config, unsigned char * bitstream, size_t nbytes)
{
	mfxStatus sts = MFX_ERR_NONE;
	mfxU32 & num_views = config->numViews;

	// try to find a sequence header in the stream
	// if header is not found this function exits with error (e.g. if device was lost and there's no header in the remaining stream)
	//while (true)
	{
		// parse bit stream and fill mfx params
		sts = _mfx_decoder->DecodeHeader(&_mfx_bitstream, &_mfx_video_params);
		if (!sts && _plugin.get() && (config->videoType == CODEC_VP8)) 
		{
			// force set format to nv12 as the vp8 plugin uses yv12
			_mfx_video_params.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
		}

		if (sts == MFX_ERR_MORE_DATA)
		{
			if (_mfx_bitstream.MaxLength == _mfx_bitstream.DataLength)
			{
				sts = ExtendMfxBitstream(&_mfx_bitstream, _mfx_bitstream.MaxLength * 2);
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			}
			// read a portion of data
			/*sts = m_FileReader->ReadNextFrame(&_mfx_bitstream);
			if ((sts == MFX_ERR_MORE_DATA) && !(_mfx_bitstream.DataFlag & MFX_BITSTREAM_EOS))
			{
				_mfx_bitstream.DataFlag |= MFX_BITSTREAM_EOS;
				sts = MFX_ERR_NONE;
			}
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

			continue;*/
		}
		else
		{
			// Enter MVC mode
			/*if (_enable_mvc)
			{
				// Check for attached external parameters - if we have them already,
				// we don't need to attach them again
				if (_mfx_video_params.ExtParam != NULL)
					break;

				// allocate and attach external parameters for MVC decoder
				sts = allocate_ext_buffer<mfxExtMVCSeqDesc>();
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

				attach_ext_parameter();
				sts = _mfx_decoder->DecodeHeader(&_mfx_bitstream, &_mfx_video_params);

				if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
				{
					sts = allocate_ext_mvc_buffers();
					_use_ext_buffers = true;

					MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
					MSDK_CHECK_POINTER(_mfx_video_params.ExtParam, MFX_ERR_MEMORY_ALLOC);
					continue;
				}
			}*/

			// if input is interlaced JPEG stream
			if (_mfx_bitstream.PicStruct == MFX_PICSTRUCT_FIELD_TFF || _mfx_bitstream.PicStruct == MFX_PICSTRUCT_FIELD_BFF)
			{
				_mfx_video_params.mfx.FrameInfo.CropH *= 2;
				_mfx_video_params.mfx.FrameInfo.Height = MSDK_ALIGN16(_mfx_video_params.mfx.FrameInfo.CropH);
				_mfx_video_params.mfx.FrameInfo.PicStruct = _mfx_bitstream.PicStruct;
			}

			switch (config->nRotation)
			{
			case 0:
				_mfx_video_params.mfx.Rotation = MFX_ROTATION_0;
				break;
			case 90:
				_mfx_video_params.mfx.Rotation = MFX_ROTATION_90;
				break;
			case 180:
				_mfx_video_params.mfx.Rotation = MFX_ROTATION_180;
				break;
			case 270:
				_mfx_video_params.mfx.Rotation = MFX_ROTATION_270;
				break;
			default:
				return MFX_ERR_UNSUPPORTED;
			}

			//break;
		}
	}

	// check DecodeHeader status
	if (sts == MFX_WRN_PARTIAL_ACCELERATION)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// If MVC mode we need to detect number of views in stream
	/*if (_enable_mvc)
	{
		mfxExtMVCSeqDesc* pSequenceBuffer;
		pSequenceBuffer = (mfxExtMVCSeqDesc*)GetExtBuffer(_mfx_video_params.ExtParam, _mfx_video_params.NumExtParam, MFX_EXTBUFF_MVC_SEQ_DESC);
		MSDK_CHECK_POINTER(pSequenceBuffer, MFX_ERR_INVALID_VIDEO_PARAM);

		mfxU32 i = 0;
		num_views = 0;
		for (i = 0; i < pSequenceBuffer->NumView; ++i)
		{
			//Some MVC streams can contain different information about
			//number of views and view IDs, e.x. numVews = 2
			//and ViewId[0, 1] = 0, 2 instead of ViewId[0, 1] = 0, 1.
			//numViews should be equal (max(ViewId[i]) + 1)
			//to prevent crashes during output files writing
			if (pSequenceBuffer->View[i].ViewId >= num_views)
				num_views = pSequenceBuffer->View[i].ViewId + 1;
		}
	}
	else*/
	{
		num_views = 1;
	}

	// specify memory type
	_mfx_video_params.IOPattern = (mfxU16)(_mem_type != MEMORY_TYPE_SYSTEM ? MFX_IOPATTERN_OUT_VIDEO_MEMORY : MFX_IOPATTERN_OUT_SYSTEM_MEMORY);
	_mfx_video_params.AsyncDepth = config->nAsyncDepth;

	return MFX_ERR_NONE;
}

template<typename Buffer> mfxStatus media_sdk_decoder_core::allocate_ext_buffer(void)
{
	std::auto_ptr<Buffer> ext_buffer(new Buffer());
	if (!ext_buffer.get())
		return MFX_ERR_MEMORY_ALLOC;

	init_ext_buffer(*ext_buffer);
	_ext_buffers.push_back(reinterpret_cast<mfxExtBuffer*>(ext_buffer.release()));

	return MFX_ERR_NONE;
}

void media_sdk_decoder_core::delete_ext_buffers(void)
{
	for (std::vector<mfxExtBuffer *>::iterator it = _ext_buffers.begin(); it != _ext_buffers.end(); ++it)
		delete *it;
	_ext_buffers.clear();
}

mfxStatus media_sdk_decoder_core::allocate_ext_mvc_buffers(void)
{
	mfxU32 i;
	mfxExtMVCSeqDesc * ext_mvc_buffer = (mfxExtMVCSeqDesc*)_mfx_video_params.ExtParam[0];
	MSDK_CHECK_POINTER(ext_mvc_buffer, MFX_ERR_MEMORY_ALLOC);

	ext_mvc_buffer->View = new mfxMVCViewDependency[ext_mvc_buffer->NumView];
	MSDK_CHECK_POINTER(ext_mvc_buffer->View, MFX_ERR_MEMORY_ALLOC);
	for (i = 0; i <ext_mvc_buffer->NumView; ++i)
	{
		MSDK_ZERO_MEMORY(ext_mvc_buffer->View[i]);
	}
	ext_mvc_buffer->NumViewAlloc = ext_mvc_buffer->NumView;

	ext_mvc_buffer->ViewId = new mfxU16[ext_mvc_buffer->NumViewId];
	MSDK_CHECK_POINTER(ext_mvc_buffer->ViewId, MFX_ERR_MEMORY_ALLOC);
	for (i = 0; i <ext_mvc_buffer->NumViewId; ++i)
	{
		MSDK_ZERO_MEMORY(ext_mvc_buffer->ViewId[i]);
	}
	ext_mvc_buffer->NumViewIdAlloc = ext_mvc_buffer->NumViewId;

	ext_mvc_buffer->OP = new mfxMVCOperationPoint[ext_mvc_buffer->NumOP];
	MSDK_CHECK_POINTER(ext_mvc_buffer->OP, MFX_ERR_MEMORY_ALLOC);
	for (i = 0; i <ext_mvc_buffer->NumOP; ++i)
	{
		MSDK_ZERO_MEMORY(ext_mvc_buffer->OP[i]);
	}
	ext_mvc_buffer->NumOPAlloc = ext_mvc_buffer->NumOP;
	return MFX_ERR_NONE;
}

void media_sdk_decoder_core::deallocate_ext_mvc_buffers(void)
{
	mfxExtMVCSeqDesc * ext_mvc_buffer = (mfxExtMVCSeqDesc*)_mfx_video_params.ExtParam[0];
	if (ext_mvc_buffer != NULL)
	{
		MSDK_SAFE_DELETE_ARRAY(ext_mvc_buffer->View);
		MSDK_SAFE_DELETE_ARRAY(ext_mvc_buffer->ViewId);
		MSDK_SAFE_DELETE_ARRAY(ext_mvc_buffer->OP);
	}

	MSDK_SAFE_DELETE(_mfx_video_params.ExtParam[0]);
	_use_ext_buffers = false;
}

mfxStatus media_sdk_decoder_core::create_allocator(void)
{
	mfxStatus sts = MFX_ERR_NONE;

	if (_mem_type != MEMORY_TYPE_SYSTEM)
	{
#if D3D_SURFACES_SUPPORT
		sts = create_hw_device();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		// provide device manager to MediaSDK
		mfxHDL hdl = NULL;
		mfxHandleType hdl_t =
#if MFX_D3D11_SUPPORT
			MEMORY_TYPE_D3D11 == _mem_type ? MFX_HANDLE_D3D11_DEVICE :
#endif // #if MFX_D3D11_SUPPORT
			MFX_HANDLE_D3D9_DEVICE_MANAGER;

		sts = _device->GetHandle(hdl_t, &hdl);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		sts = _mfx_session.SetHandle(hdl_t, hdl);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		// create D3D allocator
#if MFX_D3D11_SUPPORT
		if (_mem_type==MEMORY_TYPE_D3D11)
		{
			_mfx_allocator = new D3D11FrameAllocator;
			MSDK_CHECK_POINTER(_mfx_allocator, MFX_ERR_MEMORY_ALLOC);
			D3D11AllocatorParams * d3d11_alloc_params = new D3D11AllocatorParams;
			MSDK_CHECK_POINTER(d3d11_alloc_params, MFX_ERR_MEMORY_ALLOC);
			d3d11_alloc_params->pDevice = reinterpret_cast<ID3D11Device *>(hdl);
			_mfx_allocator_params = d3d11_alloc_params;
		}
		else
#endif // #if MFX_D3D11_SUPPORT
		{
			_mfx_allocator = new D3DFrameAllocator;
			MSDK_CHECK_POINTER(_mfx_allocator, MFX_ERR_MEMORY_ALLOC);
			D3DAllocatorParams * d3d_alloc_params = new D3DAllocatorParams;
			MSDK_CHECK_POINTER(d3d_alloc_params, MFX_ERR_MEMORY_ALLOC);
			d3d_alloc_params->pManager = reinterpret_cast<IDirect3DDeviceManager9 *>(hdl);
			_mfx_allocator_params = d3d_alloc_params;
		}
		/* In case of video memory we must provide MediaSDK with external allocator
		thus we demonstrate "external allocator" usage model.
		Call SetAllocator to pass allocator to mediasdk */
		sts = _mfx_session.SetFrameAllocator(_mfx_allocator);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		_use_external_alloc = true;
#elif LIBVA_SUPPORT
		sts = create_hw_device();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		/* It's possible to skip failed result here and switch to SW implementation,
		but we don't process this way */
		// provide device manager to MediaSDK
		VADisplay va_dpy = NULL;
		sts = _device->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL *)&va_dpy);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		sts = _mfx_session.SetHandle(MFX_HANDLE_VA_DISPLAY, va_dpy);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		// create VAAPI allocator
		_mfx_allocator = new vaapiFrameAllocator;
		MSDK_CHECK_POINTER(_mfx_allocator, MFX_ERR_MEMORY_ALLOC);
		vaapiAllocatorParams * vaapi_alloc_params = new vaapiAllocatorParams;
		MSDK_CHECK_POINTER(vaapi_alloc_params, MFX_ERR_MEMORY_ALLOC);
		vaapi_alloc_params->m_dpy = va_dpy;
		_mfx_allocator_params = vaapi_alloc_params;
		/* In case of video memory we must provide MediaSDK with external allocator
		thus we demonstrate "external allocator" usage model.
		Call SetAllocator to pass allocator to mediasdk */
		sts = _mfx_session.SetFrameAllocator(_mfx_allocator);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		_use_external_alloc = true;
#endif
	}
	else
	{
#ifdef LIBVA_SUPPORT
		//in case of system memory allocator we also have to pass MFX_HANDLE_VA_DISPLAY to HW library
		mfxIMPL impl;
		_mfx_session.QueryIMPL(&impl);
		if (MFX_IMPL_HARDWARE == MFX_IMPL_BASETYPE(impl))
		{
			sts = create_hw_device();
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			// provide device manager to MediaSDK
			VADisplay va_dpy = NULL;
			sts = _device->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL *)&va_dpy);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			sts = _mfx_session.SetHandle(MFX_HANDLE_VA_DISPLAY, va_dpy);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
#endif
		// create system memory allocator
		_mfx_allocator = new SysMemFrameAllocator;
		MSDK_CHECK_POINTER(_mfx_allocator, MFX_ERR_MEMORY_ALLOC);
		/* In case of system memory we demonstrate "no external allocator" usage model.
		We don't call SetAllocator, MediaSDK uses internal allocator.
		We use system memory allocator simply as a memory manager for application*/
	}

	// initialize memory allocator
	sts = _mfx_allocator->Init(_mfx_allocator_params);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	return MFX_ERR_NONE;
}

void media_sdk_decoder_core::delete_allocator(void)
{
	// delete allocator
	MSDK_SAFE_DELETE(_mfx_allocator);
	MSDK_SAFE_DELETE(_mfx_allocator_params);
	MSDK_SAFE_DELETE(_device);
}

mfxStatus media_sdk_decoder_core::alloc_frames(void)
{
	MSDK_CHECK_POINTER(_mfx_decoder, MFX_ERR_NULL_PTR);
	mfxStatus sts = MFX_ERR_NONE;
	mfxFrameAllocRequest Request;
	mfxU16 nSurfNum = 0; // number of surfaces for decoder
	MSDK_ZERO_MEMORY(Request);
	sts = _mfx_decoder->Query(&_mfx_video_params, &_mfx_video_params);
	MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	// calculate number of surfaces required for decoder
	sts = _mfx_decoder->QueryIOSurf(&_mfx_video_params, &Request);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	
	mfxIMPL impl = 0;
	sts = _mfx_session.QueryIMPL(&impl);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	if ((Request.NumFrameSuggested < _mfx_video_params.AsyncDepth) &&
		(impl & MFX_IMPL_HARDWARE_ANY))
		return MFX_ERR_MEMORY_ALLOC;

	nSurfNum = MSDK_MAX(Request.NumFrameSuggested, 1);
	// prepare allocation request
	Request.NumFrameSuggested = Request.NumFrameMin = nSurfNum;
	// alloc frames for decoder
	sts = _mfx_allocator->Alloc(_mfx_allocator->pthis, &Request, &_mfx_frame_alloc_response);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// prepare mfxFrameSurface1 array for decoder
	nSurfNum = _mfx_frame_alloc_response.NumFrameActual;

	sts = AllocBuffers(nSurfNum);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	for (int i = 0; i < nSurfNum; i++)
	{
		// initating each frame:
		MSDK_MEMCPY_VAR(m_pSurfaces[i].frame.Info, &(Request.Info), sizeof(mfxFrameInfo));
		if (_use_external_alloc)
		{
			m_pSurfaces[i].frame.Data.MemId = _mfx_frame_alloc_response.mids[i];
		}
		else
		{
			sts = _mfx_allocator->Lock(_mfx_allocator->pthis, _mfx_frame_alloc_response.mids[i], &(m_pSurfaces[i].frame.Data));
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
	}

	return MFX_ERR_NONE;
}

void media_sdk_decoder_core::delete_frames(void)
{
	FreeBuffers();

	_current_free_surface = NULL;
	MSDK_SAFE_FREE(_current_free_surface);

	// delete frames
	if (_mfx_allocator)
	{
		_mfx_allocator->Free(_mfx_allocator->pthis, &_mfx_frame_alloc_response);
	}

	return;
}

void media_sdk_decoder_core::attach_ext_parameter(void)
{
	_mfx_video_params.ExtParam = reinterpret_cast<mfxExtBuffer**>(&_ext_buffers[0]);
	_mfx_video_params.NumExtParam = static_cast<mfxU16>(_ext_buffers.size());
}

mfxStatus media_sdk_decoder_core::create_hw_device(void)
{
#if D3D_SURFACES_SUPPORT
	mfxStatus sts = MFX_ERR_NONE;

	HWND window = NULL;

#if MFX_D3D11_SUPPORT
	if (_mem_type==MEMORY_TYPE_D3D11)
		_device = new CD3D11Device();
	else
#endif // #if MFX_D3D11_SUPPORT
		_device = new CD3D9Device();

	if (_device==NULL)
		return MFX_ERR_MEMORY_ALLOC;

	sts = _device->Init( window, 0, MSDKAdapter::GetNumber(_mfx_session)); 
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

#elif LIBVA_SUPPORT
	mfxStatus sts = MFX_ERR_NONE;
	_device = CreateVAAPIDevice();
	if (NULL == m_hwdev) 
		return MFX_ERR_MEMORY_ALLOC;
	sts = _device->Init(NULL, 0, MSDKAdapter::GetNumber(_mfx_session));
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#endif
	return MFX_ERR_NONE;
}

mfxStatus media_sdk_decoder_core::reset_decoder(CONFIG_T * config, unsigned char * bitstream, size_t nbytes)
{
	mfxStatus sts = MFX_ERR_NONE;

	// close decoder
	sts = _mfx_decoder->Close();
	MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// free allocated frames
	delete_frames();

	// initialize parameters with values from parsed header
	sts = init_mfx_params(config, bitstream, nbytes);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// in case of HW accelerated decode frames must be allocated prior to decoder initialization
	sts = alloc_frames();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// init decoder
	sts = _mfx_decoder->Init(&_mfx_video_params);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	return MFX_ERR_NONE;
}

mfxStatus media_sdk_decoder_core::reset_device(void)
{
	return _device->Reset();
}

mfxStatus media_sdk_decoder_core::sync_output_surface(mfxU32 wait, unsigned char * output, unsigned int & osize)
{
	if (!_current_output_surface)
		_current_output_surface = m_OutputSurfacesPool.GetSurface();
	if (!_current_output_surface)
		return MFX_ERR_MORE_DATA;
	
	mfxStatus sts = _mfx_session.SyncOperation(_current_output_surface->syncp, wait);
	
	if (sts==MFX_WRN_IN_EXECUTION)
		return sts;
	
	if (sts==MFX_ERR_NONE)
	{
		// we got completely decoded frame - pushing it to the delivering thread...
		sts = deliver_output(&(_current_output_surface->surface->frame), output, osize);
		if (MFX_ERR_NONE != sts)
			sts = MFX_ERR_UNKNOWN;
		ReturnSurfaceToBuffers(_current_output_surface);
		_current_output_surface = NULL;
	}

	if (sts != MFX_ERR_NONE)
		sts = MFX_ERR_UNKNOWN;
	return sts;
}

mfxStatus media_sdk_decoder_core::deliver_output(mfxFrameSurface1 * frame, unsigned char * output, unsigned int & osize)
{
	mfxStatus status = MFX_ERR_NONE, sts = MFX_ERR_NONE;

	if (!frame) 
		return MFX_ERR_NULL_PTR;

	if (_use_external_alloc)
	{
		status = _mfx_allocator->Lock(_mfx_allocator->pthis, frame->Data.MemId, &(frame->Data));
		if (status==MFX_ERR_NONE)
		{
			status = fill_output_buffer(frame, output, osize);
			sts = _mfx_allocator->Unlock(_mfx_allocator->pthis, frame->Data.MemId, &(frame->Data));
		}
		if ((status==MFX_ERR_NONE) && (sts!=MFX_ERR_NONE))
		{
			status = sts;
		}
	}
	else 
	{
		status = fill_output_buffer(frame, output, osize);
	}

	return status;
}

mfxStatus media_sdk_decoder_core::fill_output_buffer(mfxFrameSurface1 * surface, unsigned char * output, unsigned int & osize)
{
	mfxStatus status = MFX_ERR_NONE;
	MSDK_CHECK_POINTER(surface, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(output, MFX_ERR_NULL_PTR);

	mfxFrameInfo & frame_info = surface->Info;
	mfxFrameData & frame_data = surface->Data;

	mfxU32 i, j, h, w;
	mfxU32 vid = frame_info.FrameId.ViewId;

	switch (frame_info.FourCC)
	{
	case MFX_FOURCC_YV12:
	case MFX_FOURCC_NV12:
		for (i = 0; i < frame_info.CropH; i++)
		{
			memcpy(output, frame_data.Y + (frame_info.CropY * frame_data.Pitch + frame_info.CropX) + i * frame_data.Pitch, frame_info.CropW);
			output += frame_info.CropW;
		}
		break;
	}
	switch (frame_info.FourCC)
	{
	case MFX_FOURCC_YV12:
		for (i = 0; i < (mfxU32)frame_info.CropH / 2; i++)
		{
			memcpy(output, frame_data.U + (frame_info.CropY * frame_data.Pitch / 2 + frame_info.CropX / 2) + i * frame_data.Pitch / 2, frame_info.CropW / 2);
			output += frame_info.CropW / 2;
		}
		for (i = 0; i < (mfxU32)frame_info.CropH / 2; i++)
		{
			memcpy(output, frame_data.V + (frame_info.CropY * frame_data.Pitch / 2 + frame_info.CropX / 2) + i * frame_data.Pitch / 2, frame_info.CropW / 2);
			output += frame_info.CropW / 2;
		}
		break;

	case MFX_FOURCC_NV12:
		h = frame_info.CropH / 2;
		w = frame_info.CropW;
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j += 2)
			{
				memcpy(output, frame_data.UV + (frame_info.CropY * frame_data.Pitch / 2 + frame_info.CropX) + i * frame_data.Pitch + j, 1);
				output++;
			}
		}
		for (i = 0; i < h; i++)
		{
			for (j = 1; j < w; j += 2)
			{
				memcpy(output, frame_data.UV + (frame_info.CropY * frame_data.Pitch / 2 + frame_info.CropX) + i * frame_data.Pitch + j, 1);
				output++;
			}
		}
		break;
	}
	return MFX_ERR_NONE;
}