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
 * @file <VideoSurveillance.cpp>
 *
 * @brief 
 *
 ********************************************************************************
 */

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <d3d9.h>
#include <d3d11.h>
#include <windowsx.h>
#include <vector>
#include "Debug.h"
#include "VideoDecoderUVD.h"
#include "BitStreamParser.h"
#include "VideoConverter.h"
#include "VideoEncoderVCE.h"
#include "DeviceDX9.h"
#include "DeviceDX11.h"
#include "DeviceOpenCL.h"
#include "VideoPresenter.h"

#define TOTAL_INSTANCES                 (6)
#define MAX_FILENAME_LEN                (500)
#define MAX_CHARS_PER_LINE              (512)
#define MAX_WORDS_PER_LINE              (20)
#define TIMEOUT                         (20)
#define OUTPUT_FRAMERATE                (30)
#define OUTPUT_BITRATE                  (10000000)

static amf::AMF_SURFACE_FORMAT decFormatOut    = amf::AMF_SURFACE_NV12;
static wchar_t *fileNameOut                    = L"./surveillance_output_%dx%d.h264";

#define MILLISEC_TIME     10000

static amf_pts start_time = 0;

const wchar_t szTitle[] = L"Surveillance";
const wchar_t szWindowClass[] = L"Surveillance";

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

class ParsDecThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pDecoder;
    BitStreamParserPtr      m_pParser;
public:
    ParsDecThread(amf::AMFContext *context, BitStreamParserPtr parse, amf::AMFComponent *decoder);
    ~ParsDecThread();
    virtual void Run();
    amf_bool    reachedEOF;
};

class DecConvThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pDecoder;
	amf::AMFComponentPtr    m_pConverter;
public:
    DecConvThread(amf::AMFContext *context, amf::AMFComponent *decoder, amf::AMFComponent *converter);
    ~DecConvThread();
    virtual void Run();
};

typedef AMFQueue<amf::AMFDataPtr> PictQueue;
class ConvToQueueThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pConverter;
	PictQueue              *m_pPutQ;
    
public:
    ConvToQueueThread(amf::AMFContext *context, amf::AMFComponent *converter, PictQueue *queue);
    ~ConvToQueueThread();
    virtual void Run();
};

class QueueToPresenterThread : public AMFThread
{
private:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pEncoder;
    VideoPresenterPtr       m_pPresenter;
	PictQueue              *m_pGetQ[TOTAL_INSTANCES];
    amf_uint32              m_uTotalInstances;
public:
	QueueToPresenterThread(amf::AMFContext *context, VideoPresenterPtr presenter, amf::AMFComponent *encoder, PictQueue queue[], amf_uint32 totalInstances);
	~QueueToPresenterThread();
	virtual void Run();

    amf_bool            m_bRunningStatus;
};

class OutputThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pEncoder;
    FILE                   *m_pFile;
public:
    OutputThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName);
    ~OutputThread();
    virtual void Run();

    amf_bool            m_bRunningStatus;
};

bool parseConfig(amf_int8* configFilePath,
                 amf_int8 fileName[][MAX_FILENAME_LEN],
                 amf_uint32* totalInstances)
{
    const char* const chDelimiter = " ";
    std::ifstream file;
    file.open(configFilePath);
    std::string line;
    amf_uint32 numLines = 0;

    if (!file.good())
    {
        printf("Error in reading the configuration file: %s\n", configFilePath);
        return false;
    }

    /*****************************************************************************
     * Read a line from config file and parse it                                 *
     *****************************************************************************/
    while (!file.eof())
    {
        // read an entire line into memory
        char chBuf[MAX_CHARS_PER_LINE];
        file.getline(chBuf, MAX_CHARS_PER_LINE);

        // parse the line into blank-delimited words
        int tokenIndex = 0;
        int index = 0;
        int argCheck = 0;

        // array to store memory addresses of the words in buf
        const char* token[MAX_WORDS_PER_LINE] = { };

        // parse the line
        token[0] = strtok(chBuf, chDelimiter);
        if (token[0]) // if line is non-blank
        {
            numLines++;
            if (numLines > TOTAL_INSTANCES)
                break;
            for (tokenIndex = 1; tokenIndex < MAX_WORDS_PER_LINE; tokenIndex++)
            {
                token[tokenIndex] = strtok(0, chDelimiter); // subsequent tokens
                if (!token[tokenIndex])
                {
                    break; // no more tokens
                }
            }
        }

        while (index < tokenIndex)
        {
            strcpy(fileName[numLines - 1], token[index]);
            index++;
        }
    }

    file.close();
    *totalInstances = numLines;
    if (*totalInstances > TOTAL_INSTANCES)
    {
        printf("---------------------------------------------------------------------\n");
        printf("NOTE: Supports only max of %d sessions                               \n", TOTAL_INSTANCES);
        printf("---------------------------------------------------------------------\n");
    }
    else
    {
        for(int ii = *totalInstances; ii < TOTAL_INSTANCES; ii++)
        {
            strcpy(fileName[ii], fileName[0]);
        }
    }
    *totalInstances = TOTAL_INSTANCES;
    return true;
}

static amf::AMF_MEMORY_TYPE memoryTypeIn = amf::AMF_MEMORY_DX11;
static amf::AMF_MEMORY_TYPE memoryTypeOut = amf::AMF_MEMORY_DX11;

int main(int argc, char* argv[])
{
    amf::AMFTraceSetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TRACE); 
    ::amf_increase_timer_precision();

    AMF_RESULT              res = AMF_OK; // error checking can be added later
    amf::AMFContextPtr      context;
    amf::AMFComponentPtr    decoder[TOTAL_INSTANCES];
	amf::AMFComponentPtr    encoder;
	amf::AMFComponentPtr    converter[TOTAL_INSTANCES];
    AMFDataStreamPtr        datastream[TOTAL_INSTANCES];
    BitStreamParserPtr      parser[TOTAL_INSTANCES];
    amf_int8                fileNameIn[TOTAL_INSTANCES][MAX_FILENAME_LEN];
    amf_int8                configFile[MAX_FILENAME_LEN];
    amf_int8                memType[50];
    amf_uint32              totalInstances = 0;
    DeviceDX9               deviceDX9;
    DeviceDX11              deviceDX11;
    DeviceOpenCL            deviceOpenCL;
    VideoPresenterPtr		presenter;
    amf_uint32              ii=0;
    WCHAR                   inputWfile[MAX_FILENAME_LEN];
    amf_int32               widthOut;
    amf_int32               heightOut;
	PictQueue               resizerOutQueue[TOTAL_INSTANCES];
    ParsDecThread          *pParsDecThread[TOTAL_INSTANCES];
    DecConvThread          *pDecConvThread[TOTAL_INSTANCES];
    ConvToQueueThread      *pConvQThread[TOTAL_INSTANCES];
    QueueToPresenterThread *pQueuePresThread;
    OutputThread           *pOutputThread;

    if (argc != 3)
    {
        printf("videoSurveillance.exe <Input: config file> <Memory Type: DX9 or DX11>\n");
        printf("FAIL\n");
        return AMF_FAIL;
    }
    else
    {
        strcpy(configFile, argv[1]);
        strcpy(memType, argv[2]);

        if (strcmp(memType, "DX9") == 0)
        {
            memoryTypeIn = amf::AMF_MEMORY_DX9;
            memoryTypeOut = amf::AMF_MEMORY_DX9;
        }
        else if (strcmp(memType, "DX11") == 0)
        {
            memoryTypeIn = amf::AMF_MEMORY_DX11;
            memoryTypeOut = amf::AMF_MEMORY_DX11;
        }
    }

    parseConfig(configFile, fileNameIn, &totalInstances);

    // context
    res = AMFCreateContext(&context);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
    if(memoryTypeOut == amf::AMF_MEMORY_DX9)
    {
        res = deviceDX9.Init(true, 0, false, 1, 1);
        res = context->InitDX9(deviceDX9.GetDevice());
    }
    if(memoryTypeOut == amf::AMF_MEMORY_DX11)
    {
        res = deviceDX11.Init(0, false);
        res = context->InitDX11(deviceDX11.GetDevice());
    }

    res = deviceOpenCL.Init(deviceDX9.GetDevice(), deviceDX11.GetDevice(), NULL, NULL);
    res = context->InitOpenCL(deviceOpenCL.GetCommandQueue());

    for(ii = 0; ii < totalInstances; ii++)
    {
        if(ii == 0)
        {
            widthOut  = 1280;
            heightOut = 720;
        }
        else
        {
            widthOut  = 640;
            heightOut = 360;
        }

		resizerOutQueue[ii].SetQueueSize(10);

        // initialize AMF
        datastream[ii] = AMFDataStream::Create(fileNameIn[ii], AMF_FileRead);
        if(datastream[ii] == NULL)
        {
            wprintf(L"file %s is missing", fileNameIn[ii]);
            return 1;
        }

        MultiByteToWideChar(CP_ACP, 0, fileNameIn[ii], -1, inputWfile, MAX_FILENAME_LEN);

        // initialize AMF
        datastream[ii] = AMFDataStream::Create(inputWfile, AMF_FileRead);
        if(datastream[ii] == NULL)
        {
            wprintf(L"file %s is missing\n", inputWfile);
            return AMF_FAIL;
        }

        // H264 elemntary stream parser from samples common 
        parser[ii] = BitStreamParser::Create(datastream[ii], GetStreamType(inputWfile), context);

        // component: decoder
        res = AMFCreateComponent(context, AMFVideoDecoderUVD_H264_AVC, &decoder[ii]);
        if(res != AMF_OK)
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }
        res = decoder[ii]->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(AMF_TS_DECODE)); // our sample H264 parser provides decode order timestamps - change this depend on demuxer

        if (parser[ii]->GetExtraDataSize()) 
        { // set SPS/PPS extracted from stream or container; Alternatively can use parser->SetUseStartCodes(true)
            amf::AMFBufferPtr buffer;
            context->AllocBuffer(amf::AMF_MEMORY_HOST, parser[ii]->GetExtraDataSize(), &buffer);

            memcpy(buffer->GetNative(), parser[ii]->GetExtraData(), parser[ii]->GetExtraDataSize());
            decoder[ii]->SetProperty(AMF_VIDEO_DECODER_EXTRADATA, amf::AMFVariant(buffer));
        }
        res = decoder[ii]->Init(decFormatOut, parser[ii]->GetPictureWidth(), parser[ii]->GetPictureHeight());
        if(res != AMF_OK)
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }

        // component: converter
        res = AMFCreateComponent(context, AMFVideoConverter, &converter[ii]);
        if(res != AMF_OK)
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }
        res = converter[ii]->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, memoryTypeOut);
        res = converter[ii]->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, amf::AMF_SURFACE_BGRA);
        res = converter[ii]->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_SIZE, ::AMFConstructSize(widthOut, heightOut));
        res = converter[ii]->Init(decFormatOut, parser[ii]->GetPictureWidth(), parser[ii]->GetPictureHeight());
        if(res != AMF_OK)
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }
    }

    // open output file with frame size in file name
    wchar_t fileNameOutWidthSize[2000];
    _swprintf(fileNameOutWidthSize, fileNameOut, 1920, 1080);

    // component: encoder
    res = AMFCreateComponent(context, AMFVideoEncoderVCE_AVC, &encoder);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, OUTPUT_BITRATE);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(1920, 1080));
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(OUTPUT_FRAMERATE, 1));
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
    res = encoder->Init(amf::AMF_SURFACE_BGRA, 1920, 1080);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    // create window
	HINSTANCE hInst;
    // Store the instance handle
    hInst = (HINSTANCE) GetModuleHandle(NULL);

    // Register the window class
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

    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0, 1920, 1080, NULL, NULL, hInst, NULL);
    if(hWnd == 0)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

	// Create Presenter
	presenter = VideoPresenter::Create(memoryTypeIn, hWnd, context);

    for(ii=0; ii<totalInstances; ii++)
    {
        pParsDecThread[ii] = new ParsDecThread(context, parser[ii], decoder[ii]);
        pParsDecThread[ii]->Start();

        pDecConvThread[ii] = new DecConvThread(context, decoder[ii], converter[ii]);
        pDecConvThread[ii]->Start();

        pConvQThread[ii] = new ConvToQueueThread(context, converter[ii], &resizerOutQueue[ii]);
        pConvQThread[ii]->Start();
    }

    pQueuePresThread = new QueueToPresenterThread(context, presenter, encoder, resizerOutQueue, totalInstances);
    pQueuePresThread->m_bRunningStatus = true;
	pQueuePresThread->Start();

    pOutputThread  = new OutputThread(context, encoder, fileNameOutWidthSize);
    pOutputThread->m_bRunningStatus = true;
    pOutputThread->Start();

    // Wait till all decoding threads reach EOF
    amf_bool waitTillEOF = true;
    while (1) {
        for(ii=0; ii<totalInstances; ii++)
        {
            waitTillEOF &= pParsDecThread[ii]->reachedEOF;
        }
        if (waitTillEOF == true)
            break;
        else
            waitTillEOF = true;
        
        amf_sleep(10);
    }

    for(ii=0; ii<totalInstances; ii++)
    {
        // drain decoder queue 
        res = decoder[ii]->Drain();
        pConvQThread[ii]->WaitForStop();
        pDecConvThread[ii]->WaitForStop();
        pParsDecThread[ii]->WaitForStop();
    }

    pQueuePresThread->m_bRunningStatus = false;
    pQueuePresThread->WaitForStop();

    pOutputThread->m_bRunningStatus = false;
    pOutputThread->WaitForStop();

    // cleanup in this order
    for(ii=0; ii<totalInstances; ii++)
    {
	    converter[ii]->Terminate();
        converter[ii] = NULL;
        decoder[ii]->Terminate();
        decoder[ii] = NULL;
        parser[ii] = NULL;
        datastream[ii] = NULL;
    }
    encoder->Terminate();
    encoder = NULL;
    context->Terminate();
    context = NULL; // context is the last

    printf("PASS\n");

	return 0;
}

ParsDecThread::ParsDecThread(amf::AMFContext *context, BitStreamParserPtr parser, amf::AMFComponent *decoder) : m_pContext(context), m_pParser(parser), m_pDecoder(decoder), reachedEOF(false)
{
}
ParsDecThread::~ParsDecThread()
{
}
DecConvThread::DecConvThread(amf::AMFContext *context, amf::AMFComponent *decoder, amf::AMFComponent *converter) : m_pContext(context), m_pDecoder(decoder), m_pConverter(converter)
{
}
DecConvThread::~DecConvThread()
{
}

ConvToQueueThread::ConvToQueueThread(amf::AMFContext *context, amf::AMFComponent *converter, PictQueue *q) : m_pContext(context), m_pConverter(converter), m_pPutQ(q)
{
}
ConvToQueueThread::~ConvToQueueThread()
{
}

QueueToPresenterThread::QueueToPresenterThread(amf::AMFContext *context, VideoPresenterPtr presenter, amf::AMFComponent *encoder, PictQueue queue[], amf_uint32 totalInstances) : m_pPresenter(presenter), m_pContext(context), m_uTotalInstances(totalInstances), m_bRunningStatus(false), m_pEncoder(encoder)
{
	memset(m_pGetQ, 0, sizeof(PictQueue*) * TOTAL_INSTANCES);
	for(int i=0; i<TOTAL_INSTANCES; i++)
	{
		m_pGetQ[i] = &queue[i];
	}
}

QueueToPresenterThread::~QueueToPresenterThread()
{
}

void ParsDecThread::Run()
{
    RequestStop();
	amf::AMFDataPtr data;
    bool bNeedNewInput = true;
	bool firstFrame = true;

    AMF_RESULT res = AMF_OK; // error checking can be added later

    while(true)
    {
        if(bNeedNewInput)
        {
            data = NULL;
            res = m_pParser->QueryOutput(&data); // read compressed frame into buffer
            if(res == AMF_EOF || data == NULL)
            {
                reachedEOF = true;
                break;// end of file
            }
        }

		if (firstFrame) {
			start_time = amf_high_precision_clock();
			firstFrame = false;
		}

		res = m_pDecoder->SubmitInput(data);
        if(res == AMF_INPUT_FULL || res == AMF_DECODER_NO_FREE_SURFACES)
        { // queue is full; sleep, try to get ready surfaces and repeat submission
            bNeedNewInput = false;
            amf_sleep(1); 
        }
        else
        { // submission succeeded. read new buffer from parser
            bNeedNewInput = true;
        }
    }
}

void DecConvThread::Run()
{
    RequestStop();

    AMF_RESULT res = AMF_OK; // error checking can be added later
    while(true)
    {
        amf::AMFDataPtr data;
        res = m_pDecoder->QueryOutput(&data);
        if(res == AMF_EOF)
        {
            res = m_pConverter->Drain();
			break; // Drain complete
        }
        if(data != NULL)
        {
			while(true)
			{
				res = m_pConverter->SubmitInput(data);
				if(res == AMF_INPUT_FULL)
				{
					amf_sleep(1);
				}
				else
				{
					break;
				}
			}
        }
        else
        {
            amf_sleep(1);
        }

    }

    m_pDecoder = NULL;
    m_pContext = NULL;
}

void ConvToQueueThread::Run()
{
    RequestStop();

    AMF_RESULT res = AMF_OK; // error checking can be added later
    while(true)
    {
		amf::AMFDataPtr data;
		res = m_pConverter->QueryOutput(&data);
		if(res == AMF_EOF)
		{
			break;
		}
		if(data != NULL)
		{
            while(!m_pPutQ->Add(0, data, TIMEOUT));
		}
		else
		{
			amf_sleep(1);
		}
    }

    m_pConverter = NULL;
    m_pContext = NULL;
}

void QueueToPresenterThread::Run()
{
    RequestStop();

    amf_ulong id = 0;
	amf::AMFDataPtr data;

    cl_command_queue ocl_queue = (cl_command_queue)m_pContext->GetOpenCLCommandQueue();

    while(true)
    {
		data = NULL;
		amf::AMFSurfacePtr pSurfaceOut;
        cl_int status = m_pContext->AllocSurface(memoryTypeIn, amf::AMF_SURFACE_BGRA, 1920, 1080, &pSurfaceOut);
		if(status != AMF_OK)
		{
			printf("AMFSurfrace::AllocSurface() failed\n");
			return;
		}

		status = pSurfaceOut->Convert(amf::AMF_MEMORY_OPENCL);
		if(status != AMF_OK)
		{
			printf("AMFSurfrace::Convert(amf::AMF_MEMORY_OPENCL) failed\n");
			return;
		}
		
		cl_mem cl_outMem = (cl_mem)pSurfaceOut->GetPlaneAt(0)->GetNative();
		if (!cl_outMem) {
			printf("Error getting cl_mem cl_outMem.\n");
		}

        for(amf_uint32 i = 0; i < m_uTotalInstances; i++)
        {
            amf::AMFDataPtr rgb = NULL;

			if(m_pGetQ[i]->GetSize() == 0)
                continue;

            while(!m_pGetQ[i]->Get(id, rgb, TIMEOUT));

            int xpos = 0;
            int ypos = 0;
			int xres = 0; 
			int yres = 0;

            /* OpenCL blend */
            rgb->Convert(amf::AMF_MEMORY_OPENCL);
            amf::AMFSurfacePtr inSurface(rgb);

            cl_mem cl_inMem = (cl_mem)inSurface->GetPlaneAt(0)->GetNative();
            if (!cl_inMem) {
                printf("Error getting cl_mem cl_inMem.\n");
            }

            xres = inSurface->GetPlaneAt(0)->GetWidth();
            yres = inSurface->GetPlaneAt(0)->GetHeight();

            int out_xres = 1920;
            int out_yres = 1080;

            if(i == 0)
            {
			    ypos = 0;
			    xpos = 0;
            }
            else if(i == 1)
            {
                ypos = 0;
                xpos = (640 * 2);
            }
            else if(i == 2)
            {
                ypos = (360 * 1);
                xpos = (640 * 2);
            }
            else if(i == 3)
            {
                ypos = (360 * 2);
                xpos = 0;
            }
            else if(i == 4)
            {
                ypos = (360 * 2);
                xpos = (640 * 1);
            }
            else if(i == 5)
            {
                ypos = (360 * 2);
                xpos = (640 * 2);
            }

            size_t src_origin[3] = {0, 0, 0};
            size_t dst_origin[3] = {xpos, ypos, 0};
            size_t region[3] = {xres, yres, 1};

            status = clEnqueueCopyImage(ocl_queue, cl_inMem, cl_outMem, src_origin, dst_origin, region, 0, NULL, NULL); 
            if(status != 0)
            {
                printf("error in clEnqueueCopyImage\n");
            }
		}

        status = clFinish(ocl_queue);

		m_pPresenter->SubmitInput(pSurfaceOut);

		amf::AMFDataPtr duplicate;
		pSurfaceOut->Duplicate(pSurfaceOut->GetMemoryType(), &duplicate);
        if(duplicate != nullptr)
		{
			AMF_RESULT res = m_pEncoder->SubmitInput(duplicate);
			if(res == AMF_INPUT_FULL)
			{
				amf_sleep(1);
			}
        }

        if(m_bRunningStatus == false)
            break;
	}
}

OutputThread::OutputThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName) : m_pContext(context), m_pEncoder(encoder), m_pFile(NULL)
{
    m_pFile = _wfopen(pFileName, L"wb");
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
	amf_int32 frameCount = 0;
	amf_pts latency_time = 0;
    amf_pts write_duration = 0;
    amf_pts transcode_duration = 0;
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

			transcode_duration += curr_time - last_poll_time;

			if(latency_time == 0)
			{
				latency_time = curr_time - start_time;
			}
			frameCount++;

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

        if(m_bRunningStatus == false)
            break;
    }

	printf("latency           = %.4fms\ndecode+blend(OpenCL)+encode  per frame = %.4fms\nwrite per frame   = %.4fms\n", 
	double(latency_time)/MILLISEC_TIME,
	double(transcode_duration )/MILLISEC_TIME/frameCount, 
	double(write_duration)/MILLISEC_TIME/frameCount);

    m_pEncoder = NULL;
    m_pContext = NULL;
}
