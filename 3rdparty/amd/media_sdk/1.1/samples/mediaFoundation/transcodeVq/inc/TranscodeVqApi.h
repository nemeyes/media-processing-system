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
 * @file <TranscodeVqApi.h>                          
 *                                       
 * @brief This file contains transcode API class
 *         
 ********************************************************************************
 */
#ifndef TRANSCODEVQAPI_H_
#define TRANSCODEVQAPI_H_

#include <atlbase.h>
#include <atlconv.h>
#include <initguid.h>
#include <iostream>
#include "Typedef.h"
#include "TranscodeVqSession.h"
#include "ErrorCodes.h"
#include "PrintLog.h"

/**
 * VideoEditAPI 
 * Video resize APIs
 **/
class msdk_CTranscodeVqApi
{
private:
    CComPtr<CTranscodeVqSession> mTranscodeVqSsn;
    HRESULT mLastMsftErrorCode;
    FILE *mLogFilep;

public:
    /** 
     ****************************************************************************
     *  @fn     resetLastMsftErrorCode
     *  @brief  Resets last error code value
     *          
     *  @return  : void.
     ****************************************************************************
     */
    void resetLastMsftErrorCode()
    {
        mLastMsftErrorCode = S_OK;
    }
    /** 
     ****************************************************************************
     *  @fn     getMicrosoftErrorCode
     *  @brief  Returns microsoft error code for the MFT
     *          
     *  @return  : void.
     ****************************************************************************
     */
    HRESULT getMicrosoftErrorCode()
    {
        return mLastMsftErrorCode;
    }
    /** 
     ****************************************************************************
     *  @fn     init
     *  @brief  Initializes the transcode VQ pipeline 
     *          
     *  @return int : returns error code.
     ****************************************************************************
     */
    int init(int8* sourceFileName, int8* sinkFileName, ConfigCtrl *pConfigCtrl,
                    uint32 *verboseMode)
    {
        if (*verboseMode > 0)
        {
            mLogFilep = fopen("TranscodeVqErrorlog.txt", "w");
        }
        else
        {
            mLogFilep = NULL;
        }

        mLastMsftErrorCode = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (S_OK != mLastMsftErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "Com initialization failed @",
                            __FILE__, __LINE__);
            return (ERR_COM_INITILIZATION_FAILED | ERR_INITILIZATION_FAILED);
        }
        mLastMsftErrorCode = MFStartup(MF_VERSION);
        if (S_OK != mLastMsftErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "MFStartup failed @", __FILE__,
                            __LINE__);
            return (ERR_MFT_INITILIZATION_FAILED | ERR_INITILIZATION_FAILED);
        }
        mTranscodeVqSsn = new (std::nothrow) CTranscodeVqSession();
        mLastMsftErrorCode = mTranscodeVqSsn->setParameters(sourceFileName,
                        sinkFileName, pConfigCtrl);
        if (S_OK != mLastMsftErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "Failed to SetParameters @",
                            __FILE__, __LINE__);
            return (ERR_SESSOIN_INITIALIZATION_FAILED
                            | ERR_INITILIZATION_FAILED);
        }

        if (*verboseMode > 1)
        {
            mTranscodeVqSsn->setLogFile(mLogFilep);
        }

        return ERR_NO_ERROR;
    }

    /** 
     ****************************************************************************
     *  @fn     setUp
     *  @brief  Builds and loads MFT topology
     *           
     *  @return bool : true if successful; otherwise false.
     ****************************************************************************
     */
    int setUp()
    {

        mLastMsftErrorCode = mTranscodeVqSsn->instantiateTopology();
        if (S_OK != mLastMsftErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "Failed to intantiate topology @",
                            __FILE__, __LINE__);
            return (ERR_BUILDING_TOPOLOGY_FAILED | ERR_SETUP_FAILED);
        }
        mLastMsftErrorCode = mTranscodeVqSsn->buildAndLoadTopology();
        if (S_OK != mLastMsftErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ",
                            "Failed to build and load topology @", __FILE__,
                            __LINE__);
            return (ERR_BUILDING_TOPOLOGY_FAILED | ERR_SETUP_FAILED);
        }
        return ERR_NO_ERROR;
    }

    /** 
     ****************************************************************************
     *  @fn     run
     *  @brief  Runs MFT topology
     *           
     *  @return bool : true if successful; otherwise false.
     ****************************************************************************
     */
    int run()
    {
        mLastMsftErrorCode = mTranscodeVqSsn->Run();
        if (S_OK != mLastMsftErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "Transcoding failed @", __FILE__,
                            __LINE__);
            return (ERR_TRANSCODING_FAILED | ERR_RUN_FAILED);
        }
        return ERR_NO_ERROR;
    }
    /** 
     ****************************************************************************
     *  @fn     destroy
     *  @brief  Shuts down Media Foundation and uninitializes COM
     *
     *  @return bool : true if successful; otherwise false.
     ****************************************************************************
     */
    int destroy(uint32 verbose)
    {
        mTranscodeVqSsn.Release();
        MFShutdown();
        CoUninitialize();

        if (verbose > 1)
        {
            fclose(mLogFilep);
        }
        return ERR_NO_ERROR;
    }
};

#endif
