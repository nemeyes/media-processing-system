#include "vce_encoder.h"

#define MILLISEC_TIME	10000
#define START_TIME_PROPERTY L"start_time_property"

#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024

vce_polling_thread::vce_polling_thread(vce_encoder * core)
	: _core(core)
{}

vce_polling_thread::~vce_polling_thread(void)
{}

void vce_polling_thread::Run(void)
{
	RequestStop();

	amf_pts latency_time = 0;
	amf_pts write_duration = 0;
	amf_pts encode_duration = 0;
	amf_pts last_poll_time = 0;

	AMF_RESULT res = AMF_OK; // error checking can be added later
	while (true)
	{
		amf::AMFDataPtr data;
		res = _core->_encoder->QueryOutput(&data);
		if (res == AMF_EOF)
		{
			break; // Drain complete
		}
		if (data != 0)
		{
			amf_pts poll_time = amf_high_precision_clock();
			amf_pts start_time = 0;
			data->GetProperty(START_TIME_PROPERTY, &start_time);
			if (start_time < last_poll_time) // remove wait time if submission was faster then encode
			{
				start_time = last_poll_time;
			}
			last_poll_time = poll_time;

			encode_duration += poll_time - start_time;

			if (latency_time == 0)
			{
				latency_time = poll_time - start_time;
			}

			amf::AMFBufferPtr buffer(data); // query for buffer interface
			//fwrite(buffer->GetNative(), 1, buffer->GetSize(), m_pFile);
			vce_encoder::am_enc_buffer_t * bs_buffer = _core->_enc_buffer_queue.get_available();
			if (bs_buffer && buffer->GetSize()>0)
			{
				memcpy(bs_buffer->bitstream_buffer, buffer->GetNative(), buffer->GetSize());
				bs_buffer->bitstream_buffer_size = buffer->GetSize();
			}
			write_duration += amf_high_precision_clock() - poll_time;
		}
		else
		{
			amf_sleep(1);
		}

	}
	/*
	printf("latency  = %.4fms\nencode  per frame = %.4fms\nwrite per frame   = %.4fms\n", 
			double(latency_time) / MILLISEC_TIME, 
			double(encode_duration) / MILLISEC_TIME / frameCount, 
			double(write_duration) / MILLISEC_TIME / frameCount); 
			*/
	_core->_encoder = 0;
	_core->_context = 0;
}

vce_encoder::vce_encoder(void)
	: _polling_thread(0)
	, _enc_buffer_count(0)
	, _bitstream_buffer_size(0)
{
	_polling_thread = new vce_polling_thread(this);
}

vce_encoder::~vce_encoder(void)
{
	if (_polling_thread)
	{
		delete _polling_thread;
		_polling_thread = 0;
	}
}

dk_vce_encoder::ERR_CODE vce_encoder::initialize(dk_vce_encoder::configuration_t conf)
{
	_conf = conf;

	AMF_RESULT status = AMF_OK;
	_mem_type = amf::AMF_MEMORY_DX9;

	switch (_conf.cs)
	{
	case dk_vce_encoder::COLOR_SPACE_I420 : 
		_cs = amf::AMF_SURFACE_YUV420P;
		break;
	case dk_vce_encoder::COLOR_SPACE_YV12 :
		_cs = amf::AMF_SURFACE_YV12;
		break;
	case dk_vce_encoder::COLOR_SPACE_NV12 :
		_cs = amf::AMF_SURFACE_NV12;
		break;
	case dk_vce_encoder::COLOR_SPACE_RGB24 :
		break;
	case dk_vce_encoder::COLOR_SPACE_RGB32 :
		_cs = amf::AMF_SURFACE_BGRA;
		break;
	default :
		_cs = amf::AMF_SURFACE_YV12;
	}

	_submitted = 0;

	status = AMFCreateContext(&_context);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _context->InitDX9(0);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = AMFCreateComponent(_context, AMFVideoEncoderVCE_AVC, &_encoder);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;

	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, _conf.usage);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, _conf.bitrate);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(_conf.width, _conf.height));
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(_conf.fps, 1));
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, _conf.numb);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, _conf.preset);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, _conf.fps*_conf.keyframe_interval);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, _conf.fps*_conf.keyframe_interval);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	status = _encoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, 1);
	if (status != AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
	
#if defined(ENABLE_4K)
    status = _encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;
#endif
	status = _encoder->Init(_cs, _conf.width, _conf.height);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;

	_submitted = 0;
	
	_enc_buffer_count = _conf.numb + 4;
	_bitstream_buffer_size = (_conf.bitrate / 8)*(_conf.keyframe_interval) * 2;

	if (_polling_thread)
		_polling_thread->Start();
	return dk_vce_encoder::ERR_CODE_SUCCESS;
}

dk_vce_encoder::ERR_CODE vce_encoder::encode(unsigned char * input, unsigned int & isize, unsigned char * output, unsigned int & osize, dk_vce_encoder::PIC_TYPE & pic_type, bool flush)
{
	amf_pts start_time = amf_high_precision_clock();
	AMF_RESULT status = AMF_OK;

	if(_surface==0)
	{
		status = _context->AllocSurface(_mem_type, _cs, _conf.width, _conf.height, &_surface);
		if(status!=AMF_OK)
			return dk_vce_encoder::ERR_CODE_FAILED;
		fill_surface(_context, _surface, _submitted);
	}

	status = _surface->SetProperty(START_TIME_PROPERTY, start_time);
	if(status!=AMF_OK)
		return dk_vce_encoder::ERR_CODE_FAILED;

	status = _encoder->SubmitInput(_surface);
	if(status==AMF_INPUT_FULL)
	{
		amf_sleep(1);
	}
	else
	{
		_surface = 0;
		_submitted++;
	}

	vce_encoder::am_enc_buffer_t * bs_buffer = _enc_buffer_queue.get_pending();
	if (bs_buffer)
	{
		memcpy(output, bs_buffer->bitstream_buffer, bs_buffer->bitstream_buffer_size);
		osize = bs_buffer->bitstream_buffer_size;
	}
	else
	{
		osize = 0;
	}

	return dk_vce_encoder::ERR_CODE_SUCCESS;
}

dk_vce_encoder::ERR_CODE vce_encoder::release(void)
{
	AMF_RESULT status = AMF_OK;
	// drain encoder; input queue can be full
	while (true)
	{
		status = _encoder->Drain();
		if (status != AMF_INPUT_FULL) // handle full queue
		{
			break;
		}
		amf_sleep(1); // input queue is full: wait and try again
	}

	if (_polling_thread)
		_polling_thread->WaitForStop();

	_surface = 0;
	_encoder->Terminate();
	_encoder = 0;
	_context->Terminate();
	_context = 0; // context is the last

	_submitted = 0;
	_bitstream_buffer_size = 0;
	return dk_vce_encoder::ERR_CODE_SUCCESS;
}

void vce_encoder::fill_surface(amf::AMFContext * context, amf::AMFSurface * surface, amf_int32 i)
{
    HRESULT hr = S_OK;
    // fill surface with something something useful. We fill with color and color rect
	if (surface->GetMemoryType() == amf::AMF_MEMORY_DX9)
	{
        D3DCOLOR color1 = D3DCOLOR_XYUV (128, 255, 128);
        D3DCOLOR color2 = D3DCOLOR_XYUV (128, 0, 128);
		// get native DX objects
		IDirect3DDevice9 *deviceDX9 = (IDirect3DDevice9 *)context->GetDX9Device(); // no reference counting - do not Release()
		IDirect3DSurface9* surfaceDX9 = (IDirect3DSurface9*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
		hr = deviceDX9->ColorFill(surfaceDX9, 0, color1);

		if (_x_pos + _rect_size > _conf.width)
		{
			_x_pos = 0;
		}
		if (_y_pos + _rect_size > _conf.height)
		{
			_y_pos = 0;
		}
		RECT rect = { _x_pos, _y_pos, _x_pos + _rect_size, _y_pos + _rect_size };
		hr = deviceDX9->ColorFill(surfaceDX9, &rect, color2);

		_x_pos += 2; //DX9 NV12 surfaces do not accept odd positions - do not use ++
		_y_pos += 2; //DX9 NV12 surfaces do not accept odd positions - do not use ++
	}
	else if (surface->GetMemoryType() == amf::AMF_MEMORY_DX11)
    {
		// Swapping two colors across frames
        ID3D11Device *deviceDX11 = (ID3D11Device*)context->GetDX11Device(); // no reference counting - do not Release()
        ID3D11Texture2D *textureDX11 = (ID3D11Texture2D*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
        ID3D11DeviceContext *contextDX11 = 0;
        ID3D11RenderTargetView *viewDX11 = 0;
        deviceDX11->GetImmediateContext(&contextDX11);
        hr = deviceDX11->CreateRenderTargetView(textureDX11, 0, &viewDX11);
        float color1[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
        float color2[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
        contextDX11->ClearRenderTargetView(viewDX11, (i % 2) ? color1 : color2);
        contextDX11->Flush();
        // release temp objects
        viewDX11->Release();
        contextDX11->Release();
    }
}

dk_vce_encoder::ERR_CODE vce_encoder::allocate_io_buffers(void)
{
	_enc_buffer_queue.initialize(_enc_buffer, _enc_buffer_count);
	for (int i = 0; i < _enc_buffer_count; i++)
	{
		_enc_buffer[i].bitstream_buffer_size = 0;
		_enc_buffer[i].bitstream_buffer = static_cast<unsigned char*>(malloc(BITSTREAM_BUFFER_SIZE));
	}
	return dk_vce_encoder::ERR_CODE_SUCCESS;
}

dk_vce_encoder::ERR_CODE vce_encoder::release_io_buffers(void)
{
	for (int i = 0; i < _enc_buffer_count; i++)
	{
		_enc_buffer[i].bitstream_buffer_size = 0;
		if (_enc_buffer[i].bitstream_buffer)
		{
			free(_enc_buffer[i].bitstream_buffer);
			_enc_buffer[i].bitstream_buffer = 0;
		}
	}
	return dk_vce_encoder::ERR_CODE_SUCCESS;
}