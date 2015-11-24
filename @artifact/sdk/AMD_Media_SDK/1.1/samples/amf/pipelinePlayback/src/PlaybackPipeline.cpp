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
 ******************************************************************************/

/**
 *******************************************************************************
 * @file <PlaybackPipeline.cpp>
 *
 * @brief Source file for the playback pipeline
 *
 *******************************************************************************
 */

#include "PlaybackPipeline.h"

#pragma warning(disable:4355)

class PlaybackPipeline::PipelineElementAMFComponent: public PipelineElement
{
public:
    PipelineElementAMFComponent(amf::AMFComponentPtr pComponent) :
        m_pComponent(pComponent)
    {

    }

    virtual ~PipelineElementAMFComponent()
    {
    }

    AMF_RESULT SubmitInput(amf::AMFData* pData)
    {
        AMF_RESULT res = AMF_OK;
        if (pData == NULL) // EOF
        {
            res = m_pComponent->Drain();
        }
        else
        {
            res = m_pComponent->SubmitInput(pData);
            if (res == AMF_DECODER_NO_FREE_SURFACES)
            {
                return AMF_INPUT_FULL;
            }
        }
        return res;
    }

    AMF_RESULT QueryOutput(amf::AMFData** ppData)
    {
        AMF_RESULT res = AMF_OK;
        amf::AMFDataPtr data;
        res = m_pComponent->QueryOutput(&data);
        if (res == AMF_REPEAT)
        {
            res = AMF_OK;
        }
        if (res == AMF_EOF && data == NULL)
        {
        }
        if (data != NULL)
        {
            *ppData = data.Detach();
        }
        return res;
    }
    virtual AMF_RESULT Drain()
    {
        return m_pComponent->Drain();
    }
protected:
    amf::AMFComponentPtr m_pComponent;
};

const wchar_t* PlaybackPipeline::PARAM_NAME_INPUT = L"INPUT";
const wchar_t* PlaybackPipeline::PARAM_NAME_PRESENTER = L"PRESENTER";

PlaybackPipeline::PlaybackPipeline()
{
    SetParamDescription(PARAM_NAME_INPUT, ParamCommon, L"Input file name");
    SetParamDescription(PARAM_NAME_PRESENTER, ParamCommon,  L"Specifies presenter engine type (DX9, DX11, OPENGL)", ParamConverterVideoPresenter);

    SetParam(PlaybackPipeline::PARAM_NAME_PRESENTER, amf::AMF_MEMORY_DX9);
}

PlaybackPipeline::~PlaybackPipeline()
{
    Terminate();
}

void PlaybackPipeline::Terminate()
{
    Stop();
    m_pContext = NULL;
}

double PlaybackPipeline::GetProgressSize()
{
    return (double) (m_pStream ? m_pStream->Size() : 100);
}

double PlaybackPipeline::GetProgressPosition()
{
    return (double) (m_pStream ? m_pStream->Position() : 0);
}

AMF_RESULT PlaybackPipeline::Init(HWND hwnd)
{
    Terminate();
    AMF_RESULT res = AMF_OK;
    //---------------------------------------------------------------------------------------------
    // Read Options
    std::wstring inputPath = L"";

    res = GetParamWString(PARAM_NAME_INPUT, inputPath);
    CHECK_AMF_ERROR_RETURN(res, L"Input Path");

    amf::AMF_MEMORY_TYPE presenterEngine;
    {
        amf_int64 engineInt = amf::AMF_MEMORY_UNKNOWN;
        if (GetParam(PARAM_NAME_PRESENTER, engineInt) == AMF_OK)
        {
            if (amf::AMF_MEMORY_UNKNOWN != engineInt)
            {
                presenterEngine = (amf::AMF_MEMORY_TYPE) engineInt;
            }
        }
    }

    // decode options to be played with
    AMF_VIDEO_DECODER_MODE_ENUM decoderMode = AMF_VIDEO_DECODER_MODE_COMPLIANT; //AMF_VIDEO_DECODER_MODE_REGULAR , AMF_VIDEO_DECODER_MODE_LOW_LATENCY;
    bool decodeAsAnnexBStream = false; // switches between Annex B and AVCC types of decode input.

    //---------------------------------------------------------------------------------------------
    // Init context and devices

    AMFCreateContext(&m_pContext);

    if (presenterEngine == amf::AMF_MEMORY_DX9)
    {
        res = m_deviceDX9.Init(true, 0, false, 1, 1);
        CHECK_AMF_ERROR_RETURN(res, L"m_deviceDX9.Init() failed");

        res = m_pContext->InitDX9(m_deviceDX9.GetDevice());
        CHECK_AMF_ERROR_RETURN(res, L"m_pContext->InitDX9() failed");
    }

    if (presenterEngine == amf::AMF_MEMORY_DX11)
    {
        res = m_pContext->InitDX11(NULL);
        CHECK_AMF_ERROR_RETURN(res, "Init DX11");
    }
    if(presenterEngine == amf::AMF_MEMORY_OPENGL)
    {
        res = m_pContext->InitOpenGL(NULL, hwnd, NULL);
        CHECK_AMF_ERROR_RETURN(res, "Init OpenGL");
    }

    //---------------------------------------------------------------------------------------------
    // Init Video Stream Parser

    m_pStream = AMFDataStream::Create(inputPath.c_str(), AMF_FileRead);
    CHECK_RETURN(m_pStream != NULL, AMF_FILE_NOT_OPEN, "Open File");

    m_pParser = BitStreamParser::Create(m_pStream, GetStreamType(
                    inputPath.c_str()), m_pContext);
    CHECK_RETURN(m_pParser != NULL, AMF_FILE_NOT_OPEN,
                    "BitStreamParser::Create");

    //---------------------------------------------------------------------------------------------
    // Init Video Decoder

    res = AMFCreateComponent(m_pContext, AMFVideoDecoderUVD_H264_AVC,
                    &m_pDecoder);
    CHECK_AMF_ERROR_RETURN(res, L"AMFCreateComponent("
                    << AMFVideoDecoderUVD_H264_AVC << L") failed");

    m_pDecoder->SetProperty(AMF_VIDEO_DECODER_REORDER_MODE, amf_int64(
                    decoderMode));

    if (!decodeAsAnnexBStream) // need to provide SPS/PPS if input stream will be AVCC ( not Annex B)
    {
        const unsigned char* extraData = m_pParser->GetExtraData();
        size_t extraDataSize = m_pParser->GetExtraDataSize();
        if (extraData && extraDataSize)
        {
            amf::AMFBufferPtr buffer;
            m_pContext->AllocBuffer(amf::AMF_MEMORY_HOST, extraDataSize,
                            &buffer);

            memcpy(buffer->GetNative(), extraData, extraDataSize);
            m_pDecoder->SetProperty(AMF_VIDEO_DECODER_EXTRADATA,
                            amf::AMFVariant(buffer));
        }
    }
    m_pDecoder->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(AMF_TS_DECODE)); // our H264 parser provides decode order timestamps- change depend on demuxer
    m_pDecoder->Init(amf::AMF_SURFACE_NV12, m_pParser->GetPictureWidth(), m_pParser->GetPictureHeight());

    //---------------------------------------------------------------------------------------------
    // Init Presenter
    m_pPresenter = VideoPresenter::Create(presenterEngine, hwnd, m_pContext);

    //---------------------------------------------------------------------------------------------
    // Init Video Converter

    res = AMFCreateComponent(m_pContext, AMFVideoConverter, &m_pConverter);
    CHECK_AMF_ERROR_RETURN(res, L"AMFCreateComponent(" << AMFVideoConverter
                    << L") failed");

    m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE,
                    m_pPresenter->GetMemoryType());
    m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT,
                    m_pPresenter->GetInputFormat());
    m_pConverter->Init(amf::AMF_SURFACE_NV12, m_pParser->GetPictureWidth(),
                    m_pParser->GetPictureHeight());

    //---------------------------------------------------------------------------------------------
    // Connect pipeline

    Connect(m_pParser, 10);
    Connect(PipelineElementPtr(new PipelineElementAMFComponent(m_pDecoder)), 4);
    Connect(PipelineElementPtr(new PipelineElementAMFComponent(m_pConverter)), 4);
    Connect(m_pPresenter, 4);

    return AMF_OK;
}

AMF_RESULT PlaybackPipeline::Play()
{
    switch (GetState())
    {
    case PipelineStateRunning:
        return m_pPresenter->Resume();
    case PipelineStateReady:
        return Start();
    case PipelineStateNotReady:
    case PipelineStateEof:
    default:
        break;
    }
    return AMF_WRONG_STATE;
}

AMF_RESULT PlaybackPipeline::Pause()
{
    switch (GetState())
    {
    case PipelineStateRunning:
        return m_pPresenter->Pause();
    case PipelineStateReady:
    case PipelineStateNotReady:
    case PipelineStateEof:
    default:
        break;
    }
    return AMF_WRONG_STATE;
}

AMF_RESULT PlaybackPipeline::Step()
{
    switch (GetState())
    {
    case PipelineStateRunning:
        return m_pPresenter->Step();
    case PipelineStateReady:
    case PipelineStateNotReady:
    case PipelineStateEof:
    default:
        break;
    }
    return AMF_WRONG_STATE;
}

AMF_RESULT PlaybackPipeline::Stop()
{
    Pipeline::Stop();

    m_pParser = NULL;
    if (m_pDecoder != NULL)
    {
        m_pDecoder->Terminate();
        m_pDecoder = NULL;
    }
    if (m_pConverter != NULL)
    {
        m_pConverter->Terminate();
        m_pConverter = NULL;
    }
    if (m_pPresenter != NULL)
    {
        m_pPresenter->Terminate();
        m_pPresenter = NULL;
    }
    if (m_pContext != NULL)
    {
        m_pContext->Terminate();
        m_pContext = NULL;
    }

    m_deviceDX9.Terminate();
    m_deviceDX11.Terminate();

    return AMF_OK;
}
