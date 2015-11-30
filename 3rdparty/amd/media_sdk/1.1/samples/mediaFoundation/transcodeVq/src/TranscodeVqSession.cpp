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
 * @file <TranscodeVqSession.cpp>
 *
 * @brief This file contains functions for creating the transcode with VQ pipeline
 *
 ********************************************************************************
 */
#include <initguid.h>
#include <cguid.h>
#include <wmcodecdsp.h>
//#include "AmfMft.h"
#include "TranscodeVqSession.h"
#include "ErrorCodes.h"
#include "TranscodeVqApi.h"
/**
 *******************************************************************************
 *  @fn    CTranscodeVqSession
 *  @brief Constructor
 *
 *
 *******************************************************************************
 */
CTranscodeVqSession::CTranscodeVqSession(void) :
    mRefCount(0)
{
    mTranscodeEndEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    mLogFile = NULL;
    mMftBuilderObjPtr = new msdk_CMftBuilder;
}
/**
 *******************************************************************************
 *  @fn    ~CTranscodeVqSession
 *  @brief Destructor
 *
 *
 *******************************************************************************
 */
CTranscodeVqSession::~CTranscodeVqSession(void)
{
    mMediaSource.Release();
    mPartialTopology.Release();

    mMediaSource.Release();
    mSourcePresentationDescriptor.Release();
    mMediaSinkActivate.Release();
    mSourceStreamDescriptor.Release();
    mDxgiDeviceManager.Release();

    mSourceNode.Release();
    mDecoderNode.Release();
    mVqNode.Release();
    mEncoderNode.Release();
    mSinkNode.Release();

    CloseHandle( mTranscodeEndEvent);
    delete mMftBuilderObjPtr;

}

/**
 *******************************************************************************
 *  @fn     setParameters
 *  @brief  Sets the input, output file name
 *
 *  @param[in] inputFileName        : Input video file for transcoding
 *  @param[in] outputFileName       : Output video file for transcoding
 *  @param[in] pConfigCtrl          : Pointer to the structure cotaining user
 *                                    specified configuration parameters
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::setParameters(int8* inputFileName,
                int8* outputFileName, ConfigCtrl *pConfigCtrl)
{
    if (inputFileName)
    {
        MultiByteToWideChar(CP_ACP, 0, inputFileName, -1, mInputFileName,
                        MAX_PATH);
    }
    if (outputFileName)
    {
        MultiByteToWideChar(CP_ACP, 0, outputFileName, -1, mOutputFileName,
                        MAX_PATH);
    }
    if (pConfigCtrl)
    {
        mPConfigCtrl = *pConfigCtrl;
    }

    return S_OK;
}

void CTranscodeVqSession::setLogFile(FILE *logFile)
{
    mLogFile = logFile;
}

/**
 *******************************************************************************
 *  @fn     instantiateTopology
 *  @brief  Instantiates required MFT filters for building transcode topology
 *
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::instantiateTopology()
{
    HRESULT hr;
    DWORD streamDescriptorCount;
    DWORD streamNumber = 0;
    CComPtr < IMFMediaType > sourceMediaType = NULL;

    bool useDx11 = mMftBuilderObjPtr->isDxgiSupported();

    mMftBuilderObjPtr->setLogFile(mLogFile);

    /**************************************************************************
     *  Create partial topology                                                *
     **************************************************************************/
    hr = MFCreateTopology(&mPartialTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to create partial topology @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Create source filter                                                    *
     **************************************************************************/
    hr = mMftBuilderObjPtr->createSource(mInputFileName, &mMediaSource);
    LOGIFFAILED(mLogFile, hr, "Failed to create source @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mMediaSource->CreatePresentationDescriptor(
                    &mSourcePresentationDescriptor);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to create presentation descriptor @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     *  Get source descriptor count. It will be number of audio/video streams  *
     *  in the container                                                       *
     **************************************************************************/
    hr = mSourcePresentationDescriptor->GetStreamDescriptorCount(
                    &streamDescriptorCount);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to get presentation descriptor count @ %s %d \n",
                    __FILE__, __LINE__);
    /**************************************************************************
     *  Loop until video stream if found                                       *
     **************************************************************************/
    for (DWORD i = 0; i < streamDescriptorCount; i++)
    {
        BOOL selected;
        CComPtr < IMFStreamDescriptor > sourceStreamDescriptor;
        hr = mSourcePresentationDescriptor->GetStreamDescriptorByIndex(i,
                        &selected, &sourceStreamDescriptor);
        RETURNIFFAILED(hr);

        /***********************************************************************
         *  Get the media type from the source descriptor and break if stream   *
         *  type is video.                                                      *
         *  Store the media type for setting preparing output video type        *
         ***********************************************************************/
        CComPtr < IMFMediaTypeHandler > mediaTypeHandler;
        hr = sourceStreamDescriptor->GetMediaTypeHandler(&mediaTypeHandler);
        LOGIFFAILED(mLogFile, hr,
                        "Failed to get media type handler @ %s %d \n",
                        __FILE__, __LINE__);

        CComPtr < IMFMediaType > mediaType;
        hr = mediaTypeHandler->GetCurrentMediaType(&mediaType);
        LOGIFFAILED(mLogFile, hr, "Failed to get media type @ %s %d \n",
                        __FILE__, __LINE__);

        GUID majorType;
        hr = mediaType->GetMajorType(&majorType);
        LOGIFFAILED(mLogFile, hr, "Failed to get major type @ %s %d \n",
                        __FILE__, __LINE__);

        streamNumber = i + 1;
        if (IsEqualGUID(majorType, MFMediaType_Video))
        {
            mSourceStreamDescriptor = sourceStreamDescriptor;
            uint32 testMediatype;
            hr = mediaType->GetUINT32(MF_MT_INTERLACE_MODE, &testMediatype);
            LOGIFFAILED(mLogFile, hr,
                            "Failed to get interlace mode @ %s %d \n",
                            __FILE__, __LINE__);

            if (testMediatype != MFVideoInterlace_Progressive)
            {
                /**************************************************************
                 * Set media type to progressive mode as our encoder does not *
                 * support interlaced content                                 *
                 *************************************************************/
                hr = mediaType->SetUINT32(MF_MT_INTERLACE_MODE,
                                MFVideoInterlace_Progressive);
                LOGIFFAILED(mLogFile, hr,
                                "Failed to set interlace mode @ %s %d \n",
                                __FILE__, __LINE__);
            }
            sourceMediaType = mediaType;
            break;
        }
    }
    if (sourceMediaType == NULL)
    {
        return ERR_VIDEOSTREAM_NOTFOUND;
    }
    /**************************************************************************
     *  Create H264 video type using the source media type                     *
     **************************************************************************/
    CComPtr < IMFMediaType > h264VideoType;
    createH264VideoType(&h264VideoType, sourceMediaType);
    hr = mMftBuilderObjPtr->createSinkActivate(mSourcePresentationDescriptor,
                    mOutputFileName, h264VideoType, &mMediaSinkActivate);
    LOGIFFAILED(mLogFile, hr, "Failed to create sink filter @ %s %d \n",
                    __FILE__, __LINE__);
    /**************************************************************************
     *  Instantiate the filters required for transcoding                       *
     **************************************************************************/
    hr = instantiateVideoStream(streamNumber, sourceMediaType, h264VideoType,
                    useDx11);

    LOGIFFAILED(mLogFile, hr, "Failed instantiate video stream @ %s %d \n",
                    __FILE__, __LINE__);
    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     instantiateVideoStream
 *  @brief  Instantiates MFTs required for video decoding and encoding
 *
 *  @param[in] streamNumber        : Stream number from the source
 *  @param[in] sourceMediaType     : Source Media Type
 *  @param[in] partialEncodedType  : output media type
 *  @param[in] customTransformGuid : GUID for custom transform
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
/**
 *   @brief CLSID for Color Conversion.
 */
HRESULT CTranscodeVqSession::instantiateVideoStream(DWORD streamNumber,
                IMFMediaType* sourceMediaType,
                IMFMediaType* partialEncodedType, bool useDx11)
{
    HRESULT hr;
    ULONG_PTR deviceManagerPtr;
    if (useDx11)
    {
        /**********************************************************************
         *  Create DX11 device                                                 *
         **********************************************************************/
        hr = mMftBuilderObjPtr->createDxgiDeviceManagerPtr(&deviceManagerPtr);
        LOGIFFAILED(mLogFile, hr, "Failed create Dx11 device @ %s %d \n",
                        __FILE__, __LINE__);

        /* Attach the device manager pointer to be freed later. */
        mMftBuilderObjPtr->dxgiDeviceManager.Attach(
                        reinterpret_cast<IMFDXGIDeviceManager *> (deviceManagerPtr));
    }
    else
    {
        /**********************************************************************
         *  DirectX 9 requires a window.                                       *
         **********************************************************************/
        HWND hWnd = GetDesktopWindow();
        HRESULT hr = mMftBuilderObjPtr->createDirect3DDeviceManagerPtr(hWnd,
                        &deviceManagerPtr);
        LOGIFFAILED(mLogFile, hr, "Failed create Dx9 device @ %s %d \n",
                        __FILE__, __LINE__);

        mMftBuilderObjPtr->d3dDeviceManager.Attach(
                        reinterpret_cast<IDirect3DDeviceManager9 *> (deviceManagerPtr));
    }
    /***************************************************************************
     *  Create source node                                                      *
     ***************************************************************************/
    hr = mMftBuilderObjPtr->createStreamSourceNode(mMediaSource,
                    mSourcePresentationDescriptor, mSourceStreamDescriptor,
                    &mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed create source node @ %s %d \n", __FILE__,
                    __LINE__);
    /***************************************************************************
     *  Create sink node with asf container                                     *
     ***************************************************************************/
    hr = mMftBuilderObjPtr->createStreamSinkNode(mMediaSinkActivate,
                    streamNumber, &mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed create sink node @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     * Create decoder node                                                      *
     * Supports H264,Mpeg 4 VC1                                                 *
     ***************************************************************************/
    hr = mMftBuilderObjPtr->createVideoDecoderNode(sourceMediaType,
                    &mDecoderNode, deviceManagerPtr, NULL,
                    mPConfigCtrl.commonParams.useSWCodec);
    LOGIFFAILED(mLogFile, hr, "Failed create Video decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Create VQ node                                                          *
     ***************************************************************************/
    {
        UINT32 inputImageWidth = 0;
        UINT32 inputImageHeight = 0;
        hr = MFGetAttributeSize(sourceMediaType, MF_MT_FRAME_SIZE,
                        &inputImageWidth, &inputImageHeight);
        RETURNIFFAILED(hr);

        // Get Deinterlacing method from vqTransform.
        UINT32 deinterlacingMethod = 0;

        UINT32 sourceVideoInterlaceMode;
        hr = sourceMediaType->GetUINT32(MF_MT_INTERLACE_MODE,
                        &sourceVideoInterlaceMode);
        RETURNIFFAILED(hr);

        // We admit that other interlaced types except MFVideoInterlace_Progressive are interlaced.
        BOOL isVideoInterlaced = sourceVideoInterlaceMode
                        != MFVideoInterlace_Progressive;

        CComPtr < IAMFCapabilityManager > capabilityManager;
        hr = AMFCreateCapabilityManagerMFT(IID_PPV_ARGS(&capabilityManager));
        RETURNIFFAILED(hr);

        hr = capabilityManager->Init((IUnknown*) deviceManagerPtr,
                        inputImageWidth, inputImageHeight, isVideoInterlaced,
                        deinterlacingMethod);

        if (hr != S_OK)
        {
            mIsVqSupported = false;
            std::cerr << "Platform does not support VQ filter" << std::endl;
            LOG(mLogFile, "Platform does not support VQ filter @ %s %d \n",
                            __FILE__, __LINE__);
        }
        else
        {
            mIsVqSupported = true;
        }

        if (mIsVqSupported == true)
        {
            CComPtr < IMFTransform > vqTransform;

            hr = mMftBuilderObjPtr->createVqTransform(deviceManagerPtr,
                            &vqTransform);
            LOGIFFAILED(mLogFile, hr,
                            "Failed create custrom transform node @ %s %d \n",
                            __FILE__, __LINE__);

            hr
                            = mMftBuilderObjPtr->createTransformNode(
                                            vqTransform,
                                            MF_CONNECT_ALLOW_CONVERTER /*| MF_CONNECT_RESOLVE_INDEPENDENT_OUTPUTTYPES*/,
                                            &mVqNode);

            /************************************************************************
             *  Set VQ settings using user configuration file                        *
             ************************************************************************/
            CComPtr < IMFAttributes > attributes;
            hr = vqTransform->GetAttributes(&attributes);
            LOGIFFAILED(mLogFile, hr, "Failed get VQ attributes @ %s %d \n",
                            __FILE__, __LINE__);
            hr = setVqAttributes(attributes, deviceManagerPtr, sourceMediaType,
                            AMF_CM_REALTIME);
            LOGIFFAILED(mLogFile, hr, "Failed set VQ attributes @ %s %d \n",
                            __FILE__, __LINE__);

            CComPtr < IMFMediaType > vqOutputType;
            hr = mMftBuilderObjPtr->createVideoTypeFromSource(sourceMediaType,
                            MFVideoFormat_NV12, TRUE, TRUE, &vqOutputType);
            LOGIFFAILED(mLogFile, hr,
                            "Failed create custrom transform node @ %s %d \n",
                            __FILE__, __LINE__);
            /************************************************************************
             * get frame rate from source media type and set the same to output video*
             * type                                                                  *
             ************************************************************************/
            // TODO: oefremov: remove this when VQ correctly processes negative stride values.
            hr = vqOutputType->DeleteItem(MF_MT_DEFAULT_STRIDE);
            LOGIFFAILED(mLogFile, hr,
                            "Failed create custrom transform node @ %s %d \n",
                            __FILE__, __LINE__);

            hr = vqTransform->SetOutputType(0, vqOutputType, 0);
            LOGIFFAILED(mLogFile, hr,
                            "Failed create custrom transform node @ %s %d \n",
                            __FILE__, __LINE__);
        }
    }
    /***************************************************************************
     *  Create encoder node. Supports only H264 encoder                         *
     ***************************************************************************/
    hr = mMftBuilderObjPtr->createVideoEncoderNode(partialEncodedType,
                    sourceMediaType, &mEncoderNode, deviceManagerPtr,
                    &mPConfigCtrl.vidParams,
                    mPConfigCtrl.commonParams.useSWCodec);
    LOGIFFAILED(mLogFile, hr, "Failed to create encoder node @ %s %d \n",
                    __FILE__, __LINE__);
    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     createH264VideoType
 *  @brief  Creates H264 video type
 *
 *  @param[out] encodedVideoType   : output video type to be created
 *  @param[in] sourceVideoType     : Source Media Type from which width & height
 *                                   information is used
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::createH264VideoType(
                IMFMediaType** encodedVideoType, IMFMediaType* sourceVideoType)
{
    uint32 srcWidth, srcHeight;
    if (nullptr == encodedVideoType)
    {
        return E_POINTER;
    }

    HRESULT hr;

    /**************************************************************************
     *  Get width and height from the source media type                        *
     **************************************************************************/
    hr = MFGetAttributeSize(sourceVideoType, MF_MT_FRAME_SIZE, &srcWidth,
                    &srcHeight);
    LOGIFFAILED(mLogFile, hr, "Failed to get attributes @ %s %d \n", __FILE__,
                    __LINE__);
    /**************************************************************************
     *  Create video type for storing the commpressed bit stream               *
     **************************************************************************/
    CComPtr < IMFMediaType > videoType;
    hr = mMftBuilderObjPtr->createVideoType(&videoType, MFVideoFormat_H264,
                    FALSE, FALSE, nullptr, nullptr, srcWidth, srcHeight,
                    MFVideoInterlace_Progressive);
    LOGIFFAILED(mLogFile, hr, "Failed to create video type @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     *  Set output bit rate                                                    *
     **************************************************************************/
    hr = videoType->SetUINT32(MF_MT_AVG_BITRATE,
                    mPConfigCtrl.vidParams.meanBitrate);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     *  Set encoding profile                                                   *
     **************************************************************************/
    hr = videoType->SetUINT32(MF_MT_MPEG2_PROFILE,
                    mPConfigCtrl.vidParams.compressionStandard);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    UINT32 numerator;
    UINT32 denominator;
    hr = MFGetAttributeRatio(sourceVideoType, MF_MT_FRAME_RATE, &numerator,
                    &denominator);
    LOGIFFAILED(mLogFile, hr, "Failed to get attributes @ %s %d \n", __FILE__,
                    __LINE__);
    MFSetAttributeRatio(videoType, MF_MT_FRAME_RATE, numerator, denominator);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     * get pixel aspect ratio from source media type and set the same to       *
     * output video type                                                       *
     **************************************************************************/
    hr = MFGetAttributeRatio(sourceVideoType, MF_MT_PIXEL_ASPECT_RATIO,
                    &numerator, &denominator);
    LOGIFFAILED(mLogFile, hr, "Failed to get attributes @ %s %d \n", __FILE__,
                    __LINE__);

    MFSetAttributeRatio(videoType, MF_MT_PIXEL_ASPECT_RATIO, numerator,
                    denominator);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    *encodedVideoType = videoType.Detach();

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     buildAndLoadTopology
 *  @brief  Build and load the topology
 *
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::buildAndLoadTopology()
{
    HRESULT hr = S_OK;

    /**************************************************************************
     * Add source & sink node to the topology                                  *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add source node @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mPartialTopology->AddNode(mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add sink node @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     * Add decoder, VQ & encoder node to the topology                          *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mDecoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed to decoder node @ %s %d \n", __FILE__,
                    __LINE__);

    if (mIsVqSupported == true)
    {
        hr = mPartialTopology->AddNode(mVqNode);
        LOGIFFAILED(mLogFile, hr,
                        "Failed to add Video quality node @ %s %d \n",
                        __FILE__, __LINE__);
    }

    hr = mPartialTopology->AddNode(mEncoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add encoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Connect sourceNode->DecoderNode->Vqnode->EncoderNode->sinkNode          *
     **************************************************************************/
    hr = mSourceNode->ConnectOutput(0, mDecoderNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect source node->decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    if (mIsVqSupported == true)
    {
        hr = mDecoderNode->ConnectOutput(0, mVqNode, 0);
        LOGIFFAILED(mLogFile, hr,
                        "Failed to connect decoder node->vq node @ %s %d \n",
                        __FILE__, __LINE__);

        hr = mVqNode->ConnectOutput(0, mEncoderNode, 0);
        LOGIFFAILED(mLogFile, hr,
                        "Failed to connect vq node->encoder node @ %s %d \n",
                        __FILE__, __LINE__);
    }
    else
    {
        hr = mDecoderNode->ConnectOutput(0, mEncoderNode, 0);
        LOGIFFAILED(
                        mLogFile,
                        hr,
                        "Failed to connect decoder node->encoder node @ %s %d \n",
                        __FILE__, __LINE__);
    }

    hr = mEncoderNode->ConnectOutput(0, mSinkNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect encoder node -> sink node @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Bind the sink node and activate                                         *
     **************************************************************************/
    hr = mMftBuilderObjPtr->bindOutputNodes(mPartialTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to bind nodes @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     * set hardware acceleration to the topology                               *
     **************************************************************************/
    bool setHwAcceleration = mPConfigCtrl.commonParams.useSWCodec ? false
                    : true;
    hr = mMftBuilderObjPtr->setHardwareAcceleration(mPartialTopology,
                    setHwAcceleration);
    LOGIFFAILED(mLogFile, hr, "Failed to set hardware acceleration @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Create topoloder, load the topology & resolve the same                  *
     **************************************************************************/
    CComPtr < IMFTopoLoader > topoLoader;
    hr = MFCreateTopoLoader(&topoLoader);
    LOGIFFAILED(mLogFile, hr, "Failed to create topoloder @ %s %d \n",
                    __FILE__, __LINE__);

    hr = topoLoader->Load(mPartialTopology, &mTopology, nullptr);
    LOGIFFAILED(mLogFile, hr, "Failed to load topology @ %s %d \n", __FILE__,
                    __LINE__);

    return hr;
}

HRESULT CTranscodeVqSession::checkVqParams()
{
    HRESULT hr = S_OK;
    AMF_MFT_VQPARAMS *vqParams = &mPConfigCtrl.vqParams;
    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->streadyVideoEnable);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0, 3, vqParams->streadyVideoStrength);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0, 6, vqParams->streadyVideoDelay);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(90, 100, vqParams->streadyVideoZoom);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 5, vqParams->deinterlacing);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0, 1,
                    vqParams->deinterlacingPullDownDetection);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableEdgeEnhacement);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr
                    = mMftBuilderObjPtr->checkRange(1, 100,
                                    vqParams->edgeEnhacementStrength);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableDenoise);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(1, 100, vqParams->denoiseStrength);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableMosquitoNoise);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0, 100, vqParams->mosquitoNoiseStrength);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableDeblocking);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0, 100, vqParams->deblockingStrength);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableDynamicContrast);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableColorVibrance);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0, 100, vqParams->colorVibranceStrength);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr
                    = mMftBuilderObjPtr->checkRange(0, 1,
                                    vqParams->enableSkintoneCorrectoin);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0, 100,
                    vqParams->skintoneCorrectoinStrength);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableBrighterWhites);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableGamaCorrection);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0.5, 2.5,
                    ((double) vqParams->gamaCorrectionStrength / 1000));
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->enableDynamicRange);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 200, (vqParams->effectBrightness));
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0.0, 2.0,
                    ((double) vqParams->effectContrast / 1000));
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0.0, 2.0,
                    ((double) vqParams->effectSaturation / 1000));
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(-30.0, 30.0,
                    ((double) ((int32) vqParams->effectTint - 30000) / 1000));
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1,
                    vqParams->enableFalseContourReduction);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);
    hr = mMftBuilderObjPtr->checkRange(0, 100,
                    vqParams->falseContorReductionStrength);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->checkRange(0, 1, vqParams->demoMode);
    LOGIFFAILED(mLogFile, hr,
                    "Un-supported value for video quality filter @ %s %d \n",
                    __FILE__, __LINE__);

    return S_OK;

}
/**
 *******************************************************************************
 *  @fn     setVqAttributes
 *  @brief  Sets the input, output file name
 *
 *  @param[in/out] vqAttributes        : VQ attributes to be filled
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::setVqAttributes(
                CComPtr<IMFAttributes> vqAttributes,
                ULONG_PTR deviceManagerPtr, IMFMediaType* sourceMediaType,
                AMFCMRequestType requestType)
{
    (void) sourceMediaType;
    (void) deviceManagerPtr;
    AMF_MFT_VQPARAMS *vqParams = &mPConfigCtrl.vqParams;
    HRESULT hr = S_OK;

    hr = checkVqParams();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * Enable VQ feqtures available for this system.                           *
     **************************************************************************/
    CComPtr < IAMFCapabilityManager > capabilityManager;
    hr = AMFCreateCapabilityManagerMFT(IID_PPV_ARGS(&capabilityManager));
    RETURNIFFAILED(hr);

    if (mIsVqSupported == true)
    {
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_STEADY_VIDEO))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO,
                            vqParams->streadyVideoEnable);
        }

        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_DEINTERLACING))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_DEINTERLACING,
                            vqParams->deinterlacing);
        }

        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION))
        {
            vqAttributes->SetUINT32(
                            AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION,
                            vqParams->deinterlacingPullDownDetection);
        }

        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_EDGE_ENHANCEMENT))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_EDGE_ENHANCEMENT,
                            vqParams->enableEdgeEnhacement);
        }
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_DENOISE))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_DENOISE, vqParams->enableDenoise);
        }

        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_MOSQUITO_NOISE))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_MOSQUITO_NOISE,
                            vqParams->enableMosquitoNoise);
        }

        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_DEBLOCKING))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_DEBLOCKING,
                            vqParams->enableDeblocking);
        }
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_DYNAMIC_CONTRAST))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_DYNAMIC_CONTRAST,
                            vqParams->enableDynamicContrast);
        }
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_COLOR_VIBRANCE))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_COLOR_VIBRANCE,
                            vqParams->enableColorVibrance);
        }

        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_SKINTONE_CORRECTION))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_SKINTONE_CORRECTION,
                            vqParams->enableSkintoneCorrectoin);
        }
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_BRIGHTER_WHITES))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_BRIGHTER_WHITES,
                            vqParams->enableBrighterWhites);
        }
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_GAMMA_CORRECTION))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_GAMMA_CORRECTION,
                            vqParams->enableGamaCorrection);
        }

        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_BRIGHTNESS))
        {
            int32 brightness = (vqParams->effectBrightness - 100);
            vqAttributes->SetDouble(AMF_EFFECT_BRIGHTNESS, brightness); //-100 to 100
        }

        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_CONTRAST))
        {
            vqAttributes->SetDouble(AMF_EFFECT_CONTRAST,
                            ((double) vqParams->effectContrast / 1000)); //(0 to 2000)
        }
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_SATURATION))
        {
            vqAttributes->SetDouble(AMF_EFFECT_SATURATION,
                            ((double) vqParams->effectSaturation / 1000)); // 0 to 2000
        }
        if (S_OK == capabilityManager->IsEnabled(requestType, AMF_EFFECT_TINT))
        {
            vqAttributes->SetDouble(AMF_EFFECT_TINT,
                            ((double) ((int32) vqParams->effectTint - 30000)
                                            / 1000)); // -30000 to 30000
        }
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_FALSE_CONTOUR_REDUCTION))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_FALSE_CONTOUR_REDUCTION,
                            vqParams->enableFalseContourReduction);
        }
        if (S_OK == capabilityManager->IsEnabled(requestType, AMF_EFFECT_SCALE))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_SCALE, AMF_EFFECT_SCALE_BICUBIC);
        }
        if (S_OK == capabilityManager->IsEnabled(requestType,
                        AMF_EFFECT_DEMOMODE))
        {
            vqAttributes->SetUINT32(AMF_EFFECT_DEMOMODE, vqParams->demoMode);
        }

        vqParams->effectDynamic = 0;
        vqParams->effectChanged = 1;
        vqAttributes->SetUINT32(AMF_EFFECT_DYNAMIC, vqParams->effectDynamic);
        vqAttributes->SetUINT32(AMF_EFFECT_CHANGED, vqParams->effectChanged);
    }
    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     Run
 *  @brief  Creates media source and runs the topology
 *
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::Run()
{
    if (nullptr == mTopology || !ResetEvent(mTranscodeEndEvent))
    {
        return E_FAIL;
    }

    HRESULT hr;
    /**************************************************************************
     *  Create Media source                                                    *
     **************************************************************************/
    hr = MFCreateMediaSession(nullptr, &mMediaSession);
    LOGIFFAILED(mLogFile, hr, "Failed to create media sessoin @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMediaSession->BeginGetEvent(this, nullptr);
    LOGIFFAILED(mLogFile, hr, "Failed to get event @ %s %d \n", __FILE__,
                    __LINE__);
    /**************************************************************************
     *  Set topology                                                           *
     **************************************************************************/
    hr = mMediaSession->SetTopology(MFSESSION_SETTOPOLOGY_NORESOLUTION,
                    mTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    PROPVARIANT variantStart;
    PropVariantInit(&variantStart);
    hr = mMediaSession->Start(&GUID_NULL, &variantStart);
    PropVariantClear(&variantStart);
    LOGIFFAILED(mLogFile, hr, "Failed to start media sessoin @ %s %d \n",
                    __FILE__, __LINE__);

    DWORD waitResult = WaitForSingleObject(mTranscodeEndEvent, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
    {
        return E_FAIL;
    }

    mTopology.Release();

    hr = mMediaSession->Shutdown();
    LOGIFFAILED(mLogFile, hr, "Failed to shutdown media sessoin @ %s %d \n",
                    __FILE__, __LINE__);

    mMediaSession.Release();

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     GetParameters
 *  @brief  get parameters presently not used
 *
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)
{
    (void) pdwFlags; //Not used
    (void) pdwQueue; // Not used
    return E_NOTIMPL;
}
/**
 *******************************************************************************
 *  @fn     Invoke
 *  @brief  Invoke
 *  @param[in\out] asyncResult   : async call back
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::Invoke(IMFAsyncResult *asyncResult)
{
    HRESULT hr;

    CComPtr < IMFMediaEvent > event;
    hr = mMediaSession->EndGetEvent(asyncResult, &event);
    if (FAILED(hr))
    {
        hr = mMediaSession->Close();
        return hr;
    }

    MediaEventType mediaEventType;
    hr = event->GetType(&mediaEventType);
    if (SUCCEEDED(hr))
    {
        switch (mediaEventType)
        {
        case MESessionStarted:
            printf("Started the Media Session\n");
            LOG(mLogFile, "Started the Media Session\n");
            break;

        case MEError:
        {
            HRESULT status;
            hr = event->GetStatus(&status);
            printf("Received MEError in the Media Session, status = 0x%x\n",
                            status);
            LOG(
                            mLogFile,
                            "Received MEError in the Media Session, status = 0x%x @ %s, %d\n",
                            status, __FILE__, __LINE__);

            if (FAILED(status))
            {
                hr = mMediaSession->Close();
                printf("Closed the Media Session\n");
                LOG(mLogFile, "Closed the Media Session\n");

                SetEvent( mTranscodeEndEvent);

                hr = status;
            }
            break;
        }

        case MESessionEnded:
            hr = mMediaSession->Close();
            printf("Closed the Media Session\n");
            LOG(mLogFile, "Closed the Media Session\n");

            SetEvent( mTranscodeEndEvent);

            break;
        }
    }

    if (mediaEventType != MESessionClosed && SUCCEEDED(hr))
    {
        hr = mMediaSession->BeginGetEvent(this, nullptr);
    }

    return hr;
}
/**
 *******************************************************************************
 *  @fn     QueryInterface
 *  @brief  query interface
 *
 *  @param[in] riid   : Reference iid
 *  @param[in] ppv    :
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeVqSession::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IMFAsyncCallback)
    {
        *ppv = static_cast<IMFAsyncCallback*> (this);
        AddRef();
        return S_OK;
    }
    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*> (this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}
/**
 *******************************************************************************
 *  @fn     AddRef
 *  @brief
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
ULONG CTranscodeVqSession::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}
/**
 *******************************************************************************
 *  @fn     Release
 *  @brief
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
ULONG CTranscodeVqSession::Release()
{
    unsigned long refCount = InterlockedDecrement(&mRefCount);
    if (0 == refCount)
    {
        delete this;
    }
    return refCount;
}
;