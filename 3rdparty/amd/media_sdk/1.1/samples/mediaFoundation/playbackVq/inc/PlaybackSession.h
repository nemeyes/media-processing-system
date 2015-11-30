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
 * @file <PlaybackSession.h>
 *
 * @brief This file contains common functionality required for creating the
 *        topology
 *
 ********************************************************************************
 */
#ifndef _PLAYBACKVQSESSION_H_
#define _PLAYBACKVQSESSION_H_
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include <evr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <atlbase.h>
#include "PrintLog.h"
#include "VqMft.h"
#include "MftUtils.h"
#include "Typedef.h"
#include "ErrorCodes.h"

struct PlaybackStateSubscriber;
class PlaybackSession;
/*******************************************************************************
 * Class id for Video quality filter used in playback topology                  *
 *******************************************************************************/
/**
 *   @class PlaybackSession. Builds and manages playback topology with AMD VQ HW MFT.
 */
class PlaybackSession: public IMFAsyncCallback
{
public:
    enum RendererType
    {
        RendererDx9, RendererDx11
    };

    /**
     *   @brief PlaybackSession states.
     */
    enum State
    {
        Building, // -> Ready, Failed
        Ready, // -> PlayPending, Failed
        PlayPending, // -> Playing, Failed
        Playing, // -> PausePending, StopPending, Stopped, Failed
        PausePending, // -> Playing, Failed
        Paused, // -> PlayPending, Failed
        StopPending, // -> Stopped, Failed
        Stopped, // -> PlayPending, Failed
        Closing, // -> Closed, Failed.
        Closed, // -> Failed.
        Failed
    // Destroy and recreate is only option.
    };
    bool isDx11RendererSupported();

    /**
     *   @brief instantiateTopology(). Creates required components of playback topology.
     */
    HRESULT instantiateTopology(HWND videoWindow, LPCWSTR fileName);
    /**
     *   @brief buildAndLoadTopology(). Builds & loads the playback topology.
     */
    HRESULT buildAndLoadTopology();
    HRESULT resizeVideo(int width, int height);
    HRESULT getVideoQualityAttributes(IMFAttributes** ppAttributes);
    HRESULT setRealtimePlaybackSettings();
    /**
     *   @brief create. Fabric method - creates PlaybackSession instances.
     */
    static HRESULT create(PlaybackStateSubscriber* stateSubscriber,
                    PlaybackSession** playbackSession);

    /**
     *   @brief openFile.
     */
    HRESULT openFile(LPCWSTR fileName, HWND rendererWindow,
                    RendererType rendererType);

    /**
     *   @brief play.
     */
    HRESULT play();

    /**
     *   @brief stop.
     */
    HRESULT stop();

    /**
     *   @brief shutdown.
     */
    HRESULT shutdown();

    /**
     *   @brief pause.
     */
    HRESULT pause();

    /**
     *   @brief resume.
     */
    HRESULT resume();

    /**
     *   @brief getState. Returns current state of PlaybackSession.
     */
    State getState();

    /**
     *   @brief GetRenderType. Returns current renderer type.
     */
    RendererType getRendererType();

    /**
     *   @brief getDuration. Returns duration of video.
     */
    HRESULT getDuration(UINT64* duration);

    /**
     *   @brief getTime. Returns current playback time.
     */
    HRESULT getTime(MFTIME* currentTime);

    /**
     *   @brief vqStatus. Shows VQ is supported or not.
     */
    HRESULT vqStatus(bool *isVqSupported);
    /**
     *   @brief setVideoQualityAttributes.
     */
    HRESULT setVideoQualityAttributes(IMFAttributes* vqAttributes);

    /**
     *   @brief IMFAsyncCallback::GetParameters().
     */
    virtual HRESULT STDMETHODCALLTYPE GetParameters(DWORD *pdwFlags, DWORD *pdwQueue);

    /**
     *   @brief IIMFAsyncCallback::Invoke().
     */
    virtual HRESULT STDMETHODCALLTYPE Invoke(IMFAsyncResult *pAsyncResult);

    /**
     *   @brief IUnknown::QueryInterface().
     */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

    /**
     *   @brief IUnknown::AddRef().
     */
    virtual ULONG STDMETHODCALLTYPE AddRef();

    /**
     *   @brief IUnknown::Release().
     */
    virtual ULONG STDMETHODCALLTYPE Release();

    FILE *mLogFile;

    void setLogFile(FILE *logFile);

protected:

    /**
     *   @brief Destructor.
     */
    virtual ~PlaybackSession(void);

private:

    /**
     *   @brief Constructor.
     *   @param rendererWindow HWND of window which should be used for video.
     *   @param stateSubscriber Pointer to an instance of PlaybackStateSubscriber.
     *          The instance must be alive all till PlaybackSession is destroyed.
     */
    PlaybackSession(PlaybackStateSubscriber* stateSubscriber);
    HRESULT instantiateVideoStream(DWORD streamNumber,
                    IMFMediaType* sourceMediaType, REFGUID customTransformGuid,
                    HWND videoWindow);

    HRESULT getFirstVideoMediaType(
                    IMFPresentationDescriptor* presentationDescriptor,
                    IMFMediaType** firstVideoMediaType);
    HRESULT
                    configureVq(IMFTransform* vqTransform,
                                    IMFMediaType* sourceMediaType,
                                    AMFCMRequestType requestType);

    HRESULT createCapabilityManager(IMFTransform* vqTransform,
                    IMFMediaType* sourceMediaType, ULONG_PTR deviceManagerPtr);

    HRESULT intPlay();
    void setState(State state);
    void releaseResources();
    State mState;
    RendererType mRendererType;
    CComMultiThreadModel::AutoCriticalSection mStateLock;
    HANDLE mSessionClosedEvent;

    PlaybackStateSubscriber* stateSubscriber;

    CComPtr<IMFMediaSession> mMediaSession;
    CComPtr<IMFVideoDisplayControl> mVideoDisplayControl;
    ULONG mRefCount;
    msdk_CMftBuilder *mMftBuilderObjPtr; /**< Pointer for MFT Builder utility class*/
    CComPtr<IMFTopology> mPartialTopology; /**< pointer to the partial topology */
    CComPtr<IMFMediaSource> mMediaSource; /**< media source pointer */
    CComPtr<IMFPresentationDescriptor> mSourcePresentationDescriptor; /**< Input source descriptor */
    CComPtr<IMFActivate> mMediaSinkActivate; /**< Pointer to the sink */
    CComPtr<IMFStreamDescriptor> mSourceStreamDescriptor; /**< Input stream descriptor */
    CComPtr<IMFDXGIDeviceManager> mDxgiDeviceManager; /**< Device manager pointer */
    CComPtr<IMFTopology> mTopology; /**< Pointer to the topology */
    CComPtr<IMFTransform> mVqTransform; /**< Pointer to the video quality MFT */
    CComPtr<IMFTopologyNode> mSourceNode; /**< Source node*/
    CComPtr<IMFTopologyNode> mDecoderNode; /**< Decoder node*/
    CComPtr<IMFTopologyNode> mVqNode; /**< Video Quality filter node*/
    CComPtr<IMFTopologyNode> mEncoderNode; /**< Encoder node*/
    CComPtr<IMFTopologyNode> mSinkNode; /**< sink node*/
    CComPtr<IUnknown> mDeviceManager;
    CComPtr<IAMFCapabilityManager> mCapabilityManager;

    HRESULT setVqAttributes(CComPtr<IMFAttributes> vqAttributes);
    bool mIsVqSupported;

};

/**
 *   @class PlaybackStateSubscriber. Defines an interface for PlaybackSession state updates subscribers.
 */
struct PlaybackStateSubscriber
{
    /**
     *   @brief onStateChange().
     */
    virtual void onStateChange(PlaybackSession::State newState) = 0;
};
#endif//_PLAYBACKVQSESSION_H_
