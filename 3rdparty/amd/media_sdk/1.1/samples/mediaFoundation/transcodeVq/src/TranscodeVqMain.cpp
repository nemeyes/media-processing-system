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
 * @file <TranscodeVqMain.cpp>
 *
 * @brief Main file
 *
 ********************************************************************************
 */
#include <iostream>
#include "TranscodeVqApi.h"
#include "TranscodeVqConfig.h"

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
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool parseArguments(int32 argc, int8* argv[], int8 *inputFile,
                int8 *outputFile, int8 *configFilename, uint32 *verboseMode)
{
    /**************************************************************************/
    /* Processing the command line and configuration file                     */
    /**************************************************************************/
    int32 index = 0;
    int32 argCheck = 0;
    while (index < argc)
    {
        if (strncmp(argv[index], "-h", 2) == 0)
        {
            std::cout << "Help on encoding usages and configurations...\n";
            std::cout
                            << "transcodeVq.exe -i input_mpeg4p2Video.avi \
            -o output_h264_encoded_file.asf -c config.cfg -l <1 to 2>\n";
            return false;
        }

        /**********************************************************************/
        /* Processing working directory and input file                        */
        /**********************************************************************/
        if (strncmp(argv[index], "-i", 2) == 0)
        {
            if (argv[index + 1] == NULL)
                return false;
            strcpy(inputFile, argv[index + 1]);
            argCheck++;
        }

        /**********************************************************************/
        /* Processing working directory and output file                       */
        /**********************************************************************/
        if (strncmp(argv[index], "-o", 2) == 0)
        {
            if (argv[index + 1] == NULL)
                return false;
            strcpy(outputFile, argv[index + 1]);
            argCheck++;
        }
        /**********************************************************************/
        /* Processing working directory and .cfg file                         */
        /**********************************************************************/
        if (strncmp(argv[index], "-c", 2) == 0)
        {
            if (argv[index + 1] == NULL)
                return false;
            strcpy(configFilename, argv[index + 1]);
            argCheck++;
        }
        /**********************************************************************/
        /* logging level                                                      */
        /**********************************************************************/
        if (strncmp(argv[index], "-l", 2) == 0)
        {
            if (argv[index + 1] == NULL)
                return false;
            *verboseMode = atoi(argv[index + 1]);
        }
        index++;
    }
    if (argCheck < 3)
    {
        return false;
    }
    return true;
}

int main(int32 argc, int8* argv[])
{
    msdk_CTranscodeVqApi trascodeVqObj;
    int8 inputFilePath[400];
    int8 outputFilePath[400];
    int8 configFilePath[400];
    bool status;
    uint32 returnError = ERR_NO_ERROR;
    ConfigCtrl configCtrl;
    uint32 verboseMode = 0;

    /**************************************************************************/
    /* Parse arguments                                                        */
    /* Usage : transcodeVqVs12.exe -i input_mjpegVideo.avi                    */
    /* -o output_h264_encoded_file.h264 -c config.cfg -l <1 to 2>             */
    /**************************************************************************/
    status = parseArguments(argc, argv, inputFilePath, outputFilePath,
                    configFilePath, &verboseMode);
    if (status == false)
    {
        std::cout << "Help on encoding usages and configurations...\n";
        std::cout
                        << "transcodeVq.exe -i input_mpeg4p2Video.avi -o output_h264_encoded_file.asf -c config.cfg -l <1 to 2>\n";
        printf("FAIL\n");
        return 1;
    }

    /**************************************************************************/
    /* Get the pointer to configuration controls                              */
    /* This also parses the config.cfg                                        */
    /**************************************************************************/
    ConfigCtrl *pConfigCtrl = (ConfigCtrl *) &configCtrl;
    memset(pConfigCtrl, 0, sizeof(ConfigCtrl));
    if (parseConfigFile(pConfigCtrl, configFilePath) != true)
    {
        printf("FAIL\n");
        return 1;
    }
    /**************************************************************************/
    /* This function sets the parameters parsed from the configuration file   */
    /* and initializes the MFT                                                */
    /**************************************************************************/
    returnError = trascodeVqObj.init(inputFilePath, outputFilePath,
                    pConfigCtrl, &verboseMode);
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to initialize transcodeVQ" << " Error code = "
                        << returnError << std::endl;
        std::cout
                        << "See the generated TranscodeVqErrorlog.txt file for more details.\n";
        printf("FAIL\n");
        return 1;
    }
    else
    {
        printf("Initialization successful\n");
    }
    /**************************************************************************/
    /* Instantiates the Requited MFTs and builds the topology                 */
    /**************************************************************************/
    returnError = trascodeVqObj.setUp();
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to build/load topology" << std::endl;
        DISPLAYERROR(trascodeVqObj.getMicrosoftErrorCode());
        std::cout
                        << "See the generated TranscodeVqErrorlog.txt file for more details.\n";
        printf("FAIL\n");
        return 1;
    }
    else
    {
        printf("Topology set up successful\n");
    }
    /**************************************************************************/
    /* Run MFT topology                                                       */
    /**************************************************************************/
    printf("\nTranscoding file %s ...\n", inputFilePath);
    returnError = trascodeVqObj.run();
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to run topology" << std::endl;
        DISPLAYERROR(trascodeVqObj.getMicrosoftErrorCode());
        std::cout
                        << "See the generated TranscodeVqErrorlog.txt file for more details.\n";
        printf("FAIL\n");
        return 1;
    }
    else
    {
        printf("Transcoding completed output written to : %s\n", outputFilePath);
    }
    /**************************************************************************/
    /* Destroy the Video Editor                                               */
    /**************************************************************************/
    returnError = trascodeVqObj.destroy(verboseMode);
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to destroy transcodeVQ object" << " Error code = "
                        << returnError << std::endl;
        std::cout
                        << "See the generated TranscodeVqErrorlog.txt file for more details.\n";
        printf("FAIL\n");
        return 1;
    }
    printf("PASS\n");
    return 0;
}