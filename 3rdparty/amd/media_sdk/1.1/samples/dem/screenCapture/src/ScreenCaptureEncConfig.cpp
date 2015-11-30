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
 * @file <ScreenCaptureEncConfig.cpp>
 *
 * @brief Encoder Configuration file
 *
 ********************************************************************************
 */

/******************************************************************************
 *                             INCLUDE FILES                                  *
 ******************************************************************************/
#include "ScreenCaptureEncConfig.h"
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
bool parseConfigFile(EncoderConfigCtrl *pConfig, int8 *configFilename)
{
    map < string, int32 > configTable;
    prepareConfigMap(&configTable);
    memset(pConfig, 0, sizeof(EncoderConfigCtrl));
    printf("Reading user-specified configuration file: %s\n", configFilename);
    if (readConfigFile(configFilename, pConfig, &configTable) != true)
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
void prepareConfigMap(map<string, int32>* pConfigTable)
{
    /**************************************************************************
     * Encode Create params                                                   *
     **************************************************************************/
    pConfigTable->insert(pair<string, int32> ("encUsage", 1));
    pConfigTable->insert(pair<string, int32> ("encOutputType", 1));
    pConfigTable->insert(pair<string, int32> ("encProfile", 66));
    pConfigTable->insert(pair<string, int32> ("framesToEncode", 100));

    /**************************************************************************
     * Display Width and height setting                                       *
     **************************************************************************/
    pConfigTable->insert(pair<string, int32> ("displayWidth", 1280));
    pConfigTable->insert(pair<string, int32> ("displayHeight", 720));

    /**************************************************************************
     * Encode Config params                                                   *
     **************************************************************************/
    pConfigTable->insert(pair<string, int32> ("encRateControlMethod", 0));
    pConfigTable->insert(pair<string, int32> ("encInloopDeblockFilter", 1));
    pConfigTable->insert(pair<string, int32> ("encBitrate", 5000000));
    pConfigTable->insert(pair<string, int32> ("encPeakBitrate", 10000000));
    pConfigTable->insert(pair<string, int32> ("encIDRPeriod", 150));
    pConfigTable->insert(pair<string, int32> ("encSkippedPicPeriod", 0));
    pConfigTable->insert(pair<string, int32> ("encSlicesPerFrame", 1));
    pConfigTable->insert(pair<string, int32> ("encIntraRefreshMBPerSlot", 68));
    pConfigTable->insert(pair<string, int32> ("encInitVBVbufferFullness", 100));
    pConfigTable->insert(pair<string, int32> ("encVBVbufferSize", 312500));
    pConfigTable->insert(pair<string, int32> ("encMinQP", 22));
    pConfigTable->insert(pair<string, int32> ("encMaxQP", 51));
}

/**
 *******************************************************************************
 *  @fn     setConfigParam
 *  @brief  Setting up configuration parameters
 *
 *  @param[in/out] pConfig   : Pointer to the configuration structure
 *  @param[in] pConfigTable  : Pointer to the configuration map table
 *
 *  @return void
 *******************************************************************************
 */
void setConfigParam(EncoderConfigCtrl *pConfig,
                map<string, int32>* pConfigTable)
{

    /**************************************************************************
     * fill-in the general configuration structures                           *
     **************************************************************************/
    map < string, int32 > configTable = (map<string, int32> ) * pConfigTable;
    pConfig->usage = configTable["encUsage"];
    pConfig->outputType = configTable["encOutputType"];
    pConfig->profile = configTable["encProfile"];
    pConfig->framesToEncode = configTable["framesToEncode"];

    pConfig->width = configTable["displayWidth"];
    pConfig->height = configTable["displayHeight"];

    pConfig->rateControlMethod = configTable["encRateControlMethod"];
    pConfig->encInloopDeblockFilter = configTable["encInloopDeblockFilter"];
    pConfig->bitRate = configTable["encBitrate"];
    pConfig->peakBitRate = configTable["encPeakBitrate"];
    pConfig->idrPeriod = configTable["encIDRPeriod"];
    pConfig->skippedPicPeriod = configTable["encSkippedPicPeriod"];
    pConfig->slicesPerFrame = configTable["encSlicesPerFrame"];
    pConfig->intraRefreshMBperSlot = configTable["encIntraRefreshMBPerSlot"];
    pConfig->initialVBVbuffFullness = configTable["encInitVBVbufferFullness"];
    pConfig->vbvBuffSize = configTable["encVBVbufferSize"];
    pConfig->minQP = configTable["encMinQP"];
    pConfig->maxQP = configTable["encMaxQP"];
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
bool readConfigFile(int8 *fileName, EncoderConfigCtrl *pConfig, std::map<
                std::string, int32>* pConfigTable)
{
    int8 name[1000];
    int32 index;
    int32 value;

    std::ifstream file;
    file.open(fileName);
    if (!file)
    {
        printf("Error in reading the configuration file: %s\n", fileName);
        return false;
    }
    std::string line;
    map<string, int32>::iterator itMap;
    while (std::getline(file, line))
    {
        std::string temp = line;
        index = 0;
        sscanf(line.c_str(), "%s %d", name, &value);
        itMap = pConfigTable->find(name);
        if (itMap != pConfigTable->end())
        {
            itMap->second = value;
        }
    }
    /**************************************************************************
     * Set user specified configuratin                                        *
     **************************************************************************/
    setConfigParam(pConfig, pConfigTable);
    file.close();
    return true;
}
