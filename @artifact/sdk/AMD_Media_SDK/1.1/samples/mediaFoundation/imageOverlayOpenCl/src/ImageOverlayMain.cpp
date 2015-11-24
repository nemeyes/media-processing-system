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
 * @file <ImageOverlayMain.cpp>
 *
 * @brief Shows how to use the Image Overlay pipeline using OpenCl MFTs
 *
 ********************************************************************************
 */

#include <iostream>

#include "ImageOverlayApi.h"
#include "TranscodeSession.h"

#define MAX_LEN 200

/**
 *******************************************************************************
 *  @fn     parseArguments
 *  @brief  This function parses the configuration file
 *
 *  @param[in] argc : Number of arguments
 *  @param[in] argv : Pointer to the
 *  @param[out] inputFile : Input Mpeg4 part 2 files
 *  @param[out] outputFile : Output .asf file
 *  @param[out] configFilename : Configuration file
 *  @param[out] verboseMode : Value for log dumping
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool parseArguments(int32 argc, int8* argv[], int8 *inputFile,
                int8 *outputFile, int8 *configFilename, uint32 *verboseMode)
{
    /***************************************************************************
     * Processing the command line and configuration file                      *
     **************************************************************************/
    int32 index = 0;
    int32 argCheck = 0;

    while (index < argc)
    {
        if (strncmp(argv[index], "-h", 2) == 0)
        {
            std::cout << "Help on encoding usages and configurations...\n";
            std::cout
                            << "ImageOverlayVs12.exe -i input_mpeg4p2Video.avi -o output_h264_encoded_file.asf -c config.cfg -l <0 or 1 or 2>\n";
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
        std::cout
                        << "ImageOverlayVs12.exe -i input_mpeg4p2Video.avi -o output_h264_encoded_file.asf -c config.cfg -l <0 or 1 or 2>\n";
        return false;
    }
    return true;
}

/**
 *******************************************************************************
 *  @fn     main
 *  @brief  Entry point function
 *
 *  @param[in] argc : Number of arguments
 *  @param[in] argv : Pointer to the
 *
 *  @return int : 0 if successful; otherwise error code.
 *******************************************************************************
 */
int main(int32 argc, int8* argv[])
{
    int8 inputFilePath[MAX_LEN];
    int8 outputFilePath[MAX_LEN];
    int8 configFilePath[MAX_LEN];
    bool status;
    ConfigCtrl configCtrl;
    msdk_CVideoEditApi videoEditApiObj;
    int32 returnError = ERR_NO_ERROR;
    uint32 verboseMode = 0;

    /********************************************************************************************
     * Parse arguments                                                                          *
     * Usage : imageOverlay.exe -i input_mpeg4Video.avi                                         *
     * -o output_h264_encoded_file.asf -c config.cfg -l <0 or 1 or 2>                           *
     *******************************************************************************************/
    status = parseArguments(argc, argv, inputFilePath, outputFilePath,
                    configFilePath, &verboseMode);
    if (status == false)
    {
        printf("FAIL\n");
        return 1;
    }
    /***************************************************************************
     * Get the pointer to configuration controls                               *
     * This also parses the config.cfg                                         *
     **************************************************************************/
    ConfigCtrl *pConfigCtrl = (ConfigCtrl *) &configCtrl;
    memset(pConfigCtrl, 0, sizeof(ConfigCtrl));
    if (parseConfigFile(pConfigCtrl, configFilePath) != true)
    {
        printf("FAIL\n");
        return 1;
    }

    /***************************************************************************
     * Initializes Video Edit Object                                           *
     **************************************************************************/
    returnError = videoEditApiObj.init(inputFilePath, outputFilePath,
                    pConfigCtrl, &verboseMode);
    if (ERR_NO_ERROR != returnError)
    {
        std::cout
                        << "See the generated ImageOverlayOpenClErrorLog.txt file for more details.\n";
        printf("FAIL\n");
        return returnError;
    }
    else
    {
        printf("Initialization successful.\n");
    }
    /***************************************************************************
     * Build and load MFT topology                                             *
     **************************************************************************/
    returnError = videoEditApiObj.setUp();
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to build/load topology" << std::endl;
        DISPLAYERROR(videoEditApiObj.getMicrosoftErrorCode());
        std::cout
                        << "See the generated ImageOverlayOpenClErrorLog.txt file for more details.\n";
        printf("FAIL\n");
        return 1;
    }
    else
    {
        printf("Topology set up successful.\n");
    }

    /***************************************************************************
     * Run MFT topology                                                        *
     **************************************************************************/
    printf("Running topology.It may take sometime to complete.....\n");
    returnError = videoEditApiObj.run();
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to run topology" << std::endl;
        DISPLAYERROR(videoEditApiObj.getMicrosoftErrorCode());
        std::cout
                        << "See the generated ImageOverlayOpenClErrorLog.txt file for more details.\n";
        printf("FAIL\n");
        return 1;
    }
    else
    {
        printf("Task Completed.\n");
    }
    /***************************************************************************
     * Destroy the Video Editor Object                                         *
     **************************************************************************/
    videoEditApiObj.destroy();
    if (ERR_NO_ERROR == returnError)
    {
        printf("PASS\n");
    }
    else
    {
        printf("FAIL\n");
    }
    return returnError;
}