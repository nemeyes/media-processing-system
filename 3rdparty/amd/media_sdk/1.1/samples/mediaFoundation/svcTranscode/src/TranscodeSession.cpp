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
 * @file <TranscodeSession.cpp>                          
 *                                       
 * @brief This file contains functions for building transcode topology
 *         
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include "TranscodeSession.h"

/** 
 *******************************************************************************
 *  @fn     create
 *  @brief  Creates transcode session instance. 
 *           
 *  @param[in] videoRenderWindow : Window for transcode

 *  @param[in] stateSubscriber   : Pointer to the state subscriber 
 *  @param[out] TranscodeSession  : Pointer to the transcode class instance
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::create(HWND videoRenderWindow,
                PlaybackStateSubscriber* stateSubscriber,
                SvcTranscodeSession** TranscodeSession)
{
    HRESULT hr;
    if (nullptr == TranscodeSession)
    {
        return E_POINTER;
    }
    /***************************************************************************
     * Create transcode session instance                                         *
     ***************************************************************************/
    *TranscodeSession = new (std::nothrow) SvcTranscodeSession(
                    videoRenderWindow, stateSubscriber);
    if (nullptr == *TranscodeSession)
    {
        return E_OUTOFMEMORY;
    }
    (*TranscodeSession)->AddRef();

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    RETURNIFFAILED(hr);

    hr = MFStartup(MF_VERSION);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     SvcTranscodeSession
 *  @brief  Constructor 
 *           
 *  @param[in] stateSubscriber   : Pointer to the state subscriber 
 *          
 *  @return none 
 *******************************************************************************
 */
SvcTranscodeSession::SvcTranscodeSession(HWND videoRenderWindow,
                PlaybackStateSubscriber* stateSubscriber) :
    mRefCount(0), mState(SvcTranscodeSession::Building), stateSubscriber(
                    stateSubscriber), mRendererType(
                    SvcTranscodeSession::RendererDx9), mVideoRenderWindow(
                    videoRenderWindow), mSvcCurrentLayersNumber(
                    SVC_LAYERS_NUMBER), mLogFile(NULL)
{
    mMftBuilderObjPtr = new msdk_CMftBuilder;
}
/** 
 *******************************************************************************
 *  @fn     SvcTranscodeSession
 *  @brief  Destructor 
 *           
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
SvcTranscodeSession::~SvcTranscodeSession(void)
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
    if (mMediaSession != nullptr)
    {
        mMediaSession->Shutdown();
    }
    delete mMftBuilderObjPtr;
}
/** 
 *******************************************************************************
 *  @fn     getState
 *  @brief  Returns the state of the transcode sessoin
 *           
 *  @return State : Returns state of the transcode sessoin
 *******************************************************************************
 */
SvcTranscodeSession::State SvcTranscodeSession::getState()
{
    return mState;
}
/** 
 *******************************************************************************
 *  @fn     getRendererType
 *  @brief  Returns renderer type (Dx11 or Dx9)
 *           
 *  @return RendererType : Renderer type
 *******************************************************************************
 */
SvcTranscodeSession::RendererType SvcTranscodeSession::getRendererType()
{
    return mRendererType;
}
/** 
 *******************************************************************************
 *  @fn     setState
 *  @brief  Sets the state of transcode
 *  
 *  @param[in] state   : State of the transcode 
 *           
 *  @return void.
 *******************************************************************************
 */
void SvcTranscodeSession::setState(SvcTranscodeSession::State state)
{
    mState = state;
    stateSubscriber->onStateChange(mState);
}
/** 
 *******************************************************************************
 *  @fn     setLogFile
 *  @brief  Sets log file for printing error log. This function is only enabled 
 *          for debug build
 *  
 *  @param[in] logFile   : Pointer to the log file
 *           
 *  @return void.
 *******************************************************************************
 */
void SvcTranscodeSession::setLogFile(FILE *logFile)
{
    mLogFile = logFile;
}
/** 
 *******************************************************************************
 *  @fn     releaseResources
 *  @brief  Will release the resources used by transcode
 *  
 *  @return void.
 *******************************************************************************
 */
void SvcTranscodeSession::releaseResources()
{
    mMediaSession->Shutdown();
    mMediaSession.Release();
    mSourcePresentationDescriptor.Release();
    mVqTransform.Release();
    mSvcSplitterMft.Release();
    mTopology.Release();
    mPartialTopology.Release();
    mMediaSource.Release();
    mSourceStreamDescriptor.Release();
    mSourceNode.Release();
    mDecoderNode.Release();
    mSvcEncodeNode.Release();
    mSinkNode.Release();
    mMediaSinkActivate.Release();

    mH264DecoderNode.Release();
    mSvcSplitterNode.Release();
    mCustomStreamSinkNode.Release();
    mEvrStreamSinkNode.Release();
    mTeeNode.Release();
}
/** 
 *******************************************************************************
 *  @fn     openFile
 *  @brief  This function will be called once openFile button is pressed from GUI
 *          This will intantiate and build the topology 
 *  
 *  @param[in] fileName  : Input soruce file which needs to be played
 *  @param[in] useDx9    : Use Dx9 device or Dx11
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::openFile(LPCWSTR fileName, bool useDx9)
{
    HRESULT hr;
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);
    /***************************************************************************
     * If the state of transcode is Building release the resources and           *
     * instantiate freshtly                                                     *
     * This means file which was previously played is stopped and another file  *
     * is getting opened for playing. In this case depending on the file        *
     * topology is build                                                        *
     ***************************************************************************/
    if (mState != SvcTranscodeSession::Building)
    {
        releaseResources();
    }
    if (useDx9)
    {
        mRendererType = RendererDx9;
    }
    else
    {
        mRendererType = RendererDx11;
    }
    /***************************************************************************
     * Intantiate required MFTs for transcode functionality                      *
     * pipeline : source->MJPEGDecoder->OPENCLMFT->SVC Encode->TeeNode          *
     *            TeeNode->Renderer(Layer), TeeNode->Filedump                   *
     ***************************************************************************/
    hr = instantiateTopology(mVideoRenderWindow, fileName);
    LOGIFFAILED(mLogFile, hr, "Failed to instantiate topology @ %s %d \n",
                    __FILE__, __LINE__);
    /***************************************************************************
     * Build and load the toplogy                                               *
     ***************************************************************************/
    hr = buildAndLoadTopology();
    LOGIFFAILED(mLogFile, hr, "Failed to load topology @ %s %d \n", __FILE__,
                    __LINE__);
    /***************************************************************************
     * Create media session                                                     *
     ***************************************************************************/
    hr = MFCreateMediaSession(nullptr, &mMediaSession);
    if (SUCCEEDED(hr))
    {
        hr = mMediaSession->BeginGetEvent(this, nullptr);
    }

    if (SUCCEEDED(hr))
    {
        hr = mMediaSession->SetTopology(MFSESSION_SETTOPOLOGY_NORESOLUTION,
                        mTopology);
    }

    if (FAILED(hr))
    {
        setState(SvcTranscodeSession::Failed);
        LOGIFFAILED(mLogFile, hr, "Failed to create media session @ %s %d \n",
                        __FILE__, __LINE__);
    }

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     play
 *  @brief  Called when play button is pressed from GUI
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::play()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (mState != SvcTranscodeSession::Stopped && mState
                    != SvcTranscodeSession::Paused)
    {
        return E_FAIL;
    }

    return intPlay();
}
/** 
 *******************************************************************************
 *  @fn     stop
 *  @brief  Called when stop button is pressed from GUI
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::stop()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = mMediaSession->Stop();
    if (SUCCEEDED(hr))
    {
        setState(SvcTranscodeSession::StopPending);
    }
    else
    {
        setState(SvcTranscodeSession::Failed);
        LOGIFFAILED(mLogFile, hr, "Failed to stop video file @ %s %d \n",
                        __FILE__, __LINE__);
    }
    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     shutdown
 *  @brief  Shutsdown the media session
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::shutdown()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);
    HRESULT hr = S_OK;
    if (mMediaSession != nullptr)
    {
        hr = mMediaSession->Shutdown();
    }
    setState(SvcTranscodeSession::Failed);
    LOGIFFAILED(mLogFile, hr, "Failed to shtdown media session @ %s %d \n",
                    __FILE__, __LINE__);
    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     pause
 *  @brief  Pause the media session
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::pause()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (mState != SvcTranscodeSession::Playing)
    {
        return E_FAIL;
    }
    HRESULT hr = mMediaSession->Pause();
    if (SUCCEEDED(hr))
    {
        setState(SvcTranscodeSession::PausePending);
    }
    else
    {
        setState(SvcTranscodeSession::Failed);
    }
    return hr;
}
/** 
 *******************************************************************************
 *  @fn     resume
 *  @brief  resume the media session
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::resume()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (mState != SvcTranscodeSession::Paused)
    {
        return E_FAIL;
    }

    return intPlay();
}
HRESULT SvcTranscodeSession::getDuration(UINT64* duration)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (SvcTranscodeSession::Building == mState || SvcTranscodeSession::Failed
                    == mState)
    {
        return E_FAIL;
    }

    HRESULT hr;

    hr = mSourcePresentationDescriptor->GetUINT64(MF_PD_DURATION, duration);
    RETURNIFFAILED(hr);

    return S_OK;
}

HRESULT SvcTranscodeSession::getTime(MFTIME* currentTime)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (SvcTranscodeSession::Playing != mState && SvcTranscodeSession::Paused
                    != mState)
    {
        return E_FAIL;
    }

    if (nullptr == mMediaSession)
    {
        return E_FAIL;
    }

    HRESULT hr;

    CComPtr < IMFClock > clock;
    hr = mMediaSession->GetClock(&clock);
    RETURNIFFAILED(hr);

    CComPtr < IMFPresentationClock > presentationClock;
    hr = clock->QueryInterface(&presentationClock);
    RETURNIFFAILED(hr);

    hr = presentationClock->GetTime(currentTime);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetParameters
 *  @brief  Not implemented
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::GetParameters(DWORD* /*pdwFlags*/, DWORD* /*pdwQueue*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     Invoke
 *  @brief  virtual function
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::Invoke(IMFAsyncResult *asyncResult)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);
    HRESULT hr;
    CComPtr < IMFMediaEvent > event;
    hr = mMediaSession->EndGetEvent(asyncResult, &event);

    if (FAILED(hr))
    {
        setState(SvcTranscodeSession::Failed);
        LOGIFFAILED(mLogFile, hr, "EndGetEvent failed. @ %s %d \n", __FILE__,
                        __LINE__);
        hr = mMediaSession->Close();
        LOGIFFAILED(mLogFile, hr, "Media sesson close failed. @ %s %d \n",
                        __FILE__, __LINE__);
        return hr;
    }

    MediaEventType mediaEventType;
    hr = event->GetType(&mediaEventType);
    LOGIFFAILED(mLogFile, hr, "Failed to get Media Event @ %s %d \n", __FILE__,
                    __LINE__);

    if (SUCCEEDED(hr))
    {
        switch (mediaEventType)
        {
        case MESessionTopologyStatus:
        {
            MF_TOPOSTATUS topoStatus;
            hr = event->GetUINT32(MF_EVENT_TOPOLOGY_STATUS,
                            (UINT32*) &topoStatus);
            if (SUCCEEDED(hr) && MF_TOPOSTATUS_READY == topoStatus)
            {
                intPlay();
            }

            break;
        }
        case MESessionPaused:
            setState(SvcTranscodeSession::Paused);
            break;

        case MESessionStopped:
            setState(SvcTranscodeSession::Stopped);
            /*******************************************************************
             *  Do not shutdown media session because it can be restarted       *
             *******************************************************************/
            break;

        case MESessionStarted:
            LOG(mLogFile, "Started the Media Session\n");
            setState(SvcTranscodeSession::Playing);
#ifdef _DEBUG
            {
                CComPtr<IMFTopology> fullTopology;
                mMediaSession->GetFullTopology(MFSESSION_GETFULLTOPOLOGY_CURRENT, 0, &fullTopology);
            }
#endif
            break;

        case MEError:
            LOG(mLogFile, "MEError received in the Media Session @ %s, %d\n",
                            __FILE__, __LINE__);
            setState(SvcTranscodeSession::Failed);
            mMediaSession->Shutdown();
            LOG(mLogFile, "Closed the Media Session\n");
            break;

        case MESessionEnded:
            setState(SvcTranscodeSession::Stopped);
            intPlay();
            break;
        }
    }

    if (mMediaSession != nullptr && mediaEventType != MEError && mediaEventType
                    != MESessionClosed)
    {
        hr = mMediaSession->BeginGetEvent(this, nullptr);
    }
    return hr;
}
/** 
 *******************************************************************************
 *  @fn     QueryInterface
 *  @brief  virtual function
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::QueryInterface(REFIID riid, void** ppv)
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
 *  @brief  virtual function
 *  
 *  @return ULONG 
 *******************************************************************************
 */
ULONG SvcTranscodeSession::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}
/** 
 *******************************************************************************
 *  @fn     Release
 *  @brief  virtual function
 *  
 *  @return ULONG 
 *******************************************************************************
 */
ULONG SvcTranscodeSession::Release()
{
    unsigned long refCount = InterlockedDecrement(&mRefCount);

    if (0 == refCount)
    {
        delete this;
    }

    return refCount;
}

/** 
 *******************************************************************************
 *  @fn     intPlay
 *  @brief  This function runs the transcode topology
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::intPlay()
{
    HRESULT hr;

    PROPVARIANT variantStart;
    PropVariantInit(&variantStart);

    hr = mMediaSession->Start(&GUID_NULL, &variantStart);
    if (SUCCEEDED(hr))
    {
        setState(SvcTranscodeSession::PlayPending);
    }
    else
    {
        setState(SvcTranscodeSession::Failed);
    }

    PropVariantClear(&variantStart);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     setSvcLayersNumber
 *  @brief  Sets the number of layers used 
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::setSvcLayersNumber(UINT32 num)
{
    mSvcCurrentLayersNumber = num;
    if (mSvcSplitterMft != nullptr)
    {
        CComPtr < IMFAttributes > outputAtributes;
        HRESULT hr = mSvcSplitterMft->GetOutputStreamAttributes(0,
                        &outputAtributes);
        RETURNIFFAILED(hr);

        hr = outputAtributes->SetUINT32(
                        CLSID_SVC_MFT_OUTPUT_LAYERS_NUM_PROPERTY,
                        mSvcCurrentLayersNumber);
        RETURNIFFAILED(hr);
    }
    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     instantiateTopology
 *  @brief  Instantiates required MFT filters for building transcode topology
 *          
 *  @param[in] videoWindow  : GUI window 
 *  @param[in] fileName     : Input file name
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::instantiateTopology(HWND videoWindow,
                LPCWSTR fileName)
{
    HRESULT hr;
    DWORD streamDescriptorCount;
    DWORD streamNumber = 0;
    CComPtr < IMFMediaType > sourceMediaType = NULL;

    mMftBuilderObjPtr->setLogFile(mLogFile);

    /**************************************************************************
     * Create partial topology                                                 *
     **************************************************************************/
    hr = MFCreateTopology(&mPartialTopology);
    LOGIFFAILED(mLogFile, hr, "Failed to create partial topology @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Create source filter                                                     *
     ***************************************************************************/
    hr = mMftBuilderObjPtr->createSource(fileName, &mMediaSource);
    LOGIFFAILED(mLogFile, hr, "Failed to create source @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mMediaSource->CreatePresentationDescriptor(
                    &mSourcePresentationDescriptor);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to create presentation descriptor @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Get source descriptor count. It will be number of audio/video streams    *
     * in the container                                                         *
     ***************************************************************************/
    hr = mSourcePresentationDescriptor->GetStreamDescriptorCount(
                    &streamDescriptorCount);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to get presentation descriptor count @ %s %d \n",
                    __FILE__, __LINE__);
    for (DWORD i = 0; i < streamDescriptorCount; i++)
    {
        BOOL selected;
        CComPtr < IMFStreamDescriptor > sourceStreamDescriptor;
        hr = mSourcePresentationDescriptor->GetStreamDescriptorByIndex(i,
                        &selected, &sourceStreamDescriptor);
        RETURNIFFAILED(hr);

        /***********************************************************************
         *  Get the media type from the source descriptor and break if stream    *
         *  type is video.                                                       *
         *  Store the media type for setting preparing output video type         *
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

    /***************************************************************************
     *  Instantiate the filters required for transcoding                        *
     ***************************************************************************/
    hr = instantiateVideoStream(sourceMediaType, videoWindow);

    LOGIFFAILED(mLogFile, hr, "Failed instantiate video stream @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Create sink node for dumping SVC elementary stream                      *
     ***************************************************************************/
    createCustomSinkNode(streamNumber, fileName);

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
HRESULT SvcTranscodeSession::createH264VideoType(
                IMFMediaType** encodedVideoType, IMFMediaType* sourceVideoType)
{
    uint32 srcWidth, srcHeight;
    if (nullptr == encodedVideoType)
    {
        return E_POINTER;
    }

    HRESULT hr;
    /***************************************************************************
     *  Set bit rate and profile for the encoder                                *
     ***************************************************************************/
    const UINT32 averageBitrate = AVG_BIT_RATE;
    const eAVEncH264VProfile h264Profile = eAVEncH264VProfile_ConstrainedBase;

    /***************************************************************************
     *  Get width and height from the source media type                         *
     ***************************************************************************/
    hr = MFGetAttributeSize(sourceVideoType, MF_MT_FRAME_SIZE, &srcWidth,
                    &srcHeight);
    LOGIFFAILED(mLogFile, hr, "Failed to get attributes @ %s %d \n", __FILE__,
                    __LINE__);
    /***************************************************************************
     *  Create video type for storing the commpressed bit stream                *
     ***************************************************************************/
    CComPtr < IMFMediaType > videoType;
    hr = mMftBuilderObjPtr->createVideoType(&videoType, MFVideoFormat_H264,
                    FALSE, FALSE, nullptr, nullptr, srcWidth, srcHeight,
                    MFVideoInterlace_Progressive);
    LOGIFFAILED(mLogFile, hr, "Failed to create video type @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Set output bit rate                                                     *
     ***************************************************************************/
    hr = videoType->SetUINT32(MF_MT_AVG_BITRATE, averageBitrate);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     *  Set encoding profile                                                    *
     ***************************************************************************/
    hr = videoType->SetUINT32(MF_MT_MPEG2_PROFILE, h264Profile);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    /****************************************************************************
     *  get frame rate from source media type and set the same to output video  *
     *  type                                                                    *
     ****************************************************************************/
    UINT32 numerator;
    UINT32 denominator;
    hr = MFGetAttributeRatio(sourceVideoType, MF_MT_FRAME_RATE, &numerator,
                    &denominator);
    LOGIFFAILED(mLogFile, hr, "Failed to get attributes @ %s %d \n", __FILE__,
                    __LINE__);

    MFSetAttributeRatio(videoType, MF_MT_FRAME_RATE, numerator, denominator);
    LOGIFFAILED(mLogFile, hr, "Failed to set attributes @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     *  get pixel aspect ratio from source media type and set the same to       *
     * output video type                                                        *
     ***************************************************************************/
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
 *  @fn     instantiateVideoStream
 *  @brief  Instantiates MFTs required for video decoding and encoding
 *           
 *  @param[in] sourceMediaType     : Source Media Type
 *  @param[in] videoWindow         : Playback window
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::instantiateVideoStream(
                IMFMediaType* sourceMediaType, HWND videoWindow)
{
    HRESULT hr;
    ULONG_PTR deviceManagerPtr;

    /****************************************************************************
     *  Create source node                                                       *
     ****************************************************************************/
    hr = mMftBuilderObjPtr->createStreamSourceNode(mMediaSource,
                    mSourcePresentationDescriptor, mSourceStreamDescriptor,
                    &mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed create source node @ %s %d \n", __FILE__,
                    __LINE__);

    CComPtr < IMFActivate > videoRendererActivate;
    CLSID deviceManagerIID;

    if (mRendererType == SvcTranscodeSession::RendererDx11)
    {
        deviceManagerIID = BORROWED_IID_IMFDXGIDeviceManager;
        hr = mMftBuilderObjPtr->createDx11VideoRendererActivate(videoWindow,
                        &videoRendererActivate);
        LOGIFFAILED(mLogFile, hr,
                        "Failed create Dx11 Video renderer @ %s %d \n",
                        __FILE__, __LINE__);
    }
    else
    {
        deviceManagerIID = IID_IDirect3DDeviceManager9;
        hr = MFCreateVideoRendererActivate(videoWindow, &videoRendererActivate);
        LOGIFFAILED(mLogFile, hr,
                        "Failed create Dx9 Video renderer @ %s %d \n",
                        __FILE__, __LINE__);
    }

    CComPtr < IMFMediaSink > mediaSink;
    hr = videoRendererActivate->ActivateObject(IID_PPV_ARGS(&mediaSink));
    LOGIFFAILED(mLogFile, hr, "Failed create Video decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    CComPtr < IMFGetService > getService;
    hr = mediaSink->QueryInterface(IID_PPV_ARGS(&getService));
    LOGIFFAILED(mLogFile, hr, "Failed query interface @ %s %d \n", __FILE__,
                    __LINE__);

    CComPtr < IMFStreamSink > streamSink;
    hr = mediaSink->GetStreamSinkByIndex(0, &streamSink);
    LOGIFFAILED(mLogFile, hr, "Failed to get media sink @ %s %d \n", __FILE__,
                    __LINE__);

    CComPtr < IUnknown > deviceManager;
    hr = getService->GetService(MR_VIDEO_ACCELERATION_SERVICE,
                    deviceManagerIID, (void**) &deviceManager);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to get video acceleration service @ %s %d \n",
                    __FILE__, __LINE__);

    deviceManagerPtr = reinterpret_cast<ULONG_PTR> (deviceManager.p);

    /***************************************************************************
     *  Create decoder node                                                     *
     *  Supports H264,Mpeg 4 VC1                                                *
     ***************************************************************************/
    hr = mMftBuilderObjPtr->createVideoDecoderNode(sourceMediaType,
                    &mDecoderNode, deviceManagerPtr, NULL, false);
    LOGIFFAILED(mLogFile, hr, "Failed create Video decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     * Create H264 video type using the source media type                       *
     ***************************************************************************/
    CComPtr < IMFMediaType > h264VideoType;
    hr = createH264VideoType(&h264VideoType, sourceMediaType);
    LOGIFFAILED(mLogFile, hr, "Failed create H264 videotype @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Create SVC encode node                                                  *
     ***************************************************************************/
    hr = createSvcEncoderNode(h264VideoType, &mSvcEncodeNode, deviceManagerPtr,
                    SVC_LAYERS_COUNT);
    LOGIFFAILED(mLogFile, hr, "Failed create SVC encoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Create Tee node                                                         *
     ***************************************************************************/
    hr = MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &mTeeNode);
    LOGIFFAILED(mLogFile, hr, "Failed create Tee node @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     *  SVC splitter node                                                       *
     ***************************************************************************/
    hr = createSplitterTransform(&mSvcSplitterMft, &mSvcSplitterNode,
                    deviceManagerPtr);
    LOGIFFAILED(mLogFile, hr, "Failed create SVC splitter node @ %s %d \n",
                    __FILE__, __LINE__);

    CComPtr < IMFAttributes > outputAtributes;
    hr = mSvcSplitterMft->GetOutputStreamAttributes(0, &outputAtributes);
    LOGIFFAILED(mLogFile, hr, "Failed to get stream attributes @ %s %d \n",
                    __FILE__, __LINE__);

    hr = outputAtributes->SetUINT32(CLSID_SVC_MFT_OUTPUT_LAYERS_NUM_PROPERTY,
                    mSvcCurrentLayersNumber);
    LOGIFFAILED(mLogFile, hr, "Failed to set attribute @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mSvcSplitterMft->SetInputType(0, h264VideoType, 0);
    LOGIFFAILED(mLogFile, hr, "Failed to set input type @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     *  Create H264 decoder node                                                *
     ***************************************************************************/
    hr = mMftBuilderObjPtr->createVideoDecoderNode(h264VideoType,
                    &mH264DecoderNode, deviceManagerPtr, NULL, false);
    LOGIFFAILED(mLogFile, hr, "Failed create Video decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Create sink node for Renderer                                           *
     ***************************************************************************/
    CComPtr < IMFTopologyNode > streamSinkNode;
    hr = mMftBuilderObjPtr->createStreamSinkNode(streamSink, 0,
                    &mEvrStreamSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed create EVR sink node @ %s %d \n",
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
HRESULT SvcTranscodeSession::createSplitterTransform(IMFTransform** transform,
                IMFTopologyNode **transformNode, ULONG_PTR deviceManagerPtr)
{
    HRESULT hr;
    CComPtr < IMFTransform > mft;
    hr = mftSplitterCreateInstance(IID_IMFTransform, (void**) &mft);
    RETURNIFFAILED(hr);

    if (nullptr == transformNode)
    {
        return E_POINTER;
    }

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
    CComPtr < IMFTopologyNode > node;
    hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &node);
    RETURNIFFAILED(hr);

    hr = node->SetObject(mft);
    RETURNIFFAILED(hr);

    hr = node->SetUINT32(MF_TOPONODE_CONNECT_METHOD, MF_CONNECT_ALLOW_CONVERTER
                    | MF_CONNECT_RESOLVE_INDEPENDENT_OUTPUTTYPES);
    RETURNIFFAILED(hr);

    *transformNode = node.Detach();

    if (transform != nullptr)
    {
        *transform = mft.Detach();
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     createCustomSinkNode
 *  @brief  Creates custom sink node for dumping elementary SVC stream
 *           
 *  @param[in] streamNumber : Stream number from the source
 *  @param[in] fileName     : input file name, used for creating output file name
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::createCustomSinkNode(DWORD streamNumber,
                LPCWSTR fileName)
{
    HRESULT hr;

    std::wstring sinkFileName(fileName);

    size_t dotPos = sinkFileName.rfind(L".");
    if (dotPos == std::wstring::npos)
    {
        dotPos = sinkFileName.size();
    }

    size_t tailLength = sinkFileName.size() - dotPos;
    sinkFileName.replace(dotPos, tailLength, L".bin");

    CComPtr < IMFActivate > mediaSinkActivate;
    hr
                    = createCustomMediaSinkActivate(sinkFileName.c_str(),
                                    &mediaSinkActivate);
    LOGIFFAILED(mLogFile, hr, "Failed to activate custom sink @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mMftBuilderObjPtr->createStreamSinkNode(mediaSinkActivate,
                    streamNumber, &mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed to create custom sink node @ %s %d \n",
                    __FILE__, __LINE__);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     createSvcEncoderNode
 *  @brief  Create svc encoder node
 *           
 *  @param[in] encodedVideoType   : output type of encoder
 *  @param[out] encoderNode       : Place holder for encoder node to be created
 *  @param[in] deviceManagerPtr   : Dx9 or Dx11 device manager 
 *  @param[in] temporalLayersCount: Temporal layer count for SVC encoder
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::createSvcEncoderNode(
                IMFMediaType* encodedVideoType, IMFTopologyNode **encoderNode,
                ULONG_PTR deviceManagerPtr, uint32 temporalLayersCount)
{
    HRESULT hr;

    /***************************************************************************
     *  Create AMD SVC encoder MFT                                              *
     ***************************************************************************/
    CComPtr < IMFTransform > encoderTransform;
    hr = mMftBuilderObjPtr->createTransform(CLSID_AMD_H264_HW_EncoderMFT,
                    &encoderTransform, deviceManagerPtr);
    LOGIFFAILED(mLogFile, hr, "Failed to create encoder MFT @ %s %d \n",
                    __FILE__, __LINE__);

    hr = encoderTransform->SetOutputType(0, encodedVideoType, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to setoutput type for encoder @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Create Input media type                                                 *
     ***************************************************************************/
    CComPtr < IMFMediaType > inputMediaType;
    hr = mMftBuilderObjPtr->createVideoTypeFromSource(encodedVideoType,
                    MFVideoFormat_NV12, TRUE, TRUE, &inputMediaType);
    LOGIFFAILED(mLogFile, hr, "Failed to create input video type @ %s %d \n",
                    __FILE__, __LINE__);

    hr = encoderTransform->SetInputType(0, inputMediaType, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed set input media type for encoder @ %s %d \n",
                    __FILE__, __LINE__);

    CComPtr < IMFTopologyNode > node;
    hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, encoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed create node @ %s %d \n", __FILE__,
                    __LINE__);

    hr = (*encoderNode)->SetObject(encoderTransform);
    LOGIFFAILED(mLogFile, hr, "Failed set encoder object @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     *  Get icodec interface and set the temporal layer count                   *
     ***************************************************************************/
    CComPtr < ICodecAPI > codecApi;
    hr = encoderTransform->QueryInterface(&codecApi);
    LOGIFFAILED(mLogFile, hr, "Icodec interface not supported @ %s %d \n",
                    __FILE__, __LINE__);

    VARIANT layersCount = { 0 };
    layersCount.vt = VT_UI4;
    layersCount.uintVal = temporalLayersCount;

    hr = codecApi->SetValue(&BORROWED_CODECAPI_AVEncVideoTemporalLayerCount,
                    &layersCount);
    LOGIFFAILED(
                    mLogFile,
                    hr,
                    "Failed to set temporal layer count for SVC encoder @ %s %d \n",
                    __FILE__, __LINE__);

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     buildAndLoadTopology
 *  @brief  Build and load the topology
 *           
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT SvcTranscodeSession::buildAndLoadTopology()
{
    HRESULT hr = S_OK;

    /***************************************************************************
     *  Add source & sink node to the topology                                  *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add source node @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     *  Add decoder node to the topology source->Decoder                        *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mDecoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed to decoder node @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mSourceNode->ConnectOutput(0, mDecoderNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect source node->decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Add SVCEncoder node and connect Vq->Svcencoder                          *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mSvcEncodeNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add Video quality node @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mDecoderNode->ConnectOutput(0, mSvcEncodeNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect encoder node -> sink node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Add Tee node and connect SVCencoder->Tee                                *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mTeeNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add Video quality node @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mSvcEncodeNode->ConnectOutput(0, mTeeNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect encoder node -> sink node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Add SVC splitter for splitting the layer and connect Tee[0]->SVC splitter *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mSvcSplitterNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add Video quality node @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mTeeNode->ConnectOutput(0, mSvcSplitterNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect encoder node -> sink node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Add H264 decoder for transcode and connect SVC splitter->Decoder         *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mH264DecoderNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add Video quality node @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mSvcSplitterNode->ConnectOutput(0, mH264DecoderNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect encoder node -> sink node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Add Renderer for transcode and connect Decoder->Renderer                 *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mEvrStreamSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add Video quality node @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mH264DecoderNode->ConnectOutput(0, mEvrStreamSinkNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect encoder node -> sink node @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Add sink node for dumping elementary stream and connect Tee[1]->sink    *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add Video quality node @ %s %d \n",
                    __FILE__, __LINE__);

    hr = mTeeNode->ConnectOutput(1, mSinkNode, 0);
    LOGIFFAILED(mLogFile, hr, "Failed to add sink node @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mMftBuilderObjPtr->bindOutputNodes(mPartialTopology);
    RETURNIFFAILED(hr);
    /***************************************************************************
     *  set hardware acceleration to the topology                               *
     ***************************************************************************/
    hr = mMftBuilderObjPtr->setHardwareAcceleration(mPartialTopology, true);
    LOGIFFAILED(mLogFile, hr, "Failed to set hardware acceleration @ %s %d \n",
                    __FILE__, __LINE__);

    /***************************************************************************
     *  Create topoloder, load the topology & resolve the same                  *
     ***************************************************************************/
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
 *  @fn     isDx11RendererSupported
 *  @brief  Checks support for Dx11 device 
 *           
 *  @return BOOL : ture if supports Dx11 device creation 
 *******************************************************************************
 */
bool SvcTranscodeSession::isDx11RendererSupported()
{
    return (mMftBuilderObjPtr->isDxgiSupported());
}