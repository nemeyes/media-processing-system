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
 * @file <CustomStreamSink.h>                          
 *                                       
 * @brief This file defines class necessary for Stream Sink
 *         
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#ifndef CUSTOMSTREAMSINK_H_
#define CUSTOMSTREAMSINK_H_

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <evr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <atlbase.h>
#include <initguid.h>
#include <cguid.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include <Mferror.h>
#include "CustomSinklinklist.h"

class CustomMediaSink;

/**
 * IMarker
 * IMarker is not a standard Media Foundation interface.
 */
MIDL_INTERFACE("3AC82233-933C-43a9-AF3D-ADC94EABF406")
IMarker : public IUnknown
{
    virtual STDMETHODIMP GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType) = 0;
    virtual STDMETHODIMP GetMarkerValue(PROPVARIANT *pvar) = 0;
    virtual STDMETHODIMP GetContext(PROPVARIANT *pvar) = 0;
};

/**
 * AsyncCallback
 * Helper class that routes IMFAsyncCallback::Invoke calls to a class
 * method on the parent class.
 * 
 */
template<class T>
class AsyncCallback: public IMFAsyncCallback
{
public:
    typedef HRESULT (T::*InvokeFn)(IMFAsyncResult *pAsyncResult);

    AsyncCallback(T *pParent, InvokeFn fn) :
        mPParent(pParent), mPInvokeFn(fn)
    {
    }

    /***************************************************************************
     * IUnknown                                                                 *
     ***************************************************************************/
    STDMETHODIMP_(ULONG) AddRef()
    {
        return mPParent->AddRef();
    }

    STDMETHODIMP_(ULONG) Release()
    {
        return mPParent->Release();
    }

    STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
    {
        if (!ppv)
        {
            return E_POINTER;
        }
        if (iid == __uuidof(IUnknown))
        {
            *ppv
                            = static_cast<IUnknown*> (static_cast<IMFAsyncCallback*> (this));
        }
        else if (iid == __uuidof(IMFAsyncCallback))
        {
            *ppv = static_cast<IMFAsyncCallback*> (this);
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    /***************************************************************************
     * IMFAsyncCallback                                                         *
     ***************************************************************************/
    STDMETHODIMP GetParameters(DWORD*, DWORD*)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult)
    {
        return (mPParent->*mPInvokeFn)(pAsyncResult);
    }

    T *mPParent;
    InvokeFn mPInvokeFn;
};

/**
 * CustomStreamSink
 * This class describe interface of custom stream sink.
 */
class CustomStreamSink: public IMFStreamSink, public IMFMediaTypeHandler
{
    friend class CustomMediaSink;

public:
    /***************************************************************************
     * IUnknown                                                                 *
     ***************************************************************************/
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    /***************************************************************************
     * IMFMediaEventGenerator                                                   *
     ***************************************************************************/
    STDMETHODIMP
    BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType,
                    HRESULT hrStatus, const PROPVARIANT* pvValue);

    /***************************************************************************
     * IMFStreamSink                                                            *
     ***************************************************************************/
    STDMETHODIMP GetMediaSink(IMFMediaSink **ppMediaSink);
    STDMETHODIMP GetIdentifier(DWORD *pdwIdentifier);
    STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler **ppHandler);
    STDMETHODIMP ProcessSample(IMFSample *pSample);

    STDMETHODIMP PlaceMarker(
    /* [in] */MFSTREAMSINK_MARKER_TYPE eMarkerType,
    /* [in] */const PROPVARIANT *pvarMarkerValue,
    /* [in] */const PROPVARIANT *pvarContextValue);

    STDMETHODIMP Flush();

    /***************************************************************************
     * IMFMediaTypeHandler                                                      *
     ***************************************************************************/
    STDMETHODIMP IsMediaTypeSupported(IMFMediaType *pMediaType,
                    IMFMediaType **ppMediaType);
    STDMETHODIMP GetMediaTypeCount(DWORD *pdwTypeCount);
    STDMETHODIMP GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType **ppType);
    STDMETHODIMP SetCurrentMediaType(IMFMediaType *pMediaType);
    STDMETHODIMP GetCurrentMediaType(IMFMediaType **ppMediaType);
    STDMETHODIMP GetMajorType(GUID *pguidMajorType);

private:
    /***************************************************************************
     * Defines the current state of the stream.                                 *
     ***************************************************************************/
    enum State
    {
        StateTypeNotSet = 0, /**< No media type is set */
        StateReady, /**< Media type is set, start has never been called.*/
        StateStarted, StateStopped, StatePaused,
    };

    /***************************************************************************
     * Defines various operations that can be performed on the stream.          *
     ***************************************************************************/
    enum StreamOperation
    {
        OpSetMediaType = 0,
        OpStart,
        OpRestart,
        OpPause,
        OpStop,
        OpProcessSample,
        OpPlaceMarker,
    };

    enum FlushState
    {
        DropSamples = 0, WriteSamples
    };

    /** 
     * CAsyncOperation:
     * Used to queue asynchronous operations. When we call MFPutWorkItem, we use this
     * object for the callback state (pState). Then, when the callback is invoked,
     * we can use the object to determine which asynchronous operation to perform.
     */
    class CAsyncOperation: public IUnknown
    {
    public:
        CAsyncOperation(StreamOperation op);

        StreamOperation mOp; /**< The operation to perform. */

        STDMETHODIMP QueryInterface(REFIID iid, void** ppv);STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    private:
        long mNRefCount;
        virtual ~CAsyncOperation();
    };

private:

    CustomStreamSink();
    virtual ~CustomStreamSink();

    HRESULT initialize(CustomMediaSink *pParent, LPCWSTR fileName);

    HRESULT checkShutdown() const
    {
        if (mIsShutdown)
        {
            return MF_E_SHUTDOWN;
        }
        else
        {
            return S_OK;
        }
    }

    HRESULT start(MFTIME start);
    HRESULT restart();
    HRESULT stop();
    HRESULT pause();
    HRESULT shutdown();

    HRESULT queueAsyncOperation(StreamOperation op);
    HRESULT onDispatchWorkItem(IMFAsyncResult* pAsyncResult);

    HRESULT processSamplesFromQueue(FlushState bFlushData);
    HRESULT writeSampleToFile(IMFSample *pSample);
    HRESULT sendMarkerEvent(IMarker *pMarker, FlushState bFlushData);

    long mNRefCount;
    State mState;
    BOOL mIsShutdown;
    MFTIME mStartTime; /**< Presentation time when the clock started. */
    DWORD mCbDataWritten; /**< How many bytes we have written so far. */
    CComPtr<IMFMediaType> mPtrCurrentType;

    CustomMediaSink* mPtrSink; /**< Parent media sink */

    std::string mSFileName;
    CComMultiThreadModel::AutoCriticalSection mStateLock;

    DWORD mWorkQueueId; /**< ID of the work queue for asynchronous operations. */
    AsyncCallback<CustomStreamSink> mWorkQueueCb; /**< Callback for the work queue. */
    CComPtr<IMFMediaEventQueue> mPtrEventQueue;
    ComPtrList<IUnknown> mSampleQueue;
};

/*
 * CMarker
 * Holds marker information for IMFStreamSink::PlaceMarker
 */
class CMarker: public IMarker
{
public:
    static HRESULT create(MFSTREAMSINK_MARKER_TYPE eMarkerType,
                    const PROPVARIANT* pvarMarkerValue,
                    const PROPVARIANT* pvarContextValue, IMarker **ppMarker);

    /********
     * Unknown
     *********/
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType);
    STDMETHODIMP GetMarkerValue(PROPVARIANT *pvar);
    STDMETHODIMP GetContext(PROPVARIANT *pvar);

    virtual ~CMarker();

protected:
    MFSTREAMSINK_MARKER_TYPE mEMarkerType;
    PROPVARIANT mVarMarkerValue;
    PROPVARIANT mVarContextValue;

private:
    long mNRefCount;

    CMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType);
};
#endif //CUSTOMSTREAMSINK_H_
