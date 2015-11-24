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
 * @brief This file defines class necessary for activating sink
 *         
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include "CustomMediaSink.h"
#include "CustomMediaSinkGUIDs.h"
#include "CustomSinkActivate.h"
/** 
 *******************************************************************************
 *  @fn     createInstance
 *  @brief  Creates activate object
 *           
 *  @param[in] fileName     : Pointer to the output file name to be written
 *  @param[out] ppActivate  : Pointer to the IMFActivate interface
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::createInstance(LPCWSTR fileName, IMFActivate** ppActivate)
{
    if (ppActivate == NULL)
    {
        return E_POINTER;
    }

    CActivate* pActivate = new CActivate();
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
/** 
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  IUnknown methods
 *          
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CActivate::AddRef(void)
{
    return InterlockedIncrement(&mLRefCount);
}
/** 
 *******************************************************************************
 *  @fn     QueryInterface
 *  @brief  IUnkown Methods
 *
 *  @param[in] iid :  Interface id
 *  @param[in] ppv : Pointer to the interface requested,
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv)
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
 *  @fn     Release
 *  @brief  IUnknown methods
 *          
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CActivate::Release(void)
{
    ULONG lRefCount = InterlockedDecrement(&mLRefCount);
    if (lRefCount == 0)
    {
        delete this;
    }
    return lRefCount;
}
/** 
 *******************************************************************************
 *  @fn     ActivateObject
 *  @brief  IMFActivate Methods.Creates the object associated with this 
 *          activation object.
 *
 *  @param[in] riid       :  Interface identifier
 *  @param[out] ppvObject : Pointer to the interface requested,
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::ActivateObject(__RPC__in REFIID riid,
                __RPC__deref_out_opt void** ppvObject)
{
    HRESULT hr = S_OK;

    if (mPtrMediaSink == NULL)
    {
        hr = CustomMediaSink::createInstance(IID_PPV_ARGS(&mPtrMediaSink), mSFileName.c_str());
        RETURNIFFAILED(hr);
    }

    hr = mPtrMediaSink->QueryInterface(riid, ppvObject);
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     DetachObject
 *  @brief  Detaches the created object from the activation object
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::DetachObject(void)
{
    mPtrMediaSink.Release();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     ShutdownObject
 *  @brief  Shuts down the created object.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::ShutdownObject(void)
{
    if (mPtrMediaSink != NULL)
    {
        mPtrMediaSink->Shutdown();
        mPtrMediaSink.Release();

    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetSizeMax
 *  @brief  IPersistStream Methods.Retrieves the size of the stream needed to
 *          save the object.
 *
 *  @param[out] pcbSize :  The size in bytes of the stream needed to save this 
 *                        object,
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::GetSizeMax(__RPC__out ULARGE_INTEGER* /*pcbSize*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     GetSizeMax
 *  @brief  IPersistStream Methods.Retrieves the size of the stream needed to
 *          save the object.
 *
 *  @param[out] pcbSize :  The size in bytes of the stream needed to save this 
 *                        object,
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::IsDirty(void)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     GetSizeMax
 *  @brief  IPersistStream Methods.Retrieves the size of the stream needed to
 *          save the object.
 *
 *  @param[out] pcbSize :  The size in bytes of the stream needed to save this 
 *                        object,
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::Load(__RPC__in_opt IStream* /*pStream*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     GetSizeMax
 *  @brief  IPersistStream Methods.Retrieves the size of the stream needed to
 *          save the object.
 *
 *  @param[out] pcbSize :  The size in bytes of the stream needed to save this 
 *                        object,
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::Save(__RPC__in_opt IStream* /*pStream*/, BOOL /*bClearDirty*/)
{
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     GetSizeMax
 *  @brief  IPersistStream Methods.Retrieves the size of the stream needed to
 *          save the object.
 *
 *  @param[out] pcbSize :  The size in bytes of the stream needed to save this 
 *                        object,
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CActivate::GetClassID(__RPC__out CLSID* pClassID)
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
 *  @fn     CActivate
 *  @brief  CActivate .Constructor
 *
 *          
 *******************************************************************************
 */
CActivate::CActivate(void) :
    mLRefCount(0)
{
}
/** 
 *******************************************************************************
 *  @fn     CActivate
 *  @brief  CActivate .Destructor
 *
 *          
 *******************************************************************************
 */
CActivate::~CActivate(void)
{
    mPtrMediaSink.Release();
}
