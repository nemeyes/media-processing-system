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
 * @file <EncodeConfig.h>
 *
 * @brief Header File for Encoder Configuration
 *
 ********************************************************************************
 */

#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include "AMFPlatform.h"
#include "ParametersStorage.h"

#define MAX_NEG_INTEGER_VALUE 0x80000000

typedef struct ConfigParamInt
{
    // Common Params
    amf_int32 width;
    amf_int32 height;
    amf_int32 setDynamicParamFreq;
    amf_int32 setFrameParamFreq;

    // Encoder Dynamic Params
    amf_int32 maxOfLTRFrames;
    amf_int32 targetBitrate;
    amf_int32 peakBitrate;
    amf_int32 minQP;
    amf_int32 maxQP;
    amf_int32 qpI;
    amf_int32 qpP;
    amf_int32 qpB;
    amf_int32 gopSize;
    amf_int32 vbvBufferSize;
    amf_int32 initialVBVBufferFullness;
    amf_int32 maxAUSize;
    amf_int32 bPicturesDeltaQP;
    amf_int32 referenceBPicturesDeltaQP;
    amf_int32 headerInsertionSpacing;
    amf_int32 idrPeriod;
    amf_int32 intraRefreshMBsNumberPerSlot;
    amf_int32 slicesPerFrame;
    amf_int32 bPicturesPattern;
    amf_int32 numOfTemporalEnhancmentLayers;

    // Encoder Frame Params
    amf_int32 markCurrentWithLTRIndex;
    amf_int32 forceLTRReferenceBitfield;
} ConfigParamInt;

typedef struct ConfigParamString
{
    // Encoder Static Params
    std::string engine;
    std::string displayCapability;

    // Encoder Static Params
    std::string usage;
    std::string profile;
    std::string profileLevel;

    // Encoder Dynamic Params
    std::string frameRate;
    std::string rateControlMethod;
    std::string rateControlSkipFrameEnable;
    std::string enforceHRD;
    std::string fillerDataEnable;
    std::string deBlockingFilter;
    std::string BReferenceEnable;
    std::string halfPixel;
    std::string quarterPixel;
    std::string scanType;
    std::string qualityPreset;

    // Encoder Frame Params
    std::string endOfSequence;
    std::string endOfStream;
    std::string insertSPS;
    std::string insertPPS;
    std::string insertAUD;
    std::string forcePictureType;
    std::string pictureStructure;
} ConfigParamString;

/******************************************************************************
 * configuration functions                                                    *
 ******************************************************************************/
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
bool readConfigFile(amf_int8 *fileName,
                std::map<std::string, amf_int32>* pConfigTableInt, std::map<
                                std::string, std::string>* pConfigTableString,
                ParametersStorage* pParams);
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
bool parseConfigFile(amf_int8 *configFilename, ParametersStorage* pParams);
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
void prepareConfigMapInt(std::map<std::string, amf_int32>* pConfigTable);
void prepareConfigMapString(std::map<std::string, std::string>* pConfigTable);
