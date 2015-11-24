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
 * @file <CustomSinkActivate.h>
 *
 * @brief This file contains declaration of CustomSinkActivate class
 ********************************************************************************
 */
#pragma once

#include <string>

#include <mfapi.h>
#include <assert.h>
#include <atlbase.h>
#include <mfidl.h>

/**
 * CustomSinkActivate
 * This class describe interface of activate object for custom sink.
 */
class CustomSinkActivate: public IMFActivate, public IPersistStream
{
public:
    static HRESULT createInstance(LPCWSTR fileName, IMFActivate** ppActivate);

    /**
     * @brief IUnknown interface implementation
     */
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP QueryInterface(REFIID riid,
                    __RPC__deref_out _Result_nullonfailure_ void** ppvObject);
    STDMETHODIMP_(ULONG) Release(void);

    /**
     * @brief IMFActivate interface implementation
     */
    STDMETHODIMP ActivateObject(__RPC__in REFIID riid,
                    __RPC__deref_out_opt void** ppvObject);
    STDMETHODIMP DetachObject(void);
    STDMETHODIMP ShutdownObject(void);

    /**
     * @brief IPersistStream interface implementation
     */
    STDMETHODIMP GetSizeMax(__RPC__out ULARGE_INTEGER* pcbSize);
    STDMETHODIMP IsDirty(void);STDMETHODIMP Load(__RPC__in_opt IStream* pStream);
    STDMETHODIMP Save(__RPC__in_opt IStream* pStream, BOOL bClearDirty);

    /**
     * @brief IPersist interface implementation
     */
    STDMETHODIMP GetClassID(__RPC__out CLSID* pClassID);

    /**
     * @brief IMFAttributes interface implementation
     */
    STDMETHODIMP GetItem(__RPC__in REFGUID guidKey,
                    __RPC__inout_opt PROPVARIANT* pValue)
    {
        return mPAttributes->GetItem(guidKey, pValue);
    }

    STDMETHODIMP GetItemType(__RPC__in REFGUID guidKey,
                    __RPC__out MF_ATTRIBUTE_TYPE* pType)
    {
        return mPAttributes->GetItemType(guidKey, pType);
    }

    STDMETHODIMP CompareItem(__RPC__in REFGUID guidKey,
                    __RPC__in REFPROPVARIANT Value, __RPC__out BOOL* pbResult)
    {
        return mPAttributes->CompareItem(guidKey, Value, pbResult);
    }

    STDMETHODIMP Compare(__RPC__in_opt IMFAttributes* pTheirs,
                    MF_ATTRIBUTES_MATCH_TYPE MatchType, __RPC__out BOOL* pbResult)
    {
        return mPAttributes->Compare(pTheirs, MatchType, pbResult);
    }

    STDMETHODIMP GetUINT32(__RPC__in REFGUID guidKey,
                    __RPC__out UINT32* punValue)
    {
        return mPAttributes->GetUINT32(guidKey, punValue);
    }

    STDMETHODIMP GetUINT64(__RPC__in REFGUID guidKey,
                    __RPC__out UINT64* punValue)
    {
        return mPAttributes->GetUINT64(guidKey, punValue);
    }

    STDMETHODIMP GetDouble(__RPC__in REFGUID guidKey,
                    __RPC__out double* pfValue)
    {
        return mPAttributes->GetDouble(guidKey, pfValue);
    }

    STDMETHODIMP GetGUID(__RPC__in REFGUID guidKey, __RPC__out GUID* pguidValue)
    {
        return mPAttributes->GetGUID(guidKey, pguidValue);
    }

    STDMETHODIMP GetStringLength(__RPC__in REFGUID guidKey,
                    __RPC__out UINT32* pcchLength)
    {
        return mPAttributes->GetStringLength(guidKey, pcchLength);
    }

    STDMETHODIMP GetString(__RPC__in REFGUID guidKey,
                    __RPC__out_ecount_full(cchBufSize) LPWSTR pwszValue,
                    UINT32 cchBufSize, __RPC__inout_opt UINT32* pcchLength)
    {
        return mPAttributes->GetString(guidKey, pwszValue, cchBufSize,
                        pcchLength);
    }

    STDMETHODIMP GetAllocatedString(__RPC__in REFGUID guidKey,
                    __RPC__deref_out_ecount_full_opt((*pcchLength + 1)) LPWSTR* ppwszValue,
                    __RPC__out UINT32* pcchLength)
    {
        return mPAttributes->GetAllocatedString(guidKey, ppwszValue,
                        pcchLength);
    }

    STDMETHODIMP GetBlobSize(__RPC__in REFGUID guidKey,
                    __RPC__out UINT32* pcbBlobSize)
    {
        return mPAttributes->GetBlobSize(guidKey, pcbBlobSize);
    }

    STDMETHODIMP GetBlob(__RPC__in REFGUID guidKey,
                    __RPC__out_ecount_full(cbBufSize) UINT8* pBuf, UINT32 cbBufSize,
                    __RPC__inout_opt UINT32* pcbBlobSize)
    {
        return mPAttributes->GetBlob(guidKey, pBuf, cbBufSize, pcbBlobSize);
    }

    STDMETHODIMP GetAllocatedBlob(__RPC__in REFGUID guidKey,
                    __RPC__deref_out_ecount_full_opt(*pcbSize) UINT8** ppBuf,
                    __RPC__out UINT32* pcbSize)
    {
        return mPAttributes->GetAllocatedBlob(guidKey, ppBuf, pcbSize);
    }

    STDMETHODIMP GetUnknown(__RPC__in REFGUID guidKey, __RPC__in REFIID riid,
                    __RPC__deref_out_opt LPVOID* ppv)
    {
        return mPAttributes->GetUnknown(guidKey, riid, ppv);
    }

    STDMETHODIMP SetItem(__RPC__in REFGUID guidKey,
                    __RPC__in REFPROPVARIANT Value)
    {
        return mPAttributes->SetItem(guidKey, Value);
    }

    STDMETHODIMP DeleteItem(__RPC__in REFGUID guidKey)
    {
        return mPAttributes->DeleteItem(guidKey);
    }

    STDMETHODIMP DeleteAllItems(void)
    {
        return mPAttributes->DeleteAllItems();
    }

    STDMETHODIMP SetUINT32(__RPC__in REFGUID guidKey, UINT32 unValue)
    {
        return mPAttributes->SetUINT32(guidKey, unValue);
    }

    STDMETHODIMP SetUINT64(__RPC__in REFGUID guidKey, UINT64 unValue)
    {

        return mPAttributes->SetUINT64(guidKey, unValue);
    }

    STDMETHODIMP SetDouble(__RPC__in REFGUID guidKey, double fValue)
    {

        return mPAttributes->SetDouble(guidKey, fValue);
    }

    STDMETHODIMP SetGUID(__RPC__in REFGUID guidKey, __RPC__in REFGUID guidValue)
    {

        return mPAttributes->SetGUID(guidKey, guidValue);
    }

    STDMETHODIMP SetString(__RPC__in REFGUID guidKey,
                    __RPC__in_string LPCWSTR wszValue)
    {

        return mPAttributes->SetString(guidKey, wszValue);
    }

    STDMETHODIMP SetBlob(__RPC__in REFGUID guidKey,
                    __RPC__in_ecount_full(cbBufSize) const UINT8* pBuf, UINT32 cbBufSize)
    {

        return mPAttributes->SetBlob(guidKey, pBuf, cbBufSize);
    }

    STDMETHODIMP SetUnknown(__RPC__in REFGUID guidKey,
                    __RPC__in_opt IUnknown* pUnknown)
    {

        return mPAttributes->SetUnknown(guidKey, pUnknown);
    }

    STDMETHODIMP LockStore(void)
    {

        return mPAttributes->LockStore();
    }

    STDMETHODIMP UnlockStore(void)
    {

        return mPAttributes->UnlockStore();
    }

STDMETHODIMP GetCount(__RPC__out UINT32* pcItems)
{

    return mPAttributes->GetCount(pcItems);
}

STDMETHODIMP GetItemByIndex(UINT32 unIndex, __RPC__out GUID* pguidKey,
                __RPC__inout_opt PROPVARIANT* pValue)
{

    return mPAttributes->GetItemByIndex(unIndex, pguidKey, pValue);
}

STDMETHODIMP CopyAllItems(__RPC__in_opt IMFAttributes* pDest)
{

    return mPAttributes->CopyAllItems(pDest);
}

private:
    CustomSinkActivate(void);
    ~CustomSinkActivate(void);

    /*
     * @brief initialize. Initializes the object by creating the standard
     *                    Media Foundation attribute store.
     */
    HRESULT initialize(UINT32 cInitialSize = 0)
    {
        if (mPAttributes == NULL)
        {
            return MFCreateAttributes(&mPAttributes, cInitialSize);
        }
        else
        {
            return S_OK;
        }
    }

    long mlRefCount;
    std::wstring mSFileName;

    CComPtr<IMFMediaSink> mPtrMediaSink;
    CComPtr<IMFAttributes> mPAttributes;
};
