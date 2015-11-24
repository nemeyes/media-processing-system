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
 * @file <EncodePipeline.h>
 *
 * @brief Header file for Encoder Pipeline
 *
 ********************************************************************************
 */
#pragma once

#include "Context.h"
#include "VideoEncoderVCE.h"

#include "DeviceDX9.h"
#include "DeviceDX11.h"

#include "ParametersStorage.h"
#include "EncoderParams.h"
#include "RawStreamReader.h"

#include "Pipeline.h"
#include "CapabilityManager.h"
#include "VideoEncoderVCECaps.h"
#include <sstream>
#include "PrintLog.h"

class EncodePipeline: public Pipeline
{
    class PipelineElementAMFComponent;
    class PipelineElementEncoder;
public:
    EncodePipeline();
    virtual ~EncodePipeline();
public:
    static const wchar_t* PARAM_NAME_INPUT;
    static const wchar_t* PARAM_NAME_INPUT_WIDTH;
    static const wchar_t* PARAM_NAME_INPUT_HEIGHT;

    static const wchar_t* PARAM_NAME_OUTPUT;
    static const wchar_t* PARAM_NAME_OUTPUT_WIDTH;
    static const wchar_t* PARAM_NAME_OUTPUT_HEIGHT;

    static const wchar_t* PARAM_NAME_ENGINE;

    static const wchar_t* PARAM_NAME_ADAPTERID;
    static const wchar_t* PARAM_NAME_CAPABILITY;

    AMF_RESULT Init(ParametersStorage* pParams);
    void Terminate();

    AMF_RESULT Run();

    amf_uint32 m_VerboseMode;
private:
    amf::AMFContextPtr m_pContext;

    AMFDataStreamPtr m_pStreamOut;

    amf::AMFComponentPtr m_pEncoder;
    PipelineElementPtr m_pStreamWriter;

    RawStreamReaderPtr m_pReader;

    DeviceDX9 m_deviceDX9;
    DeviceDX11 m_deviceDX11;

    FILE* m_pLogFile;
    std::wstring EncodePipeline::AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType)
    {
        std::wstring strValue;
        switch (accelType)
        {
        case amf::AMF_ACCEL_NOT_SUPPORTED:
            strValue = L"Not supported";
            break;
        case amf::AMF_ACCEL_HARDWARE:
            strValue = L"Hardware-accelerated";
            break;
        case amf::AMF_ACCEL_GPU:
            strValue = L"GPU-accelerated";
            break;
        case amf::AMF_ACCEL_SOFTWARE:
            strValue = L"Not accelerated (software)";
            break;
        }
        return strValue;
    }

    bool EncodePipeline::QueryIOCaps(amf::AMFIOCapsPtr& ioCaps)
    {
        bool result = true;
        if (ioCaps != NULL)
        {
            amf_int32 minWidth, maxWidth;
            ioCaps->GetWidthRange(&minWidth, &maxWidth);
            std::wcout << L"\t\t\tWidth: [" << minWidth << L"-" << maxWidth << L"]\n";
    
            amf_int32 minHeight, maxHeight;
            ioCaps->GetHeightRange(&minHeight, &maxHeight);
            std::wcout << L"\t\t\tHeight: [" << minHeight << L"-" << maxHeight << L"]\n";

            amf_int32 vertAlign = ioCaps->GetVertAlign();
            std::wcout << L"\t\t\tVertical alignment: " << vertAlign << L" lines.\n";

            amf_bool interlacedSupport = ioCaps->IsInterlacedSupported();
            std::wcout << L"\t\t\tInterlaced support: " << (interlacedSupport ? L"YES" : L"NO") << L"\n";

            amf_int32 numOfFormats = ioCaps->GetNumOfFormats();
            std::wcout << L"\t\t\tTotal of " << numOfFormats << L" pixel format(s) supported:\n";
            for (amf_int32 i = 0; i < numOfFormats; i++)
            {
                amf::AMF_SURFACE_FORMAT format;
                amf_bool native = false;
                if (ioCaps->GetFormatAt(i, &format, &native) == AMF_OK)
                {
                    std::wcout << L"\t\t\t\t" << i << L": " << amf::AMFSurfaceGetFormatName(format) << L" " << (native ? L"(native)" : L"") << L"\n";
                }
                else
                {
                    result = false;
                    break;
                }
            }

            if (result == true)
            {
                amf_int32 numOfMemTypes = ioCaps->GetNumOfMemoryTypes();
                std::wcout << L"\t\t\tTotal of " << numOfMemTypes << L" memory type(s) supported:\n";
                for (amf_int32 i = 0; i < numOfMemTypes; i++)
                {
                    amf::AMF_MEMORY_TYPE memType;
                    amf_bool native = false;
                    if (ioCaps->GetMemoryTypeAt(i, &memType, &native) == AMF_OK)
                    {
                        std::wcout << L"\t\t\t\t" << i << L": " << amf::AMFGetMemoryTypeName(memType) << L" " << (native ? L"(native)" : L"") << L"\n";
                    }
                }
            }
        }
        else
        {
            std::wcerr << L"ERROR: ioCaps == NULL\n";
            result = false;
        }
        return result;
    }

    bool EncodePipeline::QueryEncoderForCodec(const wchar_t *componentID, amf::AMFCapabilityManagerPtr& capsManager)
    {
        std::wcout << L"\tCodec " << componentID << L"\n";
        amf::AMFEncoderCapsPtr encoderCaps;
        bool result = false;
        if (capsManager->GetEncoderCaps(componentID, &encoderCaps) == AMF_OK)
        {
            amf::AMF_ACCELERATION_TYPE accelType = encoderCaps->GetAccelerationType();
            std::wcout << L"\t\tAcceleration Type:" << AccelTypeToString(accelType) << L"\n";

            amf::H264EncoderCapsPtr encoderH264Caps = (amf::H264EncoderCapsPtr)encoderCaps;

            amf_uint32 numProfiles = encoderH264Caps->GetNumOfSupportedProfiles();
            amf_uint32 numLevels = encoderH264Caps->GetNumOfSupportedLevels();
            std::wcout << L"\t\tnumber of supported profiles:" <<numProfiles << L"\n";
        
            for (amf_uint32 i = 0; i < numProfiles; i++)
            {
                std::wcout << L"\t\t\t" << encoderH264Caps->GetProfile(i) << L"\n";
            
            }
            std::wcout << L"\t\tnumber of supported levels:" << numLevels << L"\n";
        
            for (amf_uint32 i = 0; i < numLevels; i++)
            {
                std::wcout << L"\t\t\t" << encoderH264Caps->GetLevel(i) << L"\n";

            }

            std::wcout << L"\t\tnumber of supported Rate Control Metheds:" << encoderH264Caps->GetNumOfRateControlMethods() << L"\n";
        
            for (amf_int32 i = 0; i < encoderH264Caps->GetNumOfRateControlMethods(); i++)
            {
                std::wcout << L"\t\t\t" << encoderH264Caps->GetRateControlMethod(i) << L"\n";

            }

            std::wcout << L"\t\tNumber of temporal Layers:" << encoderH264Caps->GetMaxNumOfTemporalLayers() << L"\n";
            std::wcout << L"\t\tMax Supported Job Priority:" << encoderH264Caps->GetMaxSupportedJobPriority() << L"\n";
            std::wcout << L"\t\tIsBPictureSupported:" << encoderH264Caps->IsBPictureSupported() << L"\n\n";
            std::wcout << L"\t\tMax Number of streams supported:" << encoderH264Caps->GetMaxNumOfStreams() << L"\n";
            std::wcout << L"\t\tEncoder input:\n";
            amf::AMFIOCapsPtr inputCaps;
            if (encoderCaps->GetInputCaps(&inputCaps) == AMF_OK)
            {
                result = QueryIOCaps(inputCaps);
            }

            std::wcout << L"\t\tEncoder output:\n";
            amf::AMFIOCapsPtr outputCaps;
            if (encoderCaps->GetOutputCaps(&outputCaps) == AMF_OK)
            {
                result = QueryIOCaps(outputCaps);
            }
            return true;
        }
        else
        {
            std::wcout << AccelTypeToString(amf::AMF_ACCEL_NOT_SUPPORTED) << L"\n";
            return false;
        }
    }

    bool EncodePipeline::QueryEncoderCaps(amf::AMFCapabilityManagerPtr& capsManager)
    {
        std::wcout << L"Querying video encoder capabilities...\n";
    
        return  QueryEncoderForCodec(AMFVideoEncoderVCE_AVC, capsManager) && QueryEncoderForCodec(AMFVideoEncoderVCE_SVC, capsManager);
    }
};