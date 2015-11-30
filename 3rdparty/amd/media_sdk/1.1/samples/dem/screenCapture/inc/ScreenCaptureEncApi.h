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
 * @file <ScreenCaptureEncApi.h>
 *
 * @brief This file contains Screen capture encode API class
 *
 ********************************************************************************
 */

#ifndef SCREENCAPTURE_ENC_API_H__
#define SCREENCAPTURE_ENC_API_H__

#include "ScreenCaptureEncConfig.h"
#include "VideoEncoderVCEDEM.h"
#include "ErrorCodes.h"
#include "PrintLog.h"
#include <memory>
#include <string>
#include <iostream>
#include <windows.h>

extern volatile int gCleanup;

using namespace amf;

static const int OutputStatisticsTimeout = 1000; // Milliseconds

struct Logger
{
    enum LogMessageType
    {
        Info, Success, Error
    };

    virtual ~Logger()
    {
    }
    ;

    virtual void
                    logMessage(LogMessageType type, const std::wstring& message) = 0;
    virtual bool waitForDisplayChangeEvent(DWORD time) = 0;
    virtual void resetDisplayChangeEvent() = 0;
    virtual void updatePropertyControl(const amf::AMFPropertyInfo& propInfo,
                    const amf::AMFVariant& value) =0;

    virtual void logMessage(LogMessageType type, const std::string& message)
    {
        logMessage(type, std::wstring(message.begin(), message.end()));
    }
};

struct AmfException
{
    std::wstring operation;
    AMF_RESULT error;

    AmfException(const std::wstring& operation, AMF_RESULT error) :
        operation(operation), error(error)
    {
    }
};

/**
 * Screen capturing APIs
 * Encoding APIs
 **/
class CScreenCaptureEncApi
{
private:
    // Logger passed to constructor
    Logger* mLog;

    // Buffer counters
    struct Counter
    {
        unsigned count;
        unsigned size;

        Counter() :
            count(0), size(0)
        {
        }
    };
    std::map<amf::DemOutputType, Counter> mCounters;
    amf_pts mQueryStartTicks;
    amf_pts mPreviousFrameTicks;
    amf_int64 mPreviousTimestamp;

    std::wstring mFileNameBase;
    std::map<std::wstring, std::shared_ptr<std::ofstream>> mOutputs;

    std::ostream& getOutput(const std::wstring& extension);

    AMFEncoderVCEDEMPtr mEncoder;
    AMFDemBufferPtr mBuffer;
    int32 mFramesToEncode;
    AMF_RESULT mLastAmfErrorCode;
    FILE *mLogFilep;
public:
    int32 mSetupErrorCode;
    /**
     ****************************************************************************
     *  @fn     resetLastMsftErrorCode
     *  @brief  Resets last error code value
     *
     *  @return : void
     ****************************************************************************
     */
    void resetLastAmfErrorCode()
    {
        mLastAmfErrorCode = AMF_OK;
    }
    /**
     ****************************************************************************
     *  @fn     getMsftErrorCode
     *  @brief  Returns AMF error code value
     *
     *  @return AMF_RESULT : returns error code.
     ****************************************************************************
     */
    AMF_RESULT getAmfErrorCode()
    {
        return mLastAmfErrorCode;
    }
    /**
     ****************************************************************************
     *  @fn     init
     *  @brief  Initializes the screen capture and creates dem encoder
     *
     *  @param[in] pConfig : Encoder configuration
     *  @param[in] outputFilePath : encoded bitstream file path
     *  @param[in] verboseMode : for logging information
     *
     *  @return AMF_RESULT : returns error code.
     ****************************************************************************
     */
    int32 init(EncoderConfigCtrl *pConfig, int8 outputFilePath[],
                    uint32 *verboseMode)
    {
        mLogFilep = NULL;
        if(*verboseMode > 0)
        {
            mLogFilep = fopen("ScreenCaptureLog.txt", "w");
        }

        mLastAmfErrorCode = initialization(pConfig, outputFilePath);
        if (AMF_OK != mLastAmfErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "Initialization failed @", __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_INITILIZATION_FAILED);
        }
        return ERR_NO_ERROR;
    }

    /**
    ****************************************************************************
    *  @fn     setUp
    *  @brief  Creates both remote display and the encoder
    *
    *  @param[in] pConfig : Encoder configuration
    *
    *  @return AMF_RESULT : returns error code.
    ****************************************************************************
    */
    int32 setUp(EncoderConfigCtrl *pConfig)
    {
        int32 status;
        DISPLAY_DEVICE dispDev;

        /**********************************************************************
         * Acquiring Remote Display                                           *
         **********************************************************************/
        mLastAmfErrorCode = remoteDisplayCreate();
        if (AMF_OK != mLastAmfErrorCode)
        {
            mSetupErrorCode = ERR_REMOTEDISPLAYCREATE_FAILED;
            LOG(mLogFilep, "%s %s %d \n ", "remoteDisplayCreate failed @", __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_SETUP_FAILED);
        }

        /**********************************************************************
         * Changing Display Resolution                                        *
         **********************************************************************/
        dispDev.cb = sizeof(DISPLAY_DEVICE);

        if (!EnumDisplayDevices(NULL, 0, &dispDev, 0))
        {
            mSetupErrorCode = ERR_CHANGERESOLUTION_FAILED;
            status = GetLastError();
            printf("EnumDisplayDevices failed: %d, %s @ line %d\n", status, __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_SETUP_FAILED);
        }

        DISPLAY_DEVICE monitor;
        monitor.cb = sizeof(DISPLAY_DEVICE);
        if (!EnumDisplayDevices(dispDev.DeviceName, 0, &monitor, 0))
        {
            mSetupErrorCode = ERR_CHANGERESOLUTION_FAILED;
            status = GetLastError();
            printf("EnumDisplayDevices failed: %d, %s @ line %d\n", status, __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_SETUP_FAILED);
        }

        DEVMODE devMode;
        devMode.dmSize = sizeof(DEVMODE);
        if (!EnumDisplaySettings(dispDev.DeviceName, ENUM_CURRENT_SETTINGS, &devMode))
        {
            mSetupErrorCode = ERR_CHANGERESOLUTION_FAILED;
            status = GetLastError();
            printf("EnumDisplaySettings failed: %d, %s @ line %d\n", status, __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_SETUP_FAILED);
        }

        devMode.dmPelsWidth = pConfig->width;
        devMode.dmPelsHeight = pConfig->height;
        status = ChangeDisplaySettings(&devMode, CDS_TEST);
        if(status == DISP_CHANGE_FAILED)
        {
            mSetupErrorCode = ERR_CHANGERESOLUTION_FAILED;
            printf("Unable to process the request, ChangeDisplaySettings failed: %d, %s @ line %d\n", status, __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_SETUP_FAILED);
        }
        else
        {
            status = ChangeDisplaySettings(&devMode, CDS_UPDATEREGISTRY);

            switch(status)
            {
            case DISP_CHANGE_SUCCESSFUL:
                break;
            case DISP_CHANGE_RESTART:
                mSetupErrorCode = ERR_CHANGERESOLUTION_FAILED;
                printf("Need to reboot for the change to happen.\n");
                return (mLastAmfErrorCode | ERR_SETUP_FAILED);
                break;
            default:
                mSetupErrorCode = ERR_CHANGERESOLUTION_FAILED;
                printf("Failed to change the resolution, ChangeDisplaySettings failed: %d, %s @ line %d\n", status, __FILE__, __LINE__);
                return (mLastAmfErrorCode | ERR_SETUP_FAILED);
            }
        }

        /**********************************************************************
         * Cloning the Remote Display                                         *
         **********************************************************************/
        status = SetDisplayConfig(0, NULL, 0, NULL, (SDC_APPLY | SDC_TOPOLOGY_CLONE));
        if(status != ERROR_SUCCESS)
        {
            mSetupErrorCode = ERR_CHANGERESOLUTION_FAILED;
            printf("Cloning of the displays failed: %d, %s @ line %d\n", status, __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_SETUP_FAILED);
        }

        if(mLastAmfErrorCode == AMF_OK)
        {
            mLastAmfErrorCode = encCreate();
            if (AMF_OK != mLastAmfErrorCode)
            {
                mSetupErrorCode = ERR_ENCODERCREATE_FAILED;
                LOG(mLogFilep, "%s %s %d \n ", "encCreate failed @", __FILE__, __LINE__);
                return (mLastAmfErrorCode | ERR_SETUP_FAILED);
            }
        }

        /**********************************************************************
         * Changing config params                                             *
         **********************************************************************/
        int32 rateControlMethod = pConfig->rateControlMethod;
        int32 encInloopDeblockFilter = pConfig->encInloopDeblockFilter;
        int32 bitrate = pConfig->bitRate;
        int32 peakBitrate = pConfig->peakBitRate;
        int32 idrPeriod = pConfig->idrPeriod;
        int32 skippedPicPeriod = pConfig->skippedPicPeriod;
        int32 slicesPerFrame = pConfig->slicesPerFrame;
        int32 intraRefreshMBperSlot = pConfig->intraRefreshMBperSlot;
        int32 initialVBVbuffFullness = pConfig->initialVBVbuffFullness;
        int32 vbvBuffSize = pConfig->vbvBuffSize;
        int32 minQP = pConfig->minQP;
        int32 maxQP = pConfig->maxQP;
        int32 usage = pConfig->usage;

        mFramesToEncode = pConfig->framesToEncode;

        status = mEncoder->SetProperty(DEM_TARGET_BITRATE, AMFVariant(bitrate));
        if(status != AMF_OK)
        {
            mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
            LOGIFFAILEDAMF(mLogFilep, status, "DEM_BITRATE property setting failed @ %s, %d\n", __FILE__, __LINE__);
        }

        status = mEncoder->SetProperty(DEM_PEAK_BITRATE, AMFVariant(peakBitrate));
        if(status != AMF_OK)
        {
            mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
            LOGIFFAILEDAMF(mLogFilep, status, "DEM_PEAK_BITRATE property setting failed @ %s, %d\n", __FILE__, __LINE__);
        }

        status = mEncoder->SetProperty(DEM_IDR_PERIOD, AMFVariant(idrPeriod));
        if(status != AMF_OK)
        {
            mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
            LOGIFFAILEDAMF(mLogFilep, status, "DEM_IDR_PERIOD property setting failed @ %s, %d, %d(min), %d(max)\n", __FILE__, __LINE__, DEM_IDR_PERIOD_MIN, DEM_IDR_PERIOD_MAX);
        }

        status = mEncoder->SetProperty(DEM_SKIPPED_PIC_PERIOD, AMFVariant(skippedPicPeriod));
        if(status != AMF_OK)
        {
            mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
            LOGIFFAILEDAMF(mLogFilep, status, "DEM_SKIPPED_PIC_PERIOD property setting failed @ %s, %d, %d(min), %d(max)\n", __FILE__, __LINE__, DEM_SKIPPED_PIC_PERIOD_MIN, DEM_SKIPPED_PIC_PERIOD_MAX);
        }

        if(usage != DEM_USAGE_WIRELESS_DISPLAY)
        {
            status = mEncoder->SetProperty(DEM_SLICES_PER_FRAME, AMFVariant(slicesPerFrame));
            if(status != AMF_OK)
            {
                mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
                LOGIFFAILEDAMF(mLogFilep, status, "DEM_SLICES_PER_FRAME property setting failed @ %s, %d\n", __FILE__, __LINE__);
            }
        }

        status = mEncoder->SetProperty(DEM_INTRA_REFRESH_MB_PER_SLOT, AMFVariant(intraRefreshMBperSlot));
        if(status != AMF_OK)
        {
            mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
            LOGIFFAILEDAMF(mLogFilep, status, "DEM_INTRA_REFRESH_MB_PER_SLOT property setting failed @ %s, %d\n", __FILE__, __LINE__);
        }

        if(usage == DEM_USAGE_GENERIC)
        {
            status = mEncoder->SetProperty(DEM_RATE_CONTROL_METHOD, AMFVariant(rateControlMethod));
            if(status != AMF_OK)
            {
                mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
                LOGIFFAILEDAMF(mLogFilep, status, "DEM_RATE_CONTROL_METHOD property setting failed @ %s, %d\n", __FILE__, __LINE__);
            }

            status = mEncoder->SetProperty(DEM_INLOOP_DEBLOCKING, AMFVariant(encInloopDeblockFilter));
            if(status != AMF_OK)
            {
                mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
                LOGIFFAILEDAMF(mLogFilep, status, "DEM_INLOOP_DEBLOCKING property setting failed @ %s, %d\n", __FILE__, __LINE__);
            }

            status = mEncoder->SetProperty(DEM_INITIAL_VBV_BUFFER_FULLNESS, AMFVariant(initialVBVbuffFullness));
            if(status != AMF_OK)
            {
                mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
                LOGIFFAILEDAMF(mLogFilep, status, "DEM_INITIAL_VBV_BUFFER_FULLNESS property setting failed @ %s, %d, %d(min), %d(max)\n", __FILE__, __LINE__, DEM_INITIAL_VBV_BUFFER_FULLNESS_MIN, DEM_INITIAL_VBV_BUFFER_FULLNESS_MAX);
            }

            status = mEncoder->SetProperty(DEM_VBV_BUFFER_SIZE, AMFVariant(vbvBuffSize));
            if(status != AMF_OK)
            {
                mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
                LOGIFFAILEDAMF(mLogFilep, status, "DEM_VBV_BUFFER_SIZE property setting failed @ %s, %d\n", __FILE__, __LINE__);
            }

            status = mEncoder->SetProperty(DEM_MIN_QP, AMFVariant(minQP));
            if(status != AMF_OK)
            {
                mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
                LOGIFFAILEDAMF(mLogFilep, status, "DEM_MIN_QP property setting failed @ %s, %d\n", __FILE__, __LINE__);
            }

            status = mEncoder->SetProperty(DEM_MAX_QP, AMFVariant(maxQP));
            if(status != AMF_OK)
            {
                mSetupErrorCode = ERR_ENCODERCONFIG_FAILED;
                LOGIFFAILEDAMF(mLogFilep, status, "DEM_MAX_QP property setting failed @ %s, %d\n", __FILE__, __LINE__);
            }
        }

        return status;
    }

    /**
    ****************************************************************************
    *  @fn     run
    *  @brief  Capturing the remote display and encode
    *
    *  @return AMF_RESULT : returns error code.
    ****************************************************************************
    */
    int32 run()
    {
        int32 i;
        mLastAmfErrorCode = encStart();
        if (AMF_OK != mLastAmfErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "encStart failed @", __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_RUN_FAILED);
        }

        std::cout << "Screen Capturing started ... \n";
        for(i=0; i<mFramesToEncode; i++)
        {
            if(gCleanup)
            {
                std::cout << "Total " << i << " frames Captured\n";
                LOG(mLogFilep, "Total %d frames Captured\n", i);
                mLastAmfErrorCode = encStop();
                if (AMF_OK != mLastAmfErrorCode)
                {
                    LOG(mLogFilep, "%s %s %d \n ", "encStop failed @", __FILE__, __LINE__);
                    return (mLastAmfErrorCode | ERR_RUN_FAILED);
                }

                return ERR_NO_ERROR;
            }

            getNextFrame();
        }

        std::cout << "Total " << i << " frames Captured\n";
        LOG(mLogFilep, "%d frames Captured\n", i);

        mLastAmfErrorCode = encStop();
        if (AMF_OK != mLastAmfErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "encStop failed @", __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_RUN_FAILED);
        }

        return ERR_NO_ERROR;
    }

    /**
    ****************************************************************************
    *  @fn     destroy
    *  @brief  destroy both the encoder and remote display
    *
    *  @param[in] verboseMode : for logging information
    *
    *  @return AMF_RESULT : returns error code.
    ****************************************************************************
    */
    int32 destroy(uint32 *verboseMode)
    {
        mLastAmfErrorCode = encDestroy();
        if (AMF_OK != mLastAmfErrorCode)
        {
            LOG(mLogFilep, "%s %s %d \n ", "encDestroy failed @", __FILE__, __LINE__);
            return (mLastAmfErrorCode | ERR_DESTROY_FAILED);
        }

        if(mLastAmfErrorCode == AMF_OK)
        {
            mLastAmfErrorCode = remoteDisplayDestroy();
            if (AMF_OK != mLastAmfErrorCode)
            {
                LOG(mLogFilep, "%s %s %d \n ", "remoteDisplayDestroy failed @", __FILE__, __LINE__);
                return (mLastAmfErrorCode | ERR_DESTROY_FAILED);
            }
        }

        if(*verboseMode > 0)
        {
            fclose(mLogFilep);
        }

        return ERR_NO_ERROR;
    }

    AMF_RESULT initialization(EncoderConfigCtrl *pConfig, int8 outputFilePath[]);
    AMF_RESULT remoteDisplayCreate();
    AMF_RESULT encCreate();
    AMF_RESULT encStart();
    AMF_RESULT encStop();
    AMF_RESULT encDestroy();
    AMF_RESULT remoteDisplayDestroy();
    void getNextFrame();
};

#endif //SCREENCAPTURE_ENC_API_H__
