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
 * @file <TranscodeSession.h>                          
 *                                       
 * @brief This file contains common functionality required for creating the 
 *        topology
 *         
 ********************************************************************************
 */
#ifndef _TRANSCODESESSION_H_
#define _TRANSCODESESSION_H_
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
#include "MftSvcSplitterTransform.h"
#include "MftSvcSplitterGuids.h" 
#include "CustomMediaSinkActivate.h"
#include "MftSvcSplitterTransformApi.h"

struct PlaybackStateSubscriber;
class SvcTranscodeSession;
#define SVC_LAYERS_COUNT  3
#define AVG_BIT_RATE 5000000

/**
 *   @class TranscodeSession. Builds and manages transcode topology with AMD VQ HW MFT.
 */
class SvcTranscodeSession: public IMFAsyncCallback
{
public:
    enum RendererType
    {
        RendererDx9, RendererDx11
    };

    /**
     *   @brief TranscodeSession states.
     */
    enum State
    {
        Building, /**<  Ready, Failed */
        Ready, /**<   PlayPending, Failed */
        PlayPending, /**<   Playing, Failed */
        Playing, /**<   PausePending, Stopped, Failed */
        PausePending, /**<   Playing, Failed */
        Paused, /**<   PlayPending, Failed */
        StopPending, /**<   Stopped, Failed */
        Stopped, /**<   PlayPending, Failed */
        Failed
    /**<   Destroy and recreate is only option */
    };
    bool isDx11RendererSupported();

    /**
     *   @brief instantiateTopology(). Creates required components of transcode topology.
     */
    HRESULT instantiateTopology(HWND videoWindow, LPCWSTR fileName);
    /**
     *   @brief buildAndLoadTopology(). Builds & loads the transcode topology.
     */
    HRESULT buildAndLoadTopology();
    /**
     *   @brief create. Fabric method - creates TranscodeSession instances.
     */
    static HRESULT create(HWND videoRenderWindow,
                    PlaybackStateSubscriber* stateSubscriber,
                    SvcTranscodeSession** TranscodeSession);
    /**
     *   @brief Sets the SVC temporal layers
     */
    HRESULT setSvcLayersNumber(UINT32 num);
    /**
     *   @brief openFile.
     */
    HRESULT openFile(LPCWSTR fileName, bool useDx9);

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
     *   @brief getState. Returns current state of TranscodeSession.
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
     *   @brief getTime. Returns current transcode time.
     */
    HRESULT getTime(MFTIME* currentTime);

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

    HRESULT SetSVCLayersNumber(UINT32 num);
    FILE *mLogFile;
    void setLogFile(FILE *logFile);
protected:

    /**
     *   @brief Destructor.
     */
    virtual ~SvcTranscodeSession(void);

private:

    /**
     *   @brief Constructor.
     *   @param videoRenderWindow HWND of window which should be used for video.
     *   @param stateSubscriber Pointer to an instance of PlaybackStateSubscriber.
     *          The instance must be alive all till TranscodeSession is destroyed.
     */
    SvcTranscodeSession(HWND videoRenderWindow,
                    PlaybackStateSubscriber* stateSubscriber);
    HRESULT
                    createSplitterTransform(IMFTransform** transform,
                                    IMFTopologyNode **transformNode,
                                    ULONG_PTR deviceManagerPtr);
    HRESULT instantiateVideoStream(IMFMediaType* sourceMediaType,
                    HWND videoWindow);
    HRESULT createH264VideoType(IMFMediaType** encodedVideoType,
                    IMFMediaType* sourceVideoType);
    HRESULT createSvcEncoderNode(IMFMediaType* encodedVideoType,
                    IMFTopologyNode **encoderNode, ULONG_PTR deviceManagerPtr,
                    UINT temporalLayersCount);
    HRESULT createCustomSinkNode(DWORD streamNumber, LPCWSTR fileName);

    HRESULT intPlay();
    void setState(State state);
    void releaseResources();
    State mState;
    RendererType mRendererType;
    HWND mVideoRenderWindow;
    CComMultiThreadModel::AutoCriticalSection mStateLock;

    PlaybackStateSubscriber* stateSubscriber;

    HANDLE mTranscodeEndEvent;
    CComPtr<IMFMediaSession> mMediaSession;
    ULONG mRefCount;
    msdk_CMftBuilder *mMftBuilderObjPtr; /**< Pointer for MFT Builder utility class*/
    CComPtr<IMFTopology> mPartialTopology; /**< pointer to the partial topology */
    CComPtr<IMFMediaSource> mMediaSource; /**< media source pointer */
    CComPtr<IMFTransform> mSvcSplitterMft; /**<SVC splitter MFT */
    CComPtr<IMFPresentationDescriptor> mSourcePresentationDescriptor; /**< Input source descriptor */
    CComPtr<IMFActivate> mMediaSinkActivate; /**< Pointer to the sink */
    CComPtr<IMFStreamDescriptor> mSourceStreamDescriptor; /**< Input stream descriptor */
    CComPtr<IMFDXGIDeviceManager> mDxgiDeviceManager; /**< Device manager pointer */
    CComPtr<IMFTopology> mTopology; /**< Pointer to the topology */
    CComPtr<IMFTransform> mVqTransform; /**< Pointer to the video quality MFT */
    CComPtr<IMFTopologyNode> mSourceNode; /**< Source node*/
    CComPtr<IMFTopologyNode> mDecoderNode; /**< Decoder node*/
    CComPtr<IMFTopologyNode> mH264DecoderNode; /**< H264 Decoder node*/
    CComPtr<IMFTopologyNode> mSvcEncodeNode; /**<SvcEncodeNode */
    CComPtr<IMFTopologyNode> mSvcSplitterNode; /**<SVC splitter node */
    CComPtr<IMFTopologyNode> mCustomStreamSinkNode; /**<Custom sink node */
    CComPtr<IMFTopologyNode> mEvrStreamSinkNode; /**<EVR sink node */
    CComPtr<IMFTopologyNode> mTeeNode; /**< Tee node*/
    CComPtr<IMFTopologyNode> mSinkNode; /**< sink node*/
    uint32 mSvcCurrentLayersNumber; /**< Temporal SVC layers*/
};

/**
 *   @class PlaybackStateSubscriber. Defines an interface for TranscodeSession state updates subscribers.
 */
struct PlaybackStateSubscriber
{
    /**
     *   @brief onStateChange().
     */
    virtual void onStateChange(SvcTranscodeSession::State newState) = 0;
};
#endif//_PLAYBACKVQSESSION_H_
