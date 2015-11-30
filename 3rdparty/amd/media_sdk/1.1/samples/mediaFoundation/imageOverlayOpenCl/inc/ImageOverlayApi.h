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
 * @file <ImageOverlayApi.h>
 *
 * @brief Contains declaration for Image Overlay MFT pipeline preparation and run functions
 *
 ********************************************************************************
 */
#ifndef IMAGEOVERLAYAPI_H_
#define IMAGEOVERLAYAPI_H_

#include <atlbase.h>
#include <atlconv.h>
#include <initguid.h>
#include <iostream>
#include "Typedef.h"
#include "ErrorCodes.h"
#include "TranscodeSession.h"
#include "ImageOverlayConfig.h"
#include "PrintLog.h"

/*********************************************************
 * GUID for custom MFT used for overlaying using OpenCl    *
 *********************************************************/
// {785F30C7-0285-4792-9D12-06A32CD0913E}
DEFINE_GUID(CLSID_Overlay_OpenCLMFTDx11, 0x785f30c7, 0x285, 0x4792, 0x9d, 0x12, 0x6, 0xa3, 0x2c, 0xd0, 0x91, 0x3e);

// {3AA47257-9C08-49D9-8FC2-18D2D435DDA4}
DEFINE_GUID(CLSID_Overlay_OpenCLMFTDx9, 0x3aa47257, 0x9c08, 0x49d9, 0x8f, 0xc2, 0x18, 0xd2, 0xd4, 0x35, 0xdd, 0xa4);

// {98230571-0087-4204-B020-3282538E57D3}
DEFINE_GUID(CLSID_MS_ColorConvertMFT, 0x98230571, 0x0087, 0x4204, 0xB0, 0x20, 0x32, 0x82, 0x53, 0x8E, 0x57, 0xD3);

// {D79203E4-E0C4-4B32-AEEF-B79EC75CD6BF}
DEFINE_GUID(CLSID_ColorConvert_OpenCLMFTDx11, 0xd79203e4, 0xe0c4, 0x4b32, 0xae, 0xef, 0xb7, 0x9e, 0xc7, 0x5c, 0xd6, 0xbf);

// {2EECEF7B-93B4-4428-ADFB-BDF2462D1172}
DEFINE_GUID(CLSID_ColorConvert_OpenCLMFTDx9, 0x2eecef7b, 0x93b4, 0x4428, 0xad, 0xfb, 0xbd, 0xf2, 0x46, 0x2d, 0x11, 0x72);

/**
 * VideoEditAPI
 * Video overlay APIs
 **/
class msdk_CVideoEditApi
{

private:
    CComPtr<CTranscodeSession> mTranscodeSsn;
    HRESULT mLastMsftErrorCode;
    FILE *mLogFilep;
    uint32 mVerboseMode;
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
     *  @brief  Initializes the video edit
     *
     *  @return int : ERR_NO_ERROR if successful; otherwise appropriate error.
     ****************************************************************************
     */
    int init(int8* sourceFileName, int8* sinkFileName, ConfigCtrl *pConfigCtrl,
                    uint32 *verboseMode)
    {
        mVerboseMode = *verboseMode;
        if (mVerboseMode > 0)
        {
            mLogFilep = fopen("ImageOverlayOpenClErrorLog.txt", "w");
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
        mTranscodeSsn = new (std::nothrow) CTranscodeSession();
        mLastMsftErrorCode = mTranscodeSsn->setParameters(sourceFileName,
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
            mTranscodeSsn->setLogFile(mLogFilep);
        }
        return ERR_NO_ERROR;
    }

    /**
     ****************************************************************************
     *  @fn     setUp
     *  @brief  Builds and loads MFT topology
     *
     *  @return bool : ERR_NO_ERROR if successful; otherwise appropriate error.
     ****************************************************************************
     */
    int setUp()
    {
        mLastMsftErrorCode = mTranscodeSsn->instantiateTopology();
        if (S_OK != mLastMsftErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "Failed to instantiate topology @",
                            __FILE__, __LINE__);
            return (ERR_BUILDING_TOPOLOGY_FAILED | ERR_SETUP_FAILED);
        }

        mLastMsftErrorCode = mTranscodeSsn->buildAndLoadTopology();
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
     *  @return bool : ERR_NO_ERROR if successful; otherwise appropriate error.
     ****************************************************************************
     */
    int run()
    {
        mLastMsftErrorCode = mTranscodeSsn->Run();
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
     *  @return bool : ERR_NO_ERROR if successful; otherwise appropriate error.
     ****************************************************************************
     */
    int destroy()
    {
        mTranscodeSsn.Release();
        MFShutdown();
        CoUninitialize();
        if (mLogFilep)
            fclose(mLogFilep);
        return ERR_NO_ERROR;
    }

};
#endif
