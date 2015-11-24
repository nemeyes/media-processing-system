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
 * @file <ScreenCaptureEncApi.cpp>
 *
 * @brief API file
 *
 ********************************************************************************
 */

#include "ScreenCaptureEncApi.h"
#include "VideoEncoderVCEDEM.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

namespace {
const WORD TM_SET_FILENAME_BASE = WM_USER + 1;
const WORD TM_SET_PROP = WM_USER + 2;
const WORD TM_CALL_FN0 = WM_USER + 3;
const WORD TM_SHOW_CFG = WM_USER + 4;
const WORD TM_EXEC_SCENARIO = WM_USER + 5;

std::pair<AMF_RESULT, const char*> amfErrorMappingArray[] =
                { std::make_pair(AMF_OK, "AMF_OK"), std::make_pair(AMF_FAIL,
                                "AMF_FAIL"), std::make_pair(AMF_REPEAT,
                                "AMF_REPEAT"), std::make_pair(
                                AMF_INVALID_RESOLUTION,
                                "AMF_INVALID_RESOLUTION"), std::make_pair(
                                AMF_DEM_QUERY_OUTPUT_FAILED,
                                "AMF_DEM_QUERY_OUTPUT_FAILED"), std::make_pair(
                                AMF_ENCODER_NOT_PRESENT,
                                "AMF_ENCODER_NOT_PRESENT"), std::make_pair(
                                AMF_ALREADY_INITIALIZED,
                                "AMF_ALREADY_INITIALIZED"), std::make_pair(
                                AMF_NOT_INITIALIZED, "AMF_NOT_INITIALIZED"),
                  std::make_pair(AMF_ENCODER_NOT_PRESENT,
                                  "AMF_ENCODER_NOT_PRESENT"), std::make_pair(
                                  AMF_INVALID_ARG, "AMF_INVALID_ARG"),
                  std::make_pair(AMF_DEM_REMOTE_DISPLAY_CREATE_FAILED,
                                  "AMF_DEM_REMOTE_DISPLAY_CREATE_FAILED"),
                  std::make_pair(AMF_DEM_PROPERTY_READONLY,
                                  "AMF_DEM_PROPERTY_READONLY"), std::make_pair(
                                  AMF_WRONG_STATE,
                                  "Wrong state, no action Taken") };

std::pair<amf::DemUsage, const char*> usageMappingArray[] =
                { std::make_pair(amf::DEM_USAGE_GENERIC, "GENERIC"),
                  std::make_pair(amf::DEM_USAGE_WIRELESS_DISPLAY,
                                  "WIRELESS_DISPLAY"), std::make_pair(
                                  amf::DEM_USAGE_LOW_LATENCY, "LOWLATENCY") };

std::pair<amf::DemOutputType, const char*> outputTypeMappingArray[] =
                { std::make_pair(amf::DEM_AV_TS, "AV_TS"), std::make_pair(
                                amf::DEM_AV_ES, "AV_ES"), std::make_pair(
                                amf::DEM_V_TS, "V_TS"), std::make_pair(
                                amf::DEM_V_ES, "V_ES"), std::make_pair(
                                amf::DEM_A_ES, "A_ES") };

std::pair<amf::DemRateControlMethod, const char*>
                rateControlMethodMappingArray[] = { std::make_pair(
                                amf::DEM_PEAK_CONSTRAINED_VBR,
                                "PEAK_CONSTRAINED_VBR"), std::make_pair(
                                amf::DEM_LATENCY_CONSTRAINED_VBR,
                                "LATENCY_CONSTRAINED_VBR"), std::make_pair(
                                amf::DEM_CBR, "CBR"), std::make_pair(
                                amf::DEM_NO_RC, "NONE"), };

template<class ChType, class T, size_t N, class V> void printMapped(
                std::basic_ostream<ChType>& stream, const char* unknMessage,
                T(&arr)[N], V v)
{
    auto iter = std::find_if(std::begin(arr), std::end(arr), [=](const T& mapping)
                    {   return mapping.first == v;});
    if (iter != std::end(arr))
        stream << iter->second;
    else
        stream << unknMessage << v;
}

template<class ChType> void printError(std::basic_ostream<ChType>& stream,
                AMF_RESULT error)
{
    printMapped(stream, "error code ", amfErrorMappingArray, error);
}

void printUsage(std::wostream& stream, amf::DemUsage ot)
{
    printMapped(stream, "unknown usage: ", usageMappingArray, ot);
}

template<class ChType> void printOutputType(std::basic_ostream<ChType>& stream,
                amf::DemOutputType ot)
{
    printMapped(stream, "unknown output type: ", outputTypeMappingArray, ot);
}

void printRateControlMethod(std::wostream& stream,
                amf::DemRateControlMethod rcm)
{
    printMapped(stream, "unknown rate control method: ",
                    rateControlMethodMappingArray, rcm);
}

void printProperty(std::wostream& str, const std::wstring& name,
                const amf::AMFVariant& value)
{
    if (name == amf::DEM_USAGE)
        printUsage(
                        str,
                        static_cast<amf::DemUsage> (static_cast<amf_int64> (value)));
    else if (name == amf::DEM_OUTPUT_TYPE)
        printOutputType(
                        str,
                        static_cast<amf::DemOutputType> (static_cast<amf_int64> (value)));
    else if (name == amf::DEM_TARGET_BITRATE || name == amf::DEM_PEAK_BITRATE)
        str << (static_cast<amf_int64> (value) / 1000000.0) << L" Mbit/s";
    else if (name == amf::DEM_VBV_BUFFER_SIZE)
        str << (static_cast<amf_int64> (value) / 1000000.0) << L" M";
    else if (name == amf::DEM_RATE_CONTROL_METHOD)
        printRateControlMethod(
                        str,
                        static_cast<amf::DemRateControlMethod> (static_cast<amf_int64> (value)));
    else
        switch (value.type)
        {
        case amf::AMF_VARIANT_BOOL:
            str << (static_cast<amf_bool> (value) ? L"true" : L"false");
            break;
        case amf::AMF_VARIANT_INT64:
            str << static_cast<amf_int64> (value);
            break;
        case amf::AMF_VARIANT_WSTRING:
        case amf::AMF_VARIANT_STRING:
        default:
            str << L"\"" << value.ToWString().c_str() << L"\"";
            break;
        }
}

unsigned char wav_header[] = { 0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00,
                               0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
                               0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,
                               0x80, 0xbb, 0x00, 0x00, 0x00, 0xee, 0x02, 0x00,
                               0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61,
                               0x00, 0x00, 0x00, 0x00 };

void createWav(std::istream& lpcm, std::ostream& wav)
{
    lpcm.seekg(0, std::ios::end);
    std::vector<char> header(std::begin(wav_header), std::end(wav_header));

    amf_uint32 size = static_cast<amf_uint32> (lpcm.tellg());
    reinterpret_cast<amf_uint32&> (header[40]) = size;
    reinterpret_cast<amf_uint32&> (header[4]) = size + 36;
    wav.write(&(header.front()), header.size());

    lpcm.seekg(0, std::ios::beg);
    while (lpcm)
    {
        char buff[1024];
        lpcm.read(buff, sizeof(buff));
        wav.write(buff, lpcm.gcount());
    }
}
}

/**
 *******************************************************************************
 *  @fn     initialization
 *  @brief  Dem Encoder intializtion/creation and configuring the encoder
 *
 *  @param[in] pConfig : configuration
 *  @param[in] outputFilePath : output file
 *
 *  @return AMF_RESULT : AMF_OK if successfull; otherwise AMF error code
 *******************************************************************************
 */
AMF_RESULT CScreenCaptureEncApi::initialization(EncoderConfigCtrl *pConfig,
                int8 outputFilePath[])
{
    AMF_RESULT status = AMF_OK;
    AMFCreateEncoderVCEDEM(&mEncoder);

    string str = outputFilePath;
    std::wstring tempStr(str.begin(), str.end());

    mFileNameBase = tempStr;

    int32 usage = DEM_USAGE_WIRELESS_DISPLAY;
    int32 outputType = DEM_AV_TS;
    int32 profile = DEM_PROFILE_CONSTRAINED_BASELINE;

    usage = pConfig->usage;
    outputType = pConfig->outputType;
    profile = pConfig->profile;
    mFramesToEncode = pConfig->framesToEncode;

    /**************************************************************************
     * Encoder create settings                                                *
     **************************************************************************/
    status = mEncoder->SetProperty(DEM_USAGE, AMFVariant(usage));
    LOGIFFAILEDAMF(mLogFilep, status,
                    "DEM_USAGE property setting failed @ %s, %d\n", __FILE__,
                    __LINE__);

    status = mEncoder->SetProperty(DEM_OUTPUT_TYPE, AMFVariant(outputType));
    LOGIFFAILEDAMF(mLogFilep, status,
                    "DEM_OUTPUT_TYPE property setting failed @ %s, %d\n",
                    __FILE__, __LINE__);

    if (usage != DEM_USAGE_WIRELESS_DISPLAY)
    {
        status = mEncoder->SetProperty(DEM_PROFILE, AMFVariant(profile));
        LOGIFFAILEDAMF(mLogFilep, status,
                        "DEM_PROFILE property setting failed @ %s, %d\n",
                        __FILE__, __LINE__);
    }

    return AMF_OK;
}

/**
 *******************************************************************************
 *  @fn     remoteDisplayCreate
 *  @brief  This function creates remote display
 *
 *  @return AMF_RESULT : AMF_OK if successfull; otherwise AMF error code
 *******************************************************************************
 */
AMF_RESULT CScreenCaptureEncApi::remoteDisplayCreate()
{
    AMF_RESULT status;
    status = mEncoder->AcquireRemoteDisplay();
    LOGIFFAILEDAMF(mLogFilep, status, "AcquireRemoteDisplay failed @ %s, %d\n",
                    __FILE__, __LINE__);
    return status;
}

/**
 *******************************************************************************
 *  @fn     encCreate
 *  @brief   Creates the Video Compression Engine (VCE) session for the Display
 *           Encoding using currently defined DEM properties.
 *           All static parameters must be set before this point in time.
 *
 *  @return AMF_RESULT : AMF_OK if successfull; otherwise AMF error code
 *******************************************************************************
 */
AMF_RESULT CScreenCaptureEncApi::encCreate()
{
    AMF_RESULT status;
    status = mEncoder->CreateEncoder();
    LOGIFFAILEDAMF(mLogFilep, status, "CreateEncoder failed @ %s, %d\n",
                    __FILE__, __LINE__);
    return status;
}

/**
 *******************************************************************************
 *  @fn     encStart
 *  @brief  StartEncoding begins capturing data from the display and generating
 *          an encoded bitstream
 *
 *  @return AMF_RESULT : AMF_OK if successfull; otherwise AMF error code
 *******************************************************************************
 */
AMF_RESULT CScreenCaptureEncApi::encStart()
{
    AMF_RESULT status;
    status = mEncoder->StartEncoding();
    LOGIFFAILEDAMF(mLogFilep, status, "StartEncoding failed @ %s, %d\n",
                    __FILE__, __LINE__);
    return status;
}

/**
 *******************************************************************************
 *  @fn     encStop
 *  @brief  stops the encoding session
 *
 *  @return AMF_RESULT : AMF_OK if successfull; otherwise AMF error code
 *******************************************************************************
 */
AMF_RESULT CScreenCaptureEncApi::encStop()
{
    AMF_RESULT status;
    status = mEncoder->StopEncoding();
    LOGIFFAILEDAMF(mLogFilep, status, "StopEncoding failed @ %s, %d\n",
                    __FILE__, __LINE__);
    return status;
}

/**
 *******************************************************************************
 *  @fn     encDestroy
 *  @brief  Destroys VCE encoding session
 *
 *  @return AMF_RESULT : AMF_OK if successfull; otherwise AMF error code
 *******************************************************************************
 */
AMF_RESULT CScreenCaptureEncApi::encDestroy()
{
    AMF_RESULT status;
    status = mEncoder->DestroyEncoder();
    LOGIFFAILEDAMF(mLogFilep, status, "DestroyEncoder failed @ %s, %d\n",
                    __FILE__, __LINE__);
    return status;
}

/**
 *******************************************************************************
 *  @fn     remoteDisplayDestroy
 *  @brief  Destroy the display for encoder session
 *
 *  @return AMF_RESULT : AMF_OK if successfull; otherwise AMF error code
 *******************************************************************************
 */
AMF_RESULT CScreenCaptureEncApi::remoteDisplayDestroy()
{
    AMF_RESULT status;
    status = mEncoder->ReleaseRemoteDisplay();
    LOGIFFAILEDAMF(mLogFilep, status, "ReleaseRemoteDisplay failed @ %s, %d\n",
                    __FILE__, __LINE__);
    return status;
}

/**
 *******************************************************************************
 *  @fn     getNextFrame
 *  @brief  This function waits for the next frame to be encoded and returns it
 *
 *  @return void :
 *******************************************************************************
 */
void CScreenCaptureEncApi::getNextFrame()
{
    AMFDemBufferPtr buffer;
    AMF_RESULT result = mEncoder->GetNextFrame(&buffer);
    if (AMF_OK != result)
    {
        LOG(mLogFilep, "GetNextFrame failed @ %s, %d\n", __FILE__, __LINE__);
    }

    SYSTEMTIME currentTime;
    ::GetSystemTime(&currentTime);
    if (result == AMF_OK)
    {
        std::ostream& logStream = getOutput(L"log");
        logStream << std::setfill('0') << std::setw(2) << currentTime.wHour
                        << ":" << std::setw(2) << currentTime.wMinute << ":"
                        << std::setw(2) << currentTime.wSecond << "."
                        << std::setw(3) << currentTime.wMilliseconds
                        << ", frame received";

        logStream << ", type: ";
        printOutputType(logStream, buffer->GetDataType());

        amf_size bufferSize = buffer->GetMemorySize();
        logStream << ", size: " << bufferSize;

        amf_int64 currentTimeStamp = buffer->GetTimeStamp();
        if (currentTimeStamp)
            logStream << ", timestamp: " << currentTimeStamp;

        logStream << std::endl;

        Counter& cnt = mCounters[buffer->GetDataType()];
        cnt.count++;
        cnt.size += static_cast<unsigned> (bufferSize);

        std::wstring extension;
        switch (buffer->GetDataType())
        {
        case amf::DEM_A_ES:
            extension = L"lpcm";
            break;
        case amf::DEM_V_ES:
            extension = L"h264";
            break;
        case amf::DEM_AV_TS:
        case amf::DEM_V_TS:
            extension = L"ts";
            break;
        };

        if (bufferSize && extension.size())
        {
            void* buff = 0;
            buffer->GetMemory(&buff);
            getOutput(extension).write(static_cast<char*> (buff), bufferSize);
        }
    }
    else if (result != AMF_REPEAT)
    {
        cout << "GetNextFrame(), result: " << result << endl;

        std::ostream& logStream = getOutput(L"log");
        logStream << std::setfill('0') << std::setw(2) << currentTime.wHour
                        << ":" << std::setw(2) << currentTime.wMinute << ":"
                        << std::setw(2) << currentTime.wSecond << "."
                        << std::setw(3) << currentTime.wMilliseconds
                        << ", frame not received, error: ";
\
        printError(logStream, result);
        logStream << std::endl;

        Sleep(50);
    }
}

/**
 *******************************************************************************
 *  @fn     getOutput
 *  @brief  creating output file
 *
 *  @param[in] extension : file extension
 *
 *  @return filename with extension
 *******************************************************************************
 */
std::ostream& CScreenCaptureEncApi::getOutput(const std::wstring& extension)
{
    auto iter = mOutputs.find(extension);
    if (iter == mOutputs.end())
    {
        std::wstring fileName = mFileNameBase + L"." + extension;
        auto streamPtr = std::make_shared<std::ofstream>(fileName.c_str(),
                        std::ios::binary);

        std::wstringstream messageStream;
        messageStream << L"File \"" << fileName << "\"";
        if (*streamPtr)
            messageStream << " successfuly created";
        else
        {
            messageStream << " cannot be created, no output will be available";
        }

        iter = mOutputs.insert(std::make_pair(extension, streamPtr)).first;
    }
    return *iter->second;
}
