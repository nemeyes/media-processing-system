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
 * @brief This file contains implementation of CustomByteStreamHandler class
 *
 ********************************************************************************
 */
#include "CustomByteStreamHandler.h"

/**
 *******************************************************************************
 *  @fn    CustomByteStreamHandler
 *  @brief Constructor
 *
 *
 *******************************************************************************
 */
CustomByteStreamHandler::CustomByteStreamHandler(void) :
    mCRef(1), mPH264FSource(NULL), mObjectCreationCanceled(false)
{
}

/**
 *******************************************************************************
 *  @fn    CreateInstance
 *  @brief create an instance of the object CustomByteStreamHandler
 *  @param[in]  ppSource : CustomByteStreamHandler instance
 *
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::createInstance(
                CustomByteStreamHandler** ppSource)
{
    HRESULT hr = S_OK;

    if (ppSource == NULL)
    {
        return E_POINTER;
    }

    /**************************************************************************/
    /* Create an instance of the object                                       */
    /**************************************************************************/
    CustomByteStreamHandler* pAVFByteStreamHandler =
                    new (std::nothrow) CustomByteStreamHandler();

    if (pAVFByteStreamHandler == NULL)
    {
        return E_OUTOFMEMORY;
    }

    /**************************************************************************/
    /* Set the out pointer.                                                   */
    /**************************************************************************/
    *ppSource = pAVFByteStreamHandler;

    if (FAILED(hr) && pAVFByteStreamHandler != NULL)
    {
        delete pAVFByteStreamHandler;
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    ~CustomByteStreamHandler
 *  @brief Destructor
 *
 *
 *******************************************************************************
 */
CustomByteStreamHandler::~CustomByteStreamHandler(void)
{
    if (mPH264FSource != NULL)
    {
        mPH264FSource->Release();
        mPH264FSource = NULL;
    }
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
ULONG CustomByteStreamHandler::AddRef()
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
ULONG CustomByteStreamHandler::Release()
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
HRESULT CustomByteStreamHandler::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv == NULL)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
        *ppv
                        = static_cast<IUnknown*> (static_cast<IMFByteStreamHandler*> (this));
    }
    else if (riid == IID_IMFByteStreamHandler)
    {
        *ppv = static_cast<IMFByteStreamHandler*> (this);
    }
    else if (riid == IID_IMFAsyncCallback)
    {
        *ppv = static_cast<IMFAsyncCallback*> (this);
    }
    else if (riid == IID_IInitializeWithFile)
    {
        *ppv = static_cast<IInitializeWithFile*> (this);
    }
    else if (riid == IID_IPropertyStore)
    {
        *ppv = static_cast<IPropertyStore*> (this);
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

/******************************************************************************/
/* IMFByteStreamHandler                                                       */
/******************************************************************************/

/*
 *******************************************************************************
 *  @fn    QueryInterface
 *  @brief Begin asynchronously creating and initializing the
 *         IMFByteStreamHandler object.
 *
 *  @param[in]  pByteStream : Represents a byte stream from some data source,
 *                            which might be a local file, a network file, or
 *                            some other source; not used
 *  @param[in]  pwszURL     : URL style path of the source
 *  @param[out] dwFlags     : Source resolver flags
 *  @param[in]  pProps      : Exposes methods for enumerating, getting, and
 *                            setting property values; not used
 *  @param[out]  ppIUnknownCancelCookie :
 *  @param[in]  pCallback :
 *  @param[in]  punkState :
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::BeginCreateObject(IMFByteStream *pByteStream,
                LPCWSTR pwszURL, DWORD dwFlags, IPropertyStore *pProps,
                IUnknown **ppIUnknownCancelCookie, IMFAsyncCallback *pCallback,
                IUnknown *punkState)
{
    (void) pProps;
    (void) pByteStream;
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************/
    /* Sanity check input arguments.                                          */
    /*  BREAK_ON_NULL (pByteStream, E_POINTER);                               */
    /**************************************************************************/
    if (nullptr == pCallback)
    {
        return E_POINTER;
    }

    if (nullptr == pwszURL)
    {
        return E_POINTER;
    }

    /**************************************************************************/
    /* At this point the source should be NULL - otherwise multiple clients   */
    /* are trying to create an CustomSource concurrently with the same byte   */
    /* stream handler - that is not supported.                                */
    /**************************************************************************/
    if (mPH264FSource != NULL)
    {
        if (mPH264FSource)
        {
            mPH264FSource->Release();
            mPH264FSource = NULL;
        }
        mPResult = NULL;
        return E_UNEXPECTED;
    }

    /**************************************************************************/
    /* Verify that the caller is requesting the creation of a MediaSource     */
    /* object - the CustomByteStreamHandler doesn't support other object      */
    /* types.                                                                 */
    /**************************************************************************/
    if ((dwFlags & MF_RESOLUTION_MEDIASOURCE) == 0)
    {
        if (mPH264FSource)
        {
            mPH264FSource->Release();
            mPH264FSource = NULL;
        }
        mPResult = NULL;
        return E_INVALIDARG;
    }

    /**************************************************************************/
    /* Create an asynchronous result that will be used to indicate to the     */
    /* caller that the source has been created.  The result is stored in a    */
    /* class member variable.                                                 */
    /**************************************************************************/
    hr = MFCreateAsyncResult(NULL, pCallback, punkState, &mPResult);
    RETURNIFFAILED(hr);

    /**************************************************************************/
    /* New object - creation was not canceled, reset the cancel flag.         */
    /**************************************************************************/
    mObjectCreationCanceled = false;

    /**************************************************************************/
    /* return a pointer to the bytestream handler as the cancel cookie object */
    /**************************************************************************/
    if (ppIUnknownCancelCookie != NULL)
    {
        hr
                        = this->QueryInterface(IID_IUnknown,
                                        (void**) ppIUnknownCancelCookie);
        RETURNIFFAILED(hr);
    }

    /**************************************************************************/
    /* create the main source worker object - the CustomSource                */
    /**************************************************************************/
    hr = CustomSource::createInstance(&mPH264FSource);
    RETURNIFFAILED(hr);

    mPH264FSource->mWidth = mWidth;
    mPH264FSource->mHeight = mHeight;

    /**************************************************************************/
    /* Begin source asynchronous open operation - tell the source to call     */
    /* this object when it is done                                            */
    /**************************************************************************/
    hr = mPH264FSource->BeginOpen(pwszURL, this, NULL);
    RETURNIFFAILED(hr);

    /**************************************************************************/
    /* if something failed, release all internal variables                    */
    /**************************************************************************/
    if (FAILED(hr))
    {
        if (mPH264FSource)
        {
            mPH264FSource->Release();
            mPH264FSource = NULL;
        }
        mPResult = NULL;
    }

    return hr;
}

/*
 *******************************************************************************
 *  @fn    EndCreateObject
 *  @brief End asynchronously creating and initializing the IMFByteStreamHandler
 *         object
 *
 *  @param[in]  pResult     :
 *  @param[out] pObjectType :
 *  @param[out] ppObject    :
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::EndCreateObject(IMFAsyncResult* pResult,
                MF_OBJECT_TYPE* pObjectType, IUnknown** ppObject)
{
    HRESULT hr = S_OK;

    do
    {
        CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

        /**********************************************************************/
        // Sanity checks input arguments.
        /**********************************************************************/
        if (nullptr == pResult)
        {
            return E_POINTER;
        }

        if (nullptr == pObjectType)
        {
            return E_POINTER;
        }

        if (nullptr == ppObject)
        {
            return E_POINTER;
        }

        if (nullptr == mPH264FSource)
        {
            return E_UNEXPECTED;
        }

        /**********************************************************************/
        /* initialize output parameters - the object has not been created yet,*/
        /* so if there is an error these output parameters will contain the   */
        /* right values                                                       */
        /**********************************************************************/
        *pObjectType = MF_OBJECT_INVALID;
        *ppObject = NULL;

        // Check to see if there is an error.
        hr = pResult->GetStatus();

        if (SUCCEEDED(hr))
        {
            /******************************************************************/
            /* if we got here, result indicated success in creating the       */
            /* source - therefore we can return a flag indicating that we     */
            /* created a source                                               */
            /******************************************************************/
            *pObjectType = MF_OBJECT_MEDIASOURCE;

            /******************************************************************/
            /* Since the handler just created a media source, get the media   */
            /* source interface from the underlying CustomSource helper       */
            /* object.                                                        */
            /******************************************************************/
            hr = mPH264FSource->QueryInterface(IID_IMFMediaSource,
                            (void**) ppObject);
        }

        /**********************************************************************/
        /* whatever happens, make sure the source is in a good state by       */
        /* resetting internal variables                                       */
        /**********************************************************************/
        if (mPH264FSource)
        {
            mPH264FSource->Release();
            mPH264FSource = NULL;
        }
        mPResult = NULL;
        mObjectCreationCanceled = false;
    } while (false);

    return hr;
}

/*
 *******************************************************************************
 *  @fn    CancelObjectCreation
 *  @brief Cancel the asynchronous object creation operation.
 *
 *  @param[in]  pIUnknownCancelCookie : not used
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::CancelObjectCreation(
                IUnknown* pIUnknownCancelCookie)
{
    (void) pIUnknownCancelCookie;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************/
    /* if mPResult is NULL, nobody is trying to create an object, and there   */
    /* is nothing to cancel - return an error.  Otherwise, store the          */
    /* cancellation command.                                                  */
    /**************************************************************************/
    if (mPResult == NULL)
    {
        return E_UNEXPECTED;
    }
    else
    {
        mObjectCreationCanceled = true;
        return S_OK;
    }
}

/*
 *******************************************************************************
 *  @fn    GetMaxNumberOfBytesRequiredForResolution
 *  @brief Determine the maximum number of bytes the source needs to parse
 *         before it can determine if the file format is something that it
 *         can read.
 *
 *  @param[in]  pqwBytes : Max number of bytes required for resolution
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::GetMaxNumberOfBytesRequiredForResolution(
                QWORD* pqwBytes)
{
    if (pqwBytes == NULL)
        return E_INVALIDARG;

    *pqwBytes = 1024;

    return S_OK;
}

/*
 *******************************************************************************
 *  @fn    GetParameters
 *  @brief IMFAsyncCallback implementation
 *
 *  @param[in]  pdwFlags : Max number of bytes required for resolution, not used
 *  @param[in]  pdwQueue : Not used
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::GetParameters(DWORD* pdwFlags, DWORD* pdwQueue)
{
    (void) pdwFlags;
    (void) pdwQueue;
    return E_NOTIMPL;
}

/*
 *******************************************************************************
 *  @fn    Invoke
 *  @brief Asynchronous worker function - called when the source has finished
 *          initializing.
 *
 *  @param[out]  pResult : Provides information about the result of an
 *               asynchronous operation.
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::Invoke(IMFAsyncResult* pResult)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        if (nullptr == mPResult)
        {
            return E_UNEXPECTED;
        }

        /**********************************************************************/
        /* If object creation was canceled, just delete the CustomSource,     */
        /* and return E_ABORT.                                                */
        /**********************************************************************/
        if (mObjectCreationCanceled)
        {
            mObjectCreationCanceled = false;
            hr = E_ABORT;

            /******************************************************************/
            /* release the source - it will not be needed now                 */
            /******************************************************************/
            mPH264FSource->Release();
            mPH264FSource = NULL;
        }
        else
        {
            /******************************************************************/
            /* Call EndOpen to finish asynchronous open operation, and check  */
            /* for errors during parsing. If this failed, the HR will be      */
            /* stored in the result.                                          */
            /******************************************************************/
            hr = mPH264FSource->EndOpen(pResult);
        }

        /**********************************************************************/
        /* Store the result of the operation.                                 */
        /**********************************************************************/
        mPResult->SetStatus(hr);

        /**********************************************************************/
        /* Call back the caller with the result.                              */
        /**********************************************************************/
        hr = MFInvokeCallback(mPResult);

        /**********************************************************************/
        /* Release the result for the client - it has been used and is no     */
        /* longer needed.                                                     */
        /**********************************************************************/
        mPResult = NULL;
    } while (false);

    return hr;
}

/******************************************************************************/
/* IInitializeWithFile interface implementation                               */
/******************************************************************************/

/*
 *******************************************************************************
 *  @fn    Initialize
 *  @brief Initialize the IPropertyStore interface components of the
 *         CustomByteStreamHandler
 *
 *  @param[in]  pszFilePath :
 *  @param[in]  grfMode     :
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::Initialize(LPCWSTR pszFilePath, DWORD grfMode)
{
    (void) pszFilePath;
    (void) grfMode;
    HRESULT hr = S_OK;

    return hr;
}

/*
 *******************************************************************************
 *  @fn    Commit
 *  @brief
 *
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::Commit()
{
    if (mPPropertyStore == NULL)
        return E_UNEXPECTED;

    return mPPropertyStore->Commit();
}

/*
 *******************************************************************************
 *  @fn    GetAt
 *  @brief
 *
 *  @param[in]   iProp :
 *  @param[out]  pkey  : pointer to the property key
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::GetAt(DWORD iProp, PROPERTYKEY *pkey)
{
    if (mPPropertyStore == NULL)
        return E_UNEXPECTED;

    return mPPropertyStore->GetAt(iProp, pkey);
}

/*
 *******************************************************************************
 *  @fn    GetCount
 *  @brief
 *
 *  @param[out]  cProps : property count
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::GetCount(DWORD *cProps)
{
    if (mPPropertyStore == NULL)
        return E_UNEXPECTED;

    return mPPropertyStore->GetCount(cProps);
}

/*
 *******************************************************************************
 *  @fn    GetValue
 *  @brief Get the value of property key
 *
 *  @param[in]  key : property key
 *  @param[out] pv  : property variant
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::GetValue(REFPROPERTYKEY key, PROPVARIANT *pv)
{
    if (mPPropertyStore == NULL)
        return E_UNEXPECTED;

    return mPPropertyStore->GetValue(key, pv);
}

/*
 *******************************************************************************
 *  @fn    GetValue
 *  @brief Set the value of property key
 *
 *  @param[out]  key      : property key
 *  @param[in]   propvar  : property variant
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomByteStreamHandler::SetValue(REFPROPERTYKEY key,
                REFPROPVARIANT propvar)
{
    if (mPPropertyStore == NULL)
        return E_UNEXPECTED;

    return mPPropertyStore->SetValue(key, propvar);
}

