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
 * @file <ScreenCaptureEncMain.cpp>
 *
 * @brief Main file
 *
 ********************************************************************************
 */
#include  <stdio.h>
#include  <signal.h>
#include  <stdlib.h>
#include "ScreenCaptureEncApi.h"

volatile int gCleanup = 0;
void INThandler(int sig)
{
    char c;

    signal(sig, SIG_IGN);
    printf(
                    "Do you really want to quit? if Yes, Type " "y" " or " "Y" " and Press Enter.\n");
    c = (char) getchar();
    if (c == 'y' || c == 'Y')
    {
        gCleanup = 1;
    }
    else
    {
        gCleanup = 0;
        signal(SIGINT, INThandler);
    }
    getchar(); // Get new line character
}

/**
 *******************************************************************************
 *  @fn     parseArguments
 *  @brief  This function parses the configuration file
 *
 *  @param[in] argc : Number of arguments
 *  @param[in] argv : Pointer to the
 *  @param[out] outputFile : Output.[ts/h264/lpcm] file
 *  @param[out] configFilename : Configuration file
 *  @param[in] verboseMode : for logging information
 *
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool parseArguments(int32 argc, int8* argv[], int8 *outputFile,
                int8 *configFilename, uint32 *verboseMode)
{
    /**************************************************************************
     * Processing the command line and configuration file                     *
     **************************************************************************/
    int32 index = 0;
    int32 argCheck = 0;
    while (index < argc)
    {
        if (strncmp(argv[index], "-h", 2) == 0)
        {
            std::cout
                            << "Help on Screen Capture Encoding usages and configurations...\n";
            std::cout
                            << "screenCapture.exe -o output -c config.cfg -l <1 or 2>\n";
            return false;
        }

        /**********************************************************************
         * Processing working directory and output file                       *
         **********************************************************************/
        if (strncmp(argv[index], "-o", 2) == 0)
        {
            if (argv[index + 1] == NULL)
                return false;
            strcpy(outputFile, argv[index + 1]);
            argCheck++;
        }
        /**********************************************************************
         * Processing working directory and .cfg file                         *
         **********************************************************************/
        if (strncmp(argv[index], "-c", 2) == 0)
        {
            if (argv[index + 1] == NULL)
                return false;
            strcpy(configFilename, argv[index + 1]);
            argCheck++;
        }
        /**********************************************************************
         * logging level                                                      *
         **********************************************************************/
        if (strncmp(argv[index], "-l", 2) == 0)
        {
            if (argv[index + 1] == NULL)
                return false;
            *verboseMode = atoi(argv[index + 1]);
        }

        index++;
    }
    if (argCheck < 2)
    {
        std::cout << "Help on Dem encoding usages and configurations...\n";
        std::cout << "screenCapture.exe -o output -c config.cfg -l <1 or 2>\n";
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
 *  @param[in] argv : Pointer to the argument values
 *
 *  @return int : 0 if successful; otherwise error code.
 *******************************************************************************
 */
int32 main(int32 argc, int8* argv[])
{
    int8 outputFilePath[100];
    int8 configFilePath[100];
    EncoderConfigCtrl configCtrl;
    CScreenCaptureEncApi *screenCaptureEncObj;
    bool status;
    int32 returnError = ERR_NO_ERROR;
    uint32 verboseMode = 0;

    /**************************************************************************
     * Parse arguments                                                        *
     * Usage : screenCapture.exe -o outputfile -c config.cfg -l <1 or 2>      *
     **************************************************************************/
    status = parseArguments(argc, argv, outputFilePath, configFilePath,
                    &verboseMode);
    if (status == false)
    {
        printf("FAIL\n");
        return 1;
    }

    /**************************************************************************
     * Get the pointer to configuration controls                              *
     * This also parses the config.cfg                                        *
     **************************************************************************/
    EncoderConfigCtrl *pConfigCtrl = (EncoderConfigCtrl *) &configCtrl;
    memset(pConfigCtrl, 0, sizeof(EncoderConfigCtrl));
    if (parseConfigFile(pConfigCtrl, configFilePath) != true)
    {
        printf("FAIL\n");
        return 1;
    }

    /**************************************************************************
     * This function sets the parameters parsed from the configuration file   *
     * and creates the Dem Encoder                                            *
     **************************************************************************/
    screenCaptureEncObj = new CScreenCaptureEncApi;
    returnError = screenCaptureEncObj->init(pConfigCtrl, outputFilePath,
                    &verboseMode);
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to initialize screenCapture" << " Error code = "
                        << returnError << std::endl;
        printf("FAIL\n");
        return returnError;
    }

    /**************************************************************************
     * To catch "CTRL + C" signal                                             *
     **************************************************************************/
    signal(SIGINT, INThandler);

    /**************************************************************************
     * Creates both RemoteDisplay and Encoder                                 *
     **************************************************************************/
    returnError = screenCaptureEncObj->setUp(pConfigCtrl);
    if (ERR_NO_ERROR != returnError)
    {
        if ((screenCaptureEncObj->mSetupErrorCode == ERR_ENCODERCREATE_FAILED)
                        || (screenCaptureEncObj->mSetupErrorCode
                                        == ERR_CHANGERESOLUTION_FAILED))
            screenCaptureEncObj->remoteDisplayDestroy();
        else if (screenCaptureEncObj->mSetupErrorCode
                        == ERR_ENCODERCONFIG_FAILED)
            screenCaptureEncObj->destroy(&verboseMode);
        std::cerr << "Failed to setUp screenCapture" << " Error code = "
                        << returnError << std::endl;
        printf("FAIL\n");
        return returnError;
    }

    /**************************************************************************
     * Capture the frame(s) for remote display and encode it                  *
     **************************************************************************/
    returnError = screenCaptureEncObj->run();
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to run screenCapture" << " Error code = "
                        << returnError << std::endl;
        printf("FAIL\n");
        return returnError;
    }

    /**************************************************************************
     * Destroy both the encoder and remote display                            *
     **************************************************************************/
    returnError = screenCaptureEncObj->destroy(&verboseMode);
    if (ERR_NO_ERROR != returnError)
    {
        std::cerr << "Failed to destroy screenCapture" << " Error code = "
                        << returnError << std::endl;
        printf("FAIL\n");
        return returnError;
    }

    printf("PASS\n");
    exit(0);
}
