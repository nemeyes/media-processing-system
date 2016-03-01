#include "msdk_encoder.h"
#include <time.h>
#include <process.h>
#include <dk_fileio.h>
#include <dk_string_helper.h>
#include <d3d9.h>
#include <dk_macros.h>
//#include <d3dx9tex.h>


msdk_encoder::msdk_encoder(dk_msdk_encoder * front)
	: _front(front)
	, _config(nullptr)
	, _state(dk_msdk_encoder::ENCODER_STATE_NONE)
{
#if defined(WITH_DEBUG_ES)
	_file = ::open_file_write("test.h264");
#endif
}

msdk_encoder::~msdk_encoder(void)
{
	release_encoder();
#if defined(WITH_DEBUG_ES)
	::close_file(_file);
#endif
	_state = dk_msdk_encoder::ENCODER_STATE_NONE;
}

dk_msdk_encoder::ENCODER_STATE msdk_encoder::state(void)
{
	return _state;
}

dk_msdk_encoder::ERR_CODE msdk_encoder::initialize_encoder(dk_msdk_encoder::configuration_t * config)
{
	if ((_state != dk_msdk_encoder::ENCODER_STATE_NONE) && (_state != dk_msdk_encoder::ENCODER_STATE_RELEASED))
		return dk_msdk_encoder::ERR_CODE_FAIL;

	release_encoder();
	_state = dk_msdk_encoder::ENCODER_STATE_INITIALIZING;
	_config = config;
	
	mfxStatus code = MFX_ERR_NONE;

	mfxVersion min_version;
	mfxVersion version;

	mfxIMPL impl = MFX_IMPL_HARDWARE_ANY;
	if (config->mem_type == dk_msdk_encoder::MEMORY_TYPE_DX11)
		impl |= MFX_IMPL_VIA_D3D11;

	code = _session.Init(impl, &min_version);
	if (code != MFX_ERR_NONE)
		code = _session.Init((impl & (!MFX_IMPL_HARDWARE_ANY)) | MFX_IMPL_HARDWARE, &min_version);
	if (code != MFX_ERR_NONE)
		return dk_msdk_encoder::ERR_CODE_FAIL;
	if ((_config->codec == dk_msdk_encoder::SUBMEDIA_TYPE_MVC) && !check_version(&version, dk_msdk_encoder::MSDK_FEATURE_MVC))
	{
		_session.Close();
		_state = dk_msdk_encoder::ENCODER_STATE_NONE;
		return dk_msdk_encoder::ERR_CODE_FAIL;
	}

	/*
	if ((pParams->MVC_flags & MVC_VIEWOUTPUT) != 0 && !CheckVersion(&version, MSDK_FEATURE_MVC_VIEWOUTPUT)) {
	msdk_printf(MSDK_STRING("error: MVC Viewoutput is not supported in the %d.%d API version\n"),
	version.Major, version.Minor);
	return MFX_ERR_UNSUPPORTED;
	}
	*/

	if ((_config->codec == dk_msdk_encoder::SUBMEDIA_TYPE_JPEG) && !check_version(&version, dk_msdk_encoder::MSDK_FEATURE_JPEG_ENCODE))
	{
		_session.Close();
		_state = dk_msdk_encoder::ENCODER_STATE_NONE;
		return dk_msdk_encoder::ERR_CODE_FAIL;
	}

	if ((_config->rc_mode == dk_msdk_encoder::RC_MODE_LA) && !check_version(&version, dk_msdk_encoder::MSDK_FEATURE_LOOK_AHEAD))
	{
		_session.Close();
		_state = dk_msdk_encoder::ENCODER_STATE_NONE;
		return dk_msdk_encoder::ERR_CODE_FAIL;
	}

	code = init_encoder_parameter();
	if (code != MFX_ERR_NONE)
	{
		_session.Close();
		_state = dk_msdk_encoder::ENCODER_STATE_NONE;
		return dk_msdk_encoder::ERR_CODE_FAIL;
	}

	_encoder = new MFXVideoENCODE(_session);

	_state = dk_msdk_encoder::ENCODER_STATE_INITIALIZED;
	return dk_msdk_encoder::ERR_CODE_SUCCESS;
}

dk_msdk_encoder::ERR_CODE msdk_encoder::release_encoder(void)
{
	if ((_state != dk_msdk_encoder::ENCODER_STATE_NONE) && (_state != dk_msdk_encoder::ENCODER_STATE_INITIALIZED) && (_state != dk_msdk_encoder::ENCODER_STATE_ENCODED))
		return dk_msdk_encoder::ERR_CODE_FAIL;
	_state = dk_msdk_encoder::ENCODER_STATE_RELEASING;



	_state = dk_msdk_encoder::ENCODER_STATE_RELEASED;
	return dk_msdk_encoder::ERR_CODE_SUCCESS;
}

dk_msdk_encoder::ERR_CODE msdk_encoder::encode(dk_msdk_encoder::dk_video_entity_t * input, dk_msdk_encoder::dk_video_entity_t * bitstream)
{
	dk_msdk_encoder::ERR_CODE status = encode(input);
	if (status != dk_msdk_encoder::ERR_CODE_SUCCESS)
		return status;

	status = get_queued_data(bitstream);
	if (status != dk_msdk_encoder::ERR_CODE_SUCCESS)
		return status;

	return dk_msdk_encoder::ERR_CODE_SUCCESS;
}

dk_msdk_encoder::ERR_CODE msdk_encoder::encode(dk_msdk_encoder::dk_video_entity_t * input)
{
	if ((_state != dk_msdk_encoder::ENCODER_STATE_INITIALIZED) &&
		(_state != dk_msdk_encoder::ENCODER_STATE_ENCODED))
		return dk_msdk_encoder::ERR_CODE_SUCCESS;

	_state = dk_msdk_encoder::ENCODER_STATE_ENCODING;



	_state = dk_msdk_encoder::ENCODER_STATE_ENCODED;
	return dk_msdk_encoder::ERR_CODE_SUCCESS;
}

dk_msdk_encoder::ERR_CODE msdk_encoder::get_queued_data(dk_msdk_encoder::dk_video_entity_t * bitstream)
{
	if (_front)
	{
		bitstream->mem_type = dk_msdk_encoder::MEMORY_TYPE_HOST;
		return _front->pop((uint8_t*)bitstream->data, bitstream->data_size, bitstream->pts);
	}
	else
	{
		return dk_msdk_encoder::ERR_CODE_FAIL;
	}
}
dk_msdk_encoder::ERR_CODE msdk_encoder::encode_async(dk_video_encoder::dk_video_entity_t * input)
{
#if defined(WITH_ENCODING_THREAD)


	return dk_msdk_encoder::ERR_CODE_SUCCESS;
#else
	return dk_msdk_encoder::ERR_CODE_NOT_IMPLEMENTED;
#endif
}

dk_msdk_encoder::ERR_CODE msdk_encoder::check_encoding_finish(void)
{
#if defined(WITH_ENCODING_THREAD)
	if (::WaitForSingleObject(_encoding_finish_event, 0) == WAIT_OBJECT_0)
		return dk_msdk_encoder::ERR_CODE_SUCCESS;
	else
		return dk_msdk_encoder::ERR_CODE_ENCODING_UNDER_PROCESSING;
#else
	return dk_msdk_encoder::ERR_CODE_NOT_IMPLEMENTED;
#endif
}

#if defined(WITH_AMF_CALLBACK_THREAD)
unsigned __stdcall msdk_encoder::query_output_callback(void * param)
{
	msdk_encoder * self = static_cast<msdk_encoder*>(param);
	self->query_output();
	return 0;
}

void msdk_encoder::query_output(void)
{
	AMF_RESULT status = AMF_OK; // error checking can be added later
	_cb_run = true;
	while (_cb_run)
	{
		if (::WaitForSingleObject(_cb_event, 5000) == WAIT_OBJECT_0)
		{
			do
			{

			} while (0);
		}
	}
}
#endif

#if defined(WITH_ENCODING_THREAD)
unsigned __stdcall msdk_encoder::process_encoding_callback(void * param)
{
	msdk_encoder * self = static_cast<msdk_encoder*>(param);
	self->process_encoding();
	return 0;
}

void msdk_encoder::process_encoding(void)
{
	_encoding_run = true;
	while (_encoding_run)
	{
		if (_encoding)
		{
			//D3DLOCKED_RECT lockedRect;
			//((LPDIRECT3DSURFACE9)_encoding_param.surface)->LockRect(&lockedRect, NULL, 0);

			D3DXSaveSurfaceToFile(L"screen_1.bmp", D3DXIFF_BMP, (LPDIRECT3DSURFACE9)_encoding_param.surface, NULL, NULL);

			encode(&_encoding_param);

			//((LPDIRECT3DSURFACE9)_encoding_param.surface)->UnlockRect();

			//((IDirect3DSurface9*)_encoding_param.surface)->Release();
			//_encoding_param.surface = nullptr;
			_encoding = false;
		}
		else
		{
			::Sleep(1);
		}
	}
}

#endif

mfxStatus msdk_encoder::init_encoder_parameter(void)
{
	switch (_config->codec)
	{
	case dk_msdk_encoder::SUBMEDIA_TYPE_H264:
		_encoder_param.mfx.CodecId = MFX_CODEC_AVC;
		break;
	case dk_msdk_encoder::SUBMEDIA_TYPE_HEVC:
		_encoder_param.mfx.CodecId = MFX_CODEC_HEVC;
		break;
	case dk_msdk_encoder::SUBMEDIA_TYPE_VC1:
		_encoder_param.mfx.CodecId = MFX_CODEC_VC1;
		break;
	case dk_msdk_encoder::SUBMEDIA_TYPE_MVC:
		_encoder_param.mfx.CodecId = MFX_CODEC_AVC;
		_encoder_param.mfx.CodecProfile = MFX_PROFILE_AVC_STEREO_HIGH;
		break;
	case dk_msdk_encoder::SUBMEDIA_TYPE_JPEG:
		_encoder_param.mfx.CodecId = MFX_CODEC_JPEG;
		_encoder_param.mfx.Interleaved = 1;
		_encoder_param.mfx.Quality = _config->quality;
		_encoder_param.mfx.RestartInterval = 0;
		ZERO_MEMORY(_encoder_param.mfx.reserved5);
		break;
	case dk_msdk_encoder::SUBMEDIA_TYPE_VP8:
		_encoder_param.mfx.CodecId = MFX_CODEC_VP8;
		break;
	}

	switch (_config->usage)
	{
	case dk_msdk_encoder::USAGE_BALANCED:
		_encoder_param.mfx.TargetUsage = MFX_TARGETUSAGE_BEST_QUALITY;
		break;
	case dk_msdk_encoder::USAGE_BEST_QUALITY:
		_encoder_param.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
		break;
	case dk_msdk_encoder::USAGE_BEST_SPEED:
		_encoder_param.mfx.TargetUsage = MFX_TARGETUSAGE_BEST_SPEED;
		break;
	}
	_encoder_param.mfx.TargetKbps = _config->bitrate / 1000; // in Kbps

	switch (_config->rc_mode)
	{
	case dk_msdk_encoder::RC_MODE_CONSTQP:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_CQP;
		//_encoder_param.mfx.QPI = pInParams->nQPI;
		//_encoder_param.mfx.QPP = pInParams->nQPP;
		//_encoder_param.mfx.QPB = pInParams->nQPB;
		break;
	case dk_msdk_encoder::RC_MODE_VBR:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
		break;
	case dk_msdk_encoder::RC_MODE_CBR:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_CBR;
		break;
	case dk_msdk_encoder::RC_MODE_AVBR:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_AVBR;
		break;
	case dk_msdk_encoder::RC_MODE_LA:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_LA;
		break;
	case dk_msdk_encoder::RC_MODE_ICQ:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_ICQ;
		break;
	case dk_msdk_encoder::RC_MODE_VCM:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_VCM;
		break;
	case dk_msdk_encoder::RC_MODE_LA_ICQ:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_LA_ICQ;
		break;
	case dk_msdk_encoder::RC_MODE_LA_EXT:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_LA_EXT;
		break;
	case dk_msdk_encoder::RC_MODE_LA_HRD:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_LA_HRD;
		break;
	case dk_msdk_encoder::RC_MODE_QVBR:
		_encoder_param.mfx.RateControlMethod = MFX_RATECONTROL_QVBR;
		break;
	}

	_encoder_param.mfx.NumSlice = _config->slice_per_frame;
	convert_fps(_config->fps, &_encoder_param.mfx.FrameInfo.FrameRateExtN, &_encoder_param.mfx.FrameInfo.FrameRateExtD);
	_encoder_param.mfx.EncodedOrder = 0; // binary flag, 0 signals encoder to take frames in display order

	if (_config->mem_type == dk_msdk_encoder::MEMORY_TYPE_HOST)
		_encoder_param.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
	else
		_encoder_param.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY;

	_encoder_param.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
	_encoder_param.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
	_encoder_param.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	_encoder_param.mfx.FrameInfo.Width = ALIGN16(_config->width);
	_encoder_param.mfx.FrameInfo.Height = (_encoder_param.mfx.FrameInfo.PicStruct == MFX_PICSTRUCT_PROGRESSIVE) ? ALIGN16(_config->height) : ALIGN32(_config->height);
	_encoder_param.mfx.FrameInfo.CropX = 0;
	_encoder_param.mfx.FrameInfo.CropY = 0;
	_encoder_param.mfx.FrameInfo.CropW = _config->width;
	_encoder_param.mfx.FrameInfo.CropH = _config->height;

	/*// configure and attach external parameters
	if (MVC_ENABLED & pInParams->MVC_flags)
	m_EncExtParams.push_back((mfxExtBuffer *)&m_MVCSeqDesc);

	if (MVC_VIEWOUTPUT & pInParams->MVC_flags)
	{
	// ViewOuput option requested
	m_CodingOption.ViewOutput = MFX_CODINGOPTION_ON;
	m_EncExtParams.push_back((mfxExtBuffer *)&m_CodingOption);
	}

	// configure the depth of the look ahead BRC if specified in command line
	if (pInParams->nLADepth || pInParams->nMaxSliceSize)
	{
	m_CodingOption2.LookAheadDepth = pInParams->nLADepth;
	m_CodingOption2.MaxSliceSize = pInParams->nMaxSliceSize;
	m_EncExtParams.push_back((mfxExtBuffer *)&m_CodingOption2);
	}

	// In case of HEVC when height and/or width divided with 8 but not divided with 16
	// add extended parameter to increase performance
	if ((!((m_mfxEncParams.mfx.FrameInfo.CropW & 15) ^ 8) ||
	!((m_mfxEncParams.mfx.FrameInfo.CropH & 15) ^ 8)) &&
	(m_mfxEncParams.mfx.CodecId == MFX_CODEC_HEVC))
	{
	m_ExtHEVCParam.PicWidthInLumaSamples = m_mfxEncParams.mfx.FrameInfo.CropW;
	m_ExtHEVCParam.PicHeightInLumaSamples = m_mfxEncParams.mfx.FrameInfo.CropH;
	m_EncExtParams.push_back((mfxExtBuffer*)&m_ExtHEVCParam);
	}

	if (!m_EncExtParams.empty())
	{
	_encoder_param.ExtParam = &m_EncExtParams[0]; // vector is stored linearly in memory
	_encoder_param.NumExtParam = (mfxU16)m_EncExtParams.size();
	}*/

	_encoder_param.AsyncDepth = 4;

	return MFX_ERR_NONE;
}

mfxStatus msdk_encoder::init_vpp_parameter(void)
{

}

bool msdk_encoder::check_version(mfxVersion * version, dk_msdk_encoder::MSDK_API_FEATURE feature)
{
	if (!version->Major || (version->Major > 1)) 
	{
		return false;
	}

	switch (feature) 
	{
		case dk_msdk_encoder::MSDK_FEATURE_NONE:
			return true;
		case dk_msdk_encoder::MSDK_FEATURE_MVC:
			if ((version->Major == 1) && (version->Minor >= 3)) 
			{
				return true;
			}
			break;
		case dk_msdk_encoder::MSDK_FEATURE_JPEG_DECODE:
			if ((version->Major == 1) && (version->Minor >= 3)) 
			{
				return true;
			}
			break;
		case dk_msdk_encoder::MSDK_FEATURE_LOW_LATENCY:
			if ((version->Major == 1) && (version->Minor >= 3)) 
			{
				return true;
			}
			break;
		case dk_msdk_encoder::MSDK_FEATURE_MVC_VIEWOUTPUT:
			if ((version->Major == 1) && (version->Minor >= 4)) 
			{
				return true;
			}
			break;
		case dk_msdk_encoder::MSDK_FEATURE_JPEG_ENCODE:
			if ((version->Major == 1) && (version->Minor >= 6)) 
			{
				return true;
			}
			break;
		case dk_msdk_encoder::MSDK_FEATURE_LOOK_AHEAD:
			if ((version->Major == 1) && (version->Minor >= 7)) 
			{
				return true;
			}
			break;
		case dk_msdk_encoder::MSDK_FEATURE_PLUGIN_API:
			if ((version->Major == 1) && (version->Minor >= 8)) 
			{
				return true;
			}
			break;
		default:
			return false;
	}
	return false;
}

mfxStatus msdk_encoder::convert_fps(mfxF64 dFrameRate, mfxU32 * pnFrameRateExtN, mfxU32 * pnFrameRateExtD)
{
	mfxU32 fr;
	fr = (mfxU32)(dFrameRate + .5);

	if (fabs(fr - dFrameRate) < 0.0001)
	{
		*pnFrameRateExtN = fr;
		*pnFrameRateExtD = 1;
		return MFX_ERR_NONE;
	}

	fr = (mfxU32)(dFrameRate * 1.001 + .5);

	if (fabs(fr * 1000 - dFrameRate * 1001) < 10)
	{
		*pnFrameRateExtN = fr * 1000;
		*pnFrameRateExtD = 1001;
		return MFX_ERR_NONE;
	}

	*pnFrameRateExtN = (mfxU32)(dFrameRate * 10000 + .5);
	*pnFrameRateExtD = 10000;

	return MFX_ERR_NONE;
}

mfxF64 msdk_encoder::calculate_fps(mfxU32 nFrameRateExtN, mfxU32 nFrameRateExtD)
{
	if (nFrameRateExtN && nFrameRateExtD)
		return (mfxF64)nFrameRateExtN / nFrameRateExtD;
	else
		return 0;
}
