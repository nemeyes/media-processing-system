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
 * @file <main.cpp>
 *
 * @brief This sample showcases H264 to H264 transcoding, first H264 ES is
 *        decoded by using UVD, Sobel filtering is done on decoded luma pixels
 *        using OpenCL (chroma is filled with constant) and encoded back to H264
 *        using VCE.
 *
 ********************************************************************************
 */

#include <stdio.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3d11.h>
#include "DeviceDx9.h"
#include "DeviceDx11.h"
#include "DeviceOpenCL.h"
#include "Debug.h"
#include "VideoDecoderUVD.h"
#include "BitStreamParser.h"
#include "VideoEncoderVCE.h"
#include "OpenCLVideoFilter.h"

static amf::AMF_SURFACE_FORMAT formatOut    = amf::AMF_SURFACE_NV12;

static amf_int32 frameRateOut				= 30;
static amf_int64 bitRateOut                 = 1000000L; // in bits, 1MB

#define MILLISEC_TIME     10000

static amf_pts start_time = 0;

class FilterThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pDecoder;
	OpenCLVideoFilterPtr    videoFilter;
	amf::AMFComponentPtr    m_pEncoder;
public:
    FilterThread(amf::AMFContext *context, amf::AMFComponent *decoder, OpenCLVideoFilterPtr videoFilter, amf::AMFComponent *encoder);
    ~FilterThread();
    virtual void Run();
};

class OutputThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pEncoder;
    FILE                    *m_pFile;
public:
    OutputThread(amf::AMFContext *context, amf::AMFComponent *encoder, const wchar_t *pFileName);
    ~OutputThread();
    virtual void Run();
};

int _tmain(int argc, _TCHAR* argv[])
{
    amf::AMFTraceSetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TRACE); 
    ::amf_increase_timer_precision();

    AMF_RESULT              res = AMF_OK; // error checking can be added later
    amf::AMFContextPtr      context;
    amf::AMFComponentPtr    decoder;
	amf::AMFComponentPtr    encoder;
	OpenCLVideoFilterPtr    videoFilter;
    AMFDataStreamPtr        datastream;
    BitStreamParserPtr      parser;
	DeviceDX9               deviceDX9;
    DeviceDX11              deviceDX11;
    DeviceOpenCL            deviceOpenCL;
	amf_int32				width, height;

    static wchar_t fileNameIn[500];
	static wchar_t fileNameOut[500];

	static amf::AMF_MEMORY_TYPE memoryTypeIn = amf::AMF_MEMORY_DX9;
    static amf::AMF_MEMORY_TYPE memoryTypeOut = amf::AMF_MEMORY_DX9;
    static wchar_t memType[50];

    if (argc < 4)
    {
        printf("openCLVideoFilter.exe <input H.264 Elementary Stream> <output H.264 Elementary Stream> <Memory Type: DX9 or DX11>\n");
        printf("FAIL\n");
        return AMF_FAIL;
    }
    else
    {
        wcscpy(fileNameIn, argv[1]);
        wcscpy(fileNameOut, argv[2]);
		wcscpy(memType, argv[3]);

		if (wcscmp(memType, L"DX9") == 0)
		{
			memoryTypeIn = amf::AMF_MEMORY_DX9;
			memoryTypeOut = amf::AMF_MEMORY_DX9;
		}
		else if (wcscmp(memType, L"DX11") == 0)
		{
			memoryTypeIn = amf::AMF_MEMORY_DX11;
			memoryTypeOut = amf::AMF_MEMORY_DX11;
		}
    }

    // initialize AMF
    datastream = AMFDataStream::Create(fileNameIn, AMF_FileRead);
    if(datastream == NULL)
    {
        wprintf(L"file %s is missing", fileNameIn);
        printf("FAIL\n");
        return AMF_FAIL;
    }

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

    // H264 elemntary stream parser from samples common 
    parser = BitStreamParser::Create(datastream, GetStreamType(fileNameIn), context);

    // open output file with frame size in file name
    wchar_t fileNameOutWidthSize[2000];
    _swprintf(fileNameOutWidthSize, fileNameOut, parser->GetPictureWidth(), parser->GetPictureHeight());

    // component: decoder
    res = AMFCreateComponent(context, AMFVideoDecoderUVD_H264_AVC, &decoder);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
    res = decoder->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(AMF_TS_DECODE)); // our sample H264 parser provides decode order timestamps - change this depend on demuxer

    if (parser->GetExtraDataSize()) 
    { // set SPS/PPS extracted from stream or container; Alternatively can use parser->SetUseStartCodes(true)
        amf::AMFBufferPtr buffer;
        context->AllocBuffer(amf::AMF_MEMORY_HOST, parser->GetExtraDataSize(), &buffer);

        memcpy(buffer->GetNative(), parser->GetExtraData(), parser->GetExtraDataSize());
        decoder->SetProperty(AMF_VIDEO_DECODER_EXTRADATA, amf::AMFVariant(buffer));
    }

	width = parser->GetPictureWidth();
    height = parser->GetPictureHeight();

    res = decoder->Init(formatOut, width, height);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
	
	// OpenCL component
    videoFilter = OpenCLVideoFilterPtr(new OpenCLVideoFilter);
    res = videoFilter->Init(context, memoryTypeOut, width, height);	
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    // component: encoder
    res = AMFCreateComponent(context, AMFVideoEncoderVCE_AVC, &encoder);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitRateOut);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(width, height));
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(frameRateOut, 1));
	res = encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
    res = encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
    res = encoder->Init(formatOut, width, height);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    FilterThread filterThread(context, decoder, videoFilter, encoder);
    filterThread.Start();

    OutputThread outThread(context, encoder, fileNameOutWidthSize);
    outThread.Start();
	
	amf::AMFDataPtr data;
    bool bNeedNewInput = true;
	bool firstFrame = true;

    while(true)
    {
        if(bNeedNewInput)
        {
            data = NULL;
            res = parser->QueryOutput(&data); // read compressed frame into buffer
            if(res == AMF_EOF || data == NULL)
            {
                break;// end of file
            }
        }

		if (firstFrame) {
			start_time = amf_high_precision_clock();
			firstFrame = false;
		}

		res = decoder->SubmitInput(data);
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

    // drain decoder queue 
    res = decoder->Drain();
	outThread.WaitForStop();
    filterThread.WaitForStop();

    // cleanup in this order
    data = NULL;
	encoder->Terminate();
    encoder = NULL;
	videoFilter->Terminate();
    videoFilter = NULL;
    decoder->Terminate();
    decoder = NULL;
    parser = NULL;
    datastream = NULL;
    context->Terminate();
    context = NULL; // context is the last

    printf("PASS\n");

	return 0;
}


FilterThread::FilterThread(amf::AMFContext *context, amf::AMFComponent *decoder, OpenCLVideoFilterPtr videoFilter, amf::AMFComponent *encoder) : m_pContext(context), m_pDecoder(decoder), videoFilter(videoFilter), m_pEncoder(encoder)
{
}
FilterThread::~FilterThread()
{
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

void FilterThread::Run()
{
    RequestStop();

    AMF_RESULT res = AMF_OK; // error checking can be added later
	amf::AMFDataPtr pFiltData;
	
    while(true)
    {
        amf::AMFDataPtr data;
        res = m_pDecoder->QueryOutput(&data);
        if(res == AMF_EOF)
        {
			while(true)
			{
				// Drain the encoder
				res = m_pEncoder->Drain();
				if(res != AMF_INPUT_FULL) // handle full queue
				{
					break;
				}
				amf_sleep(1); // input queue is full: wait and try again
			}
			break; // Drain complete
        }
		
		if(data != NULL)
        {
			res = videoFilter->Process(data, &pFiltData);
        }

        if(pFiltData != NULL)
        {
			while(true)
			{
				res = m_pEncoder->SubmitInput(pFiltData);
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

	videoFilter = NULL;
    m_pDecoder = NULL;
	m_pEncoder = NULL;
    m_pContext = NULL;
}

void OutputThread::Run()
{
    RequestStop();

    AMF_RESULT res = AMF_OK; // error checking can be added later
	amf_int32 frameCount = 0;
	amf_pts latency_time = 0;
    amf_pts write_duration = 0;
    amf_pts filter_duration = 0;
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

			filter_duration += curr_time - last_poll_time;

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
    }

	printf("latency           = %.4fms\nTranscoding + Filtering  per frame = %.4fms\nwrite per frame   = %.4fms\n", 
	double(latency_time)/MILLISEC_TIME,
	double(filter_duration )/MILLISEC_TIME/frameCount, 
	double(write_duration)/MILLISEC_TIME/frameCount);

    m_pEncoder = NULL;
    m_pContext = NULL;
}
