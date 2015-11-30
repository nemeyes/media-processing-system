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
 * @file <D3DMultiEncoder.cpp>
 *
 * @brief This sample encodes BGRA frames using AMF Encoder and writes them 
 *        to H.264 elementary stream - Multi Instances across Multi GPUs
 *
 ********************************************************************************
 */

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <d3d9.h>
#include <d3d11.h>
#include "Debug.h"
#include "VideoEncoderVCE.h"
#include "DeviceDX9.h"
#include "DeviceDX11.h"
#include "AMFPlatform.h"
#include "PlatformWindows.h"
#include "Thread.h"
#include "VideoPresenter.h"
#include <windowsx.h>
#include <vector>

#define MAX_NUM_OF_SESSIONS             10
#define MILLISEC_TIME                   10000
#define MAX_FILENAME_LEN                500

static amf::AMF_MEMORY_TYPE memoryTypeIn  = amf::AMF_MEMORY_DX9;
static amf::AMF_SURFACE_FORMAT formatIn   = amf::AMF_SURFACE_BGRA;
static amf_int32 rectSize                 = 50;
static amf_int32 frameCount               = 300;
static amf_pts start_time                 = 0;
const wchar_t szTitle[]                   = L"Multi Encoding";
const wchar_t szWindowClass[]             = L"Multi Encoding Sample";

static void FillSurface(amf::AMFContext *context, amf::AMFSurface *surface, amf_int32 i);

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT)
{
    return TRUE;
}

BOOL OnClose(HWND hwnd)
{
    return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hWnd, WM_CLOSE, OnClose);
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

class OutputThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pEncoder;
    FILE                    *m_pFile;
    amf_int32               m_iWidth;
    amf_int32               m_iHeight;
    amf_int32               m_iSessionNo;
public:
    OutputThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName, amf_int32 sessionNo);
    ~OutputThread();
    virtual void Run();
};

class EncodingSession : public AMFThread
{
protected:
    DeviceDX9               m_deviceDX9;
    DeviceDX11              m_deviceDX11;
    amf::AMFContextPtr      m_pContext;
	amf::AMFComponentPtr    m_pEncoder;
	VideoPresenterPtr		m_pPresenter;
    OutputThread*           m_ptOutputThread;
    amf_int32               m_iDeviceID;
    wchar_t                 m_fOutWidthSize[2000];

public:
    EncodingSession(amf_uint32 sessionNo, amf_uint32 present);
    ~EncodingSession();
    virtual void Run();
    AMF_RESULT InitSession();
    void CloseSession();
	void FillSurface(amf::AMFContext *context, amf::AMFSurface *surface, amf_int32 i);

    amf_int32               m_iSessionNo;
    amf_bool                m_bRunningStatus;
	amf_uint32				m_presentInput;
	amf_int32				m_width;
	amf_int32				m_height;
	amf_int32 				xPos;
	amf_int32 				yPos;	
};

int main(int argc, char* argv[])
{
    amf::AMFTraceSetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TRACE); 
    ::amf_increase_timer_precision();
    
    AMF_RESULT res = AMF_OK; // error checking can be added later

    //static amf::AMF_MEMORY_TYPE memoryTypeIn = amf::AMF_MEMORY_DX9;
    amf_int8	memType[50];
	amf_uint32  sessionCount;
	amf_uint32	presentInput;
	amf_uint32  ii=0;
    DeviceDX9   deviceDX9;
    DeviceDX11  deviceDX11;

    if ((argc != 4))
    {
        printf("D3DMultiEncoder.exe <In Memory Type: DX9 or DX11> <No. of sessions> <Present the input: 0 and 1>\n");
        return 1;
    }
    
    strcpy(memType, argv[1]);

    if (strcmp(memType, "DX9") == 0)
    {
        memoryTypeIn = amf::AMF_MEMORY_DX9;
    }
    else if (strcmp(memType, "DX11") == 0)
    {
        memoryTypeIn = amf::AMF_MEMORY_DX11;
    }
    else
    {
        printf("Incorrect Input memory Type.  Supported values : DX9 or DX11\n");
        printf("FAIL\n");
        return AMF_FAIL;
    }
		
	sessionCount = atoi(argv[2]);
    if (sessionCount > MAX_NUM_OF_SESSIONS)
    {
        printf("---------------------------------------------------------------------\n");
        printf("NOTE: Supports only max of %d sessions                               \n", MAX_NUM_OF_SESSIONS);
        printf("---------------------------------------------------------------------\n");
        sessionCount = MAX_NUM_OF_SESSIONS;
    }
    
	presentInput = atoi(argv[3]);
	if (presentInput < 0 || presentInput > 1)
    {
        printf("Incorrect value of Present Input.  Supported values : 0 or 1\n");
        printf("FAIL\n");
        return AMF_FAIL;
    }

    std::vector<EncodingSession*> threads;
    for(ii = 0; ii < sessionCount; ii++)
    {
        EncodingSession *session = new EncodingSession(ii, presentInput);
        if(res == AMF_OK)
        {
            threads.push_back(session);
        }
        else
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }
    }

    ii=0;
    for(std::vector<EncodingSession*>::iterator it = threads.begin(); it != threads.end(); it++)
    {
        res = (*it)->InitSession();
        if(res != AMF_OK)
        {
            printf("Init Session is failed\n");
            printf("FAIL\n");
            return AMF_FAIL;
        }
        ii++;
    }

    ii=0;
    for(std::vector<EncodingSession*>::iterator it = threads.begin(); it != threads.end(); it++)
    {
        (*it)->Start();
        printf("Session %d: started\n", ii++);
    }

    for(std::vector<EncodingSession*>::iterator it = threads.begin(); it != threads.end(); it++)
    {
        while((*it)->m_bRunningStatus)
        {
            amf_sleep(10);
        }
    }

    ii=0;
    for(std::vector<EncodingSession*>::iterator it = threads.begin(); it != threads.end(); it++)
    {
        (*it)->WaitForStop();
        delete (*it);
        (*it) = NULL;

        printf("Session %d: ended\n", ii++);
    }

    printf("PASS\n");
	return 0;
}

void EncodingSession::FillSurface(amf::AMFContext *context, amf::AMFSurface *surface, amf_int32 i)
{
    HRESULT hr = S_OK;
    // fill surface with something something useful. We fill with color and color rect
	if (surface->GetMemoryType() == amf::AMF_MEMORY_DX9)
	{
		D3DCOLOR color1 = D3DCOLOR_XYUV (0, 0, 128);
		D3DCOLOR color2 = D3DCOLOR_XYUV (255, 0, 255);
		// get native DX objects
		IDirect3DDevice9 *deviceDX9 = (IDirect3DDevice9 *)context->GetDX9Device(); // no reference counting - do not Release()
		IDirect3DSurface9* surfaceDX9 = (IDirect3DSurface9*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
		hr = deviceDX9->ColorFill(surfaceDX9, NULL, color1);

		if(xPos + rectSize > m_width)
		{
			xPos = 0;
		}
		if(yPos + rectSize > m_height)
		{
			yPos = 0;
		}
		RECT rect = {xPos, yPos, xPos + rectSize, yPos + rectSize};
		hr = deviceDX9->ColorFill(surfaceDX9, &rect, color2);

		xPos+=20; //DX9 NV12 surfaces do not accept odd positions - do not use ++
		yPos+=20; //DX9 NV12 surfaces do not accept odd positions - do not use ++
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

OutputThread::OutputThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName, amf_int32 sesNo) : 
    m_pContext(context), m_pEncoder(encoder), m_pFile(NULL), m_iSessionNo(sesNo)
{
    AMFSize size;
    m_pFile = _wfopen(pFileName, L"wb");

    encoder->GetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, &size);
    m_iWidth = size.width;
    m_iHeight = size.height;
}

OutputThread::~OutputThread()
{
    if(m_pFile)
    {
        fclose(m_pFile);
    }
}

void OutputThread::Run()
{
    RequestStop();

    AMF_RESULT res = AMF_OK; // error checking can be added later

    amf_int32 outFrameCount = 0;
	amf_pts latency_time = 0;
    amf_pts write_duration = 0;
    amf_pts encode_duration = 0;
	amf_pts last_poll_time = 0;

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
			amf_pts curr_time = amf_high_precision_clock();

			encode_duration += curr_time - last_poll_time;

			if(latency_time == 0)
			{
				latency_time = curr_time - start_time;
			}
			outFrameCount++;

			// this operation is slow nneed to remove it from stat
            res = data->Convert(amf::AMF_MEMORY_HOST); // convert to system memory

			amf::AMFBufferPtr buffer(data); // query for buffer interface
			fwrite(buffer->GetNative(), 1, buffer->GetSize(), m_pFile);

			amf_pts timePostWrite = amf_high_precision_clock();
			write_duration += timePostWrite - curr_time;

			//  Set timer
			last_poll_time = timePostWrite;
        }
        else
        {
            amf_sleep(1);
        }
    }

    printf("\n-------------------------------------------------------------------\n");
    printf("Session # %d: D3DMultiEncoder to %dx%d\n", m_iSessionNo, m_iWidth, m_iHeight);
    printf("-------------------------------------------------------------------\n");
	printf("latency           = %.4fms\nencode  per frame = %.4fms\nwrite per frame   = %.4fms\ntotal frames = %d\n", 
	double(latency_time)/MILLISEC_TIME,
	double(encode_duration )/MILLISEC_TIME/outFrameCount, 
    double(write_duration)/MILLISEC_TIME/outFrameCount, outFrameCount);
    printf("-------------------------------------------------------------------\n\n");

    m_pEncoder = NULL;
    m_pContext = NULL;
}

AMF_RESULT EncodingSession::InitSession()
{
    AMF_RESULT res = AMF_OK;

    amf_int32 frameRateOut;
    amf_int64 bitRateOut;
	static wchar_t *fileNameOut = L"./output_%dx%d_session%d.h264";
	amf_uint32 numAdapters;

    m_bRunningStatus = true;
	xPos = 0;
	yPos = 0;

    // context
    res = AMFCreateContext(&m_pContext);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    if(memoryTypeIn == amf::AMF_MEMORY_DX9)
    {
		// Get no. of devices
		m_deviceDX9.GetAdapterCount(&numAdapters);
		m_iDeviceID = m_iSessionNo % numAdapters;

        res = m_deviceDX9.Init(true, m_iDeviceID, false, 1, 1);
        res = m_pContext->InitDX9(m_deviceDX9.GetDevice());
    }
    if(memoryTypeIn == amf::AMF_MEMORY_DX11)
    {
		// Get no. of devices
		m_deviceDX11.GetAdapterCount(&numAdapters);
		m_iDeviceID = m_iSessionNo % numAdapters;

        res = m_deviceDX11.Init(m_iDeviceID, false);
        res = m_pContext->InitDX11(m_deviceDX11.GetDevice());
    }

	if(m_iSessionNo % 2 == 0)
	{
		m_width  = 1920;
		m_height = 1080;
        frameRateOut = 30;
        bitRateOut = 10000000;
	}
	else
	{
		m_width  = 1280;
		m_height = 720;
        frameRateOut = 30;
        bitRateOut = 4000000;
	}

	// open output file with frame size in file name
    _swprintf(m_fOutWidthSize, fileNameOut, m_width, m_height, m_iSessionNo);

	// component: encoder
	res = AMFCreateComponent(m_pContext, AMFVideoEncoderVCE_AVC, &m_pEncoder);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitRateOut);
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(m_width, m_height));
	res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(frameRateOut, 1));
    res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
    res = m_pEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
	res = m_pEncoder->Init(formatIn, m_width, m_height);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

	if (m_presentInput) {
		HINSTANCE hInst;
		/**************************************************************************
		 * Store the instance handle                                              *
		 **************************************************************************/
		hInst = (HINSTANCE) GetModuleHandle(NULL);

		/**************************************************************************
		 * Register the window class                                              *
		 **************************************************************************/
		WNDCLASSEX wcex;
		ZeroMemory(&wcex, sizeof(WNDCLASSEX));
		wcex.cbSize        = sizeof(WNDCLASSEX);
		wcex.style         = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc   = WindowProc;
		wcex.hInstance     = hInst;
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName  = NULL;
		wcex.lpszClassName = szWindowClass;

		if(RegisterClassEx(&wcex) == 0)
		{
		   // return AMF_FAIL;
		}

		HWND mhWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 10, 10, /*CW_USEDEFAULT, 0,*/ m_width, m_height,/*CW_USEDEFAULT, 0,*/ NULL, NULL, hInst, NULL);
		if(mhWnd == 0)
		{
            printf("FAIL\n");
            return AMF_FAIL;
		}
		ShowWindow(mhWnd, SW_SHOWDEFAULT);
		UpdateWindow(mhWnd);

		// Create Presenter
		m_pPresenter = VideoPresenter::Create(memoryTypeIn, mhWnd, m_pContext);
	}

    return res;
}

EncodingSession::EncodingSession(amf_uint32 sessonNo, amf_uint32 presentIn) :
    m_iSessionNo(sessonNo), m_presentInput(presentIn)
{
}

EncodingSession::~EncodingSession()
{
}

void EncodingSession::Run()
{
    RequestStop();
    AMF_RESULT res = AMF_OK;

    m_ptOutputThread = new OutputThread(m_pContext, m_pEncoder, m_fOutWidthSize, m_iSessionNo);
	m_ptOutputThread->Start();

	amf::AMFSurfacePtr surfaceIn;
	bool firstFrame = true;
    amf_int32 submitted = 0;

    while(submitted < frameCount)
    {
        if(surfaceIn == NULL)
        {
            surfaceIn = NULL;
            res = m_pContext->AllocSurface(memoryTypeIn, formatIn, m_width, m_height, &surfaceIn);
            FillSurface(m_pContext, surfaceIn, submitted);
        }
		
		if (firstFrame) {
			start_time = amf_high_precision_clock();
			firstFrame = false;
		}

		// Duplicate and submit to presenter
		if (m_presentInput) {
			amf::AMFDataPtr pDuplicated;
			surfaceIn->Duplicate(memoryTypeIn, &pDuplicated);
			m_pPresenter->SubmitInput(pDuplicated);
		}

        res = m_pEncoder->SubmitInput(surfaceIn);
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
        res = m_pEncoder->Drain();
        if(res != AMF_INPUT_FULL) // handle full queue
        {
            break;
        }
        amf_sleep(1); // input queue is full: wait and try again
    }

	m_ptOutputThread->WaitForStop();

	// cleanup in this order
	m_pEncoder->Terminate();
	m_pEncoder = NULL;
	surfaceIn = NULL;

    m_bRunningStatus = false;
}

void EncodingSession::CloseSession()
{
    m_pContext->Terminate();
    m_pContext = NULL; // context is the last
}
