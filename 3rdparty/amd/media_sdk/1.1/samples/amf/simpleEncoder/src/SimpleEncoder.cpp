/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice, 
 this list of conditions and the following disclaimer in the 
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 ********************************************************************************
 * @file <SimpleEncoder.cpp>
 *
 * @brief This sample encodes NV12 frames using AMF Encoder and writes them 
 *        to H.264 elementary stream 
 *
 ********************************************************************************
 */

#include <stdio.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3d11.h>
#include "VideoEncoderVCE.h"
#include "AMFPlatform.h"
#include "PlatformWindows.h"
#include "Thread.h"

//static amf::AMF_MEMORY_TYPE memoryTypeIn  = amf::AMF_MEMORY_DX9;
static amf::AMF_SURFACE_FORMAT formatIn   = amf::AMF_SURFACE_NV12;
#if defined(ENABLE_4K)
static amf_int32 widthIn                  = 1920*2;
static amf_int32 heightIn                 = 1080*2;
#else
static amf_int32 widthIn                  = 1920;
static amf_int32 heightIn                 = 1080;
#endif
static amf_int32 frameRateIn              = 30;
static amf_int64 bitRateIn                = 5000000L; // in bits, 1MB
static amf_int32 rectSize                 = 50;
static amf_int32 frameCount               = 500;

#define START_TIME_PROPERTY L"StartTimeProperty" // custom property ID to store submission time in a frame - all custom properties are copied from input to output
static wchar_t *fileNameOut = L"./output.h264";

static amf_int32 xPos = 0;
static amf_int32 yPos = 0;

#define MILLISEC_TIME     10000

class PollingThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pEncoder;
    FILE                    *m_pFile;
public:
    PollingThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName);
    ~PollingThread();
    virtual void Run();
};

static void FillSurface(amf::AMFContext *context, amf::AMFSurface *surface, amf_int32 i);

int _tmain(int argc, _TCHAR* argv[])
{
    ::amf_increase_timer_precision();
    
    AMF_RESULT res = AMF_OK; // error checking can be added later

    static amf::AMF_MEMORY_TYPE memoryTypeIn = amf::AMF_MEMORY_DX9;
    static wchar_t memType[50];

    if ((argc > 2))
    {
        printf("simpleEncoder.exe <In Memory Type: DX9 or DX11>\n");
        printf("FAIL\n");
        return AMF_FAIL;
    }
    else if (argc == 2)
    {
        wcscpy(memType, argv[1]);

        if (wcscmp(memType, L"DX9") == 0)
        {
            memoryTypeIn = amf::AMF_MEMORY_DX9;
        }
        else if (wcscmp(memType, L"DX11") == 0)
        {
			formatIn     = amf::AMF_SURFACE_BGRA;
            memoryTypeIn = amf::AMF_MEMORY_DX11;
        }
        else
        {
            printf("Incorrect Input memory Type.  Supported values : DX9 or DX11\n");
            printf("FAIL\n");
            return AMF_FAIL;
        }
    }

    // initialize AMF
    amf::AMFContextPtr context;
    amf::AMFComponentPtr encoder;
    amf::AMFSurfacePtr surfaceIn;

    // context
    res = AMFCreateContext(&context);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
    if(memoryTypeIn == amf::AMF_MEMORY_DX9)
    {
        res = context->InitDX9(NULL); // can be DX9 or DX9Ex device
    }
    if(memoryTypeIn == amf::AMF_MEMORY_DX11)
    {
        res = context->InitDX11(NULL); // can be DX11 device
    }
    // component: encoder
    res = AMFCreateComponent(context, AMFVideoEncoderVCE_AVC, &encoder);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitRateIn);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(widthIn, heightIn));
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(frameRateIn, 1));
	res = encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
	
#if defined(ENABLE_4K)
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);
#endif
	
    res = encoder->Init(formatIn, widthIn, heightIn);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    // create input surface
    PollingThread thread(context, encoder, fileNameOut);
    thread.Start();

    // encode some frames
    amf_int32 submitted = 0;
    while(submitted < frameCount)
    {
        if(surfaceIn == NULL)
        {
            surfaceIn = NULL;
            res = context->AllocSurface(memoryTypeIn, formatIn, widthIn, heightIn, &surfaceIn);
            FillSurface(context, surfaceIn, submitted);
        }
        // encode
        amf_pts start_time = amf_high_precision_clock();
        surfaceIn->SetProperty(START_TIME_PROPERTY, start_time);
        res = encoder->SubmitInput(surfaceIn);
        if(res == AMF_INPUT_FULL) // handle full queue
        {
            amf_sleep(1); // input queue is full: wait, poll and submit again
        }
        else
        {
            surfaceIn = NULL;
            submitted++;
        }
    }
    // drain encoder; input queue can be full
    while(true)
    {
        res = encoder->Drain();
        if(res != AMF_INPUT_FULL) // handle full queue
        {
            break;
        }
        amf_sleep(1); // input queue is full: wait and try again
    }
    thread.WaitForStop();
   
    // clean-up in this order
    surfaceIn = NULL;
    encoder->Terminate();
    encoder = NULL;
    context->Terminate();
    context = NULL; // context is the last

    printf("PASS\n");

    return 0;
}

static void FillSurface(amf::AMFContext *context, amf::AMFSurface *surface, amf_int32 i)
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
		hr = deviceDX9->ColorFill(surfaceDX9, NULL, color1);

		if(xPos + rectSize > widthIn)
		{
			xPos = 0;
		}
		if(yPos + rectSize > heightIn)
		{
			yPos = 0;
		}
		RECT rect = {xPos, yPos, xPos + rectSize, yPos + rectSize};
		hr = deviceDX9->ColorFill(surfaceDX9, &rect, color2);

        xPos+=2; //DX9 NV12 surfaces do not accept odd positions - do not use ++
        yPos+=2; //DX9 NV12 surfaces do not accept odd positions - do not use ++
	}
	else if (surface->GetMemoryType() == amf::AMF_MEMORY_DX11)
    {
		// Swapping two colors across frames
        ID3D11Device *deviceDX11 = (ID3D11Device*)context->GetDX11Device(); // no reference counting - do not Release()
        ID3D11Texture2D *textureDX11 = (ID3D11Texture2D*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
        ID3D11DeviceContext *contextDX11 = NULL;
        ID3D11RenderTargetView *viewDX11 = NULL;
        deviceDX11->GetImmediateContext(&contextDX11);
        hr = deviceDX11->CreateRenderTargetView(textureDX11, NULL, &viewDX11);
        float color1[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
        float color2[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
        contextDX11->ClearRenderTargetView(viewDX11, (i % 2) ? color1 : color2);
        contextDX11->Flush();
        // release temp objects
        viewDX11->Release();
        contextDX11->Release();
    }
}

PollingThread::PollingThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName) : m_pContext(context), m_pEncoder(encoder), m_pFile(NULL)
{
    m_pFile = _wfopen(pFileName, L"wb");
}
PollingThread::~PollingThread()
{
    if(m_pFile)
    {
        fclose(m_pFile);
    }
}
void PollingThread::Run()
{
    RequestStop();

    amf_pts latency_time = 0;
    amf_pts write_duration = 0;
    amf_pts encode_duration = 0;
    amf_pts last_poll_time = 0;

    AMF_RESULT res = AMF_OK; // error checking can be added later
    while(true)
    {
        amf::AMFDataPtr data;
        res = m_pEncoder->QueryOutput(&data);
        if(res == AMF_EOF)
        {
            break; // Drain complete
        }
        if(data != NULL)
        {
            amf_pts poll_time = amf_high_precision_clock();
            amf_pts start_time = 0;
            data->GetProperty(START_TIME_PROPERTY, &start_time);
            if(start_time < last_poll_time ) // remove wait time if submission was faster then encode
            {
                start_time = last_poll_time;
            }
            last_poll_time = poll_time;

            encode_duration += poll_time - start_time;

            if(latency_time == 0)
            {
                latency_time = poll_time - start_time;
            }

            amf::AMFBufferPtr buffer(data); // query for buffer interface
            fwrite(buffer->GetNative(), 1, buffer->GetSize(), m_pFile);
            
            write_duration += amf_high_precision_clock() - poll_time;
        }
        else
        {
            amf_sleep(1);
        }

    }
    printf("latency           = %.4fms\nencode  per frame = %.4fms\nwrite per frame   = %.4fms\n", 
        double(latency_time)/MILLISEC_TIME,
        double(encode_duration )/MILLISEC_TIME/frameCount, 
        double(write_duration)/MILLISEC_TIME/frameCount);

    m_pEncoder = NULL;
    m_pContext = NULL;
}

