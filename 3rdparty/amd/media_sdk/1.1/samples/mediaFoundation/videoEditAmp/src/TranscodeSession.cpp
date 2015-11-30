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
 * @file <TranscodeSession.cpp>
 *
 * @brief This file contains functions for creating the transcode with C++ AMP based resizer pipeline
 *
 ********************************************************************************
 */

#include <initguid.h>
#include <cguid.h>
#include <wmcodecdsp.h>

#include "MftUtils.h"
#include "TranscodeSession.h"
#include "VideoEditApi.h"
#include "ErrorCodes.h"

/**
 *******************************************************************************
 *  @fn    CTranscodeSession
 *  @brief Constructor
 *
 *
 *******************************************************************************
 */
CTranscodeSession::CTranscodeSession(void) :
    mRefCount(0)
{
    mTranscodeEndEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    mMftBuilderObjPtr = new msdk_CMftBuilder;
    mLogFile = NULL;
}

/**
 *******************************************************************************
 *  @fn    ~CTranscodeSession
 *  @brief Destructor
 *
 *
 *******************************************************************************
 */
CTranscodeSession::~CTranscodeSession(void)
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
    mScalarNode.Release();
    mEncoderNode.Release();
    mSinkNode.Release();
    CloseHandle( mTranscodeEndEvent);
    delete mMftBuilderObjPtr;
}

/**
 *******************************************************************************
 *  @fn     GetParameters
 *  @brief  GetParameters functions is presently not used
 *
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeSession::GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)
{
    (void) pdwFlags; //Not used
    (void) pdwQueue; // Not used
    return E_NOTIMPL;
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
HRESULT CTranscodeSession::setParameters(int8* inputFileName,
                int8* outputFileName, ConfigCtrl *pConfigCtrl)
{
    if (inputFileName)
        MultiByteToWideChar(CP_ACP, 0, inputFileName, -1, mInputFileName,
                        MAX_PATH);
    if (outputFileName)
        MultiByteToWideChar(CP_ACP, 0, outputFileName, -1, mOutputFileName,
                        MAX_PATH);
    if (pConfigCtrl)
        mConfigCtrl = *pConfigCtrl;
    return S_OK;
}

/**
 *******************************************************************************
 *  @fn     setLogFile
 *  @brief  Sets the log file for dumping logs in verbose mode
 *
 *  @param[in] logFile        : Log file pointer

 *
 *  @return void
 *******************************************************************************
 */
void CTranscodeSession::setLogFile(FILE *logFile)
{
    mLogFile = logFile;
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
HRESULT CTranscodeSession::Invoke(IMFAsyncResult *asyncResult)
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
HRESULT CTranscodeSession::QueryInterface(REFIID riid, void** ppv)
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
ULONG CTranscodeSession::AddRef()
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
ULONG CTranscodeSession::Release()
{
    unsigned long refCount = InterlockedDecrement(&mRefCount);
    if (0 == refCount)
    {
        delete this;
    }

    return refCount;
}
;

/**
 *******************************************************************************
 *  @fn     instantiateTopology
 *  @brief  Instantiates required MFT filters for building transcode topology
 *
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeSession::instantiateTopology()
{
    HRESULT hr = S_OK;
    DWORD streamDescriptorCount;
    DWORD streamNumber = 0;
    CComPtr < IMFMediaType > sourceMediaType;

    bool useDx11 = mMftBuilderObjPtr->isDxgiSupported();

    mMftBuilderObjPtr->setLogFile(mLogFile);

    if (!useDx11)
    {
        LOGIFFAILED(
                        mLogFile,
                        E_FAIL,
                        "Exiting since DX11 support is not available @ %s %d \n",
                        __FILE__, __LINE__);
    }

    /***************************************************************************
     * Create partial topology                                                 *
     **************************************************************************/
    hr = MFCreateTopology(&mPartialTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to create partial topology @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Create media source                                                     *
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
     * Get source descriptor count. It will be number of audio/video streams  *
     * in the container                                                       *
     *************************************************************************/
    hr = mSourcePresentationDescriptor->GetStreamDescriptorCount(
                    &streamDescriptorCount);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to get stream descriptor count @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Loop until video stream if found                                        *
     **************************************************************************/
    for (DWORD i = 0; i < streamDescriptorCount; i++)
    {
        BOOL selected;
        CComPtr < IMFStreamDescriptor > sourceStreamDescriptor;
        hr = mSourcePresentationDescriptor->GetStreamDescriptorByIndex(i,
                        &selected, &sourceStreamDescriptor);
        RETURNIFFAILED(hr);

        /***********************************************************************
         * Get the media type from the source descriptor and break if stream   *
         * type is video.                                                      *
         * Store the media type for setting preparing output video type        *
         **********************************************************************/
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

        if (IsEqualGUID(majorType, MFMediaType_Video))
        {
            uint32 testMediatype;
            streamNumber = i + 1;
            mSourceStreamDescriptor = sourceStreamDescriptor;

            hr = mediaType->GetUINT32(MF_MT_INTERLACE_MODE, &testMediatype);
            LOGIFFAILED(mLogFile, hr,
                            "Failed to get interlace mode @ %s %d \n",
                            __FILE__, __LINE__);

            if (testMediatype != MFVideoInterlace_Progressive)
            {
                /*******************************************************************
                 * Set media type to progressive mode as our encoder does not       *
                 * support interlaced content                                       *
                 *******************************************************************/
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

    /*************************************************************************
     * Create output H264 video type using the source media type              *
     *************************************************************************/
    CComPtr < IMFMediaType > h264VideoType;
    createH264VideoType(&h264VideoType, sourceMediaType,
                    mConfigCtrl.resizerParams.outputWidth,
                    mConfigCtrl.resizerParams.outputHeight);

    hr = mMftBuilderObjPtr->createSinkActivate(mSourcePresentationDescriptor,
                    mOutputFileName, h264VideoType, &mMediaSinkActivate);
    LOGIFFAILED(mLogFile, hr, "Failed to create sink filter @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Instantiate the required components for transcoding                     *
     **************************************************************************/
    hr = instantiateVideoStream(streamNumber, sourceMediaType, h264VideoType,
                    useDx11);

    return hr;
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
HRESULT CTranscodeSession::Run()
{
    if (nullptr == mResolvedTopology || !ResetEvent(mTranscodeEndEvent))
    {
        return E_FAIL;
    }

    HRESULT hr;

    /***************************************************************************
     *Create Media Session                                                     *
     **************************************************************************/
    hr = MFCreateMediaSession(nullptr, &mMediaSession);
    LOGIFFAILED(mLogFile, hr, "Failed to create media session @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMediaSession->BeginGetEvent(this, nullptr);
    LOGIFFAILED(mLogFile, hr, "Failed to get event @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     * Set topology on Media Session                                           *
     **************************************************************************/
    hr = mMediaSession->SetTopology(MFSESSION_SETTOPOLOGY_NORESOLUTION,
                    mResolvedTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    PROPVARIANT variantStart;
    PropVariantInit(&variantStart);
    hr = mMediaSession->Start(&GUID_NULL, &variantStart);
    PropVariantClear(&variantStart);
    LOGIFFAILED(mLogFile, hr, "Failed to start media session @ %s %d \n",
                    __FILE__, __LINE__);

    DWORD waitResult = WaitForSingleObject(mTranscodeEndEvent, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
    {
        return E_FAIL;
    }

    mResolvedTopology.Release();

    hr = mMediaSession->Shutdown();
    LOGIFFAILED(mLogFile, hr, "Failed to shutdown media session @ %s %d \n",
                    __FILE__, __LINE__);

    mMediaSession.Release();

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
 *******************************************************************************/
HRESULT CTranscodeSession::instantiateVideoStream(DWORD streamNumber,
                IMFMediaType* sourceMediaType,
                IMFMediaType* partialEncodedType, bool useDx11)
{

    HRESULT hr = S_OK;
    CComPtr < ID3D11Device > d3dDevice;
    CComPtr < ID3D11DeviceContext > d3dContext;

    ULONG_PTR deviceManagerPtr;

    if (useDx11)
    {
        /***********************************************************************
         * Create DX11 device                                                  *
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
        /***********************************************************************
         * DirectX 9 requires a window.                                        *
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
     * Create media source node                                                *
     **************************************************************************/
    hr = mMftBuilderObjPtr->createStreamSourceNode(mMediaSource,
                    mSourcePresentationDescriptor, mSourceStreamDescriptor,
                    &mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed create source node @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     *  Create sink node with asf container                                    *
     **************************************************************************/
    hr = mMftBuilderObjPtr->createStreamSinkNode(mMediaSinkActivate,
                    streamNumber, &mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed create sink node @ %s %d \n", __FILE__,
                    __LINE__);

    /**************************************************************************
     * Create decoder node supports H264,Mpeg 4 VC1                           *
     *************************************************************************/
    hr = mMftBuilderObjPtr->createVideoDecoderNode(sourceMediaType,
                    &mDecoderNode, deviceManagerPtr, NULL,
                    mConfigCtrl.commonParams.useSWCodec);
    LOGIFFAILED(mLogFile, hr, "Failed create Video decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Create C++ AMP resizer MFT Node                                         *
     **************************************************************************/
    CComPtr < IMFTransform > scalarTransform;
    hr = createAmpTransformNode(partialEncodedType, &mScalarNode,
                    deviceManagerPtr, &scalarTransform);
    LOGIFFAILED(mLogFile, hr,
                    "Failed create custrom transform node @ %s %d \n",
                    __FILE__, __LINE__);
    RETURNIFFAILED(hr);
    /***************************************************************************
     * Create encoder node. Supports only H264 encoder                         *
     **************************************************************************/
    hr
                    = mMftBuilderObjPtr->createVideoEncoderNode(
                                    partialEncodedType, partialEncodedType,
                                    &mEncoderNode, deviceManagerPtr,
                                    &mConfigCtrl.vidParams,
                                    mConfigCtrl.commonParams.useSWCodec);
    LOGIFFAILED(mLogFile, hr, "Failed to create encoder node @ %s %d \n",
                    __FILE__, __LINE__);
    return S_OK;
}
/** 
 *****************************************************************************
 *  @fn     createTransform
 *  @brief  Creates the transform with given class id
 *           
 *  @param[in] clsid             : Class id of the transform 
 *  @param[in/out] transform     : pointer to the transform created
 *  @param[in] deviceManagerPtr  : dx9 or dx11 device manager 
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *****************************************************************************
 */
HRESULT CTranscodeSession::createTransform(IMFTransform** transform,
                ULONG_PTR deviceManagerPtr)
{
    HRESULT hr;
    CComPtr < IMFTransform > mft;
    hr = ampResizerCreateInstance(IID_IMFTransform, (void**) &mft);
    RETURNIFFAILED(hr);

    CComPtr < IMFAttributes > transformAttributes;
    hr = mft->GetAttributes(&transformAttributes);
    if (hr != E_NOTIMPL)
    {
        RETURNIFFAILED(hr);

        UINT32 transformAsync;
        hr
                        = transformAttributes->GetUINT32(MF_TRANSFORM_ASYNC,
                                        &transformAsync);
        if (SUCCEEDED(hr) && TRUE == transformAsync)
        {
            hr
                            = transformAttributes->SetUINT32(
                                            MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
            RETURNIFFAILED(hr);
        }

        if (deviceManagerPtr != NULL)
        {
            CComPtr < IUnknown > deviceManagerUnknown
                            = reinterpret_cast<IUnknown*> (deviceManagerPtr);

            CComPtr < IUnknown > dxgiDeviceManager;

            const CLSID
                            & d3dAwareAttribute =
                                            S_OK
                                                            == deviceManagerUnknown->QueryInterface(
                                                                            BORROWED_IID_IMFDXGIDeviceManager,
                                                                            (void**) (&dxgiDeviceManager)) ? BORROWED_MF_SA_D3D11_AWARE
                                                            : MF_SA_D3D_AWARE;

            UINT32 d3dAware;
            hr = transformAttributes->GetUINT32(d3dAwareAttribute, &d3dAware);
            if (SUCCEEDED(hr) && d3dAware != 0)
            {
                hr = mft->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER,
                                deviceManagerPtr);
                RETURNIFFAILED(hr);
            }
        }
    }

    *transform = mft.Detach();

    return S_OK;
}
/** 
 *****************************************************************************
 *  @fn     createResizerTransformNode
 *  @brief  Create custom transform node
 *           
 *  @param[in] targetVideoType     : Encoder video type
 *  @param[out] transformNode      : source video type
 *  @param[in] deviceManagerPtr    : Pointer to the Device manager
 *  @param[out] transform          : Pointer to the transform
 *  @param[out] useDx11            : Use Dx11 or Dx9
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *****************************************************************************
 */
HRESULT CTranscodeSession::createAmpTransformNode(
                IMFMediaType* targetVideoType, IMFTopologyNode **transformNode,
                ULONG_PTR deviceManagerPtr, IMFTransform** transform)
{
    if (nullptr == transformNode)
    {
        return E_POINTER;
    }

    HRESULT hr;

    CComPtr < IMFTransform > customTransform;
    hr = createTransform(&customTransform, deviceManagerPtr);
    RETURNIFFAILED(hr);

    if (targetVideoType != nullptr)
    {
        CComPtr < IMFMediaType > outputVideoType;
        hr = mMftBuilderObjPtr->createVideoTypeFromSource(targetVideoType,
                        MFVideoFormat_NV12, TRUE, TRUE, &outputVideoType);
        RETURNIFFAILED(hr);

        hr = customTransform->SetOutputType(0, outputVideoType, 0);
        RETURNIFFAILED(hr);
    }

    CComPtr < IMFTopologyNode > node;
    hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &node);
    RETURNIFFAILED(hr);

    hr = node->SetObject(customTransform);
    RETURNIFFAILED(hr);

    hr = node->SetUINT32(MF_TOPONODE_CONNECT_METHOD, MF_CONNECT_ALLOW_CONVERTER
                    | MF_CONNECT_RESOLVE_INDEPENDENT_OUTPUTTYPES);
    RETURNIFFAILED(hr);

    *transformNode = node.Detach();

    if (transform != nullptr)
    {
        *transform = customTransform.Detach();
    }

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     createH264VideoType
 *  @brief  Creates H264 video type
 *
 *  @param[out] encodedVideoType   : output video type to be created
 *  @param[in] sourceVideoType     : Source Media Type
 *  @param[in] width               : output width, can differ from source width
 *  @param[in] height              : output height, can differ from source height

 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CTranscodeSession::createH264VideoType(IMFMediaType** encodedVideoType,
                IMFMediaType* sourceVideoType, uint32 width, uint32 height)
{
    if (nullptr == encodedVideoType)
    {
        return E_POINTER;
    }

    HRESULT hr;

    /***************************************************************************
     * Create video type for storing the commpressed bit stream                *
     **************************************************************************/
    CComPtr < IMFMediaType > videoType;
    hr = mMftBuilderObjPtr->createVideoType(&videoType, MFVideoFormat_H264,
                    FALSE, FALSE, nullptr, nullptr, width, height,
                    MFVideoInterlace_Progressive);
    LOGIFFAILED(mLogFile, hr, "Failed to create video type @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Set output bit rate                                                     *
     **************************************************************************/
    hr = videoType->SetUINT32(MF_MT_AVG_BITRATE,
                    mConfigCtrl.vidParams.meanBitrate);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     * Set output profile                                                      *
     **************************************************************************/
    hr = videoType->SetUINT32(MF_MT_MPEG2_PROFILE,
                    mConfigCtrl.vidParams.compressionStandard);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    UINT32 numerator;
    UINT32 denominator;

    /***************************************************************************
     * Set out frame rate same as input                                        *
     **************************************************************************/
    hr = MFGetAttributeRatio(sourceVideoType, MF_MT_FRAME_RATE, &numerator,
                    &denominator);
    LOGIFFAILED(mLogFile, hr, "Failed to get attributes @ %s %d \n", __FILE__,
                    __LINE__);
    MFSetAttributeRatio(videoType, MF_MT_FRAME_RATE, numerator, denominator);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     * Set out pixel aspect ratio same as input                                *
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
HRESULT CTranscodeSession::buildAndLoadTopology()
{
    HRESULT hr = S_OK;

    /***************************************************************************
     *  Add source node to the topology                                        *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add source node @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     * Add sink node to the topology                                           *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add sink node @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     * Add decoder node to the topology                                        *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mDecoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed to decoder node @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     * Connect source and decoder nodes                                        *
     **************************************************************************/
    hr = mSourceNode->ConnectOutput(0, mDecoderNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect source node->decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Add C++ AMP scalar node to the topology                                 *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mScalarNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add C++ AMP scalar node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Connect decoder and C++ AMP scalar nodes                                *
     **************************************************************************/
    hr = mDecoderNode->ConnectOutput(0, mScalarNode, 0);
    LOGIFFAILED(
                    mLogFile,
                    hr,
                    "Failed to connect decoder node->C++ AMP scalar node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Add encoder node to the topology                                        *
     **************************************************************************/
    hr = mPartialTopology->AddNode(mEncoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add encoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Connect C++ AMP scalar and encoder nodes                                *
     **************************************************************************/
    hr = mScalarNode->ConnectOutput(0, mEncoderNode, 0);
    LOGIFFAILED(
                    mLogFile,
                    hr,
                    "Failed to connect C++ AMP scalar node ->encoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Connect encoder and sink nodes                                          *
     **************************************************************************/
    hr = mEncoderNode->ConnectOutput(0, mSinkNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect encoder node -> sink node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Bind the sink node and activate                                         *
     **************************************************************************/
    hr = mMftBuilderObjPtr->bindOutputNodes(mPartialTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to bind nodes @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     * Set hardware acceleration to the topology                               *
     **************************************************************************/
    bool setHwAcceleration = mConfigCtrl.commonParams.useSWCodec ? false : true;
    hr = mMftBuilderObjPtr->setHardwareAcceleration(mPartialTopology,
                    setHwAcceleration);
    LOGIFFAILED(mLogFile, hr, "Failed to set hardware acceleration @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Create topoloder, load and resolve the topology                         *
     **************************************************************************/
    CComPtr < IMFTopoLoader > topoLoader;
    hr = MFCreateTopoLoader(&topoLoader);
    LOGIFFAILED(mLogFile, hr, "Failed to create topoloder @ %s %d \n",
                    __FILE__, __LINE__);

    hr = topoLoader->Load(mPartialTopology, &mResolvedTopology, nullptr);

    LOGIFFAILED(mLogFile, hr, "Failed to load topology @ %s %d \n", __FILE__,
                    __LINE__);

    return hr;
}

