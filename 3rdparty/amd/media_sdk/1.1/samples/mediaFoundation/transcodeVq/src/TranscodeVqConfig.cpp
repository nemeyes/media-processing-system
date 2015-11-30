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
 * @file <TranscodeVqConfig.cpp>                          
 *                                       
 * @brief This file contains functions for reading the configuration file
 *         
 ********************************************************************************
 */
/*******************************************************************************
 *                             INCLUDE FILES                                    *
 *******************************************************************************/
#include "TranscodeVqConfig.h"
using namespace std;

/** 
 *******************************************************************************
 *  @fn     parseConfigFile
 *  @brief  Reads configuration file provided by user 
 *           
 *  @param[in/out] pConfig      : Pointer to the configuration structure
 *  @param[in] configFilename	: User configuration file name
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool parseConfigFile(ConfigCtrl *pConfig, int8 *configFilename)
{
    map < string, int32 > configTable;
    prepareConfigMap(&configTable);
    memset(&pConfig->vidParams, 0, sizeof(pConfig->vidParams));
    memset(&pConfig->commonParams, 0, sizeof(pConfig->commonParams));
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
    /**************************************************************************/
    /* Encode Specifications                                                   */
    /**************************************************************************/
    pConfigTable->insert(pair<string, int32> ("encGOPSize", 20));
    pConfigTable->insert(pair<string, int32> ("encNumBFrames", 0));
    pConfigTable->insert(pair<string, int32> ("encMeanBitrate", 3000000));
    pConfigTable->insert(pair<string, int32> ("encMaxBitrate", 4000000));
    pConfigTable->insert(pair<string, int32> ("encBufferSize", 2000000));
    pConfigTable->insert(pair<string, int32> ("encRateControlMethod", 1));
    pConfigTable->insert(pair<string, int32> ("encLowLatencyMode", 0));
    pConfigTable->insert(pair<string, int32> ("encQualityVsSpeed", 60));
    pConfigTable->insert(pair<string, int32> ("encCommonQuality", 50));
    pConfigTable->insert(pair<string, int32> ("encCompressionStandard", 77));
    pConfigTable->insert(pair<string, int32> ("encEnableCABAC", 0));
    pConfigTable->insert(pair<string, int32> ("encIDRperiod", 300));
    pConfigTable->insert(pair<string, int32> ("encNumBFrames", 1));

    pConfigTable->insert(pair<string, int32> ("vqEnableSteadyVideo", 0));
    pConfigTable->insert(pair<string, int32> ("vqSteadyVideoStrength", 0));
    pConfigTable->insert(pair<string, int32> ("vqSteadyVideoZoom", 95));
    pConfigTable->insert(pair<string, int32> ("vqSteadyVideoDelay", 0));
    pConfigTable->insert(pair<string, int32> ("vqDeinterlacing", 0));
    pConfigTable->insert(pair<string, int32> (
                    "vqDeinterlacingPullDownDetection", 1));
    pConfigTable->insert(pair<string, int32> ("vqEnableEdgeEnhacement", 0));
    pConfigTable->insert(pair<string, int32> ("vqEdgeEnhacementStrength", 50));
    pConfigTable->insert(pair<string, int32> ("vqEnableDenoise", 0));
    pConfigTable->insert(pair<string, int32> ("vqDenoiseStrength", 50));
    pConfigTable->insert(pair<string, int32> ("vqEnableMosquitoNoise", 0));
    pConfigTable->insert(pair<string, int32> ("vqMosquitoNoiseStrength", 64));
    pConfigTable->insert(pair<string, int32> ("vqEnableDeblocking", 0));
    pConfigTable->insert(pair<string, int32> ("vqDeblockingStrength", 50));
    pConfigTable->insert(pair<string, int32> ("vqEnableDynamicContrast", 1));
    pConfigTable->insert(pair<string, int32> ("vqEnableColorVibrance", 0));
    pConfigTable->insert(pair<string, int32> ("vqColorVibranceStrength", 0));
    pConfigTable->insert(pair<string, int32> ("vqEnableSkintoneCorrection", 0));
    pConfigTable->insert(pair<string, int32> ("vqSkintoneCorrectionStrength",
                    50));

    pConfigTable->insert(pair<string, int32> ("vqEnableBrighterWhites", 0));
    pConfigTable->insert(pair<string, int32> ("vqEnableGamaCorrection", 0));
    pConfigTable->insert(pair<string, int32> ("vqGamaCorrectionStrength", 1000));
    pConfigTable->insert(pair<string, int32> ("vqDynamicRangeEffect", 0));
    pConfigTable->insert(pair<string, int32> ("vqBrightnessEffect", 100));
    pConfigTable->insert(pair<string, int32> ("vqContrastEffect", 1000));
    pConfigTable->insert(pair<string, int32> ("vqSaturationEffect", 1000));
    pConfigTable->insert(pair<string, int32> ("vqTintEffect", 30000));

    pConfigTable->insert(pair<string, int32> ("vqEnableFalseContourReduction",
                    0));
    pConfigTable->insert(pair<string, int32> (
                    "vqFalseContourReductionStrength", 50));

    pConfigTable->insert(pair<string, int32> ("vqDemoMode", 0));

    pConfigTable->insert(pair<string, int32> ("useSWCodec", 0));
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
void setConfigParam(ConfigCtrl *pConfig, map<string, int32>* pConfigTable)
{

    /**************************************************************************/
    /* fill-in the general configuration structures                           */
    /**************************************************************************/
    map < string, int32 > configTable = (map<string, int32> ) * pConfigTable;
    pConfig->vidParams.compressionStandard
                    = configTable["encCompressionStandard"];
    pConfig->vidParams.meanBitrate = configTable["encMeanBitrate"];
    pConfig->vidParams.maxBitrate = configTable["encMaxBitrate"];
    pConfig->vidParams.numBFrames = configTable["encNumBFrames"];
    pConfig->vidParams.gopSize = configTable["encGOPSize"];
    pConfig->vidParams.bufSize = configTable["encBufferSize"];
    pConfig->vidParams.lowLatencyMode = configTable["encLowLatencyMode"];
    pConfig->vidParams.qualityVsSpeed = configTable["encQualityVsSpeed"];
    pConfig->vidParams.commonQuality = configTable["encCommonQuality"];
    pConfig->vidParams.enableCabac = configTable["encEnableCABAC"];
    pConfig->vidParams.idrPeriod = configTable["encIDRperiod"];
    pConfig->vidParams.rateControlMethod = configTable["encRateControlMethod"];
    pConfig->vidParams.numBFrames = configTable["encNumBFrames"];

    pConfig->vqParams.streadyVideoEnable = configTable["vqEnableSteadyVideo"];
    pConfig->vqParams.streadyVideoStrength
                    = configTable["vqSteadyVideoStrength"];
    pConfig->vqParams.streadyVideoZoom = configTable["vqSteadyVideoZoom"];
    pConfig->vqParams.streadyVideoDelay = configTable["vqSteadyVideoDelay"];
    pConfig->vqParams.deinterlacing = configTable["vqDeinterlacing"];
    pConfig->vqParams.deinterlacingPullDownDetection
                    = configTable["vqDeinterlacingPullDownDetection"];
    pConfig->vqParams.enableEdgeEnhacement
                    = configTable["vqEnableEdgeEnhacement"];
    pConfig->vqParams.edgeEnhacementStrength
                    = configTable["vqEdgeEnhacementStrength"];
    pConfig->vqParams.enableDenoise = configTable["vqEnableDenoise"];
    pConfig->vqParams.denoiseStrength = configTable["vqDenoiseStrength"];
    pConfig->vqParams.enableMosquitoNoise
                    = configTable["vqEnableMosquitoNoise"];
    pConfig->vqParams.mosquitoNoiseStrength
                    = configTable["vqMosquitoNoiseStrength"];
    pConfig->vqParams.enableDeblocking = configTable["vqEnableDeblocking"];
    pConfig->vqParams.deblockingStrength = configTable["vqDeblockingStrength"];
    pConfig->vqParams.enableDynamicContrast
                    = configTable["vqEnableDynamicContrast"];
    pConfig->vqParams.enableColorVibrance
                    = configTable["vqEnableColorVibrance"];
    pConfig->vqParams.colorVibranceStrength
                    = configTable["vqColorVibranceStrength"];
    pConfig->vqParams.enableSkintoneCorrectoin
                    = configTable["vqEnableSkintoneCorrection"];
    pConfig->vqParams.skintoneCorrectoinStrength
                    = configTable["vqSkintoneCorrectionStrength"];

    pConfig->vqParams.effectBrightness = configTable["vqEnableBrighterWhites"];
    pConfig->vqParams.enableGamaCorrection
                    = configTable["vqEnableGamaCorrection"];
    pConfig->vqParams.gamaCorrectionStrength
                    = configTable["vqGamaCorrectionStrength"];
    pConfig->vqParams.enableDynamicRange = configTable["vqDynamicRangeEffect"];
    pConfig->vqParams.effectBrightness = configTable["vqBrightnessEffect"];
    pConfig->vqParams.effectContrast = configTable["vqContrastEffect"];
    pConfig->vqParams.effectSaturation = configTable["vqSaturationEffect"];
    pConfig->vqParams.effectTint = configTable["vqTintEffect"];
    pConfig->vqParams.enableFalseContourReduction
                    = configTable["vqEnableFalseContourReduction"];
    pConfig->vqParams.falseContorReductionStrength
                    = configTable["vqFalseContourReductionStrength"];
    pConfig->vqParams.demoMode = configTable["vqDemoMode"];

    pConfig->commonParams.useSWCodec = configTable["useSWCodec"];
}

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
                int32>* pConfigTable)
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
    /**************************************************************************/
    /* Set user specified configuratin                                        */
    /**************************************************************************/
    setConfigParam(pConfig, pConfigTable);
    file.close();
    return true;
}
