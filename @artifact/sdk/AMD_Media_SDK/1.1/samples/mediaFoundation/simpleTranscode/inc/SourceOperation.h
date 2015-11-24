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
 * @file <SourceOperation.h>
 *
 * @brief Source Operation implementation
 *
 ********************************************************************************
 */
#pragma once

#include <atlbase.h>
#include <Mfidl.h>
#include <Mferror.h>

class PropVariantGeneric: public PROPVARIANT
{
public:
    PropVariantGeneric()
    {
        PropVariantInit(this);
    }

    ~PropVariantGeneric()
    {
        PropVariantClear(this);
    }
};

/**
 * Operation type.
 */
enum SourceOperationType
{
    SourceOperationOpen,
    SourceOperationStart,
    SourceOperationPause,
    SourceOperationStop,
    SourceOperationStreamNeedData,
    SourceOperationEndOfStream
};

/**
 * ISourceOperation COM IID.
 * {35D8883D-3239-4ABE-84BD-43EAC5ED2304}
 */
DEFINE_GUID(IID_ISourceOperation, 0x35d8883d, 0x3239, 0x4abe, 0x84, 0xbd, 0x43,
                0xea, 0xc5, 0xed, 0x23, 0x4);

/**
 * ISourceOperation COM interface
 */
struct ISourceOperation: public IUnknown
{
public:
    virtual HRESULT GetPresentationDescriptor(
                    IMFPresentationDescriptor** ppPresentationDescriptor) = 0;
    virtual HRESULT SetData(const PROPVARIANT& data, bool isSeek) = 0;
    virtual PROPVARIANT& GetData() = 0;
    virtual bool IsSeek(void) = 0;
    virtual SourceOperationType Type(void) = 0;
    virtual WCHAR* GetUrl(void) = 0;
    virtual HRESULT GetCallerAsyncResult(IMFAsyncResult** pCallerResult)= 0;
};

/**
 * COM object used to pass commands between threads.
 */
class SourceOperation: public ISourceOperation
{
public:
    SourceOperation(SourceOperationType operation);
    SourceOperation(SourceOperationType operation, LPCWSTR pUrl,
                    IMFAsyncResult* pCallerResult);
    SourceOperation(SourceOperationType operation,
                    IMFPresentationDescriptor* pPresentationDescriptor);
    SourceOperation(const SourceOperation& operation);

    /**
     * @brief ISourceOperation interface implementation
     */
    virtual HRESULT GetPresentationDescriptor(
                    IMFPresentationDescriptor** ppPresentationDescriptor);
    virtual PROPVARIANT& GetData();
    virtual bool IsSeek(void)
    {
        return mIsSeek;
    }
    virtual SourceOperationType Type(void)
    {
        return mOperationType;
    }
    virtual WCHAR* GetUrl(void)
    {
        return mPUrl;
    }
    ;
    virtual HRESULT GetCallerAsyncResult(IMFAsyncResult** ppCallerResult);

    /**
     * IUnknown interface implementation
     */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                    void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    HRESULT SetData(const PROPVARIANT& data, bool isSeek);

private:
    ~SourceOperation();
    void init(SourceOperationType operation,
                    IMFPresentationDescriptor* pPresentationDescriptor);

    volatile long mCRef; /* reference count */
    bool mIsSeek;
    SourceOperationType mOperationType;
    PropVariantGeneric mData;
    CComPtr<IMFPresentationDescriptor> mPPresentationDescriptor;

    /* variables used during BeginOpen operation - URL of file to open and
     * client's result that will be invoked when open is complete */
    CComPtr<IMFAsyncResult> mPCallerResult;
    WCHAR* mPUrl;
};

