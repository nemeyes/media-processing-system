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
 * @file <CustomByteStreamHandler.h>
 *
 * @brief This file contains declaration of AsyncState class
 *
 ********************************************************************************
 */
#ifndef CUSTOMBYTESTREAMHANDLER_H
#define CUSTOMBYTESTREAMHANDLER_H
#include "CustomSource.h"
#include "VideoInput.h"

#include <mfapi.h>

class CustomByteStreamHandler: public IMFByteStreamHandler,
                public IMFAsyncCallback,
                public IInitializeWithFile,
                public IPropertyStore
{
public:
    CustomByteStreamHandler(void);
    ~CustomByteStreamHandler(void);

    static HRESULT
                    createInstance(
                                    CustomByteStreamHandler **ppByteStreamHandler);

    /**
     * @brief IMFByteStreamHandler interface implementation
     */
    STDMETHODIMP BeginCreateObject(IMFByteStream* pByteStream, LPCWSTR pwszURL,
                    DWORD dwFlags, IPropertyStore* pProps,
                    IUnknown** ppIUnknownCancelCookie,
                    IMFAsyncCallback* pCallback, IUnknown *punkState);

    STDMETHODIMP EndCreateObject(IMFAsyncResult* pResult,
                    MF_OBJECT_TYPE* pObjectType, IUnknown** ppObject);

    STDMETHODIMP CancelObjectCreation(IUnknown* pIUnknownCancelCookie);
    STDMETHODIMP GetMaxNumberOfBytesRequiredForResolution(QWORD* pqwBytes);

    /**
     * @brief IMFAsyncCallback interface implementation
     */
    STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue);
    STDMETHODIMP Invoke(IMFAsyncResult* pResult);

    /**
     * @brief IUnknown interface implementation
     */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    /**
     * @brief IInitializeWithFile interface implementation
     */
    STDMETHODIMP Initialize(LPCWSTR pszFilePath, DWORD grfMode);

    /**
     * @brief IPropertyStore interface implementation
     */
    STDMETHODIMP Commit();
    STDMETHODIMP GetAt(DWORD iProp, PROPERTYKEY* pkey);
    STDMETHODIMP GetCount(DWORD* cProps);
    STDMETHODIMP GetValue(REFPROPERTYKEY key, PROPVARIANT* pv);
    STDMETHODIMP SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar);

    DWORD mWidth;
    DWORD mHeight;
private:
    volatile long mCRef; /* ref count */
    CComAutoCriticalSection mCritSec;

    CustomSource* mPH264FSource;
    CComPtr<IMFAsyncResult> mPResult;
    CComPtr<IPropertyStore> mPPropertyStore;

    // holds a value indicating that the creation is being canceled
    bool mObjectCreationCanceled;
};
#endif
