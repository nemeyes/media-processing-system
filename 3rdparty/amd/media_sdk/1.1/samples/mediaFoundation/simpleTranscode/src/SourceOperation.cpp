/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1 Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.
 2 Redistributions in binary form must reproduce the above copyright notice, 
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
 * @brief This file contains implementation of SourceOperation class
 *         
 ********************************************************************************
 */
#include "SourceOperation.h"

/** 
 *******************************************************************************
 *  @fn     SourceOperation
 *  @brief  Constructor
 *           
 *  @param[in]  operation : source operation
 *          
 *  @return 
 *******************************************************************************
 */
SourceOperation::SourceOperation(SourceOperationType operation) :
    mCRef(0), mPUrl(NULL)
{
    init(operation, NULL);
}

/** 
 *******************************************************************************
 *  @fn     SourceOperation
 *  @brief  Constructor
 *           
 *  @param[in] operation : source operation
 *  @param[in] pUrl : URL style path
 *  @param[in] pCallerResult : pointer to the Caller result
 *          
 *  @return 
 *******************************************************************************
 */
SourceOperation::SourceOperation(SourceOperationType operation, LPCWSTR pUrl,
                IMFAsyncResult* pCallerResult) :
    mCRef(0), mPUrl(NULL)

{
    if (pUrl == NULL)
        return;

    mPUrl = new WCHAR[wcslen(pUrl) + 1];
    if (mPUrl == NULL)
        return;

    mOperationType = operation;

    wcscpy_s(mPUrl, wcslen(pUrl) + 1, pUrl);

    mPCallerResult = pCallerResult;
}

/** 
 *******************************************************************************
 *  @fn     SourceOperation
 *  @brief  Constructor
 *           
 *  @param[in] operation : Source operation
 *  @param[in] pPresentationDescriptor : pointer to the presentation descriptor
 *          
 *  @return 
 *******************************************************************************
 */
SourceOperation::SourceOperation(SourceOperationType operation,
                IMFPresentationDescriptor* pPresentationDescriptor) :
    mCRef(0), mPUrl(NULL)
{
    init(operation, pPresentationDescriptor);
}

/** 
 *******************************************************************************
 *  @fn     SourceOperation
 *  @brief  Constructor
 *           
 *  @param[in]  operation : source operation
 *          
 *  @return 
 *******************************************************************************
 */
SourceOperation::SourceOperation(const SourceOperation& operation) :
    mCRef(0), mPUrl(NULL)
{
    init(operation.mOperationType, operation.mPPresentationDescriptor);
}

/** 
 *******************************************************************************
 *  @fn     init
 *  @brief  initializing Source operation
 *           
 *  @param[in] operation : source operation
 *  @param[in] pPresentationDescriptor : pointer to the presentation descriptor
 *          
 *  @return 
 *******************************************************************************
 */
void SourceOperation::init(SourceOperationType operation,
                IMFPresentationDescriptor* pPresentationDescriptor)
{
    mIsSeek = false;
    mOperationType = operation;
    if (pPresentationDescriptor)
    {
        mPPresentationDescriptor = pPresentationDescriptor;
    }
}

/** 
 *******************************************************************************
 *  @fn     GetPresentationDescriptor
 *  @brief  get the presentation descriptor
 *           
 *  @param[out] pPresentationDescriptor : pointer to the presentation descriptor
 *          
 *  @return HRESULT : S_OK if successful; else returns MF error codes
 *******************************************************************************
 */
HRESULT SourceOperation::GetPresentationDescriptor(
                IMFPresentationDescriptor** ppPresentationDescriptor)
{
    CComPtr < IMFPresentationDescriptor > spPresentationDescriptor
                    = mPPresentationDescriptor;
    if (spPresentationDescriptor)
    {
        *ppPresentationDescriptor = spPresentationDescriptor.Detach();
        return S_OK;
    }
    return MF_E_INVALIDREQUEST;
}

/** 
 *******************************************************************************
 *  @fn     SetData
 *  @brief  sets the data for source operation
 *           
 *  @param[in] data : Data to be set
 *  @param[in] isSeek : 
 *          
 *  @return 
 *******************************************************************************
 */
HRESULT SourceOperation::SetData(const PROPVARIANT& data, bool isSeek)
{
    mIsSeek = isSeek;
    return PropVariantCopy(&mData, &data);
}

/** 
 *******************************************************************************
 *  @fn     GetData
 *  @brief  gets the data ftom source operation
 *           
 *          
 *  @return mData : data
 *******************************************************************************
 */
PROPVARIANT& SourceOperation::GetData(void)
{
    return mData;
}

/** 
 *******************************************************************************
 *  @fn     GetCallerAsyncResult
 *  @brief  
 *           
 *  @param[out] ppCallerResult : pointer to the caller result
 *          
 *  @return HRESULT : S_OK if successful; else returns MF error codes
 *******************************************************************************
 */
HRESULT SourceOperation::GetCallerAsyncResult(IMFAsyncResult** ppCallerResult)
{
    if (ppCallerResult == NULL)
        return E_POINTER;

    *ppCallerResult = mPCallerResult;
    (*ppCallerResult)->AddRef();
    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     ~SourceOperation
 *  @brief  Destructor
 *           
 *  @return 
 *******************************************************************************
 */
SourceOperation::~SourceOperation(void)
{
    if (mPUrl != NULL)
        delete mPUrl;
}

/** 
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  Standard IUnknown interface implementation
 *           
 *  @return 
 *******************************************************************************
 */
ULONG SourceOperation::AddRef()
{
    return InterlockedIncrement(&mCRef);
}

/** 
 *******************************************************************************
 *  @fn     Release
 *  @brief  Standard IUnknown interface implementation
 *          
 *  @return refCount : reference count
 *******************************************************************************
 */
ULONG SourceOperation::Release()
{
    ULONG refCount = InterlockedDecrement(&mCRef);
    if (refCount == 0)
    {
        delete this;
    }

    return refCount;
}

/** 
 *******************************************************************************
 *  @fn     QueryInterface
 *  @brief  Standard IUnknown interface implementation
 *           
 *  @param[in]  riid : interface identifier
 *  @param[out] ppv : interface pointer requested in the riid
 *          
 *  @return HRESULT : S_OK if successful; else E_NONINTERFACE
 *******************************************************************************
 */
HRESULT SourceOperation::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv == NULL)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*> (this);
    }
    else if (riid == IID_ISourceOperation)
    {
        *ppv = static_cast<ISourceOperation*> (this);
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
        AddRef();

    return hr;
}
