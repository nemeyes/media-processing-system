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
 * @file <SimpleTranscodeSession.cpp>
 *
 * @brief This file contains functions for creating the simple transcode pipeline
 *
 ********************************************************************************
 */
#include <initguid.h>
#include <cguid.h>
#include <wmcodecdsp.h>
#include "MftUtils.h"
#include "SimpleTranscodeSession.h"
#include "ErrorCodes.h"
#include "SimpleTranscodeApi.h"
#include "CustomSinkActivate.h"
#include <windowsx.h>

/**
 *******************************************************************************
 *  @fn    CSimpleTranscodeSession
 *  @brief Constructor
 *
 *
 *******************************************************************************
 */
CSimpleTranscodeSession::CSimpleTranscodeSession(void) :
    mRefCount(0)
{
    mTranscodeEndEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    mLogFile = NULL;
    mMftBuilderObjPtr = new msdk_CMftBuilder;
}
/**
 *******************************************************************************
 *  @fn    ~CSimpleTranscodeSession
 *  @brief Destructor
 *
 *
 *******************************************************************************
 */
CSimpleTranscodeSession::~CSimpleTranscodeSession(void)
{
    mTopology.Release();
    mMediaSession->Shutdown();
    mMediaSession.Release();
    mSourceNode.Release();
    mSourceStreamDescriptor.Release();
    mSourcePresentationDescriptor.Release();
    mMediaSource.Release();
    mCustomSource.Release();
    mSinkNode.Release();
    mMediaSinkActivate.Release();
    mDecoderNode.Release();
    mEncoderNode.Release();
    mPartialTopology.Release();

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
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::setParameters(int8* inputFileName,
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

void CSimpleTranscodeSession::setLogFile(FILE *logFile)
{
    mLogFile = logFile;
}

/**
 *******************************************************************************
 *  @fn     instantiateTopology
 *  @brief  Instantiates required MFT filters for building transcode topology
 *
 *
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::instantiateTopology(int8* sinkFileName,
                ConfigCtrl *pConfigCtrl)
{
    HRESULT hr;
    DWORD streamDescriptorCount;
    DWORD streamNumber = 0;
    CComPtr < IMFMediaType > sourceMediaType = NULL;

    bool useDx11 = mMftBuilderObjPtr->isDxgiSupported();

    mMftBuilderObjPtr->setLogFile(mLogFile);

    /**************************************************************************
     *  Create partial topology                                               *
     **************************************************************************/
    hr = MFCreateTopology(&mPartialTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to create partial topology @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Create custom source                                                   *
     **************************************************************************/
    mCustomSource = new (std::nothrow) CustomMediaSource();
    hr = mCustomSource->createCustomSource(mInputFileName, &mMediaSource,
                    pConfigCtrl);
    LOGIFFAILED(mLogFile, hr, "Failed to create source @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mMediaSource->CreatePresentationDescriptor(
                    &mSourcePresentationDescriptor);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to create presentation descriptor @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Get source descriptor count. It will be number of audio/video streams  *
     * in the container                                                       *
     **************************************************************************/
    hr = mSourcePresentationDescriptor->GetStreamDescriptorCount(
                    &streamDescriptorCount);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to get presentation descriptor count @ %s %d \n",
                    __FILE__, __LINE__);
    /**************************************************************************
     *  Loop until video stream if found                                      *
     **************************************************************************/
    for (DWORD i = 0; i < streamDescriptorCount; i++)
    {
        BOOL selected;
        CComPtr < IMFStreamDescriptor > sourceStreamDescriptor;
        hr = mSourcePresentationDescriptor->GetStreamDescriptorByIndex(i,
                        &selected, &sourceStreamDescriptor);
        RETURNIFFAILED(hr);

        /**********************************************************************
         *  Get the media type from the source descriptor and break if stream *
         *  type is video.                                                    *
         *  Store the media type for setting preparing output video type      *
         **********************************************************************/
        CComPtr < IMFMediaTypeHandler > mediaTypeHandler;
        hr = sourceStreamDescriptor->GetMediaTypeHandler(&mediaTypeHandler);
        LOGIFFAILED(mLogFile, hr, "Failed to get mediatype handler @ %s %d \n",
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
                 **************************************************************/
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
     *  Create H264 video type using the source media type                    *
     **************************************************************************/
    CComPtr < IMFMediaType > h264VideoType;
    createH264VideoType(&h264VideoType, sourceMediaType);

    std::string str = sinkFileName;
    std::wstring tempStr(str.begin(), str.end());
    LPCWSTR out = tempStr.c_str();

    /**************************************************************************
     *  Create Custom sink                                                     *
     **************************************************************************/
    hr = CustomSinkActivate::createInstance(out, &mMediaSinkActivate);
    LOGIFFAILED(mLogFile, hr, "Failed to create custom sink filter @ %s %d \n",
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
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::instantiateVideoStream(DWORD streamNumber,
                IMFMediaType* sourceMediaType,
                IMFMediaType* partialEncodedType, bool useDx11)
{
    HRESULT hr;
    ULONG_PTR deviceManagerPtr;
    if (useDx11)
    {
        /**********************************************************************
         *  Create DX11 device                                                *
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
         *  DirectX 9 requires a window.                                      *
         **********************************************************************/
        HWND hWnd = GetDesktopWindow();
        HRESULT hr = mMftBuilderObjPtr->createDirect3DDeviceManagerPtr(hWnd,
                        &deviceManagerPtr);
        LOGIFFAILED(mLogFile, hr, "Failed create Dx9 device @ %s %d \n",
                        __FILE__, __LINE__);

        mMftBuilderObjPtr->d3dDeviceManager.Attach(
                        reinterpret_cast<IDirect3DDeviceManager9 *> (deviceManagerPtr));
    }
    /**************************************************************************
     *  Create source node                                                    *
     **************************************************************************/
    hr = mMftBuilderObjPtr->createStreamSourceNode(mMediaSource,
                    mSourcePresentationDescriptor, mSourceStreamDescriptor,
                    &mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed create source node @ %s %d \n", __FILE__,
                    __LINE__);
    /**************************************************************************
     *  Create sink node with asf container                                   *
     **************************************************************************/
    hr = mMftBuilderObjPtr->createStreamSinkNode(mMediaSinkActivate,
                    streamNumber, &mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed create sink node @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     * Create decoder node                                                    *
     * Supports H264,Mpeg 4 VC1                                               *
     **************************************************************************/
    hr = mMftBuilderObjPtr->createVideoDecoderNode(sourceMediaType,
                    &mDecoderNode, deviceManagerPtr, NULL,
                    mPConfigCtrl.commonParams.useSWCodec);
    LOGIFFAILED(mLogFile, hr, "Failed create Video decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     *  Create encoder node. Supports only H264 encoder                       *
     **************************************************************************/
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
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::createH264VideoType(
                IMFMediaType** encodedVideoType, IMFMediaType* sourceVideoType)
{
    uint32 srcWidth, srcHeight;
    if (nullptr == encodedVideoType)
    {
        return E_POINTER;
    }

    HRESULT hr;

    /**************************************************************************
     *  Get width and height from the source media type                       *
     **************************************************************************/
    hr = MFGetAttributeSize(sourceVideoType, MF_MT_FRAME_SIZE, &srcWidth,
                    &srcHeight);
    LOGIFFAILED(mLogFile, hr, "Failed to get attributes @ %s %d \n", __FILE__,
                    __LINE__);
    /**************************************************************************
     *  Create video type for storing the commpressed bit stream              *
     **************************************************************************/
    CComPtr < IMFMediaType > videoType;
    hr = mMftBuilderObjPtr->createVideoType(&videoType, MFVideoFormat_H264,
                    FALSE, FALSE, nullptr, nullptr, srcWidth, srcHeight,
                    MFVideoInterlace_Progressive);
    LOGIFFAILED(mLogFile, hr, "Failed to create video type @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     *  Set output bit rate                                                   *
     **************************************************************************/
    hr = videoType->SetUINT32(MF_MT_AVG_BITRATE,
                    mPConfigCtrl.vidParams.meanBitrate);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     *  Set encoding profile                                                  *
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
     * get pixel aspect ratio from source media type and set the same to      *
     * output video type                                                      *
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
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::buildAndLoadTopology(ConfigCtrl *pConfigCtrl)
{
    HRESULT hr = S_OK;

    /**************************************************************************
     * Add source & sink node to the topology                                 *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add source node @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mPartialTopology->AddNode(mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add sink node @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     * Add decoder, encoder node to the topology                              *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mDecoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed to decoder node @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mPartialTopology->AddNode(mEncoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add encoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Connect sourceNode->DecoderNode->EncoderNode->sinkNode                 *
     **************************************************************************/
    hr = mSourceNode->ConnectOutput(0, mDecoderNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect source node->decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mDecoderNode->ConnectOutput(0, mEncoderNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect decoder node ->encoder node @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mEncoderNode->ConnectOutput(0, mSinkNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect encoder node -> sink node @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Bind the sink node and activate                                        *
     **************************************************************************/
    hr = mMftBuilderObjPtr->bindOutputNodes(mPartialTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to bind nodes @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     * set hardware acceleration to the topology                              *
     **************************************************************************/
    bool setHwAcceleration = mPConfigCtrl.commonParams.useSWCodec ? false
                    : true;
    hr = mMftBuilderObjPtr->setHardwareAcceleration(mPartialTopology,
                    setHwAcceleration);
    LOGIFFAILED(mLogFile, hr, "Failed to set hardware acceleration @ %s %d \n",
                    __FILE__, __LINE__);

    /**************************************************************************
     * Create topoloder, load the topology & resolve the same                 *
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

/**
 *******************************************************************************
 *  @fn     Run
 *  @brief  Creates media source and runs the topology
 *
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::run()
{
    if (nullptr == mTopology || !ResetEvent(mTranscodeEndEvent))
    {
        return E_FAIL;
    }

    HRESULT hr;
    /**************************************************************************
     *  Create Media session                                                   *
     **************************************************************************/
    hr = MFCreateMediaSession(nullptr, &mMediaSession);
    LOGIFFAILED(mLogFile, hr, "Failed to create media sessoin @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMediaSession->BeginGetEvent(this, nullptr);
    LOGIFFAILED(mLogFile, hr, "Failed to get event @ %s %d \n", __FILE__,
                    __LINE__);
    /**************************************************************************
     *  Set topology                                                          *
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

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     GetParameters
 *  @brief  get parameters presently not used
 *
 *
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)
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
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::Invoke(IMFAsyncResult *asyncResult)
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
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CSimpleTranscodeSession::QueryInterface(REFIID riid, void** ppv)
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
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
ULONG CSimpleTranscodeSession::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}
/**
 *******************************************************************************
 *  @fn     Release
 *  @brief
 *
 *  @return HRESULT : S_OK if successful; else returns micorsoft error codes.
 *******************************************************************************
 */
ULONG CSimpleTranscodeSession::Release()
{
    unsigned long refCount = InterlockedDecrement(&mRefCount);
    if (0 == refCount)
    {
        delete this;
    }
    return refCount;
}
;