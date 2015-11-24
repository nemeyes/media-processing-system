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
 * @file <CustomMediaSource.cpp>
 *
 * @brief This file contains implementation of CustomMediaSource class
 *
 ********************************************************************************
 */
#include "CustomMediaSource.h"
#include "AsyncState.h"

#include "Common.h"

/**
 *******************************************************************************
 *  @fn    CustomMediaSource
 *  @brief Constructor
 *
 *
 *******************************************************************************
 */
CustomMediaSource::CustomMediaSource(void) :
    _refCount(0)
{
    mSourceCreationEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

/**
 *******************************************************************************
 *  @fn    ~CustomMediaSource
 *  @brief Destructor
 *
 *
 *******************************************************************************
 */
CustomMediaSource::~CustomMediaSource(void)
{
    mMediaSource.Release();
    mPByteStreamHandler->Release();
    CloseHandle( mSourceCreationEvent);
}

/**
 *******************************************************************************
 *  @fn    Invoke
 *  @brief
 *
 *  param[in]   pResult : provides result of an asynchronous operation.
 *
 *******************************************************************************
 */
HRESULT CustomMediaSource::Invoke(IMFAsyncResult* pResult)
{
    HRESULT hr = S_OK;
    CComPtr < IUnknown > pUnkState;
    CComQIPtr < IAsyncState > pAsyncState;

    /**************************************************************************
     * see if the event indicates a failure - if so, fail and return the      *
     * MEError event                                                          *
     **************************************************************************/
    hr = pResult->GetStatus();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * get the IAsyncState state from the result                              *
     **************************************************************************/
    hr = pResult->GetState(&pUnkState);
    RETURNIFFAILED(hr);

    pAsyncState = pUnkState;

    /**************************************************************************
     * figure out the type of the operation from the state, and then proxy    *
     * the call to the right function                                         *
     **************************************************************************/
    if (pAsyncState->EventType() == AsyncEventType_ByteStreamHandlerEvent)
    {
        hr = handleByteStreamHandlerEvent(pResult);
    }
    return hr;
}

/**
 *******************************************************************************
 *  @fn    CreateCustomSource
 *  @brief creating custom source
 *
 *  @param[in]  sourceFileName : source file name
 *  @param[out] mediaSource    : Generate media data
 *
 *  @return HRESULT : S_OK if successful; else returns microsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSource::createCustomSource(LPCWSTR sourceFileName,
                IMFMediaSource** mediaSource, ConfigCtrl *pConfigCtrl)
{
    HRESULT hr = S_OK;
    CComPtr < IMFByteStream > pByteStream;
    CComPtr < IAsyncState > pState;

    /**************************************************************************
     * create an IMFByteStreamHandler object for the MP3 file                 *
     **************************************************************************/
    hr = CustomByteStreamHandler::createInstance(&mPByteStreamHandler);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * create a state object that will identify the asynchronous operation to *
     * the Invoke                                                             *
     **************************************************************************/
    pState = new (std::nothrow) CAsyncState(
                    AsyncEventType_ByteStreamHandlerEvent);
    if (pState == NULL)
    {
        return E_OUTOFMEMORY;
    }

    mPByteStreamHandler->mWidth = pConfigCtrl->width;
    mPByteStreamHandler->mHeight = pConfigCtrl->height;
    /**************************************************************************
     * create the media source from the IMFByteStreamHandler for the MP3 file *
     **************************************************************************/
    hr = mPByteStreamHandler->BeginCreateObject(nullptr, // byte stream to the file
                    sourceFileName, // URL-style path to the file
                    MF_RESOLUTION_MEDIASOURCE, // create a media source
                    NULL, // no custom configuration properties
                    NULL, // no cancel cookie
                    this, // IMFAsyncCallback that will be called
                    pState); // custom state obj indicating event type

    DWORD waitResult = WaitForSingleObject(mSourceCreationEvent, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
    {
        return E_FAIL;
    }

    *mediaSource = mMediaSource;

    return hr;
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
HRESULT CustomMediaSource::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IMFAsyncCallback)
    {
        *ppv = static_cast<IMFAsyncCallback*> (this);

        AddRef();

        return S_OK;
    }

    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*> (this);

        AddRef();

        return S_OK;
    }

    return E_NOINTERFACE;
}

/**
 *******************************************************************************
 *  @fn    AddRef
 *  @brief Increment reference count of the object.
 *
 *  @return ULONG : New reference count
 *******************************************************************************
 */
ULONG CustomMediaSource::AddRef()
{
    return InterlockedIncrement(&_refCount);
}

/**
 *******************************************************************************
 *  @fn    GetParameters
 *  @brief
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomMediaSource::GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)
{
    (void) pdwFlags;
    (void) pdwQueue;
    return E_NOTIMPL;
}

/**
 *******************************************************************************
 *  @fn    Release
 *  @brief Decrement the reference count of the object.
 *
 *  @return ULONG : New reference count
 *******************************************************************************
 */
ULONG CustomMediaSource::Release()
{
    unsigned long refCount = InterlockedDecrement(&_refCount);
    if (0 == refCount)
    {
        delete this;
    }

    return refCount;
}
;

/**
 *******************************************************************************
 *  @fn    HandleByteStreamHandlerEvent
 *  @brief
 *
 *  param[in]   pResult : provides result of an asynchronous operation.
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomMediaSource::handleByteStreamHandlerEvent(IMFAsyncResult* pResult)
{
    HRESULT hr = S_OK;
    MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
    CComPtr < IUnknown > pUnkSource;

    /**************************************************************************
     * get the actual source by calling IMFByteStreamHandler::EndCreateObject()
     **************************************************************************/
    hr
                    = mPByteStreamHandler->EndCreateObject(pResult,
                                    &objectType, &pUnkSource);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * make sure that what was created was the media source                   *
     **************************************************************************/
    if (objectType != MF_OBJECT_MEDIASOURCE)
    {
        return E_UNEXPECTED;
    }

    /**************************************************************************
     * get the IMFMediaSource pointer from the IUnknown we got from           *
     * EndCreateObject                                                        *
     **************************************************************************/
    mMediaSource = pUnkSource;
    if (mMediaSource == NULL)
    {
        return E_UNEXPECTED;
    }

    SetEvent( mSourceCreationEvent);

    return hr;
}