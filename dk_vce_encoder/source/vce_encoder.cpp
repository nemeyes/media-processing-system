#include "vce_encoder.h"
#include <time.h>
#include <amf/core/Buffer.h>
#include <process.h>
#include <dk_fileio.h>
#include <dk_string_helper.h>
#include <d3d9.h>
#include <d3dx9tex.h>

vce_encoder::vce_encoder(dk_vce_encoder * front)
	: _front(front)
	, _config(nullptr)
#if defined(WITH_AMF_CALLBACK_THREAD)
	, _cb_thread(INVALID_HANDLE_VALUE)
	, _cb_event(INVALID_HANDLE_VALUE)
#endif
#if defined(WITH_ENCODING_THREAD)
	, _encoding_run(false)
	, _encoding(false)
	, _encoding_finish_event(INVALID_HANDLE_VALUE)
#endif
	, _surface_observer(nullptr)
	, _surface(nullptr)
	, _encoder(nullptr)
	, _context(nullptr)
	, _state(dk_vce_encoder::ENCODER_STATE_NONE)
{
#if defined(WITH_DEBUG_ES)
	_file = ::open_file_write("test.h264");
#endif
}

vce_encoder::~vce_encoder(void)
{
	release_encoder();
#if defined(WITH_DEBUG_ES)
	::close_file(_file);
#endif
	_state = dk_vce_encoder::ENCODER_STATE_NONE;
}

dk_vce_encoder::ENCODER_STATE vce_encoder::state(void)
{
	return _state;
}

dk_vce_encoder::ERR_CODE vce_encoder::initialize_encoder(dk_vce_encoder::configuration_t * config)
{
	if ((_state != dk_vce_encoder::ENCODER_STATE_NONE) && (_state != dk_vce_encoder::ENCODER_STATE_RELEASED))
		return dk_vce_encoder::ERR_CODE_FAIL;

	release_encoder();
	_state = dk_vce_encoder::ENCODER_STATE_INITIALIZING;

	_config = config;
	AMF_RESULT status = AMF_OK;

	status = AMFCreateContext(&_context);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;

	status = AMF_FAIL;
	switch (_config->mem_type)
	{
	case dk_vce_encoder::MEMORY_TYPE_DX9:
	case dk_vce_encoder::MEMORY_TYPE_DX9EX:
		status = _context->InitDX9(_config->d3d_device);
		break;
	case dk_vce_encoder::MEMORY_TYPE_DX10:
	case dk_vce_encoder::MEMORY_TYPE_DX10_1:
		break;
	case dk_vce_encoder::MEMORY_TYPE_DX11_1:
	case dk_vce_encoder::MEMORY_TYPE_DX11_2:
	case dk_vce_encoder::MEMORY_TYPE_DX11_3:
		status = _context->InitDX11(_config->d3d_device);
		break;
	case dk_vce_encoder::MEMORY_TYPE_DX12:
		break;
	}
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;

	status = AMFCreateComponent(_context, AMFVideoEncoderVCE_AVC, &_encoder);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;

	//Static Parameters
	status = AMF_FAIL;
	switch (_config->profile)
	{
	case dk_vce_encoder::CODEC_PROFILE_TYPE_BASELINE:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_BASELINE);
		break;
	case dk_vce_encoder::CODEC_PROFILE_TYPE_MAIN:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_MAIN);
		break;
	case dk_vce_encoder::CODEC_PROFILE_TYPE_HIGH:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH);
		break;
	}
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;

	if (_config->enable_4k)
	{
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);
		if (status != AMF_OK)
			return dk_vce_encoder::ERR_CODE_FAIL;
	}

	status = AMF_FAIL;
	switch (_config->usage)
	{
	case dk_vce_encoder::USAGE_TRANSCONDING:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
		break;
	case dk_vce_encoder::USAGE_ULTRA_LOW_LATENCY:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
		break;
	case dk_vce_encoder::USAGE_LOW_LATENCY:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
		break;
	case dk_vce_encoder::USAGE_WEBCAM:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_WEBCAM);
		break;
	}
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;

	switch (_config->cs)
	{
	case dk_vce_encoder::COLOR_SPACE_I420:
		_cs = amf::AMF_SURFACE_YUV420P;
		break;
	case dk_vce_encoder::COLOR_SPACE_YV12:
		_cs = amf::AMF_SURFACE_YV12;
		break;
	case dk_vce_encoder::COLOR_SPACE_NV12:
		_cs = amf::AMF_SURFACE_NV12;
		break;
	case dk_vce_encoder::COLOR_SPACE_RGB32:
		_cs = amf::AMF_SURFACE_BGRA;
		break;
	default:
		_cs = amf::AMF_SURFACE_YV12;
	}
	status = _encoder->Init(_cs, _config->width, _config->height);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;


	//Dynamic Parameters
	if (_config->usage == dk_vce_encoder::USAGE_ULTRA_LOW_LATENCY)
	{
		if (_config->bitrate > 6000000)
			_config->bitrate = 6000000;
	}
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, _config->bitrate);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;

	if (_config->usage != dk_vce_encoder::USAGE_ULTRA_LOW_LATENCY)
	{
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, _config->peak_bitrate);
		if (status != AMF_OK)
			return dk_vce_encoder::ERR_CODE_FAIL;
	}

	//switch (_config->rc_mode)
	//{
	//case dk_vce_encoder::RC_MODE_CONSTQP:
	//	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP);
	//	break;
	//case dk_vce_encoder::RC_MODE_CBR:
	//	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR);
	//	break;
	//case dk_vce_encoder::RC_MODE_PEAK_CONSTRAINED_VBR:
	//	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR);
	//	break;
	//case dk_vce_encoder::RC_MODE_LATENCY_CONSTRAINED_VBR:
	//	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR);
	//	break;
	//}
	//if (status != AMF_OK)
	//	return dk_vce_encoder::ERR_CODE_FAIL;


	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(_config->width, _config->height));
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(_config->fps, 1));
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, _config->numb <= 0 ? 0 : _config->numb);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;

	//if (_config->usage != dk_vce_encoder::USAGE_ULTRA_LOW_LATENCY)
	//{
	//	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, _config->numb > 0 ? true : false);
	//	if (status != AMF_OK)
	//		return dk_vce_encoder::ERR_CODE_FAIL;
	//}

	status = AMF_FAIL;
	switch (_config->preset)
	{
	case dk_vce_encoder::PRESET_TYPE_QUALITY:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);
		break;
	case dk_vce_encoder::PRESET_TYPE_BALANCED:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED);
		break;
	case dk_vce_encoder::PRESET_TYPE_SPEED:
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
		break;
	}
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;

	if (_config->usage != dk_vce_encoder::USAGE_ULTRA_LOW_LATENCY)
	{
		status = _encoder->SetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, _config->fps * _config->keyframe_interval);
		if (status != AMF_OK)
			return dk_vce_encoder::ERR_CODE_FAIL;
	}
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, _config->fps * _config->keyframe_interval);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, _config->slice_per_frame);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAIL;


	_surface_observer = new vce_surface_observer();

#if defined(WITH_AMF_CALLBACK_THREAD)
	unsigned int query_cb_thrd_addr = 0;
	_cb_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	_cb_thread = (HANDLE)::_beginthreadex(NULL, 0, &vce_encoder::query_output_callback, this, 0, &query_cb_thrd_addr);
#endif

#if defined(WITH_ENCODING_THREAD)
	unsigned int encoding_thrd_addr = 0;
	_encoding = false;
	_encoding_finish_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	SetEvent(_encoding_finish_event); //for first encoding
	_encoding_thread = (HANDLE)::_beginthreadex(NULL, 0, &vce_encoder::process_encoding_callback, this, 0, &encoding_thrd_addr);
#endif

	_submited = 0;

	_state = dk_vce_encoder::ENCODER_STATE_INITIALIZED;
	return dk_vce_encoder::ERR_CODE_SUCCESS;
}

dk_vce_encoder::ERR_CODE vce_encoder::release_encoder(void)
{
	if ((_state != dk_vce_encoder::ENCODER_STATE_NONE) && (_state != dk_vce_encoder::ENCODER_STATE_INITIALIZED) && (_state != dk_vce_encoder::ENCODER_STATE_ENCODED))
		return dk_vce_encoder::ERR_CODE_FAIL;
	_state = dk_vce_encoder::ENCODER_STATE_RELEASING;

	AMF_RESULT status = AMF_OK;
	if (_encoder)
	{
		while (true)
		{
			status = _encoder->Drain();
			if (status != AMF_INPUT_FULL) // handle full queue
				break;
			amf_sleep(1); // input queue is full: wait and try again
		}

#if !defined(WITH_AMF_CALLBACK_THREAD)
		while (true)
		{
			amf::AMFDataPtr data;
			status = _encoder->QueryOutput(&data);
			if (status == AMF_EOF)
				break;
		}
#endif
	}

#if defined(WITH_ENCODING_THREAD)
	if (_encoding_thread != INVALID_HANDLE_VALUE)
	{
		_encoding_run = false;
		::WaitForSingleObject(_encoding_thread, INFINITE);
		::CloseHandle(_encoding_thread);
		_encoding_thread = INVALID_HANDLE_VALUE;
	}

	if (_encoding_finish_event != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_encoding_finish_event);
		_encoding_finish_event = INVALID_HANDLE_VALUE;
	}
#endif

#if defined(WITH_AMF_CALLBACK_THREAD)
	if (_cb_thread != INVALID_HANDLE_VALUE)
	{
		_cb_run = false;
		::WaitForSingleObject(_cb_thread, INFINITE);
		::CloseHandle(_cb_thread);
		_cb_thread = INVALID_HANDLE_VALUE;
	}

	if (_cb_event != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_cb_event);
		_cb_event = INVALID_HANDLE_VALUE;
	}
#endif

	if (_surface)
	{
		_surface = nullptr;
	}

	if (_surface_observer)
	{
		delete _surface_observer;
		_surface_observer = nullptr;
	}

	if (_encoder)
	{
		_encoder->Terminate();
		_encoder = nullptr;
	}

	if (_context)
	{
		_context->Terminate();
		_context = nullptr;
	}

	_state = dk_vce_encoder::ENCODER_STATE_RELEASED;
	return dk_vce_encoder::ERR_CODE_SUCCESS;
}

dk_vce_encoder::ERR_CODE vce_encoder::encode(dk_vce_encoder::dk_video_entity_t * input, dk_vce_encoder::dk_video_entity_t * bitstream)
{
	dk_vce_encoder::ERR_CODE status = encode(input);
	if (status != dk_vce_encoder::ERR_CODE_SUCCESS)
		return status;

	status = get_queued_data(bitstream);
	if (status != dk_vce_encoder::ERR_CODE_SUCCESS)
		return status;

	return dk_vce_encoder::ERR_CODE_SUCCESS;
}

dk_vce_encoder::ERR_CODE vce_encoder::encode(dk_vce_encoder::dk_video_entity_t * input)
{
	if ((_state != dk_vce_encoder::ENCODER_STATE_INITIALIZED) &&
		(_state != dk_vce_encoder::ENCODER_STATE_ENCODED))
		return dk_vce_encoder::ERR_CODE_SUCCESS;

	_state = dk_vce_encoder::ENCODER_STATE_ENCODING;

	AMF_RESULT status = AMF_OK;
	if (_config->mem_type != input->mem_type)
		return dk_vce_encoder::ERR_CODE_FAIL;

	if (input->surface == nullptr)
		return dk_vce_encoder::ERR_CODE_FAIL;

	switch (input->mem_type)
	{
	case dk_vce_encoder::MEMORY_TYPE_DX9:
	case dk_vce_encoder::MEMORY_TYPE_DX9EX:
		status = _context->CreateSurfaceFromDX9Native(input->surface, &_surface, _surface_observer);
		break;
	case dk_vce_encoder::MEMORY_TYPE_DX10:
	case dk_vce_encoder::MEMORY_TYPE_DX10_1:
		break;
	case dk_vce_encoder::MEMORY_TYPE_DX11_1:
	case dk_vce_encoder::MEMORY_TYPE_DX11_2:
	case dk_vce_encoder::MEMORY_TYPE_DX11_3:
		status = _context->CreateSurfaceFromDX11Native(input->surface, &_surface, _surface_observer);
		break;
	case dk_vce_encoder::MEMORY_TYPE_DX12:
		break;
	}

	_surface->SetProperty(AMF_VIDEO_ENCODER_END_OF_SEQUENCE, false);
	_surface->SetProperty(AMF_VIDEO_ENCODER_END_OF_STREAM, false);
	_surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_AUD, false);
	if (_config->usage == dk_vce_encoder::USAGE_ULTRA_LOW_LATENCY)
	{
		_surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, false);
		_surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, false);
	}
	else
	{
		_submited++;
		if (_submited==(_config->fps*_config->keyframe_interval))
		{
			_surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, true);
			_surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, true);
			_surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
			_submited = 0;
}
		else
		{
			_surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, false);
			_surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, false);
			_surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_P);
		}
	}
	_surface->SetProperty(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE, AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_FRAME);

	status = _encoder->SubmitInput(_surface);
	while (status == AMF_INPUT_FULL)
	{
		amf_sleep(1);
		status = _encoder->SubmitInput(_surface);
	}

	if (status == AMF_OK)
	{
#if defined(WITH_AMF_CALLBACK_THREAD)
		::SetEvent(_cb_event);
#endif
	}

#if !defined(WITH_AMF_CALLBACK_THREAD)
	amf::AMFDataPtr data;
	status = _encoder->QueryOutput(&data);
	while (status == AMF_REPEAT)
		status = _encoder->QueryOutput(&data);

	if (status == AMF_OK)
	{
		amf::AMFBufferPtr buffer(data); // query for buffer interface
		//buffer->Convert(amf::AMF_MEMORY_DX9);
		if (buffer->GetSize()>0)
		{
			uint8_t * bs = (uint8_t*)buffer->GetNative();
			size_t bs_size = buffer->GetSize();

#if defined(WITH_DEBUG_ES)
			DWORD nbytes = 0;
			if (_file != INVALID_HANDLE_VALUE)
			{
				uint32_t bytes2write = bs_size;
				uint32_t bytes_written = 0;
				do
				{
					uint32_t nb_write = 0;
					write_file(_file, bs, bytes2write, &nb_write, NULL);
					bytes_written += nb_write;
					if (bytes2write == bytes_written)
						break;
				} while (1);
			}
#endif

			uint8_t * begin = bs;
			uint8_t * end = bs;
			//size_t remained_size = bs_size;
			while (begin < bs + bs_size)
			{
				int nalu_begin, nalu_end;
				int nalu_size = next_nalu(begin, (bs + bs_size) - begin, &nalu_begin, &nalu_end);
				if (nalu_size == 0)
				{
					break;
				}
				else if (nalu_size < 0)
				{
					begin += nalu_begin;
					if ((begin[0] & 0x1F) != 0x09) //exclude AUD
						_front->push(begin - 4, (bs + bs_size) - begin + 4);
					break;
				}
				else
				{
					begin += nalu_begin;
					end += nalu_end;
					if ((begin[0] & 0x1F) != 0x09) //exclude AUD
						_front->push(begin - 4, nalu_end - nalu_begin + 4);
				}
			}
			}
		}
#endif

	_surface = nullptr;
#if defined(WITH_ENCODING_THREAD)
	SetEvent(_encoding_finish_event);
#endif

	_state = dk_vce_encoder::ENCODER_STATE_ENCODED;
	return dk_vce_encoder::ERR_CODE_SUCCESS;
}

dk_vce_encoder::ERR_CODE vce_encoder::get_queued_data(dk_vce_encoder::dk_video_entity_t * bitstream)
{
	if (_front)
	{
		bitstream->mem_type = dk_vce_encoder::MEMORY_TYPE_HOST;
		return _front->pop((uint8_t*)bitstream->data, bitstream->data_size);
	}
	else
	{
		return dk_vce_encoder::ERR_CODE_FAIL;
	}
}
dk_vce_encoder::ERR_CODE vce_encoder::encode_async(dk_video_encoder::dk_video_entity_t * input)
{
#if defined(WITH_ENCODING_THREAD)
	bool encoding = false;
	if ((input->mem_type == dk_vce_encoder::MEMORY_TYPE_DX9) || (input->mem_type == dk_vce_encoder::MEMORY_TYPE_DX9EX))
	{
		if (_encoding_param.surface)
		{
			((IDirect3DSurface9*)_encoding_param.surface)->Release();
			_encoding_param.surface = nullptr;
		}
		_encoding_param = *(input);

		D3DSURFACE_DESC d3d_desc;
		((IDirect3DSurface9*)input->surface)->GetDesc(&d3d_desc);

		IDirect3DTexture9 * d3d_texture = nullptr;
		IDirect3DSurface9 * surface_clone = nullptr;

		HRESULT hres = D3DXCreateTexture((IDirect3DDevice9*)_config->d3d_device, d3d_desc.Width, d3d_desc.Height,
			D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &d3d_texture);
		if (SUCCEEDED(hres) && d3d_texture)
		{
			hres = d3d_texture->GetSurfaceLevel(0, &surface_clone);
			if (SUCCEEDED(hres) && surface_clone)
			{
				hres = ((IDirect3DDevice9*)_config->d3d_device)->StretchRect((IDirect3DSurface9*)input->surface, NULL, surface_clone, NULL, D3DTEXF_NONE);
				if (SUCCEEDED(hres))
				{
					//D3DXSaveSurfaceToFile(L"screen.bmp", D3DXIFF_BMP, surface_clone, NULL, NULL);
					_encoding_param.surface = surface_clone;
					encoding = true;
				}
			}
			d3d_texture->Release();
			d3d_texture = nullptr;
		}

		if (encoding)
			_encoding = true;
		else
			::SetEvent(_encoding_finish_event);
	}

	return dk_vce_encoder::ERR_CODE_SUCCESS;
#else
	return dk_vce_encoder::ERR_CODE_FAIL;
#endif
}

dk_vce_encoder::ERR_CODE vce_encoder::check_encoding_flnish(void)
{
#if defined(WITH_ENCODING_THREAD)
	if (::WaitForSingleObject(_encoding_finish_event, 0) == WAIT_OBJECT_0)
		return dk_vce_encoder::ERR_CODE_SUCCESS;
	else
		return dk_vce_encoder::ERR_CODE_ENCODING_UNDER_PROCESSING;
#else
	return dk_vce_encoder::ERR_CODE_NOT_IMPLEMENTED;
#endif
}

#if defined(WITH_AMF_CALLBACK_THREAD)
unsigned __stdcall vce_encoder::query_output_callback(void * param)
{
	vce_encoder * self = static_cast<vce_encoder*>(param);
	self->query_output();
	return 0;
}

void vce_encoder::query_output(void)
{
	AMF_RESULT status = AMF_OK; // error checking can be added later
	_cb_run = true;
	while (_cb_run)
	{
		if (::WaitForSingleObject(_cb_event, 5000) == WAIT_OBJECT_0)
		{
			do
			{
				amf::AMFDataPtr data;
				status = _encoder->QueryOutput(&data);
				OutputDebugStringW(L"_encoder->QueryOutput\n");
				if (status == AMF_EOF)
				{
					data = nullptr;
					break; // Drain complete
				}
				else if (status == AMF_OK)
				{
					amf::AMFBufferPtr buffer(data); // query for buffer interface
					if (buffer->GetSize() > 0)
					{
						uint8_t * bs = (uint8_t*)buffer->GetNative();
						size_t bs_size = buffer->GetSize();

#if defined(WITH_DEBUG_ES)
						DWORD nbytes = 0;
						if (_file != INVALID_HANDLE_VALUE)
						{
							uint32_t bytes2write = bs_size;
							uint32_t bytes_written = 0;
							do
							{
								uint32_t nb_write = 0;
								write_file(_file, bs, bytes2write, &nb_write, NULL);
								bytes_written += nb_write;
								if (bytes2write == bytes_written)
									break;
							} while (1);
						}
#endif

						uint8_t * begin = bs;
						uint8_t * end = bs;
						//size_t remained_size = bs_size;
						while (begin < bs + bs_size)
						{
							int nalu_begin, nalu_end;
							int nalu_size = next_nalu(begin, (bs + bs_size) - begin, &nalu_begin, &nalu_end);
							if (nalu_size == 0)
							{
								break;
							}
							else if (nalu_size < 0)
							{
								begin += nalu_begin;
								if ((begin[0] & 0x1F) != 0x09) //exclude AUD
								{
									_front->on_acquire_bitstream(begin - 4, (bs + bs_size) - begin + 4);
								}
								break;
							}
							else
							{
								begin += nalu_begin;
								end += nalu_end;
								if ((begin[0] & 0x1F) != 0x09) //exclude AUD
								{
									_front->on_acquire_bitstream(begin - 4, nalu_end - nalu_begin + 4);
								}
							}
						}
					}
				}
				else
				{
					break;
					//amf_sleep(1);
				}
			} while (0);
		}
	}
}
#endif

#if defined(WITH_ENCODING_THREAD)
unsigned __stdcall vce_encoder::process_encoding_callback(void * param)
{
	vce_encoder * self = static_cast<vce_encoder*>(param);
	self->process_encoding();
	return 0;
}

void vce_encoder::process_encoding(void)
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

const int vce_encoder::next_nalu(uint8_t * bitstream, size_t size, int * nal_start, int * nal_end)
{
	int i;
	*nal_start = 0;
	*nal_end = 0;

	i = 0;
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0 || bitstream[i + 3] != 0x01))
	{
		i++;
		if (i + 4 >= size)
			return 0;
	}

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
		i++;

	if (bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01)
		return 0;/* error, should never happen */

	i += 3;
	*nal_start = i;
	while ((bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0) &&
		(bitstream[i] != 0 || bitstream[i + 1] != 0 || bitstream[i + 2] != 0x01))
	{
		i++;
		if (i + 3 >= size)
		{
			*nal_end = size;
			return -1;
		}
	}

	*nal_end = i;
	return (*nal_end - *nal_start);
}