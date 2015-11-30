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
 * @file <MultiBatchTranscoder.cpp>
 *
 * @brief This sample showcases H264 to H264 Batch transcoding with multi-session
 *        support, each batch transcode session runs on user configured device
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
#include "VideoDecoderUVD.h"
#include "BitStreamParser.h"
#include "VideoConverter.h"
#include "VideoEncoderVCE.h"
#include "DeviceDX9.h"
#include "DeviceDX11.h"

#define MAX_NUM_OF_SESSIONS             4
#define TOTAL_TRANSCODES_PER_SESSION    3

static amf::AMF_MEMORY_TYPE memoryTypeIn  = amf::AMF_MEMORY_DX9;
static amf::AMF_MEMORY_TYPE memoryTypeOut = amf::AMF_MEMORY_DX9;

#define MILLISEC_TIME                   10000
#define MAX_CHARS_PER_LINE              512
#define MAX_WORDS_PER_LINE              50
#define MAX_FILENAME_LEN                500

static amf_pts start_time = 0;

class DecConvThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pDecoder;
    amf::AMFComponentPtr    m_pConverter[TOTAL_TRANSCODES_PER_SESSION];
public:
    DecConvThread(amf::AMFContext *context, amf::AMFComponent *decoder, amf::AMFComponentPtr *converter);
    ~DecConvThread();
    virtual void Run();
};

class ConvEncThread : public AMFThread
{
protected:
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pConverter;
    amf::AMFComponentPtr    m_pEncoder;
public:
    ConvEncThread(amf::AMFContext *context, amf::AMFComponent *converter, amf::AMFComponent *encoder);
    ~ConvEncThread();
    virtual void Run();
};

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

class TranscodeSession : public AMFThread
{
protected:
    DeviceDX9               m_deviceDX9;
    DeviceDX11              m_deviceDX11;
    amf::AMFContextPtr      m_pContext;
    amf::AMFComponentPtr    m_pDecoder;
    amf::AMFComponentPtr    m_pEncoder[TOTAL_TRANSCODES_PER_SESSION];
    amf::AMFComponentPtr    m_pConverter[TOTAL_TRANSCODES_PER_SESSION];
    AMFDataStreamPtr        m_pDatastream;
    BitStreamParserPtr      m_pParser;
    DecConvThread*          m_ptDecConvThread;
    ConvEncThread*          m_ptConvEncThread[TOTAL_TRANSCODES_PER_SESSION];
    OutputThread*           m_ptOutputThread[TOTAL_TRANSCODES_PER_SESSION];
    amf_int32               m_iDeviceID;
    wchar_t                 m_fOutWidthSize[TOTAL_TRANSCODES_PER_SESSION][2000];

public:
    TranscodeSession(amf_uint32 sessionNo, amf_uint32 adapterCnt, DeviceDX9 dx9, DeviceDX11 dx11);
    ~TranscodeSession();
    virtual void Run();
    AMF_RESULT InitSession(amf_int8 fileNameIn[MAX_FILENAME_LEN]);
    void CloseSession();

    amf_int32               m_iSessionNo;
    amf_bool                m_bRunningStatus;
};

bool parseConfig(amf_int8* configFilePath,
                 amf_int8 fileName[][MAX_FILENAME_LEN],
                 amf_int32 deviceID[],
                 amf_uint32* totalInstances)
{
    const char* const chDelimiter = " ";
    std::ifstream file;
    file.open(configFilePath);
    std::string line;
    amf_uint32 numLines = 0;

    if (!file.good())
    {
        printf("Error in reading the configuration file: %s\n",
                        configFilePath);
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
            if (numLines > MAX_NUM_OF_SESSIONS)
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

            deviceID[numLines - 1] = atoi(token[index]);
            index++;
        }
    }

    file.close();
    *totalInstances = numLines;
    if (*totalInstances > MAX_NUM_OF_SESSIONS)
    {
        printf("---------------------------------------------------------------------\n");
        printf("NOTE: Supports only max of %d sessions                               \n", MAX_NUM_OF_SESSIONS);
        printf("---------------------------------------------------------------------\n");
        *totalInstances = MAX_NUM_OF_SESSIONS;
    }
    return true;
}


int main(int argc, char* argv[])
{
    amf::AMFTraceSetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TRACE);
    ::amf_increase_timer_precision();

    static amf_int32            widthIn;
    static amf_int32            heightIn;
    AMF_RESULT                  res = AMF_OK; // error checking can be added later
    amf_int8                    fileNameIn[MAX_NUM_OF_SESSIONS][MAX_FILENAME_LEN];
    DeviceDX9                   deviceDX9;
    DeviceDX11                  deviceDX11;
    amf_int8                    configFile[MAX_FILENAME_LEN];
    amf_int8                    memType[50];
    amf_uint32                  sessionCount;
    amf_uint32                  ii=0;
    amf_int32                   deviceID[MAX_NUM_OF_SESSIONS];

    if (argc != 3)
    {
        printf("multiBatchTranscoder.exe <Config file> <Memory Type: DX9 or DX11>\n");
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

    parseConfig(configFile, fileNameIn, deviceID, &sessionCount);

    std::vector<TranscodeSession*> threads;
    for(ii = 0; ii < sessionCount; ii++)
    {
        TranscodeSession *session = new TranscodeSession(ii, deviceID[ii], deviceDX9, deviceDX11);
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
    for(std::vector<TranscodeSession*>::iterator it = threads.begin(); it != threads.end(); it++)
    {
        res = (*it)->InitSession(fileNameIn[ii]);
        if(res != AMF_OK)
        {
            printf("Init Session is failed\n");
            printf("FAIL\n");
            return AMF_FAIL;
        }
        ii++;
    }

    ii=0;
    for(std::vector<TranscodeSession*>::iterator it = threads.begin(); it != threads.end(); it++)
    {
        (*it)->Start();
        printf("Session %d: started\n", ii++);
    }

    for(std::vector<TranscodeSession*>::iterator it = threads.begin(); it != threads.end(); it++)
    {
        while((*it)->m_bRunningStatus)
        {
            amf_sleep(10);
        }
    }

    ii=0;
    for(std::vector<TranscodeSession*>::iterator it = threads.begin(); it != threads.end(); it++)
    {
        (*it)->WaitForStop();
        delete (*it);
        (*it) = NULL;

        printf("Session %d: ended\n", ii++);
    }

    printf("PASS\n");

    return 0;
}

DecConvThread::DecConvThread(amf::AMFContext *context, amf::AMFComponent *decoder, amf::AMFComponentPtr *converter) : m_pContext(context), m_pDecoder(decoder)
{
    for(int ii=0; ii<TOTAL_TRANSCODES_PER_SESSION; ii++)
    {
        m_pConverter[ii] = converter[ii];
    }
}

DecConvThread::~DecConvThread()
{
}

ConvEncThread::ConvEncThread(amf::AMFContext *context, amf::AMFComponent *converter, amf::AMFComponent *encoder) : m_pContext(context), m_pConverter(converter), m_pEncoder(encoder)
{
}
ConvEncThread::~ConvEncThread()
{
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

void DecConvThread::Run()
{
    amf_int32 ii;
    RequestStop();

    AMF_RESULT res = AMF_OK; // error checking can be added later
    amf::AMFDataPtr data[TOTAL_TRANSCODES_PER_SESSION];
    amf::AMFDataPtr output;
    while(true)
    {
        res = m_pDecoder->QueryOutput(&output);
        if(res == AMF_EOF)
        {
            for(ii = 0; ii < TOTAL_TRANSCODES_PER_SESSION; ii++)
            {
                res = m_pConverter[ii]->Drain();
            }
            break; // Drain complete
        }

        if(output != NULL)
        {
            for(ii = 0; ii < TOTAL_TRANSCODES_PER_SESSION; ii++)
            {
                res = output->Duplicate(output->GetMemoryType(), &data[ii]);
                while(true)
                {
                    res = m_pConverter[ii]->SubmitInput(data[ii]);
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
        }
        else
        {
            amf_sleep(1);
        }
    }
    m_pDecoder = NULL;
    m_pContext = NULL;
}

void ConvEncThread::Run()
{
    RequestStop();

    AMF_RESULT res = AMF_OK; // error checking can be added later
    while(true)
    {
        amf::AMFDataPtr data;
        res = m_pConverter->QueryOutput(&data);
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
            while (true) {
                res = m_pEncoder->SubmitInput(data);
                if(res == AMF_INPUT_FULL) // handle full queue
                {
                    amf_sleep(1); // input queue is full: wait, poll and submit again
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
    m_pConverter = NULL;
    m_pContext = NULL;
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
    }

    printf("\n-------------------------------------------------------------------\n");
    printf("Session # %d: batchTranscoding to %dx%d\n", m_iSessionNo, m_iWidth, m_iHeight);
    printf("-------------------------------------------------------------------\n");
    printf("latency           = %.4fms\ntranscode  per frame = %.4fms\nwrite per frame   = %.4fms\ntotal frames = %d\n",
    double(latency_time)/MILLISEC_TIME,
    double(transcode_duration )/MILLISEC_TIME/frameCount,
    double(write_duration)/MILLISEC_TIME/frameCount, frameCount);
    printf("-------------------------------------------------------------------\n\n");

    m_pEncoder = NULL;
    m_pContext = NULL;
}

AMF_RESULT TranscodeSession::InitSession(amf_int8 *fileNameIn)
{
    AMF_RESULT res = AMF_OK;
    amf_int32 cnt;
    amf_int32 widthOut;
    amf_int32 heightOut;
    amf_int32 frameRateOut;
    amf_int64 bitRateOut;
    static wchar_t *fileNameOut = L"./output_%dx%d_session%d_dev%d.h264";

    m_bRunningStatus = true;

    // context
    res = AMFCreateContext(&m_pContext);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    if(memoryTypeIn == amf::AMF_MEMORY_DX9)
    {
        res = m_deviceDX9.Init(true, m_iDeviceID, false, 1, 1);
        res = m_pContext->InitDX9(m_deviceDX9.GetDevice());
    }
    if(memoryTypeIn == amf::AMF_MEMORY_DX11)
    {
        res = m_deviceDX11.Init(m_iDeviceID, false);
        res = m_pContext->InitDX11(m_deviceDX11.GetDevice());
    }

    WCHAR inputWfile[MAX_FILENAME_LEN];
    MultiByteToWideChar(CP_ACP, 0, fileNameIn, -1, inputWfile, MAX_FILENAME_LEN);

    // initialize AMF
    m_pDatastream = AMFDataStream::Create(inputWfile, AMF_FileRead);
    if(m_pDatastream == NULL)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    // H264 elementary stream parser from samples common
    m_pParser = BitStreamParser::Create(m_pDatastream, GetStreamType(inputWfile), m_pContext);

    // component: decoder
    res = AMFCreateComponent(m_pContext, AMFVideoDecoderUVD_H264_AVC, &m_pDecoder);
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }
    res = m_pDecoder->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(AMF_TS_DECODE)); // our sample H264 parser provides decode order timestamps - change this depend on demuxer

    if (m_pParser->GetExtraDataSize())
    {
        // set SPS/PPS extracted from stream or container; Alternatively can use parser->SetUseStartCodes(true)
        amf::AMFBufferPtr buffer;
        m_pContext->AllocBuffer(amf::AMF_MEMORY_HOST, m_pParser->GetExtraDataSize(), &buffer);

        memcpy(buffer->GetNative(), m_pParser->GetExtraData(), m_pParser->GetExtraDataSize());
        m_pDecoder->SetProperty(AMF_VIDEO_DECODER_EXTRADATA, amf::AMFVariant(buffer));
    }

    res = m_pDecoder->Init(amf::AMF_SURFACE_NV12, m_pParser->GetAlignedWidth(), m_pParser->GetAlignedHeight());
    if(res != AMF_OK)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    for(cnt = 0; cnt < TOTAL_TRANSCODES_PER_SESSION; cnt++)
    {
        if(cnt == 0)
        {
            widthOut  = 1920;
            heightOut = 1080;
            frameRateOut = 30;
            bitRateOut = 10000000;
        }
        else if(cnt == 1)
        {
            widthOut  = 1280;
            heightOut = 720;
            frameRateOut = 30;
            bitRateOut = 4000000;
        }
        else if(cnt == 2)
        {
            widthOut  = 640;
            heightOut = 480;
            frameRateOut = 30;
            bitRateOut = 1000000;
        }

        // open output file with frame size in file name
        _swprintf(m_fOutWidthSize[cnt], fileNameOut, widthOut, heightOut, m_iSessionNo, m_iDeviceID);

        // component: converter
        res = AMFCreateComponent(m_pContext, AMFVideoConverter, &m_pConverter[cnt]);
        if(res != AMF_OK)
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }
        res = m_pConverter[cnt]->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, memoryTypeOut);
        res = m_pConverter[cnt]->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, amf::AMF_SURFACE_NV12);
        res = m_pConverter[cnt]->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_SIZE, ::AMFConstructSize(widthOut, heightOut));
        res = m_pConverter[cnt]->Init(amf::AMF_SURFACE_NV12, m_pParser->GetPictureWidth(), m_pParser->GetPictureHeight());
        if(res != AMF_OK)
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }

        // component: encoder
        res = AMFCreateComponent(m_pContext, AMFVideoEncoderVCE_AVC, &m_pEncoder[cnt]);
        if(res != AMF_OK)
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }
        res = m_pEncoder[cnt]->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
        res = m_pEncoder[cnt]->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitRateOut);
        res = m_pEncoder[cnt]->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(widthOut, heightOut));
        res = m_pEncoder[cnt]->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(frameRateOut, 1));
        res = m_pEncoder[cnt]->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
        res = m_pEncoder[cnt]->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
        res = m_pEncoder[cnt]->Init(amf::AMF_SURFACE_NV12, widthOut, heightOut);
        if(res != AMF_OK)
        {
            printf("FAIL\n");
            return AMF_FAIL;
        }
    }

    return res;
}

TranscodeSession::TranscodeSession(amf_uint32 sessonNo, amf_uint32 devID, DeviceDX9 dx9, DeviceDX11 dx11) :
    m_iSessionNo(sessonNo), m_iDeviceID(devID),m_deviceDX9(dx9), m_deviceDX11(dx11)
{
}

TranscodeSession::~TranscodeSession()
{
}

void TranscodeSession::Run()
{
    RequestStop();
    amf_int32 cnt;
    AMF_RESULT res = AMF_OK;
    for(cnt = 0; cnt < TOTAL_TRANSCODES_PER_SESSION; cnt++)
    {
        if(cnt == 0)
        {
            m_ptDecConvThread = new DecConvThread(m_pContext, m_pDecoder, m_pConverter);
            m_ptDecConvThread->Start();
        }

        m_ptConvEncThread[cnt] = new ConvEncThread(m_pContext, m_pConverter[cnt], m_pEncoder[cnt]);
        m_ptConvEncThread[cnt]->Start();

        m_ptOutputThread[cnt] = new OutputThread(m_pContext, m_pEncoder[cnt], m_fOutWidthSize[cnt], m_iSessionNo);
        m_ptOutputThread[cnt]->Start();
    }

    amf::AMFDataPtr data;
    bool bNeedNewInput = true;
    bool firstFrame = true;

    while(true)
    {
        if(bNeedNewInput)
        {
            data = NULL;
            res = m_pParser->QueryOutput(&data); // read compressed frame into buffer
            if(res == AMF_EOF || data == NULL)
            {
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

    // drain decoder queue
    res = m_pDecoder->Drain();

    for(cnt = 0; cnt < TOTAL_TRANSCODES_PER_SESSION; cnt++)
    {
        m_ptOutputThread[cnt]->WaitForStop();
        m_ptConvEncThread[cnt]->WaitForStop();
        if(cnt == 0)
            m_ptDecConvThread->WaitForStop();
        // cleanup in this order
        m_pEncoder[cnt]->Terminate();
        m_pEncoder[cnt] = NULL;
        m_pConverter[cnt]->Terminate();
        m_pConverter[cnt] = NULL;
    }

    m_bRunningStatus = false;
    data = NULL;
}

void TranscodeSession::CloseSession()
{
    m_pDecoder->Terminate();
    m_pDecoder = NULL;
    m_pParser = NULL;
    m_pDatastream = NULL;
    m_pContext->Terminate();
    m_pContext = NULL; // context is the last
}

