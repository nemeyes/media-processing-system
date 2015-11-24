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
 * @file <CustomSource.h>
 *
 * @brief This file contains declaration of CustomSource class
 *
 ********************************************************************************
 */
#pragma once
#include <atlbase.h>
#include <Mfobjects.h>
#include <mfidl.h>

#include "VideoInput.h"
#include "SourceOperation.h"
#include "CustomStream.h"

#include "Common.h"

#include <vector>
using namespace std;

class CustomStream;

/**
 * Custom source class.
 */
class CustomSource: public IMFMediaSource, public IMFAsyncCallback
{
public:
    static HRESULT createInstance(CustomSource **ppAVFSource);

    /**
     * @brief IUnknown interface implementation
     */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                    void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

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
     * @brief IMFMediaSource interface implementation
     */
    STDMETHODIMP CreatePresentationDescriptor(
                    IMFPresentationDescriptor** ppPresDescriptor);
    STDMETHODIMP GetCharacteristics(DWORD* pdwCharacteristics);
    STDMETHODIMP Start(IMFPresentationDescriptor* pPresentationDescriptor,
                    const GUID* pguidTimeFormat,
                    const PROPVARIANT* pvarStartPosition);
    STDMETHODIMP Stop(void);
    STDMETHODIMP Pause(void);
    STDMETHODIMP Shutdown(void);

    /**
     * @brief IMFAsyncCallback interface implementation
     */
    STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue);
    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult);

    /**
     * @brief Helper methods called by the bytestream handler.
     */
    HRESULT BeginOpen(LPCWSTR pwszURL, IMFAsyncCallback *pCallback,
                    IUnknown *pUnkState);
    HRESULT EndOpen(IMFAsyncResult *pResult);

    HRESULT checkShutdown(void) const; /* Check if Source is shutdown */
    HRESULT isInitialized(void) const; /* Check if Source is initialized */

    DWORD mWidth;
    DWORD mHeight;
private:
    CustomSource(HRESULT* pHr);

    /**
     * @brief file handling methods used to parse the file and initialize
     * the objects
     */
    HRESULT parseHeader(void);
    HRESULT internalCreatePresentationDescriptor(void);
    HRESULT createVideoStream(IMFStreamDescriptor** pStreamDescriptor);

    HRESULT sendOperation(SourceOperationType operationType);

    /**
     * @brief internal asynchronous event handler methods
     */
    HRESULT internalOpen(ISourceOperation* pCommand);
    HRESULT internalStart(ISourceOperation* pCommand);
    HRESULT internalStop(void);
    HRESULT internalPause(void);
    HRESULT internalRequestSample(void);
    HRESULT internalEndOfStream(void);

    HRESULT sendSampleToStream(CustomStream* pStream);

    HRESULT selectStreams(IMFPresentationDescriptor *pPresentationDescriptor,
                    const PROPVARIANT varStart, bool isSeek);

    ~CustomSource(void);

    friend class CustomStream;

private:
    volatile long mCRef; /* reference count */

    size_t mPendingEndOfStream;
    VideoInput* mPInputFileParser;
    CComAutoCriticalSection mCritSec; /* critical section */

    CComPtr<IMFMediaEventQueue> mPEventQueue;
    CComPtr<IMFPresentationDescriptor> mPPresentationDescriptor;

    /* an STL vector with media stream pointers */
    vector<CustomStream*> mMediaStreams;

    /* current state of the source */
    SourceState mState;
};
