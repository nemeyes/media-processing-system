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
 * @file <PlaybackSession.cpp>
 *
 * @brief This file contains functions for building playback topology
 *
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include "PlaybackSession.h"

/**
 *******************************************************************************
 *  @fn     create
 *  @brief  Creates playback session instance.
 *
 *  @param[in] stateSubscriber   : Pointer to the state subscriber
 *  @param[out] playbackSession  : Pointer to the playback class instance
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::create(PlaybackStateSubscriber* stateSubscriber,
                PlaybackSession** playbackSession)
{
    HRESULT hr;
    if (nullptr == playbackSession)
    {
        return E_POINTER;
    }
    /***************************************************************************
     * Create playback session instance                                         *
     ***************************************************************************/
    *playbackSession = new (std::nothrow) PlaybackSession(stateSubscriber);
    if (nullptr == *playbackSession)
    {
        return E_OUTOFMEMORY;
    }
    if ((*playbackSession)->mState != PlaybackSession::Building)
    {
        delete *playbackSession;
        return E_FAIL;
    }
    (*playbackSession)->AddRef();

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    RETURNIFFAILED(hr);

    hr = MFStartup(MF_VERSION);
    RETURNIFFAILED(hr);

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     PlaybackSession
 *  @brief  Constructor
 *
 *  @param[in] stateSubscriber   : Pointer to the state subscriber
 *
 *  @return none
 *******************************************************************************
 */
PlaybackSession::PlaybackSession(PlaybackStateSubscriber* stateSubscriber) :
    mRefCount(0), mState(PlaybackSession::Building), stateSubscriber(
                    stateSubscriber), mRendererType(
                    PlaybackSession::RendererDx9), mSessionClosedEvent(NULL),
                    mLogFile(NULL)
{
    mSessionClosedEvent = ::CreateEvent(NULL, FALSE, FALSE, _T(
                    "Media session closed event"));
    if (NULL == mSessionClosedEvent)
    {
        TRACE_MSG("Failed to create event object", 0);
        mState = PlaybackSession::Failed;
    }
    mMftBuilderObjPtr = new msdk_CMftBuilder;
}
/**
 *******************************************************************************
 *  @fn     PlaybackSession
 *  @brief  Destructor
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
PlaybackSession::~PlaybackSession(void)
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
    if (mMediaSession != nullptr)
    {
        mMediaSession->Shutdown();
    }
    if (mSessionClosedEvent != NULL)
    {
        CloseHandle( mSessionClosedEvent);
    }
    delete mMftBuilderObjPtr;
}
/**
 *******************************************************************************
 *  @fn     getState
 *  @brief  Returns the state of the playback sessoin
 *
 *  @return State : Returns state of the playback sessoin
 *******************************************************************************
 */
PlaybackSession::State PlaybackSession::getState()
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
PlaybackSession::RendererType PlaybackSession::getRendererType()
{
    return mRendererType;
}
/**
 *******************************************************************************
 *  @fn     setState
 *  @brief  Sets the state of playback
 *
 *  @param[in] state   : State of the playback
 *
 *  @return void.
 *******************************************************************************
 */
void PlaybackSession::setState(PlaybackSession::State state)
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
void PlaybackSession::setLogFile(FILE *logFile)
{
    mLogFile = logFile;
}

/**
 *******************************************************************************
 *  @fn     releaseResources
 *  @brief  Will release the resources used by playback
 *
 *  @return void.
 *******************************************************************************
 */
void PlaybackSession::releaseResources()
{
    mDeviceManager.Release();
    mCapabilityManager.Release();
    mVideoDisplayControl.Release();
    mSourcePresentationDescriptor.Release();
    mVqTransform.Release();
    mTopology.Release();
    mPartialTopology.Release();
    mMediaSource.Release();
    mSourceStreamDescriptor.Release();
    mSourceNode.Release();
    mDecoderNode.Release();
    mVqNode.Release();
    mEncoderNode.Release();
    mSinkNode.Release();
    mMediaSinkActivate.Release();
    mDxgiDeviceManager.Release();
    mMediaSession->Close();
}
/**
 *******************************************************************************
 *  @fn     openFile
 *  @brief  This function will be called once openFile button is pressed from GUI
 *          This will intantiate and build the topology
 *
 *  @param[in] fileName       : Input soruce file which needs to be played
 *  @param[in] rendererWindow : Window for rendering the video
 *  @param[in] rendererType   : Type of the renderer used : Dx11(win8) or Dx9 (Win8 & 7)
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::openFile(LPCWSTR fileName, HWND rendererWindow,
                RendererType rendererType)
{
    HRESULT hr;
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);
    /***************************************************************************
     * If the state of playback is Building release the resources and           *
     * instantiate freshtly                                                     *
     * This means file which was previously played is stopped and another file  *
     * is getting opened for playing. In this case depending on the file        *
     * topology is build                                                        *
     ***************************************************************************/
    if (mState != PlaybackSession::Building)
    {
        mMediaSession->Shutdown();
        releaseResources();
        mMediaSession.Release();
    }
    mRendererType = rendererType;
    /***************************************************************************
     * Intantiate required MFTs for playback functionality                      *
     * pipeline : source->AMDDecoder->VideoQuality filter->renderer             *
     ***************************************************************************/
    hr = instantiateTopology(rendererWindow, fileName);
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
        setState(PlaybackSession::Failed);
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
HRESULT PlaybackSession::play()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (mState != PlaybackSession::Stopped && mState != PlaybackSession::Paused)
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
HRESULT PlaybackSession::stop()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = mMediaSession->Stop();
    if (SUCCEEDED(hr))
    {
        setState(PlaybackSession::StopPending);
    }
    else
    {
        mMediaSession->Shutdown();
        setState(PlaybackSession::Failed);
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
HRESULT PlaybackSession::shutdown()
{
    if (PlaybackSession::Failed == mState)
    {
        return S_OK;
    }
    {
        CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                        mStateLock, true);
        if (mMediaSession == nullptr)
        {
            return E_FAIL;
        }
        releaseResources();
        setState(PlaybackSession::Closing);
    }

    const DWORD maxWaitTime = 60000;
    DWORD waitResult = WaitForSingleObject(mSessionClosedEvent, maxWaitTime);
    if (WAIT_OBJECT_0 != waitResult)
    {
        TRACE_MSG("Failed to gracefully close media session.", waitResult);
    }
    else
    {
        TRACE_MSG("Media session closed.", 0);
    }

    {
        CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                        mStateLock, true);

        HRESULT hr = mMediaSession->Shutdown();
        LOGIFFAILED(mLogFile, hr,
                        "Failed to shutdown media session @ %s %d \n",
                        __FILE__, __LINE__);
        mMediaSession.Release();
        setState(PlaybackSession::Failed);
    }
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
HRESULT PlaybackSession::pause()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (mState != PlaybackSession::Playing)
    {
        return E_FAIL;
    }
    HRESULT hr = mMediaSession->Pause();
    if (SUCCEEDED(hr))
    {
        setState(PlaybackSession::PausePending);
    }
    else
    {
        setState(PlaybackSession::Failed);
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
HRESULT PlaybackSession::resume()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (mState != PlaybackSession::Paused)
    {
        return E_FAIL;
    }

    return intPlay();
}
/**
 *******************************************************************************
 *  @fn     getDuration
 *  @brief  getDuration
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::getDuration(UINT64* duration)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (PlaybackSession::Building == mState || PlaybackSession::Failed
                    == mState)
    {
        return E_FAIL;
    }

    HRESULT hr;

    hr = mSourcePresentationDescriptor->GetUINT64(MF_PD_DURATION, duration);
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     getTime
 *  @brief  getTime
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::getTime(MFTIME* currentTime)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (PlaybackSession::Playing != mState && PlaybackSession::Paused != mState)
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
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    CComPtr < IMFPresentationClock > presentationClock;
    hr = clock->QueryInterface(&presentationClock);
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    hr = presentationClock->GetTime(currentTime);
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     setVideoQualityAttributes
 *  @brief  This function will enables/sets video quality filters
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::setVideoQualityAttributes(IMFAttributes* vqAttributes)
{
    HRESULT hr;
    if (nullptr == vqAttributes)
    {
        return E_POINTER;
    }
    if (nullptr == mVqTransform)
    {
        return E_FAIL;
    }
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);
    CComPtr < IMFAttributes > vqTransformAttributes;

    hr = mVqTransform->GetAttributes(&vqTransformAttributes);
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    hr = vqAttributes->CopyAllItems(vqTransformAttributes);
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    hr = vqTransformAttributes->SetUINT32(AMF_EFFECT_CHANGED,
                    static_cast<UINT32> (TRUE));
    LOGIFFAILED(mLogFile, hr, "Failed to set topo attributes @ %s %d \n",
                    __FILE__, __LINE__);

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     resizeVideo
 *  @brief  Called when window resize happens
 *
 *  @param[in] width  : Width
 *  @param[in] height : Height
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::resizeVideo(int width, int height)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (nullptr != mVideoDisplayControl)
    {
        HRESULT hr;

        hr = mVideoDisplayControl->RepaintVideo();
        LOGIFFAILED(mLogFile, hr, "Failed to Repaint video @ %s %d \n",
                        __FILE__, __LINE__);

        RECT rectDest = { 0, 0, width, height };
        hr = mVideoDisplayControl->SetVideoPosition(NULL, &rectDest);
        LOGIFFAILED(mLogFile, hr, "Failed to set video position @ %s %d \n",
                        __FILE__, __LINE__);

    }

    if (nullptr != mVqTransform)
    {
        HRESULT hr;

        CComPtr < IMFAttributes > vqAttributes;
        hr = mVqTransform->GetAttributes(&vqAttributes);
        RETURNIFFAILED(hr);

        hr = vqAttributes->SetUINT32(AMF_EFFECT_SCALE_WIDTH, width);
        RETURNIFFAILED(hr);

        hr = vqAttributes->SetUINT32(AMF_EFFECT_SCALE_HEIGHT, height);
        RETURNIFFAILED(hr);
    }

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
HRESULT PlaybackSession::GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)
{
    (void) pdwFlags;
    (void) pdwQueue;
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
HRESULT PlaybackSession::Invoke(IMFAsyncResult *asyncResult)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr;

    CComPtr < IMFMediaEvent > event;
    hr = mMediaSession->EndGetEvent(asyncResult, &event);

    if (FAILED(hr))
    {
        setState(PlaybackSession::Failed);
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
            setState(PlaybackSession::Paused);
            break;

        case MESessionStopped:
            setState(PlaybackSession::Stopped);
            /*******************************************************************
             *  Do not shutdown media session because it can be restarted       *
             *******************************************************************/
            break;

        case MESessionStarted:
            LOG(mLogFile, "Started the Media Session\n");
            setState(PlaybackSession::Playing);
            break;

        case MEError:
            LOG(mLogFile, "Received MEError in the Media Session @ %s, %d\n",
                            __FILE__, __LINE__);
            setState(PlaybackSession::Failed);
            mMediaSession->Shutdown();
            break;

        case MESessionEnded:
            LOG(mLogFile, "Stopped the Media Session\n");
            setState(PlaybackSession::Stopped);
            intPlay();
            break;
        case MESessionClosed:
            LOG(mLogFile, "Closed the Media Session\n");
            SetEvent( mSessionClosedEvent);
            setState(PlaybackSession::Closed);
            break;
        }
    }

    if (mMediaSession != nullptr && mediaEventType != MEError && mediaEventType
                    != MESessionClosed)
    {
        hr = mMediaSession->BeginGetEvent(this, nullptr);
        RETURNIFFAILED(hr);
    }
    else
    {
        TRACE_MSG("Stopped getting new events", mediaEventType);
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
HRESULT PlaybackSession::QueryInterface(REFIID riid, void** ppv)
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
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
ULONG PlaybackSession::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}
/**
 *******************************************************************************
 *  @fn     Release
 *  @brief  virtual function
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
ULONG PlaybackSession::Release()
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
 *  @fn     intPlay
 *  @brief  This function runs the playback topology
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::intPlay()
{
    HRESULT hr;

    PROPVARIANT variantStart;
    PropVariantInit(&variantStart);

    hr = mMediaSession->Start(&GUID_NULL, &variantStart);
    if (SUCCEEDED(hr))
    {
        setState(PlaybackSession::PlayPending);
    }
    else
    {
        hr = mMediaSession->Shutdown();
        setState(PlaybackSession::Failed);
        return hr;
    }

    PropVariantClear(&variantStart);
    mVideoDisplayControl.Release();

    hr = MFGetService(mMediaSession, MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(
                    &mVideoDisplayControl));
    RETURNIFFAILED(hr);

    return hr;
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
HRESULT PlaybackSession::instantiateTopology(HWND videoWindow, LPCWSTR fileName)
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
    /***************************************************************************
     *  Loop until video stream if found                                        *
     ***************************************************************************/
    for (DWORD i = 0; i < streamDescriptorCount; i++)
    {
        BOOL selected;
        CComPtr < IMFStreamDescriptor > sourceStreamDescriptor; /**< Input stream descriptor */
        hr = mSourcePresentationDescriptor->GetStreamDescriptorByIndex(i,
                        &selected, &sourceStreamDescriptor);
        RETURNIFFAILED(hr);

        /************************************************************************
         *  Get the media type from the source descriptor and break if stream    *
         *  type is video.                                                       *
         *  Store the media type for setting preparing output video type         *
         ************************************************************************/
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
    hr = instantiateVideoStream(streamNumber, sourceMediaType,
                    CLSID_AMFVideoTransform, videoWindow);

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
 *  @param[in] customTransformGuid : GUID for custom transform
 *  @param[in] videoWindow         : Playback window
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::instantiateVideoStream(DWORD streamNumber,
                IMFMediaType* sourceMediaType, REFGUID customTransformGuid,
                HWND videoWindow)
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

    if (mRendererType == PlaybackSession::RendererDx11)
    {
        deviceManagerIID = IID_IMFDXGIDeviceManager;
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
    RETURNIFFAILED(hr);

    CComPtr < IMFGetService > getService;
    hr = mediaSink->QueryInterface(IID_PPV_ARGS(&getService));
    RETURNIFFAILED(hr);

    CComPtr < IMFStreamSink > streamSink;
    hr = mediaSink->GetStreamSinkByIndex(0, &streamSink);
    RETURNIFFAILED(hr);

    hr = getService->GetService(MR_VIDEO_ACCELERATION_SERVICE,
                    deviceManagerIID, (void**) &mDeviceManager);
    RETURNIFFAILED(hr);

    deviceManagerPtr = reinterpret_cast<ULONG_PTR> (mDeviceManager.p);

    /****************************************************************************
     *  Create decoder node                                                      *
     *  Supports H264,Mpeg 4 VC1                                                 *
     ****************************************************************************/
    hr = mMftBuilderObjPtr->createVideoDecoderNode(sourceMediaType,
                    &mDecoderNode, deviceManagerPtr, NULL, false);
    LOGIFFAILED(mLogFile, hr, "Failed create Video decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    /****************************************************************************
     *  Create VQ node                                                           *
     ****************************************************************************/
    if (customTransformGuid != GUID_NULL)
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

        hr = AMFCreateCapabilityManagerMFT(IID_PPV_ARGS(&mCapabilityManager));
        RETURNIFFAILED(hr);

        hr = mCapabilityManager->Init((IUnknown*) deviceManagerPtr,
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
            hr = mMftBuilderObjPtr->createVqTransform(deviceManagerPtr,
                            &mVqTransform);
            LOGIFFAILED(mLogFile, hr,
                            "Failed create custrom transform node @ %s %d \n",
                            __FILE__, __LINE__);

            hr = mMftBuilderObjPtr->createTransformNode(mVqTransform,
                            MF_CONNECT_ALLOW_CONVERTER, &mVqNode);

            CComPtr < IMFMediaType > vqOutputType;
            hr = mMftBuilderObjPtr->createVideoTypeFromSource(sourceMediaType,
                            MFVideoFormat_RGB32, TRUE, TRUE, &vqOutputType);
            LOGIFFAILED(mLogFile, hr,
                            "Failed create custrom transform node @ %s %d \n",
                            __FILE__, __LINE__);
            /************************************************************************
             * get frame rate from source media type and set the same to output video*
             * type                                                                  *
             ************************************************************************/
            hr = vqOutputType->DeleteItem(MF_MT_DEFAULT_STRIDE);
            LOGIFFAILED(mLogFile, hr,
                            "Failed create custrom transform node @ %s %d \n",
                            __FILE__, __LINE__);

            hr = mVqTransform->SetOutputType(0, vqOutputType, 0);
            LOGIFFAILED(mLogFile, hr,
                            "Failed create custrom transform node @ %s %d \n",
                            __FILE__, __LINE__);
        }
    }

    /****************************************************************************
     *  Create sink node                                                         *
     ****************************************************************************/
    hr = mMftBuilderObjPtr->createStreamSinkNode(streamSink, streamNumber,
                    &mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed create sink node @ %s %d \n", __FILE__,
                    __LINE__);

    return S_OK;
}
/**
 *******************************************************************************
 *  @fn     getVideoQualityAttributes
 *  @brief  Returns the video quality attributes set
 *
 *  @param[out] ppAttributes        : Pointer to the IMFAttributes interface
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::getVideoQualityAttributes(IMFAttributes** ppAttributes)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (nullptr == mVqTransform)
    {
        TRACE_MSG("Invalid call for GetVideoQualityAttributes.", 0);

        return E_FAIL;
    }

    return mVqTransform->GetAttributes(ppAttributes);
}

/**
 *******************************************************************************
 *  @fn     buildAndLoadTopology
 *  @brief  Build and load the topology
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT PlaybackSession::buildAndLoadTopology()
{
    HRESULT hr = S_OK;

    /***************************************************************************
     *  Add source & sink node to the topology                                  *
     ***************************************************************************/
    hr = mPartialTopology->AddNode(mSourceNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add source node @ %s %d \n", __FILE__,
                    __LINE__);

    hr = mPartialTopology->AddNode(mSinkNode);
    LOGIFFAILED(mLogFile, hr, "Failed to add sink node @ %s %d \n", __FILE__,
                    __LINE__);

    /***************************************************************************
     *  Add decoder, VQ node to the topology                                    *
     ***************************************************************************/
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

    /***************************************************************************
     *  Connect sourceNode->DecoderNode->Vqnode->sinkNode                       *
     ***************************************************************************/
    hr = mSourceNode->ConnectOutput(0, mDecoderNode, 0);
    LOGIFFAILED(mLogFile, hr,
                    "Failed to connect source node->decoder node @ %s %d \n",
                    __FILE__, __LINE__);

    if (mIsVqSupported == true)
    {
        hr = mDecoderNode->ConnectOutput(0, mVqNode, 0);
        LOGIFFAILED(
                        mLogFile,
                        hr,
                        "Failed to connect decodernode->vqnode node @ %s %d \n",
                        __FILE__, __LINE__);

        hr = mVqNode->ConnectOutput(0, mSinkNode, 0);
        LOGIFFAILED(mLogFile, hr,
                        "Failed to connect vqnode -> sink node @ %s %d \n",
                        __FILE__, __LINE__);
    }
    else
    {
        hr = mDecoderNode->ConnectOutput(0, mSinkNode, 0);
        LOGIFFAILED(mLogFile, hr,
                        "Failed to connect decodernode->sink node @ %s %d \n",
                        __FILE__, __LINE__);
    }

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
 *  @brief  Is dx11 rendering supported
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
bool PlaybackSession::isDx11RendererSupported()
{
    return (mMftBuilderObjPtr->isDxgiSupported());
}

HRESULT PlaybackSession::setRealtimePlaybackSettings()
{
    if (nullptr == mSourcePresentationDescriptor)
    {
        return E_FAIL;
    }

    HRESULT hr;

    CComPtr < IMFMediaType > sourceVideoMediaType;
    hr = getFirstVideoMediaType(mSourcePresentationDescriptor,
                    &sourceVideoMediaType);
    RETURNIFFAILED(hr);

    hr = configureVq(mVqTransform, sourceVideoMediaType, AMF_CM_REALTIME);
    RETURNIFFAILED(hr);

    return S_OK;
}
HRESULT PlaybackSession::getFirstVideoMediaType(
                IMFPresentationDescriptor* presentationDescriptor,
                IMFMediaType** firstVideoMediaType)
{
    if (nullptr == presentationDescriptor)
    {
        return E_INVALIDARG;
    }

    if (nullptr == firstVideoMediaType)
    {
        return E_POINTER;
    }

    HRESULT hr;

    DWORD streamDescriptorCount;
    hr = presentationDescriptor->GetStreamDescriptorCount(
                    &streamDescriptorCount);
    RETURNIFFAILED(hr);

    for (DWORD i = 0; i < streamDescriptorCount; i++)
    {
        BOOL selected;
        CComPtr < IMFStreamDescriptor > streamDescriptor;
        hr = presentationDescriptor->GetStreamDescriptorByIndex(i, &selected,
                        &streamDescriptor);
        RETURNIFFAILED(hr);

        CComPtr < IMFMediaTypeHandler > mediaTypeHandler;
        hr = streamDescriptor->GetMediaTypeHandler(&mediaTypeHandler);
        RETURNIFFAILED(hr);

        CComPtr < IMFMediaType > mediaType;
        hr = mediaTypeHandler->GetCurrentMediaType(&mediaType);
        RETURNIFFAILED(hr);

        GUID majorType;
        hr = mediaType->GetMajorType(&majorType);
        RETURNIFFAILED(hr);

        if (IsEqualGUID(majorType, MFMediaType_Video))
        {
            return mediaType->QueryInterface(IID_PPV_ARGS(firstVideoMediaType));
        }
    }

    return E_FAIL;
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
HRESULT PlaybackSession::configureVq(IMFTransform* vqTransform,
                IMFMediaType* sourceMediaType, AMFCMRequestType requestType)
{
    HRESULT hr = S_OK;

    if (nullptr == mVqTransform || nullptr == sourceMediaType)
    {
        return E_INVALIDARG;
    }

    CComPtr < IMFAttributes > vqAttributes;
    hr = vqTransform->GetAttributes(&vqAttributes);
    RETURNIFFAILED(hr);

    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_STEADY_VIDEO))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO, 0);
    }

    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION, 0);
    }

    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_EDGE_ENHANCEMENT))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_EDGE_ENHANCEMENT, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_EDGE_ENHANCEMENT, 0);
    }
    if (S_OK == mCapabilityManager->IsEnabled(requestType, AMF_EFFECT_DENOISE))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_DENOISE, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_DENOISE, 0);
    }

    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_MOSQUITO_NOISE))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_MOSQUITO_NOISE, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_MOSQUITO_NOISE, 0);
    }

    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_DEBLOCKING))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_DEBLOCKING, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_DEBLOCKING, 0);
    }
    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_DYNAMIC_CONTRAST))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_DYNAMIC_CONTRAST, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_DYNAMIC_CONTRAST, 0);
    }
    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_COLOR_VIBRANCE))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_COLOR_VIBRANCE, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_COLOR_VIBRANCE, 0);
    }

    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_SKINTONE_CORRECTION))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_SKINTONE_CORRECTION, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_SKINTONE_CORRECTION, 0);
    }
    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_BRIGHTER_WHITES))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_BRIGHTER_WHITES, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_BRIGHTER_WHITES, 0);
    }
    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_GAMMA_CORRECTION))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_GAMMA_CORRECTION, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_GAMMA_CORRECTION, 0);
    }

    if (S_OK == mCapabilityManager->IsEnabled(requestType,
                    AMF_EFFECT_FALSE_CONTOUR_REDUCTION))
    {
        vqAttributes->SetUINT32(AMF_EFFECT_FALSE_CONTOUR_REDUCTION, 1);
    }
    else
    {
        vqAttributes->SetUINT32(AMF_EFFECT_FALSE_CONTOUR_REDUCTION, 0);
    }

    vqAttributes->SetUINT32(AMF_EFFECT_DYNAMIC, 1);
    vqAttributes->SetUINT32(AMF_EFFECT_CHANGED, 1);
    return S_OK;
}

HRESULT PlaybackSession::vqStatus(bool *pIsVqSupported)
{
    *pIsVqSupported = mIsVqSupported;
    return S_OK;
}