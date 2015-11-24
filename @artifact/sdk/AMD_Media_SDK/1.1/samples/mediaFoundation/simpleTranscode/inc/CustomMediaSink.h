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
 * @file <CustomMediaSink.h>
 *
 * @brief This file contains declaration of CustomMediaSink class
 *        which describe interface
 ********************************************************************************
 */
#pragma once

#include <string>
#include <atlbase.h>
#include <Mferror.h>
#include "MftUtils.h"

#include "CustomStreamSink.h"

/**
 * CustomMediaSink
 * This class describe interface of extension.
 */
class CustomMediaSink: public IMFMediaSink, public IMFClockStateSink
{
public:
    static HRESULT
                    createInstance(REFIID iid, void **ppSource,
                                    LPCWSTR fileName);

    /**
     * @brief IUnknown interface implementation
     */
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    /**
     * @brief IMFMediaSink interface implementation
     */
    STDMETHODIMP GetCharacteristics(DWORD *pdwCharacteristics);

    STDMETHODIMP AddStreamSink(DWORD dwStreamSinkIdentifier,
                    IMFMediaType *pMediaType, IMFStreamSink **ppStreamSink);

    STDMETHODIMP RemoveStreamSink(DWORD dwStreamSinkIdentifier);
    STDMETHODIMP GetStreamSinkCount(DWORD *pcStreamSinkCount);
    STDMETHODIMP GetStreamSinkByIndex(DWORD dwIndex,
                    IMFStreamSink **ppStreamSink);
    STDMETHODIMP GetStreamSinkById(DWORD dwIdentifier,
                    IMFStreamSink **ppStreamSink);
    STDMETHODIMP SetPresentationClock(IMFPresentationClock *pPresentationClock);
    STDMETHODIMP GetPresentationClock(
                    IMFPresentationClock **ppPresentationClock);
    STDMETHODIMP Shutdown();

    /**
     * @brief IMFClockStateSink interface implementation
     */
    STDMETHODIMP
                    OnClockStart(MFTIME hnsSystemTime,
                                    LONGLONG llClockStartOffset);
    STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);

    ~CustomMediaSink();

private:
    HRESULT initialize(LPCWSTR fileName);

private:

    CustomMediaSink();

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

    CustomStreamSink mStreamSink;

    CComMultiThreadModel::AutoCriticalSection mStateLock;
    BOOL mIsShutdown; /**< Flag to indicate Shutdown() */
    CComPtr<IMFPresentationClock> mPtrClock;
    LONG mRefCount;
    std::wstring mSFileName;
};
