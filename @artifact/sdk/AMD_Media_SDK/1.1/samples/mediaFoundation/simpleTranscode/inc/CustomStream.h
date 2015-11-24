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
 * @file <CustomStream.h>
 *
 * @brief This file contains declaration of CustomStream class
 *
 ********************************************************************************
 */
#pragma once

#include <atlbase.h>
#include <atlcoll.h>

#include <Mferror.h>
#include <mfapi.h>
#include <mfidl.h>
#include "Common.h"

#define SAMPLE_BUFFER_SIZE 2

class CustomSource;

/**
 * Custom stream class.
 */
class CustomStream: public IMFMediaStream
{
public:
    static HRESULT createInstance(CustomStream** ppMediaStream,
                    CustomSource *pMediaSource,
                    IMFStreamDescriptor *pStreamDescriptor);

    /**
     * @brief IMFMediaEventGenerator interface implementation
     */
    STDMETHODIMP
                    BeginGetEvent(IMFAsyncCallback* pCallback,
                                    IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType,
                    HRESULT hrStatus, const PROPVARIANT* pvValue);

    /**
     * @brief IMFMediaStream interface implementation
     */
    STDMETHODIMP GetMediaSource(IMFMediaSource** ppMediaSource);
    STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor);
    STDMETHODIMP RequestSample(IUnknown* pToken);

    /**
     * @brief IUnknown interface implementation
     */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                    void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    /**
     * @brief Helper functions used by CustomStream
     */
    HRESULT deliverSample(IMFSample *pSample);
    void activate(bool active);
    HRESULT start(const PROPVARIANT& varStart, bool isSeek);
    HRESULT pause(void);
    HRESULT stop(void);
    HRESULT endOfStream();
    bool isActive(void) const
    {
        return mActive;
    }
    HRESULT shutdown();
    bool needsData(void);

private:
    CustomStream(void);
    HRESULT init(CustomSource *pMediaSource,
                    IMFStreamDescriptor *pStreamDescriptor);
    HRESULT checkShutdown(void);
    HRESULT dispatchSamples(void);
    HRESULT sendSamplesOut(void);
    ~CustomStream(void);

private:
    volatile long mCRef;
    CustomSource* mPMediaSource;
    bool mActive;
    bool mEndOfStream;
    SourceState mState;

    volatile int mNSamplesRequested;

    CComAutoCriticalSection mCritSec; /* critical section */

    CComPtr<IMFStreamDescriptor> mPStreamDescriptor;
    CComPtr<IMFMediaEventQueue> mPEventQueue;
    CInterfaceList<IMFSample> mPSampleList;
    CInterfaceList<IUnknown, &IID_IUnknown> mPTokenList;
};
