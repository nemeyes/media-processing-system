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
 * @file <ScreenCaptureEncConfig.h>
 *
 * @brief This file contains Screen capture encode configuration structures and 
 *        function prototypes
 *
 ********************************************************************************
 */

#ifndef SCREENCAPTURE_ENC_CONFIG_H__
#define SCREENCAPTURE_ENC_CONFIG_H__

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include "Typedef.h"

typedef struct EncoderConfigCtrl
{
    // Create params
    int32 usage;
    int32 outputType;
    int32 profile;
    int32 framesToEncode;

    // Config params
    int32 rateControlMethod;
    int32 encInloopDeblockFilter;
    int32 idrPeriod;
    int32 skippedPicPeriod;
    int32 bitRate;
    int32 peakBitRate;
    int32 slicesPerFrame;
    int32 intraRefreshMBperSlot;
    int32 initialVBVbuffFullness;
    int32 vbvBuffSize;
    int32 minQP;
    int32 maxQP;

    // display parameters
    int32 width;
    int32 height;
}EncoderConfigCtrl, far * pConfig;

/******************************************************************************
 * configuration functions                                                    *
 ******************************************************************************/
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
                std::map<std::string, int32>* pConfigTable);
/**
 *******************************************************************************
 *  @fn     readConfigFile
 *  @brief  Reading in user-specified configuration file
 *
 *  @param[in] fileName           : user specified configuration file name
 *  @param[in/out] pConfig       : Pointer to the configuration structure
 *  @param[in/out] pConfigTable   : Pointer to the configuration map table
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool readConfigFile(int8 *fileName, EncoderConfigCtrl *pConfig, std::map<
                std::string, int32>* pConfigTable);
/**
 *******************************************************************************
 *  @fn     parseConfigFile
 *  @brief  Reads configuration file provided by user
 *
 *  @param[in/out] pConfig        : Pointer to the configuration structure
 *  @param[in] configFilename : User configuration file name
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool parseConfigFile(EncoderConfigCtrl *pConfig, int8 *configFilename);
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
void prepareConfigMap(std::map<std::string, int32>* pConfigTable);

#endif //SCREENCAPTURE_ENC_CONFIG_H__
