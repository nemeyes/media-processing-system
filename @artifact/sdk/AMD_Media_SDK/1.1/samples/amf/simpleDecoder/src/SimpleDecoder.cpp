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
 * @file <SimpleDecoder.cpp>
 *
 * @brief This sample decodes H.264 elementary stream to NV12 frames using 
 *        AMF Decoder and writes the frames into raw file 
 *
 ********************************************************************************
 */

#include <stdio.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3d11.h>
#include "Debug.h"
#include "VideoDecoderUVD.h"
#include "BitStreamParser.h"

static wchar_t *fileNameOut                 = L"./output_%dx%d.nv12";
static amf::AMF_SURFACE_FORMAT formatOut    = amf::AMF_SURFACE_NV12;

static void WritePlane(amf::AMFPlane *plane, FILE *f);
static void WaitDecoder(amf::AMFContext *context, amf::AMFSurface *surface); // Waits till decoder finishes decode the surface. Need for accurate profiling only. Do not use in the product!!!
static bool bWriteToFile = false;
#define START_TIME_PROPERTY L"StartTimeProperty" // custom property ID to store submission time in a frame - all custom properties are copied from input to output
#define MILLISEC_TIME     10000
class PollingThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pDecoder;
    FILE                    *m_pFile;
public:
    PollingThread(amf::AMFContext *context, amf::AMFComponent *decoder, const wchar_t *pFileName);
    ~PollingThread();
    virtual void Run();
};

int _tmain(int argc, _TCHAR* argv[])
{
    amf::AMFTraceSetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TRACE); 
    ::amf_increase_timer_precision();

    AMF_RESULT              res = AMF_OK; // error checking can be added later
    amf::AMFContextPtr      context;
    amf::AMFComponentPtr    decoder;
    AMFDataStreamPtr        datastream;
    BitStreamParserPtr      parser;
    static wchar_t fileNameIn[500];
    static amf::AMF_MEMORY_TYPE memoryTypeOut = amf::AMF_MEMORY_DX9;
    static wchar_t memType[50];
	wchar_t writeOutput[10];

    if (argc != 4)
    {
        printf("simpleDecoder.exe <input H.264 Elementary Stream> <Out Memory Type: DX9 or DX11> <Write to File: True or False>\n");
        printf("FAIL\n");
        return AMF_FAIL;
    }
    else
    {
        wcscpy(fileNameIn, argv[1]);
        if (argc == 4) {
            wcscpy(memType, argv[2]);

            if (wcscmp(memType, L"DX9") == 0)
            {
                memoryTypeOut = amf::AMF_MEMORY_DX9;
            }
            else if (wcscmp(memType, L"DX11") == 0)
            {
                memoryTypeOut = amf::AMF_MEMORY_DX11;
            }
            else
            {
                printf("Incorrect Output memory Type.  Supported values : DX9 or DX11\n");
                return 1;
            }
			wcscpy(writeOutput, argv[3]);
			bWriteToFile = writeOutput && wcsicmp(writeOutput, L"true") == 0;
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
        res = context->InitDX9(NULL); // can be DX9 or DX9Ex device
    }
    if(memoryTypeOut == amf::AMF_MEMORY_DX11)
    {
        res = context->InitDX11(NULL); // can be DX11 device
    }
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
    res = decoder->Init(formatOut, parser->GetPictureWidth(), parser->GetPictureHeight());
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    PollingThread thread(context, decoder, fileNameOutWidthSize);
    thread.Start();
    amf::AMFDataPtr data;
    bool bNeedNewInput = true;
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
        amf_pts start_time = amf_high_precision_clock();
        data->SetProperty(START_TIME_PROPERTY, start_time);
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
    thread.WaitForStop();

    // cleanup in this order
    data = NULL;
    decoder->Terminate();
    decoder = NULL;
    parser = NULL;
    datastream = NULL;
    context->Terminate();
    context = NULL; // context is the last

    printf("PASS\n");

	return 0;
}

        

static void WritePlane(amf::AMFPlane *plane, FILE *f)
{
    // write NV12 surface removing offsets and alignments
    amf_uint8 *data     = reinterpret_cast<amf_uint8*>(plane->GetNative());
    amf_int32 offsetX   = plane->GetOffsetX();
    amf_int32 offsetY   = plane->GetOffsetY();
    amf_int32 pixelSize = plane->GetPixelSizeInBytes();
    amf_int32 height    = plane->GetHeight();
    amf_int32 width     = plane->GetWidth();
    amf_int32 pitchH    = plane->GetHPitch();

    for( amf_int32 y = 0; y < height; y++)
    {
        amf_uint8 *line = data + (y + offsetY) * pitchH;
        fwrite(line + offsetX * pixelSize, pixelSize, width, f);
    }
}
// Waits till decoder finishes decode the surface. Need for accurate profiling only. Do not use in the product!!!
static void WaitDecoder(amf::AMFContext *context, amf::AMFSurface *surface) 
{
    // copy of four pixels will force DX to wait for UVD decoder and will not add a significant delay
    HRESULT hr = S_OK;
    amf::AMFSurfacePtr outputSurface;
    context->AllocSurface(surface->GetMemoryType(), surface->GetFormat(), 2, 2, &outputSurface); // NV12 must be devisible by 2

    switch(surface->GetMemoryType())
    {
    case amf::AMF_MEMORY_DX9:
        {
            IDirect3DDevice9 *deviceDX9 = (IDirect3DDevice9 *)context->GetDX9Device(); // no reference counting - do not Release()
            IDirect3DSurface9* surfaceDX9src = (IDirect3DSurface9*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
            IDirect3DSurface9* surfaceDX9dst = (IDirect3DSurface9*)outputSurface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
            RECT rect = {0, 0, 2, 2};
            // a-sync copy
            hr = deviceDX9->StretchRect(surfaceDX9src,&rect ,surfaceDX9dst, &rect, D3DTEXF_NONE);
            // wait
            outputSurface->Convert(amf::AMF_MEMORY_HOST);
        }
        break;
    case amf::AMF_MEMORY_DX11:
        {
            ID3D11Device *deviceDX11 = (ID3D11Device*)context->GetDX11Device(); // no reference counting - do not Release()
            ID3D11Texture2D *textureDX11src = (ID3D11Texture2D*)surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
            ID3D11Texture2D *textureDX11dst = (ID3D11Texture2D*)outputSurface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
            ID3D11DeviceContext *contextDX11 = NULL;
            deviceDX11->GetImmediateContext(&contextDX11);
            D3D11_BOX srcBox = {0, 0, 0, 2, 2, 1};
            contextDX11->CopySubresourceRegion(textureDX11dst, 0, 0, 0, 0, textureDX11src, 0, &srcBox);
            contextDX11->Flush();
            // release temp objects
            contextDX11->Release();
            outputSurface->Convert(amf::AMF_MEMORY_HOST);

        }
        break;
    }
}

PollingThread::PollingThread(amf::AMFContext *context, amf::AMFComponent *decoder, const wchar_t *pFileName) : m_pContext(context), m_pDecoder(decoder), m_pFile(NULL)
{
    if(bWriteToFile)
    {
        m_pFile = _wfopen(pFileName, L"wb");
    }
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
    amf_pts convert_duration = 0;
    amf_pts write_duration = 0;
    amf_pts decode_duration = 0;
    amf_pts last_poll_time = 0;
	amf_int32 frameCount = 0;

    AMF_RESULT res = AMF_OK; // error checking can be added later
    while(true)
    {
        amf::AMFDataPtr data;
        res = m_pDecoder->QueryOutput(&data);
        if(res == AMF_EOF)
        {
            break; // Drain complete
        }
        if(data != NULL)
        {
            WaitDecoder(m_pContext, amf::AMFSurfacePtr(data)); // Waits till decoder finishes decode the surface. Need for accurate profiling only. Do not use in the product!!!
			frameCount++;

            amf_pts poll_time = amf_high_precision_clock();
            amf_pts start_time = 0;
            data->GetProperty(START_TIME_PROPERTY, &start_time);
            if(start_time < last_poll_time ) // correct if submission was faster then decode
            {
                start_time = last_poll_time;
            }
            last_poll_time = poll_time;

            decode_duration += poll_time - start_time;

            if(latency_time == 0)
            {
                latency_time = poll_time - start_time;
            }
            if(bWriteToFile)
            {
                // this operation is slow nneed to remove it from stat
                res = data->Convert(amf::AMF_MEMORY_HOST); // convert to system memory

                amf_pts convert_time = amf_high_precision_clock();
                convert_duration += convert_time - poll_time;

                amf::AMFSurfacePtr surface(data); // query for surface interface
    
                WritePlane(surface->GetPlane(amf::AMF_PLANE_Y), m_pFile); // get y-plane pixels
                WritePlane(surface->GetPlane(amf::AMF_PLANE_UV), m_pFile); // get uv-plane pixels
            
                write_duration += amf_high_precision_clock() - convert_time;
            }
        }
        else
        {
            amf_sleep(1);
        }

    }
    printf("latency           = %.4fms\ndecode  per frame = %.4fms\nconvert per frame = %.4fms\nwrite per frame   = %.4fms\n", 
        double(latency_time)/MILLISEC_TIME,
        double(decode_duration )/MILLISEC_TIME/frameCount, 
        double(convert_duration )/MILLISEC_TIME/frameCount, 
        double(write_duration)/MILLISEC_TIME/frameCount);

    m_pDecoder = NULL;
    m_pContext = NULL;
}
