#include "msdk_encoder.h"
#include <time.h>
#include <process.h>
#include <dk_fileio.h>
#include <dk_string_helper.h>
#include <d3d9.h>
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
	if (_config->mvc && !check_version(&version, dk_msdk_encoder::MSDK_FEATURE_MVC))
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

	if ((_config->codec == dk_msdk_encoder::SUBMEDIA_TYPE_MJPEG) && !check_version(&version, dk_msdk_encoder::MSDK_FEATURE_JPEG_ENCODE))
	{
		_session.Close();
		_state = dk_msdk_encoder::ENCODER_STATE_NONE;
		return dk_msdk_encoder::ERR_CODE_FAIL;
	}

	if ((_config->rc_mode == dk_msdk_encoder::RC_MODE_LOOK_AHEAD) && !check_version(&version, dk_msdk_encoder::MSDK_FEATURE_LOOK_AHEAD))
	{
		_session.Close();
		_state = dk_msdk_encoder::ENCODER_STATE_NONE;
		return dk_msdk_encoder::ERR_CODE_FAIL;
	}



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