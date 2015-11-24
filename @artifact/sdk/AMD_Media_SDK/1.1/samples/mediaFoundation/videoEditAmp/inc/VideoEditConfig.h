/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1              Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.
 2              Redistributions in binary form must reproduce the above copyright notice, 
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
 * @file <VideoEditConfig.h>                          
 *                                       
 * @brief Contains declaration for configuration file reading functions
 *         
 ********************************************************************************
 */

#ifndef VIDEORESIZECONFIG_H_
#define VIDEORESIZECONFIG_H_

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <initguid.h>
#include "Typedef.h"
#include "codecapi.h"
#include "MftUtils.h"
#define FRAME_RATE_DENOMINATOR  1000
/*******************************************************************************
 * Enum for resizing profles                                                    *
 *******************************************************************************/
typedef enum
{
    HD1080, HD720, WVGA, VGA, QVGA
} ResizingProfiles;

typedef struct ResizerParam
{
    /***************************************************************************
     * Output width in pixels                                                  *
     **************************************************************************/
    DWORD outputWidth;
    /***************************************************************************
     * Output height in pixels                                                 *
     **************************************************************************/
    DWORD outputHeight;
}ResizerParam, far * pResizerParams;

static const ResizerParam ResizerProfiles[] = { { 1920, 1080 }, { 1280, 720 },
                                                 { 800, 480 }, { 640, 480 },
                                                 { 320, 240 } };
typedef struct ConfigCtrl
{
    AMF_MFT_VIDEOENCODERPARAMS vidParams;
    AMF_MFT_COMMONPARAMS commonParams;
    /***************************************************************************
     * Resizer parameters                                                      *
     **************************************************************************/
    ResizerParam resizerParams;
}ConfigCtrl, far * pConfig;

/*****************************************************************************
 * Configuration functions                                                    *
 *****************************************************************************/
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
void setConfigParam(ConfigCtrl *pConfig,
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
bool readConfigFile(int8 *fileName, ConfigCtrl *pConfig, std::map<std::string,
                int32>* pConfigTable);
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
bool parseConfigFile(ConfigCtrl *pConfig, int8 *configFilename);
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

#endif

