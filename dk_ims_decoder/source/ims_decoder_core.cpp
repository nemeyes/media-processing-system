#include "intel_media_sdk\mfx_config.h"
#include "intel_media_sdk\ms_defs.h"

#if defined(_WIN32) || defined(_WIN64)
#include <tchar.h>
#include <windows.h>
#endif

#include <ctime>
#include <algorithm>
#include "ims_decoder_core.h"
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

ims_decoder_core::ims_decoder_core(void)
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

	_deliver_output_semaphore = NULL;
	_delivered_event = NULL;
	_error = MFX_ERR_NONE;
	_stop_deliver_loop = false;

	_enable_mvc = false;
	_use_ext_buffers = false;
	_use_video_wall = false;
	_complete_frame = false;
	_print_latency = false;

	_timeout = 0;
	_max_fps = 0;

	_vec_latency.reserve(1000); // reserve some space to reduce dynamic reallocation impact on pipeline execution

	_device = NULL;
}

ims_decoder_core::~ims_decoder_core(void)
{

}

dk_ims_decoder::ERR_CODE ims_decoder_core::initialize(unsigned int width, unsigned int height)
{
	CONFIG_T config;
	config.bUseHWLib = true;
	config.videoType = MFX_CODEC_AVC;
	config.memType = MEMORY_TYPE_D3D9;
	config.bLowLat = true;
	config.bCalLat = true;
	//config.gpuCopy = MFX_GPUCOPY_OFF;
	config.nMaxFPS = 60;
	config.width = width;
	config.height = height;
	config.fourcc = MFX_FOURCC_YV12;
	config.nAsyncDepth = 4; //default


	mfxStatus sts = MFX_ERR_NONE;
	_mem_type = config.memType;
	_max_fps = config.nMaxFPS;
	_nframes = config.nFrames ? config.nFrames : MFX_INFINITE;


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

	if (config.nThreadsNum) 
	{
		mfx_threads_param.NumThread = config.nThreadsNum;
		need_init_mfx_ext_param = true;
	}
	if (config.SchedulingType) 
	{
		mfx_threads_param.SchedulingType = config.SchedulingType;
		need_init_mfx_ext_param = true;
	}
	if (config.Priority) 
	{
		mfx_threads_param.Priority = config.Priority;
		need_init_mfx_ext_param = true;
	}
	if (need_init_mfx_ext_param) 
	{
		mfx_ext_buffers[0] = (mfxExtBuffer*)&mfx_threads_param;
		mfx_init_param.ExtParam = mfx_ext_buffers;
		mfx_init_param.NumExtParam = 1;
	}

	// Init session
	if (config.bUseHWLib)
	{
		// try searching on all display adapters
		mfx_init_param.Implementation = MFX_IMPL_HARDWARE_ANY;
		// if d3d11 surfaces are used ask the library to run acceleration through D3D11
		// feature may be unsupported due to OS or MSDK API version
		if (MEMORY_TYPE_D3D11 == config.memType)
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
		return dk_ims_decoder::ERR_CODE_FAILED;

	sts = MFXQueryVersion(_mfx_session, &mfx_version); // get real API version of the loaded library
	if (sts != MFX_ERR_NONE)
		return dk_ims_decoder::ERR_CODE_FAILED;

	if (config.bIsMVC && !CheckVersion(&mfx_version, MSDK_FEATURE_MVC)) {
		//msdk_printf(MSDK_STRING("error: MVC is not supported in the %d.%d API version\n"), version.Major, version.Minor);
		return dk_ims_decoder::ERR_CODE_FAILED;

	}
	if (config.bLowLat && !CheckVersion(&mfx_version, MSDK_FEATURE_LOW_LATENCY)) {
		//msdk_printf(MSDK_STRING("error: Low Latency mode is not supported in the %d.%d API version\n"), version.Major, version.Minor);
		return dk_ims_decoder::ERR_CODE_FAILED;
	}

	// create decoder
	_mfx_decoder = new MFXVideoDECODE(_mfx_session);
	// set video type in parameters
	_mfx_video_params.mfx.CodecId = config.videoType;
	// prepare bit stream
	if (MFX_CODEC_CAPTURE != config.videoType)
	{
		sts = InitMfxBitstream(&_mfx_bitstream, 1024 * 1024);
		if (sts != MFX_ERR_NONE)
			return dk_ims_decoder::ERR_CODE_FAILED;
	}

	if (CheckVersion(&mfx_version, MSDK_FEATURE_PLUGIN_API))
	{
		/* Here we actually define the following codec initialization scheme:
		*  1. If plugin path or guid is specified: we load user-defined plugin (example: VP8 sample decoder plugin)
		*  2. If plugin path not specified:
		*    2.a) we check if codec is distributed as a mediasdk plugin and load it if yes
		*    2.b) if codec is not in the list of mediasdk plugins, we assume, that it is supported inside mediasdk library
		*/
		// Load user plug-in, should go after CreateAllocator function (when all callbacks were initialized)
		if (config.pluginParams.type == MFX_PLUGINLOAD_TYPE_FILE && strlen(config.pluginParams.strPluginPath))
		{
			_user_module.reset(new MFXVideoUSER(_mfx_session));
			if (config.videoType == CODEC_VP8 || config.videoType == MFX_CODEC_HEVC)
			{
				_plugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, _mfx_session, config.pluginParams.pluginGuid, 1, config.pluginParams.strPluginPath, (mfxU32)strlen(config.pluginParams.strPluginPath)));
			}
			if (_plugin.get() == NULL) 
				sts = MFX_ERR_UNSUPPORTED;
		}
		else
		{
			if (AreGuidsEqual(config.pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
			{
				mfxIMPL impl = config.bUseHWLib ? MFX_IMPL_HARDWARE : MFX_IMPL_SOFTWARE;
				config.pluginParams.pluginGuid = msdkGetPluginUID(impl, MSDK_VDECODE, config.videoType);
			}
			if (!AreGuidsEqual(config.pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
			{
				_plugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, _mfx_session, config.pluginParams.pluginGuid, 1));
				if (_plugin.get() == NULL) 
					sts = MFX_ERR_UNSUPPORTED;
			}
		}
		if (sts != MFX_ERR_NONE)
			return dk_ims_decoder::ERR_CODE_FAILED;
	}

	// Populate parameters. Involves DecodeHeader call
	sts = init_mfx_params(&config);
	if (sts != MFX_ERR_NONE)
		return dk_ims_decoder::ERR_CODE_FAILED;

	// create device and allocator
	sts = CreateAllocator();
	if (sts != MFX_ERR_NONE)
		return dk_ims_decoder::ERR_CODE_FAILED;

	// in case of HW accelerated decode frames must be allocated prior to decoder initialization
	sts = AllocFrames();
	if (sts != MFX_ERR_NONE)
		return dk_ims_decoder::ERR_CODE_FAILED;

	sts = _mfx_decoder->Init(&_mfx_video_params);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	if (sts != MFX_ERR_NONE)
		return dk_ims_decoder::ERR_CODE_FAILED;

	return dk_ims_decoder::ERR_CODE_SUCCESS;
}

dk_ims_decoder::ERR_CODE ims_decoder_core::release(void)
{
	WipeMfxBitstream(&_mfx_bitstream);
	MSDK_SAFE_DELETE(_mfx_decoder);

	DeleteFrames();

	if (_use_ext_buffers)
	{
		deallocate_ext_mvc_buffers();
		delete_ext_buffers();
	}

	_plugin.reset();
	_mfx_session.Close();

	// allocator if used as external for MediaSDK must be deleted after decoder
	DeleteAllocator();

	return dk_ims_decoder::ERR_CODE_SUCCESS;
}

dk_ims_decoder::ERR_CODE ims_decoder_core::decode(unsigned char * input, unsigned int isize, unsigned int stride, unsigned char * output, unsigned int & osize)
{
	return dk_ims_decoder::ERR_CODE_SUCCESS;
}

mfxStatus ims_decoder_core::init_mfx_params(CONFIG_T * config)
{
	mfxStatus sts = MFX_ERR_NONE;
	mfxU32 & num_views = config->numViews;

	// try to find a sequence header in the stream
	// if header is not found this function exits with error (e.g. if device was lost and there's no header in the remaining stream)
	while (true)
	{
		// parse bit stream and fill mfx params
		sts = _mfx_decoder->DecodeHeader(&_mfx_bitstream, &_mfx_video_params);
		if (sts == MFX_ERR_MORE_DATA)
		{
			if (_mfx_bitstream.MaxLength == _mfx_bitstream.DataLength)
			{
				sts = ExtendMfxBitstream(&_mfx_bitstream, _mfx_bitstream.MaxLength * 2);
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			}
			// read a portion of data
			sts = m_FileReader->ReadNextFrame(&_mfx_bitstream);
			if ((sts == MFX_ERR_MORE_DATA) && !(_mfx_bitstream.DataFlag & MFX_BITSTREAM_EOS))
			{
				_mfx_bitstream.DataFlag |= MFX_BITSTREAM_EOS;
				sts = MFX_ERR_NONE;
			}
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

			continue;
		}
		else
		{
			// Enter MVC mode
			if (_enable_mvc)
			{
				// Check for attached external parameters - if we have them already,
				// we don't need to attach them again
				if (_mfx_video_params.ExtParam != NULL)
					break;

				// allocate and attach external parameters for MVC decoder
				sts = allocate_ext_buffer<mfxExtMVCSeqDesc>();
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

				AttachExtParam();
				sts = _mfx_decoder->DecodeHeader(&_mfx_bitstream, &_mfx_video_params);

				if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
				{
					sts = allocate_ext_mvc_buffers();
					_use_ext_buffers = true;

					MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
					MSDK_CHECK_POINTER(_mfx_video_params.ExtParam, MFX_ERR_MEMORY_ALLOC);
					continue;
				}
			}

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

			break;
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
	if (_enable_mvc)
	{
		mfxExtMVCSeqDesc* pSequenceBuffer;
		pSequenceBuffer = (mfxExtMVCSeqDesc*)GetExtBuffer(_mfx_video_params.ExtParam, _mfx_video_params.NumExtParam, MFX_EXTBUFF_MVC_SEQ_DESC);
		MSDK_CHECK_POINTER(pSequenceBuffer, MFX_ERR_INVALID_VIDEO_PARAM);

		mfxU32 i = 0;
		num_views = 0;
		for (i = 0; i < pSequenceBuffer->NumView; ++i)
		{
			/* Some MVC streams can contain different information about
			number of views and view IDs, e.x. numVews = 2
			and ViewId[0, 1] = 0, 2 instead of ViewId[0, 1] = 0, 1.
			numViews should be equal (max(ViewId[i]) + 1)
			to prevent crashes during output files writing */
			if (pSequenceBuffer->View[i].ViewId >= num_views)
				num_views = pSequenceBuffer->View[i].ViewId + 1;
		}
	}
	else
	{
		num_views = 1;
	}

	// specify memory type
	_mfx_video_params.IOPattern = (mfxU16)(_mem_type != MEMORY_TYPE_SYSTEM ? MFX_IOPATTERN_OUT_VIDEO_MEMORY : MFX_IOPATTERN_OUT_SYSTEM_MEMORY);
	_mfx_video_params.AsyncDepth = config->nAsyncDepth;

	return MFX_ERR_NONE;
}

template<typename Buffer> mfxStatus ims_decoder_core::allocate_ext_buffer(void)
{
	std::auto_ptr<Buffer> ext_buffer(new Buffer());
	if (!ext_buffer.get())
		return MFX_ERR_MEMORY_ALLOC;

	init_ext_buffer(*ext_buffer);
	_ext_buffers.push_back(reinterpret_cast<mfxExtBuffer*>(ext_buffer.release()));

	return MFX_ERR_NONE;
}

void ims_decoder_core::delete_ext_buffers(void)
{
	for (std::vector<mfxExtBuffer *>::iterator it = _ext_buffers.begin(); it != _ext_buffers.end(); ++it)
		delete *it;
	_ext_buffers.clear();
}

mfxStatus ims_decoder_core::allocate_ext_mvc_buffers(void)
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

void ims_decoder_core::deallocate_ext_mvc_buffers(void)
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

mfxStatus ims_decoder_core::create_allocator()
{

}

void ims_decoder_core::delete_allocator()
{

}

mfxStatus ims_decoder_core::alloc_frames()
{

}

void ims_decoder_core::delete_frames()
{

}