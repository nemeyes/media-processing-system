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
 * @file <CustomSinkActivate.cpp>
 *
 * @brief This file contains implementation of CustomSinkActivate class
 *
 ********************************************************************************
 */

#include "CustomMediaSink.h"
#include "CustomMediaSinkGUIDs.h"

#include "CustomSinkActivate.h"

/**
 *******************************************************************************
 *  @fn    createInstance
 *  @brief creating Custom sink activate instance
 *
 *  @param[in]  fileName    : sink file name
 *  @param[out] ppActivate  : Enables the application to defer the creation of
 *                            an object
 *
 *  @return HRESULT : S_OK if successful; else returns microsoft error codes.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::createInstance(LPCWSTR fileName,
                IMFActivate** ppActivate)
{
    if (ppActivate == NULL)
    {
        return E_POINTER;
    }

    CustomSinkActivate* pActivate = new CustomSinkActivate();
    if (pActivate == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = S_OK;

    hr = pActivate->initialize();
    RETURNIFFAILED(hr);

    hr = pActivate->QueryInterface(IID_PPV_ARGS(ppActivate));
    RETURNIFFAILED(hr);

    pActivate->mSFileName = std::wstring(fileName);

    return hr;
}

/******************************************************************************
 * IUnknown                                                                   *
 ******************************************************************************/
/**
 *******************************************************************************
 *  @fn    AddRef
 *  @brief Increment reference count of the object.
 *
 *  @return ULONG : New reference count
 *******************************************************************************
 */
ULONG CustomSinkActivate::AddRef(void)
{
    return InterlockedIncrement(&mlRefCount);
}

/**
 *******************************************************************************
 *  @fn    QueryInterface
 *  @brief Get the interface specified by the riid from the class
 *
 *  @param[in]  riid : The identifier of the interface being requested.
 *  @param[out] ppv  : The address of a pointer variable that receives the
 *                     interface pointer requested in the riid parameter
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::QueryInterface(REFIID iid,
                __RPC__deref_out _Result_nullonfailure_ void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFActivate*>(this));
    }
    else if (iid == __uuidof(IMFActivate))
    {
        *ppv = static_cast<IMFActivate*>(this);
    }
    else if (iid == __uuidof(IPersistStream))
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if (iid == __uuidof(IPersist))
    {
        *ppv = static_cast<IPersist*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

/**
 *******************************************************************************
 *  @fn    Release
 *  @brief Decrement the reference count of the object.
 *
 *  @return ULONG : New reference count
 *******************************************************************************
 */
ULONG CustomSinkActivate::Release(void)
{
    ULONG lRefCount = InterlockedDecrement(&mlRefCount);
    if (lRefCount == 0)
    {
        delete this;
    }
    return lRefCount;
}

/******************************************************************************
 * IMFActivate                                                                *
 ******************************************************************************/

/**
 *******************************************************************************
 *  @fn    ActivateObject
 *  @brief activating the custom sink activate object
 *
 *  @param[in]  riid       : The identifier of the interface being requested.
 *  @param[out] ppvObject  : The address of a pointer variable that receives the
 *                           interface pointer requested in the riid parameter
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::ActivateObject(__RPC__in REFIID riid,
                __RPC__deref_out_opt void** ppvObject)
{
    HRESULT hr = S_OK;

    if (mPtrMediaSink == NULL)
    {
        hr = CustomMediaSink::createInstance(IID_PPV_ARGS(&mPtrMediaSink),
                        mSFileName.c_str());
        RETURNIFFAILED(hr);
    }

    hr = mPtrMediaSink->QueryInterface(riid, ppvObject);
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    DetachObject
 *  @brief Detaching object from the custom sink activate object
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::DetachObject(void)
{
    mPtrMediaSink.Release();

    return S_OK;
}

/**
 *******************************************************************************
 *  @fn    ShutdownObject
 *  @brief shutting down the object from the custom sink activate object
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::ShutdownObject(void)
{
    if (mPtrMediaSink != NULL)
    {
        mPtrMediaSink->Shutdown();
        mPtrMediaSink.Release();

    }

    return S_OK;
}

/******************************************************************************
 * IPersistStream                                                             *
 ******************************************************************************/

/**
 *******************************************************************************
 *  @fn    GetSizeMax
 *  @brief
 *
 *  @param[in]  pcbSize       :
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::GetSizeMax(__RPC__out ULARGE_INTEGER* pcbSize)
{
    return E_NOTIMPL;
}

/**
 *******************************************************************************
 *  @fn    IsDirty
 *  @brief
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::IsDirty(void)
{
    return E_NOTIMPL;
}

/**
 *******************************************************************************
 *  @fn    Load
 *  @brief
 *
 *  @param[in]  pStream       :
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::Load(__RPC__in_opt IStream* pStream)
{
    return E_NOTIMPL;
}

/**
 *******************************************************************************
 *  @fn    Save
 *  @brief
 *
 *  @param[in]  pStream       :
 *  @param[in]  bClearDirty   :
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::Save(__RPC__in_opt IStream* pStream,
                BOOL bClearDirty)
{
    return E_NOTIMPL;
}

/**
 *******************************************************************************
 *  @fn    GetClassID
 *  @brief Get the class id of custom sink object
 *
 *  @param[in]  pClassID : Class id pointer
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSinkActivate::GetClassID(__RPC__out CLSID* pClassID)
{
    if (pClassID == NULL)
    {
        return E_POINTER;
    }

    *pClassID = CLSID_CustomMediaSink;
    return S_OK;
}

/**
 *******************************************************************************
 *  @fn    CustomSinkActivate
 *  @brief Constructor
 *
 *******************************************************************************
 */
CustomSinkActivate::CustomSinkActivate(void) :
    mlRefCount(0)
{
}

/**
 *******************************************************************************
 *  @fn    ~CustomSinkActivate
 *  @brief Destructor
 *
 *******************************************************************************
 */
CustomSinkActivate::~CustomSinkActivate(void)
{
    mPtrMediaSink.Release();
}
