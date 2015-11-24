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
 * @file <AsyncState.h>
 *
 * @brief This file contains declaration of AsyncState class
 *
 ********************************************************************************
 */
#ifndef ASYNCSTATE_H
#define ASYNCSTATE_H

#include "unknwn.h"

#include <atlbase.h>
#include <Mfidl.h>
#include <Mferror.h>
#include <InitGuid.h>

enum AsyncEventType
{
    AsyncEventType_SourceEvent,
    AsyncEventType_SourceStreamEvent,
    AsyncEventType_StreamSinkEvent,
    AsyncEventType_ByteStreamHandlerEvent,
    AsyncEventType_SyncMftSampleRequest
};

// {88ACF5E6-2ED1-4780-87B1-D71814C2D42A}
DEFINE_GUID(IID_IAsyncState, 0x88acf5e6, 0x2ed1, 0x4780, 0x87, 0xb1, 0xd7, 0x18, 0x14, 0xc2, 0xd4, 0x2a);

// Microsoft-specific extension necessary to support the __uuidof(IAsyncState) notation.
class __declspec(uuid("88ACF5E6-2ED1-4780-87B1-D71814C2D42A")) IAsyncState;

class IAsyncState: public IUnknown
{
public:
    virtual AsyncEventType EventType(void) = 0;
};

class CAsyncState: public IAsyncState
{
public:
    CAsyncState(AsyncEventType type);
    ~CAsyncState(void)
    {
    }
    ;
    /**
     * @brief IUnknown interface implementation
     */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    virtual AsyncEventType EventType(void)
    {
        return mEventType;
    }

private:
    volatile long mCRef; /* reference count */

    AsyncEventType mEventType;
};
#endif
