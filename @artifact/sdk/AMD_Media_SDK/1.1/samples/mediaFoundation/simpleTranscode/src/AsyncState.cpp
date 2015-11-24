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
 * @file <AsyncState.cpp>
 *
 * @brief Contains the implemeantation of CAsyncState class
 *
 ********************************************************************************
 */
#include "AsyncState.h"

/**
 *******************************************************************************
 *  @fn    CAsyncState
 *  @brief Constructor
 *
 *
 *******************************************************************************
 */
CAsyncState::CAsyncState(AsyncEventType type) :
    mCRef(0), mEventType(type)
{
}

/******************************************************************************/
/* Standard IUnknown interface implementation                                 */
/******************************************************************************/

/**
 *******************************************************************************
 *  @fn    AddRef
 *  @brief Increment reference count of the object.
 *
 *
 *******************************************************************************
 */
ULONG CAsyncState::AddRef()
{
    return InterlockedIncrement(&mCRef);
}

/**
 *******************************************************************************
 *  @fn    Release
 *  @brief Decrement the reference count of the object.
 *
 *
 *******************************************************************************
 */
ULONG CAsyncState::Release()
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
HRESULT CAsyncState::QueryInterface(REFIID riid, void** ppv)
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
    else if (riid == IID_IAsyncState)
    {
        *ppv = static_cast<IAsyncState*> (this);
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