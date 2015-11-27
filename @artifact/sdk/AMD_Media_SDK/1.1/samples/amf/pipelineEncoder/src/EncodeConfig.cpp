/*******************************************************************************
 Copyright ?014 Advanced Micro Devices, Inc. All rights reserved.

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
 * @file <EncodeConfig.cpp>
 *
 * @brief Source file for the Encoder Configuration
 *
 ********************************************************************************
 */

/******************************************************************************
 *                             INCLUDE FILES                                  *
 ******************************************************************************/
#include "EncodeConfig.h"
using namespace std;

/**
 *******************************************************************************
 *  @fn     parseConfigFile
 *  @brief  Reads configuration file provided by user
 *
 *  @param[in/out] pConfig      : Pointer to the configuration structure
 *  @param[in] configFilename   : User configuration file name
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool parseConfigFile(amf_int8 *configFilename, ParametersStorage* pParams)
{
    map < string, amf_int32 > configTableInt;
    map < string, string > configTableString;
    prepareConfigMapInt(&configTableInt);
    prepareConfigMapString(&configTableString);
    printf("Reading user-specified configuration file: %s\n", configFilename);
    if (readConfigFile(configFilename, &configTableInt, &configTableString,
                    pParams) != true)
    {
        return false;
    }
    return true;
}

/**
 *******************************************************************************
 *  @fn     prepareConfigMap
 *  @brief  configuration mapping table, used for mapping values from user
 *          configuration to config control structure
 *
 *  @param[in/out] pConfigTable   : Pointer to the configuration map table
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
void prepareConfigMapInt(map<string, amf_int32>* pConfigTable)
{
    /**************************************************************************
     * Common params                                                          *
     **************************************************************************/
    pConfigTable->insert(pair<string, amf_int32> ("width", 1920));
    pConfigTable->insert(pair<string, amf_int32> ("height", 1080));
    pConfigTable->insert(pair<string, amf_int32> ("setDynamicParamFreq", 0));
    pConfigTable->insert(pair<string, amf_int32> ("setFrameParamFreq", 0));
    pConfigTable->insert(pair<string, amf_int32> ("MaxOfLTRFrames", 0));

    /**************************************************************************
     * Encoder Dynamic params                                                 *
     **************************************************************************/
    pConfigTable->insert(pair<string, amf_int32> ("TargetBitrate", 5000000));
    pConfigTable->insert(pair<string, amf_int32> ("PeakBitrate", 5000000));
    pConfigTable->insert(pair<string, amf_int32> ("MinQP", 18));
    pConfigTable->insert(pair<string, amf_int32> ("MaxQP", 51));
    pConfigTable->insert(pair<string, amf_int32> ("QPI", 22));
    pConfigTable->insert(pair<string, amf_int32> ("QPP", 22));
    pConfigTable->insert(pair<string, amf_int32> ("QPB", 22));
    pConfigTable->insert(pair<string, amf_int32> ("GOPSize", 60));
    pConfigTable->insert(pair<string, amf_int32> ("VBVBufferSize", 20000000));
    pConfigTable->insert(pair<string, amf_int32> ("InitialVBVBufferFullness",
                    64));
    pConfigTable->insert(pair<string, amf_int32> ("MaxAUSize", 0));
    pConfigTable->insert(pair<string, amf_int32> ("BPicturesDeltaQP", 4));
    pConfigTable->insert(pair<string, amf_int32> ("ReferenceBPicturesDeltaQP",
                    2));
    pConfigTable->insert(pair<string, amf_int32> ("HeaderInsertionSpacing", 0));
    pConfigTable->insert(pair<string, amf_int32> ("IDRPeriod", 30));
    pConfigTable->insert(pair<string, amf_int32> (
                    "IntraRefreshMBsNumberPerSlot", 0));
    pConfigTable->insert(pair<string, amf_int32> ("SlicesPerFrame", 1));
    pConfigTable->insert(pair<string, amf_int32> ("BPicturesPattern", 3));
    pConfigTable->insert(pair<string, amf_int32> (
                    "NumOfTemporalEnhancmentLayers", 0));

    /**************************************************************************
     * Encoder Frame params                                                   *
     **************************************************************************/
    pConfigTable->insert(
                    pair<string, amf_int32> ("MarkCurrentWithLTRIndex", -1));
    pConfigTable->insert(pair<string, amf_int32> ("ForceLTRReferenceBitfield",
                    0));
}

void prepareConfigMapString(map<string, string>* pConfigTable)
{
    /**************************************************************************
     * Common params                                                          *
     **************************************************************************/
    pConfigTable->insert(pair<string, string> ("engine", "DX9EX"));
    pConfigTable->insert(pair<string, string> ("displayCapability", "FALSE"));

    /**************************************************************************
     * Encoder Static Params                                                  *
     **************************************************************************/
    pConfigTable->insert(pair<string, string> ("Usage", "TRANSCODING"));
    pConfigTable->insert(pair<string, string> ("Profile", "MAIN"));
    pConfigTable->insert(pair<string, string> ("ProfileLevel", "4.2"));
    pConfigTable->insert(pair<string, string> ("ScanType", "PROGRESSIVE"));
    pConfigTable->insert(pair<string, string> ("QualityPreset", "BALANCED"));

    /**************************************************************************
     * Encoder Dynamic params                                                 *
     **************************************************************************/
    pConfigTable->insert(pair<string, string> ("FrameRate", "30,1"));
    pConfigTable->insert(pair<string, string> ("RateControlMethod", "VBR"));
    pConfigTable->insert(pair<string, string> ("RateControlSkipFrameEnable",
                    "FALSE"));
    pConfigTable->insert(pair<string, string> ("EnforceHRD", "FALSE"));
    pConfigTable->insert(pair<string, string> ("FillerDataEnable", "FALSE"));
    pConfigTable->insert(pair<string, string> ("DeBlockingFilter", "TRUE"));
    pConfigTable->insert(pair<string, string> ("BReferenceEnable", "TRUE"));
    pConfigTable->insert(pair<string, string> ("HalfPixel", "TRUE"));
    pConfigTable->insert(pair<string, string> ("QuarterPixel", "TRUE"));

    /**************************************************************************
     * Encoder Frame params                                                   *
     **************************************************************************/
    pConfigTable->insert(pair<string, string> ("EndOfSequence", "FALSE"));
    pConfigTable->insert(pair<string, string> ("EndOfStream", "FALSE"));
    pConfigTable->insert(pair<string, string> ("InsertSPS", "FALSE"));
    pConfigTable->insert(pair<string, string> ("InsertPPS", "FALSE"));
    pConfigTable->insert(pair<string, string> ("InsertAUD", "FALSE"));
    pConfigTable->insert(pair<string, string> ("ForcePictureType", "NONE"));
    pConfigTable->insert(pair<string, string> ("PictureStructure", "NONE"));
}

/**
 *******************************************************************************
 *  @fn     readConfigFile
 *  @brief  Reading in user-specified configuration file
 *
 *  @param[in] fileName           : user specified configuration file name
 *  @param[in/out] pConfig        : Pointer to the configuration structure
 *  @param[in/out] pConfigTable   : Pointer to the configuration map table
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool readConfigFile(amf_int8 *fileName,
                std::map<std::string, amf_int32>* pConfigTableInt, std::map<
                                std::string, std::string>* pConfigTableString,
                ParametersStorage* pParams)
{
    amf_int8 name[1000];
    amf_int32 index;
    amf_int32 value;
    amf_int8 valueStr[1000];

    std::ifstream file;
    file.open(fileName);
    if (!file)
    {
        printf("Error in reading the configuration file: %s\n", fileName);
        return false;
    }
    std::string line;
    map<string, amf_int32>::iterator itMap;
    while (std::getline(file, line))
    {
        std::string temp = line;
        index = 0;
        value = MAX_NEG_INTEGER_VALUE;
        sscanf(line.c_str(), "%s %d", name, &value);
        itMap = pConfigTableInt->find(name);
        if (itMap != pConfigTableInt->end())
        {
            const std::wstring name_str = std::wstring(name, name
                            + strlen(name));

            std::wstring value_str;
            if (value == MAX_NEG_INTEGER_VALUE)
                value_str = L""; // No value specified.  Hence not passing any value to underlying library.
            else
                value_str = std::to_wstring(static_cast<long long> (value));

            pParams->SetParamAsString(name_str, value_str);
        }
    }

    file.close();

    file.open(fileName);
    if (!file)
    {
        printf("Error in reading the configuration file: %s\n", fileName);
        return false;
    }
    map<string, string>::iterator itMapStr;
    while (std::getline(file, line))
    {
        std::string temp = line;
        index = 0;
        *valueStr = 0;
        sscanf(line.c_str(), "%s %s", name, valueStr);
        itMapStr = pConfigTableString->find(name);
        if (itMapStr != pConfigTableString->end())
        {
            const std::wstring name_str = std::wstring(name, name
                            + strlen(name));
            const std::wstring value_str = std::wstring(valueStr, valueStr
                            + strlen(valueStr));
            pParams->SetParamAsString(name_str, value_str);
        }
    }

    file.close();

    return true;
}
