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
 * @file <EncodePipeline.cpp>
 *
 * @brief Source File for Encoder Pipeline
 *
 ********************************************************************************
 */

#pragma warning(disable:4355)

#include "EncodePipeline.h"

const wchar_t* EncodePipeline::PARAM_NAME_INPUT = L"INPUT";
const wchar_t* EncodePipeline::PARAM_NAME_INPUT_WIDTH = L"WIDTH";
const wchar_t* EncodePipeline::PARAM_NAME_INPUT_HEIGHT = L"HEIGHT";

const wchar_t* EncodePipeline::PARAM_NAME_OUTPUT = L"OUTPUT";
const wchar_t* EncodePipeline::PARAM_NAME_OUTPUT_WIDTH = L"OUTPUT_WIDTH";
const wchar_t* EncodePipeline::PARAM_NAME_OUTPUT_HEIGHT = L"OUTPUT_HEIGHT";

const wchar_t* EncodePipeline::PARAM_NAME_ENGINE = L"ENGINE";

const wchar_t* EncodePipeline::PARAM_NAME_ADAPTERID = L"ADAPTERID";
const wchar_t* EncodePipeline::PARAM_NAME_CAPABILITY = L"DISPLAYCAPABILITY";

#define ENCODER_SUBMIT_TIME     L"EncoderSubmitTime"  // private property to track submit tyme

class EncodePipeline::PipelineElementAMFComponent: public PipelineElement
{
public:
    PipelineElementAMFComponent(amf::AMFComponentPtr pComponent) :
        m_pComponent(pComponent)
    {

    }

    virtual ~PipelineElementAMFComponent()
    {
    }

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData)
    {
        AMF_RESULT res = AMF_OK;
        if (pData == NULL) // EOF
        {
            res = m_pComponent->Drain();
        }
        else
        {
            res = m_pComponent->SubmitInput(pData);
            if (res == AMF_DECODER_NO_FREE_SURFACES || res == AMF_INPUT_FULL)
            {
                return AMF_INPUT_FULL;
            }
        }
        return res;
    }

    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData)
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

class EncodePipeline::PipelineElementEncoder: public PipelineElementAMFComponent
{
public:
    PipelineElementEncoder(amf::AMFComponentPtr pComponent,
                    ParametersStorage* pParams, amf_int64 frameParameterFreq,
                    amf_int64 dynamicParameterFreq) :
        PipelineElementAMFComponent(pComponent), m_pParams(pParams),
                        m_framesSubmitted(0), m_framesQueried(0),
                        m_frameParameterFreq(frameParameterFreq),
                        m_dynamicParameterFreq(dynamicParameterFreq),
                        m_maxLatencyTime(0), m_TotalLatencyTime(0),
                        m_maxLatencyFrame(0), m_LastReadyFrameTime(0)
    {

    }

    virtual ~PipelineElementEncoder()
    {
    }

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData)
    {
        AMF_RESULT res = AMF_OK;
        if (pData == NULL) // EOF
        {
            res = m_pComponent->Drain();
        }
        else
        {
            amf_int64 submitTime = 0;
            amf_int64 currentTime = amf_high_precision_clock();
            if (pData->GetProperty(ENCODER_SUBMIT_TIME, &submitTime) != AMF_OK)
            {
                pData->SetProperty(ENCODER_SUBMIT_TIME, currentTime);
            }
            if (m_frameParameterFreq != 0 && m_framesSubmitted != 0
                            && (m_framesSubmitted % m_frameParameterFreq) == 0)
            { // apply frame-specific properties to the current frame
                PushParamsToPropertyStorage(m_pParams, ParamEncoderFrame, pData);
            }
            if (m_dynamicParameterFreq != 0 && m_framesSubmitted != 0
                            && (m_framesSubmitted % m_dynamicParameterFreq)
                                            == 0)
            { // apply dynamic properties to the encoder
                PushParamsToPropertyStorage(m_pParams, ParamEncoderDynamic,
                                m_pComponent);
            }
            res = m_pComponent->SubmitInput(pData);
            if (res == AMF_DECODER_NO_FREE_SURFACES || res == AMF_INPUT_FULL)
            {
                return AMF_INPUT_FULL;
            }
            m_framesSubmitted++;
        }
        return res;
    }

    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData)
    {
        AMF_RESULT ret = PipelineElementAMFComponent::QueryOutput(ppData);
        if (ret == AMF_OK && *ppData != NULL)
        {
            amf_int64 currentTime = amf_high_precision_clock();
            amf_int64 submitTime = 0;
            if ((*ppData)->GetProperty(ENCODER_SUBMIT_TIME, &submitTime)
                            == AMF_OK)
            {
                amf_int64 latencyTime = currentTime - AMF_MAX(submitTime,
                                m_LastReadyFrameTime);
                if (m_maxLatencyTime < latencyTime)
                {
                    m_maxLatencyTime = latencyTime;
                    m_maxLatencyFrame = m_framesQueried;
                }
                m_TotalLatencyTime += latencyTime;
            }
            m_framesQueried++;
            m_LastReadyFrameTime = currentTime;
        }
        return ret;
    }
    virtual std::wstring GetDisplayResult()
    {
        std::wstring ret;
        if (m_framesSubmitted > 0)
        {
            std::wstringstream messageStream;
            messageStream.precision(1);
            messageStream.setf(std::ios::fixed, std::ios::floatfield);
            double averageLatency = double(m_TotalLatencyTime) / 10000.
                            / m_framesQueried;
            double maxLatency = double(m_maxLatencyTime) / 10000.;
            messageStream << L" Average (Max, fr#) Encode Latency: "
                            << averageLatency << L" ms (" << maxLatency
                            << " ms frame# " << m_maxLatencyFrame << L")";
            ret = messageStream.str();
        }
        return ret;
    }
protected:
    ParametersStorage* m_pParams;
    amf_int m_framesSubmitted;
    amf_int m_framesQueried;
    amf_int64 m_frameParameterFreq;
    amf_int64 m_dynamicParameterFreq;
    amf_int64 m_maxLatencyTime;
    amf_int64 m_TotalLatencyTime;
    amf_int64 m_LastReadyFrameTime;
    amf_int m_maxLatencyFrame;
};

EncodePipeline::EncodePipeline() :
    m_pContext()
{
    m_pLogFile = NULL;
}

EncodePipeline::~EncodePipeline()
{
    Terminate();
}

void EncodePipeline::Terminate()
{
    Pipeline::Stop();

    m_pStreamOut = NULL;
    m_pReader = NULL;

    if (m_pEncoder != NULL)
    {
        m_pEncoder->Terminate();
        m_pEncoder = NULL;
    }

    m_pStreamWriter = NULL;

    if (m_pContext != NULL)
    {
        m_pContext->Terminate();
        m_pContext = NULL;
    }

    m_deviceDX9.Terminate();
    m_deviceDX11.Terminate();

    if (m_VerboseMode > 1)
    {
        if (m_pLogFile != NULL)
        {
            fclose( m_pLogFile);
        }
    }
}

AMF_RESULT EncodePipeline::Init(ParametersStorage* pParams)
{
    amf::AMFCapabilityManagerPtr capsManager;
    Terminate();

    if (m_VerboseMode > 0)
    {
        m_pLogFile = fopen("PipelineEncoderErrorLog.txt", "w");
    }
    else
    {
        m_pLogFile = NULL;
    }

    AMF_RESULT res = AMF_OK;

    //---------------------------------------------------------------------------------------------
    // Read Options

    // Input
    std::wstring inputPath = L"";
    res = pParams->GetParamWString(PARAM_NAME_INPUT, inputPath);
    if (res != AMF_OK)
    {
        LOG(m_pLogFile, "%s %s %d \n ", "GetParam failed for inputPath@",
                        __FILE__, __LINE__);
        CHECK_AMF_ERROR_RETURN(res, L"Input Path");
    }

    amf_int input_width = 0;
    amf_int input_height = 0;

    res = pParams->GetParam(PARAM_NAME_INPUT_WIDTH, input_width);
    if (res != AMF_OK)
    {
        LOG(m_pLogFile, "%s %s %d \n ", "GetParam failed for input_width@",
                        __FILE__, __LINE__);
    }

    res = pParams->GetParam(PARAM_NAME_INPUT_HEIGHT, input_height);
    if (res != AMF_OK)
    {
        LOG(m_pLogFile, "%s %s %d \n ", "GetParam failed input_height@",
                        __FILE__, __LINE__);
    }

    // Output
    std::wstring outputPath = L"";
    res = pParams->GetParamWString(PARAM_NAME_OUTPUT, outputPath);
    if (res != AMF_OK)
    {
        LOG(m_pLogFile, "%s %s %d \n ", "GetParam failed for outputPath@",
                        __FILE__, __LINE__);
        CHECK_AMF_ERROR_RETURN(res, L"Output Path");
    }

    // Engine
    amf::AMF_MEMORY_TYPE engineMemoryType = amf::AMF_MEMORY_UNKNOWN;

    std::wstring engineStr = L"DX9EX";
    pParams->GetParamWString(PARAM_NAME_ENGINE, engineStr);
    engineStr = toUpper(engineStr);

    //Encoder
    // frequency of dynamic changes in the encoder parameters
    amf_int frameParameterFreq = 0;
    amf_int dynamicParameterFreq = 0;
    pParams->GetParam(SETFRAMEPARAMFREQ_PARAM_NAME, frameParameterFreq);
    pParams->GetParam(SETDYNAMICPARAMFREQ_PARAM_NAME, dynamicParameterFreq);

    //---------------------------------------------------------------------------------------------
    // Init context and devices

    AMFCreateContext(&m_pContext);

    // Check the adapterID
    amf_uint32 adapterID = 0;
    pParams->GetParam(PARAM_NAME_ADAPTERID, adapterID);

    if ((engineStr == L"DX9") || (engineStr == L"DX9EX"))
    {
        engineMemoryType = amf::AMF_MEMORY_DX9;
        res = m_deviceDX9.Init(true, adapterID, false, input_width,
                        input_height);
        if (res != AMF_OK)
        {
            LOG(m_pLogFile, "%s %s %d \n ", "m_deviceDX9.Init() failed @",
                            __FILE__, __LINE__);
            CHECK_AMF_ERROR_RETURN(res, L"m_deviceDX9.Init() failed");
        }

        res = m_pContext->InitDX9(m_deviceDX9.GetDevice());
        if (res != AMF_OK)
        {
            LOG(m_pLogFile, "%s %s %d \n ", "m_pContext->InitDX9() failed @",
                            __FILE__, __LINE__);
            CHECK_AMF_ERROR_RETURN(res, L"m_pContext->InitDX9() failed");
        }
    }
    else if (engineStr == L"DX11")
    {
        engineMemoryType = amf::AMF_MEMORY_DX11;
        res = m_deviceDX11.Init(adapterID, false);
        if (res != AMF_OK)
        {
            LOG(m_pLogFile, "%s %s %d \n ", "m_deviceDX11.Init() failed @",
                            __FILE__, __LINE__);
            CHECK_AMF_ERROR_RETURN(res, L"m_deviceDX11.Init() failed");
        }
        res = m_pContext->InitDX11(m_deviceDX11.GetDevice());
        if (res != AMF_OK)
        {
            LOG(m_pLogFile, "%s %s %d \n ", "m_pContext->InitDX11() failed @",
                            __FILE__, __LINE__);
            CHECK_AMF_ERROR_RETURN(res, L"m_pContext->InitDX11() failed");
        }
    }

    std::wstring printCapsStr = L"FALSE";
    pParams->GetParamWString(PARAM_NAME_CAPABILITY, printCapsStr);
    if (printCapsStr == L"TRUE")
    {
        if (AMFCreateCapsManager(&capsManager) == AMF_OK)
        {
            bool deviceInit = false;
            int deviceIdx = 0;
#ifdef _WIN32
            OSVERSIONINFO osvi;
            memset(&osvi, 0, sizeof(osvi));
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            GetVersionEx(&osvi);
            if (osvi.dwMajorVersion >= 6)
            {
                if (osvi.dwMinorVersion >= 2)   //  Win 8 or Win Server 2012 or newer
                {
                    deviceInit = (capsManager->InitDX11(m_deviceDX11.GetDevice()) == AMF_OK);
                }
                else
                {
                    deviceInit = (capsManager->InitDX9(m_deviceDX9.GetDevice()) == AMF_OK);
                }
            }
            else    //  Older than Vista - not supported
            {
                std::wcerr << L"This version of Windows is too old\n";
            }
#endif
            bool result = false;
            if (deviceInit == true)
            {
                QueryEncoderCaps(capsManager);
                result = true;
            }
        }
    }
    //---------------------------------------------------------------------------------------------
    // Init Raw Stream Reader

    m_pReader = RawStreamReaderPtr(new RawStreamReader);
    res = m_pReader->Init(pParams, m_pContext);
    if (res != AMF_OK)
    {
        LOG(m_pLogFile, "%s %s %d \n ", "m_pReader->Init() failed @", __FILE__,
                        __LINE__);
        CHECK_AMF_ERROR_RETURN(res, L"m_pReader->Init Failed");
    }

    //---------------------------------------------------------------------------------------------
    // Init Video Encoder

    const wchar_t *encoderID = AMFVideoEncoderVCE_AVC;
    amf_int64 usage = 0;
    if (pParams->GetParam(AMF_VIDEO_ENCODER_USAGE, usage) == AMF_OK)
    {
        if (usage == amf_int64(AMF_VIDEO_ENCODER_USAGE_WEBCAM))
        {
            encoderID = AMFVideoEncoderVCE_SVC;
        }
    }

    res = AMFCreateComponent(m_pContext, encoderID, &m_pEncoder);
    if (res != AMF_OK)
    {
        LOG(m_pLogFile, "%s %s %d \n ",
                        "AMFCreateComponent(AMFVideoEncoderVCE_AVC) failed @",
                        __FILE__, __LINE__);
        CHECK_AMF_ERROR_RETURN(res, L"AMFCreateComponent(" << encoderID
                        << L") failed");
    }

    // Usage is preset that will set many parameters
    PushParamsToPropertyStorage(pParams, ParamEncoderUsage, m_pEncoder);
    // override some usage parameters
    PushParamsToPropertyStorage(pParams, ParamEncoderStatic, m_pEncoder);

    res = m_pEncoder->Init(m_pReader->GetFormat(), input_width, input_height);
    if (res != AMF_OK)
    {
        LOG(m_pLogFile, "%s %s %d \n ", "m_pEncoder->Init() failed @",
                        __FILE__, __LINE__);
        CHECK_AMF_ERROR_RETURN(res, L"m_pEncoder->Init() failed");
    }

    PushParamsToPropertyStorage(pParams, ParamEncoderDynamic, m_pEncoder);

    //---------------------------------------------------------------------------------------------
    // Init Stream Writer

    m_pStreamOut = AMFDataStream::Create(outputPath.c_str(), AMF_FileWrite);
    if (m_pStreamOut == NULL)
    {
        LOG(m_pLogFile, "%s %s %d \n ", "m_pStreamOut file open failed @",
                        __FILE__, __LINE__);
        CHECK_RETURN(m_pStreamOut != NULL, AMF_FILE_NOT_OPEN, "Open File");
    }

    m_pStreamWriter = StreamWriterPtr(new StreamWriter(m_pStreamOut));

    //---------------------------------------------------------------------------------------------
    // Connect pipeline

    Connect(m_pReader, 4);
    Connect(PipelineElementPtr(new PipelineElementEncoder(m_pEncoder, pParams,
                    frameParameterFreq, dynamicParameterFreq)), 10);
    Connect(m_pStreamWriter, 5);

    return res;
}

AMF_RESULT EncodePipeline::Run()
{
    AMF_RESULT res = AMF_OK;
    res = Pipeline::Start();
    if (res != AMF_OK)
    {
        LOG(m_pLogFile, "%s %s %d \n ", "Pipeline::Start() failed @", __FILE__,
                        __LINE__);
        CHECK_AMF_ERROR_RETURN(res, L"Pipeline::Start() failed");
    }

    return AMF_OK;
}
