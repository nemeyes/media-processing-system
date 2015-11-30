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
 * @file <EncodeMain.cpp>
 *
 * @brief Source File for Encoder
 *
 ********************************************************************************
 */

#include <comdef.h>
#include "INITGUID.H"
#include "DXGIDebug.h"
#include "Debug.h"

#include "CmdLogger.h"
#include "ParametersStorage.h"
#include "EncodePipeline.h"
#include "EncodeConfig.h"

#define MAX_LEN 200

AMF_RESULT RegisterParams(ParametersStorage* pParams)
{
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_INPUT, ParamCommon,
                    L"Input file name");
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_INPUT_WIDTH,
                    ParamCommon, L"Input Frame width (integer, default = 0)");
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_INPUT_HEIGHT,
                    ParamCommon, L"Input Frame height (integer, default = 0)");
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_OUTPUT,
                    ParamCommon, L"Output file name");
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_OUTPUT_WIDTH,
                    ParamCommon, L"Output Frame width (integer, default = 0)");
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_OUTPUT_HEIGHT,
                    ParamCommon, L"Output Frame height (integer, default = 0)");
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_ENGINE,
                    ParamCommon,
                    L"Specifies decoder/encoder engine type (DX9, DX11)");
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_ADAPTERID,
                    ParamCommon, L"Specifies adapter ID (integer, default = 0)");
    pParams->SetParamDescription(EncodePipeline::PARAM_NAME_CAPABILITY,
                    ParamCommon, L"Enable/Disable to display the device capabilities (true, false default =  false)");

    RegisterEncoderParams(pParams);

    // to demo frame-specific properties - will be applied to each N-th frame (force IDR)
    pParams->SetParam(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, amf_int64(
                    AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR));

    return AMF_OK;
}

bool parseArguments(amf_int32 argc, amf_int8* argv[], amf_int8 *inputFile,
                amf_int8 *outputFile, amf_int8 *configFilename,
                amf_uint32 *adapterID, amf_uint32 *verboseMode)
{
    /***************************************************************************
     * Processing the command line and configuration file                      *
     **************************************************************************/
    amf_int32 index = 0;
    amf_int32 argCheck = 0;

    while (index < argc)
    {
        if (strncmp(argv[index], "-h", 2) == 0)
        {
            std::cout << "Help on encoding usages and configurations...\n";
            std::cout << "pipelineEncoder.exe -i <Raw Input> -o <Encoded Output> -c <config file> -a <optional: adapterID. Default=0> -l <optional: logging level. Default=0. Range:0 or 1>\n";
            std::cout << "Raw Input file format should be in one of the following extensions\n";
            std::cout << "Input Format \t Extension\n";
            std::cout << "NV12         \t <filename>.nv12\n";
            std::cout << "RGBA         \t <filename>.rgba\n";
            std::cout << "BGRA         \t <filename>.bgra\n";
            std::cout << "ARGB         \t <filename>.argb\n";
            std::cout << "YUV420P      \t <filename>.420p OR <filename>.yuv OR <filename>.I420\n";
            std::cout << "YV12         \t <filename>.yv12\n";
            return false;
        }

        /***********************************************************************
         * Processing working directory and input file                         *
         **********************************************************************/
        if (strncmp(argv[index], "-i", 2) == 0)
        {
            strcpy(inputFile, argv[index + 1]);
            argCheck++;
        }

        /***********************************************************************
         * Processing working directory and output file                        *
         **********************************************************************/
        if (strncmp(argv[index], "-o", 2) == 0)
        {
            strcpy(outputFile, argv[index + 1]);
            argCheck++;
        }
        /***********************************************************************
         * Processing working directory and .cfg file                          *
         **********************************************************************/
        if (strncmp(argv[index], "-c", 2) == 0)
        {
            strcpy(configFilename, argv[index + 1]);
            argCheck++;
        }

        /***********************************************************************
         * adapterID                                                            *
         **********************************************************************/
        if (strncmp(argv[index], "-a", 2) == 0)
        {
            *adapterID = atoi(argv[index + 1]);
            argCheck++;
        }

        /***********************************************************************
         * logging level                                                       *
         **********************************************************************/
        if (strncmp(argv[index], "-l", 2) == 0)
        {
            *verboseMode = atoi(argv[index + 1]);
            argCheck++;
        }
        index++;
    }
    if (argCheck < 3)
    {
        std::cout << "Help on encoding usages and configurations...\n";
        std::cout << "pipelineEncoder.exe -i <Raw Input> -o <Encoded Output> -c <config file> -a <optional: adapterID. Default=0> -l <optional: logging level. Default=0. Range:0 or 1>\n";
        std::cout << "Raw Input file format should be in one of the following extensions\n";
        std::cout << "Input Format \t Extension\n";
        std::cout << "NV12         \t <filename>.nv12\n";
        std::cout << "RGBA         \t <filename>.rgba\n";
        std::cout << "BGRA         \t <filename>.bgra\n";
        std::cout << "ARGB         \t <filename>.argb\n";
        std::cout << "YUV420P      \t <filename>.420p OR <filename>.yuv OR <filename>.I420\n";
        std::cout << "YV12         \t <filename>.yv12\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    //AMFCustomTraceWriter writer(AMF_TRACE_INFO);
    amf::AMFAssertsEnable(false);
    amf_increase_timer_precision();

    amf_int8 inputFilePath[MAX_LEN];
    amf_int8 outputFilePath[MAX_LEN];
    amf_int8 configFilePath[MAX_LEN];
    amf_uint32 adapterID = 0;
    amf_uint32 verboseMode = 0;
    bool status;

    /********************************************************************************************
     * Parse arguments                                                                          *
     * Usage : pipelineEncoder.exe -i <Raw Input> -o <Encoded Output> -c <config file>             *
     * -a <optional: adapterID. Default=0>                                                      *
     * -l <optional: logging level. Default=0. Range:0 or 1>                                    *
     *******************************************************************************************/
    status = parseArguments(argc, argv, inputFilePath, outputFilePath,
                    configFilePath, &adapterID, &verboseMode);
    if (status == false)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    if ((verboseMode != 0) && (verboseMode != 1))
    {
        printf("Incorrect Verbose Mode.  Values supported: 0 or 1\n");
        printf("FAIL\n");
        return AMF_FAIL;
    }

    if ((adapterID < 0))
    {
        printf("Incorrect adapterID.  Value should be >= 0\n");
        printf("FAIL\n");
        return AMF_FAIL;
    }

    /**************************************************************************
     * Get the pointer to configuration controls                              *
     * This also parses the config.cfg                                        *
     **************************************************************************/
    ParametersStorage params;
    RegisterParams(&params);

    // Set Input and Output File
    const std::wstring inputName = L"input";
    const std::wstring inputPath = std::wstring(inputFilePath, inputFilePath
                    + strlen(inputFilePath));
    params.SetParamAsString(inputName, inputPath);

    const std::wstring outputName = L"output";
    const std::wstring outputPath = std::wstring(outputFilePath, outputFilePath
                    + strlen(outputFilePath));
    params.SetParamAsString(outputName, outputPath);

    const std::wstring name_str = L"adapterID";
    const std::wstring value_str = std::to_wstring(
                    static_cast<long long> (adapterID));
    params.SetParamAsString(name_str, value_str);

    // Parse Config File
    if (parseConfigFile(configFilePath, &params) != true)
    {
        printf("FAIL\n");
        return AMF_FAIL;
    }

    // Initialize
    EncodePipeline *pipeline = new EncodePipeline();

    pipeline->m_VerboseMode = verboseMode;

    AMF_RESULT res = pipeline->Init(&params);
    if (res != AMF_OK)
    {
        pipeline->Terminate();
        delete pipeline;
        printf("FAIL\n");
        return -1;
    }

    // Run
    pipeline->Run();

    // wait till end
    while (true)
    {
        amf_sleep(100);
        bool bRunning = false;
        if (pipeline->GetState() != PipelineStateEof)
        {
            bRunning = true;
        }

        if (!bRunning)
        {
            break;
        }
    }
    double encodeFPS = 0;

    // Display Result
    pipeline->DisplayResult();
    encodeFPS += pipeline->GetFPS();

    pipeline->Terminate();
    delete pipeline;

    std::wstringstream messageStream;
    messageStream.precision(1);
    messageStream.setf(std::ios::fixed, std::ios::floatfield);

    messageStream << L" Average FPS: " << encodeFPS;

    LOG_SUCCESS(messageStream.str());
    printf("PASS\n");

    return 0;
}
