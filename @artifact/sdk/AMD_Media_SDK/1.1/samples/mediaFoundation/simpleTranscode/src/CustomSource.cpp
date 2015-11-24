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
 * @file <CustomSource.cpp>
 *
 * @brief This file contains implementation of CustomSource class
 *
 ********************************************************************************
 */
#include "CustomSource.h"

/**
 *******************************************************************************
 *  @fn    createInstance
 *  @brief creating Custom sink activate instance
 *
 *  @param[in]  ppAVFSource : sink file name
 *
 *  @return HRESULT : S_OK if successful; else returns microsoft error codes.
 *******************************************************************************
 */
HRESULT CustomSource::createInstance(CustomSource** ppAVFSource)
{
    HRESULT hr = S_OK;
    CustomSource* pAVFSource = NULL;

    if (nullptr == ppAVFSource)
    {
        return E_POINTER;
    }

    // create an instance of the object
    pAVFSource = new (std::nothrow) CustomSource(&hr);
    if (nullptr == pAVFSource)
    {
        return E_OUTOFMEMORY;
    }

    RETURNIFFAILED(hr);

    // Set the out pointer.
    *ppAVFSource = pAVFSource;

    if (FAILED(hr) && pAVFSource != NULL)
    {
        delete pAVFSource;
    }

    return hr;
}

/******************************************************************************/
/* Standard IUnknown interface implementation                                 */
/******************************************************************************/

/**
 *******************************************************************************
 *  @fn    AddRef
 *  @brief Increment reference count of the object.
 *
 *  @return ULONG : New reference count
 *******************************************************************************
 */
ULONG CustomSource::AddRef()
{
    return InterlockedIncrement(&mCRef);
}

/**
 *******************************************************************************
 *  @fn    Release
 *  @brief Decrement the reference count of the object.
 *
 *  @return ULONG : New reference count
 *******************************************************************************
 */
ULONG CustomSource::Release()
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
HRESULT CustomSource::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv == NULL)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
        *ppv
                        = static_cast<IUnknown*> (static_cast<IMFMediaEventGenerator*> (this));
    }
    else if (riid == IID_IMFMediaEventGenerator)
    {
        *ppv = static_cast<IMFMediaEventGenerator*> (this);
    }
    else if (riid == IID_IMFMediaSource)
    {
        *ppv = static_cast<IMFMediaSource*> (this);
    }
    else if (riid == IID_IMFAsyncCallback)
    {
        *ppv = static_cast<IMFAsyncCallback*> (this);
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
/*  IMFMediaEventGenerator interface implementation                           */
/******************************************************************************/

/**
 *******************************************************************************
 *  @fn    BeginGetEvent
 *  @brief Begin processing an event from the event queue asynchronously
 *
 *  @param[in]  pCallback  : callback of the object interested in events
 *  @param[out] punkState  : some custom state object returned with event
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::BeginGetEvent(IMFAsyncCallback* pCallback,
                IUnknown* punkState)
{
    HRESULT hr = S_OK;
    CComPtr < IMFMediaEventQueue > pLocQueue = mPEventQueue;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * get the next event from the event queue                                *
     **************************************************************************/
    hr = pLocQueue->BeginGetEvent(pCallback, punkState);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    EndGetEvent
 *  @brief Complete asynchronous event processing
 *
 *  @param[in]  pResult  : result of an asynchronous operation
 *  @param[out] ppEvent  : event extracted from the queue
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::EndGetEvent(IMFAsyncResult* pResult,
                IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;
    CComPtr < IMFMediaEventQueue > pLocQueue = mPEventQueue;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    hr = pLocQueue->EndGetEvent(pResult, ppEvent);
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    GetEvent
 *  @brief Synchronously retrieve the next event from the event queue
 *
 *  @param[in]  dwFlags  : flag with the event behavior
 *  @param[out] ppEvent  : event extracted from the queue
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;
    CComPtr < IMFMediaEventQueue > pLocQueue = mPEventQueue;

    /**************************************************************************
     * Check whether the source is shut down but do it in a separate locked   *
     * section- GetEvent() may block, and we don't want to hold the critical  *
     * section during that time.                                              *
     **************************************************************************/
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * Get the event from the queue - note, may block!                        *
     **************************************************************************/
    hr = pLocQueue->GetEvent(dwFlags, ppEvent);
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    QueueEvent
 *  @brief Store the event in the internal event queue of the source
 *
 *  @param[in]  met              : media event type
 *  @param[in]  guidExtendedType : GUID_NULL for standard events or an
 *                                 extension GUID
 *  @param[out] hrStatus         : status of the operation
 *  @param[out] pvValue          : a VARIANT with some event value or NULL
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::QueueEvent(MediaEventType met, REFGUID guidExtendedType,
                HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * queue the passed in event on the internal event queue                  *
     **************************************************************************/
    hr = mPEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus,
                    pvValue);
    RETURNIFFAILED(hr);

    return hr;
}

/******************************************************************************/
/* IMFAsyncCallback implementation                                            */
/******************************************************************************/

/**
 *******************************************************************************
 *  @fn    GetParameters
 *  @brief Get the behavior information (duration, etc.) of the asynchronous
 *         callback operation - not implemented.
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::GetParameters(DWORD*, DWORD*)
{
    return E_NOTIMPL;
}

/**
 *******************************************************************************
 *  @fn    Invoke
 *  @brief Do an asynchronous task - execute a queued command (operation).
 *
 *  @param[in]  pResult  : Asynchrounous operation result
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::Invoke(IMFAsyncResult* pResult)
{
    HRESULT hr = S_OK;
    CComPtr < IMFAsyncResult > pAsyncResult = pResult;
    CComPtr < IMFAsyncResult > pCallerResult;
    CComPtr < ISourceOperation > pCommand;
    CComPtr < IUnknown > pState;

    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * Get the state object associated with this asynchronous call            *
     **************************************************************************/
    hr = pAsyncResult->GetState(&pState);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * QI the IUnknown state variable for the ISourceOperation interface      *
     **************************************************************************/
    hr = pState->QueryInterface(IID_ISourceOperation, (void**) &pCommand);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * Make sure the source is not shut down - if the source is shut down,    *
     * just exit                                                              *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * figure out what the requested command is, and then dispatch it to one  *
     * of the internal handler objects                                        *
     **************************************************************************/
    switch (pCommand->Type())
    {
    case SourceOperationOpen:
        hr = internalOpen(pCommand);
        break;
    case SourceOperationStart:
        hr = internalStart(pCommand);
        break;
    case SourceOperationStop:
        hr = internalStop();
        break;
    case SourceOperationPause:
        hr = internalPause();
        break;
    case SourceOperationStreamNeedData:
        hr = internalRequestSample();
        break;
    case SourceOperationEndOfStream:
        hr = internalEndOfStream();
        break;
    }

    return hr;
}

/******************************************************************************/
/*  IMFMediaSource interface implementation                                   */
/******************************************************************************/

/**
 *******************************************************************************
 *  @fn    CreatePresentationDescriptor
 *  @brief Get a descriptor object for the current presentation
 *
 *  @param[in] ppPresentationDescriptor : pointer to the presentation descriptor
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::CreatePresentationDescriptor(
                IMFPresentationDescriptor** ppPresentationDescriptor)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * sanity check parameters and internal variables                         *
     **************************************************************************/
    if (nullptr == ppPresentationDescriptor)
    {
        return E_POINTER;
    }

    if (nullptr == mPPresentationDescriptor)
    {
        return MF_E_NOT_INITIALIZED;
    }

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * create a copy of the stored presentation descriptor                    *
     **************************************************************************/
    hr = mPPresentationDescriptor->Clone(ppPresentationDescriptor);
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    GetCharacteristics
 *  @brief Get the characteristics and capabilities of the MF source
 *
 *  @param[out] pdwCharacteristics : pointer to the media source characteristics
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::GetCharacteristics(DWORD* pdwCharacteristics)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * sanity check the passed in parameter                                   *
     **************************************************************************/
    if (nullptr == pdwCharacteristics)
    {
        return E_POINTER;
    }

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * Indicate that the source can pause and supports seeking                *
     **************************************************************************/
    *pdwCharacteristics = MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_CAN_SEEK;

    return hr;
}

/**
 *******************************************************************************
 *  @fn    Start
 *  @brief Start playback at the specified time
 *
 *  @param[in] pPresentationDescriptor : pointer to the presentation descriptor
 *  @param[in] pguidTimeFormat         : format of the following time variable
 *  @param[in] pvarStartPosition       : stream time where to start playback
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::Start(IMFPresentationDescriptor* pPresentationDescriptor,
                const GUID* pguidTimeFormat,
                const PROPVARIANT* pvarStartPosition)
{
    HRESULT hr = S_OK;
    bool isSeek = false;
    CComPtr < ISourceOperation > pOperation;

    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    if (nullptr == pvarStartPosition)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pPresentationDescriptor)
    {
        return E_INVALIDARG;
    }

    /**************************************************************************
     * The IMFMediaSource::Start() function can support various time formats  *
     * for input, but this implementation supports only the default version - *
     * time indicated in 100-ns units                                         *
     **************************************************************************/
    if ((pguidTimeFormat != NULL) && (*pguidTimeFormat != GUID_NULL))
    {
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    /**************************************************************************
     * make sure we have the start time in the pvarStartPosition PROPVARIANT  *
     * structure                                                              *
     **************************************************************************/
    if ((pvarStartPosition->vt != VT_I8) && (pvarStartPosition->vt != VT_EMPTY))
    {
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * make sure the source is initialized                                    *
     **************************************************************************/
    hr = isInitialized();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * figure out whether the caller is trying to seek or to just start       *
     * playback                                                               *
     **************************************************************************/
    if (pvarStartPosition->vt == VT_I8)
    {
        if (mState != SourceStateStopped)
        {
            isSeek = true;
        }
    }

    /**************************************************************************
     * create the new command that will tell us to start playback             *
     **************************************************************************/
    pOperation = new (std::nothrow) SourceOperation(SourceOperationStart, // store command type - start command
                    pPresentationDescriptor); // store presentation descriptor param

    if (nullptr == pOperation)
    {
        return E_OUTOFMEMORY;
    }

    /**************************************************************************
     * set the internal information in the new command                        *
     **************************************************************************/
    hr = pOperation->SetData(*pvarStartPosition, isSeek);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * queue the start command work item                                      *
     **************************************************************************/
    hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, // work queue to use
                    this, // IMFAsyncCallback object to call
                    static_cast<IUnknown*> (pOperation)); // state variable - the command
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    Pause
 *  @brief Pause the source
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::Pause()
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);
    CComPtr < ISourceOperation > pOperation;

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * create a new SourceOperationType command                               *
     **************************************************************************/
    pOperation = new (std::nothrow) SourceOperation(SourceOperationPause);
    if (nullptr == pOperation)
    {
        return E_OUTOFMEMORY;
    }

    /**************************************************************************
     * put the command on the work queue                                      *
     **************************************************************************/
    hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, // work queue to use
                    this, // IMFAsyncCallback object to call
                    static_cast<IUnknown*> (pOperation)); // state variable - the command
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    Stop
 *  @brief Stop playback
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::Stop()
{
    HRESULT hr = S_OK;
    CComPtr < ISourceOperation > pOperation;

    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * make sure the source is initialized                                    *
     **************************************************************************/
    hr = isInitialized();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * create a new SourceOperationType command                               *
     **************************************************************************/
    pOperation = new (std::nothrow) SourceOperation(SourceOperationStop);
    if (nullptr == pOperation)
    {
        return E_OUTOFMEMORY;
    }

    /**************************************************************************
     * put the command on the work queue                                      *
     **************************************************************************/
    hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, this,
                    static_cast<IUnknown*> (pOperation));
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    Shutdown
 *  @brief Shutdown the source
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::Shutdown()
{
    HRESULT hr = S_OK;

    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * make sure the source is not shut down                                  *
     **************************************************************************/
    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * go through every underlying stream and send a shutdown command to it   *
     **************************************************************************/
    for (DWORD x = 0; x < mMediaStreams.size(); x++)
    {
        CustomStream* pStream = NULL;

        pStream = mMediaStreams[x];
        RETURNIFFAILED(hr);
        if (nullptr == pStream)
        {
            return E_UNEXPECTED;
        }

        pStream->shutdown();
        pStream->Release();
    }
    RETURNIFFAILED(hr);

    /**************************************************************************
     * shut down the event queue                                              *
     **************************************************************************/
    if (mPEventQueue)
    {
        mPEventQueue->Shutdown();
    }

    /**************************************************************************
     * clear the vector of streams                                            *
     **************************************************************************/
    EXCEPTIONTOHR(mMediaStreams.clear());

    if (mPInputFileParser)
    {
        delete mPInputFileParser;
        mPInputFileParser = NULL;
    }

    /**************************************************************************
     * set the internal state variable to indicate that the source is shutdown*
     **************************************************************************/
    mState = SourceStateShutdown;

    return hr;
}

/******************************************************************************/
/* Public helper methods                                                      */
/******************************************************************************/

/**
 *******************************************************************************
 *  @fn    BeginOpen
 *  @brief Begin the asynchronous open operation
 *
 *  @param[in] pwszURL   : URL path
 *  @param[in] pCallback : pointer to the call back functions
 *  @param[in] pUnkState : some custom state object returned with event
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::BeginOpen(LPCWSTR pwszURL, IMFAsyncCallback* pCallback,
                IUnknown* pUnkState)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);
    CComPtr < IMFAsyncResult > pResult;
    CComPtr < ISourceOperation > pOperation;

    /**************************************************************************
     * Pack the needed arguments into an AsyncResult object                   *
     **************************************************************************/
    hr = MFCreateAsyncResult(NULL, pCallback, pUnkState, &pResult);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * create a new SourceOperation object with the Open command              *
     **************************************************************************/
    pOperation = new (std::nothrow) SourceOperation(SourceOperationOpen,
                    pwszURL, pResult);
    if (nullptr == pOperation)
    {
        return E_OUTOFMEMORY;
    }

    /**************************************************************************
     * Submit the request into the background thread                          *
     **************************************************************************/
    hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, this, pOperation);
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    EndOpen
 *  @brief End the asynchronous open operation
 *
 *  @param[in] pAsyncResult : pointer to the asynchronous operation result
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::EndOpen(IMFAsyncResult* pAsyncResult)
{
    HRESULT hr = S_OK;

    if (nullptr == pAsyncResult)
    {
        return E_POINTER;
    }

    hr = pAsyncResult->GetStatus();

    return hr;
}

/**
 *******************************************************************************
 *  @fn    checkShutdown
 *  @brief Return an error if the source is shut down, and S_OK otherwise
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::checkShutdown() const
{
    if (mState == SourceStateShutdown)
    {
        return MF_E_SHUTDOWN;
    }
    else
    {
        return S_OK;
    }
}

/**
 *******************************************************************************
 *  @fn    IsInitialized
 *  @brief Return a failure if the source is not initialized
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::isInitialized() const
{
    if (mState == SourceStateOpening || mState == SourceStateUninitialized)
    {
        return MF_E_NOT_INITIALIZED;
    }
    else
    {
        return S_OK;
    }
}

/******************************************************************************/
/* Private helper methods                                                     *
 /******************************************************************************/

/**
 *******************************************************************************
 *  @fn    SendOperation
 *  @brief Helper function that schedules the passed-in command on the work
 *         queue
 *
 *  @param[in] operationType : Source operation type
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::sendOperation(SourceOperationType operationType)
{
    HRESULT hr = S_OK;
    CComPtr < ISourceOperation > pOperation;

    /**************************************************************************
     * create a new SourceOperationType command                               *
     **************************************************************************/
    pOperation = new (std::nothrow) SourceOperation(operationType);
    if (nullptr == pOperation)
    {
        return E_OUTOFMEMORY;
    }

    /**************************************************************************
     * queue the command on the queue                                         *
     **************************************************************************/
    hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, this,
                    static_cast<IUnknown*> (pOperation));
    RETURNIFFAILED(hr);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    InternalOpen
 *  @brief Initialize the underlying FileParser, and open the file in the
 *         operation object.
 *
 *  @param[in] pOp : Source operation
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::internalOpen(ISourceOperation* pOp)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);
    WCHAR* pUrl = NULL;
    CComPtr < ISourceOperation > pOperation = pOp;
    CComPtr < IMFAsyncResult > pCallerResult;

    if (nullptr == pOperation)
    {
        return E_UNEXPECTED;
    }

    /**************************************************************************
     * Get the async result that will be sent once the open operation is      *
     * complete.                                                              *
     **************************************************************************/
    hr = pOperation->GetCallerAsyncResult(&pCallerResult);
    RETURNIFFAILED(hr);
    if (nullptr == pCallerResult)
    {
        return E_UNEXPECTED;
    }

    /**************************************************************************
     * get the file URL from the operation                                    *
     **************************************************************************/
    pUrl = pOperation->GetUrl();
    if (nullptr == pUrl)
    {
        return E_UNEXPECTED;
    }

    hr = VideoInput::createInstance(pUrl, &mPInputFileParser);
    RETURNIFFAILED(hr);

    mPInputFileParser->mWidth = mWidth;
    mPInputFileParser->mHeight = mHeight;

    /**************************************************************************
     * parse the file header and instantiate the individual stream objects    *
     **************************************************************************/
    hr = parseHeader();

    /**************************************************************************
     * return the result whether we succeeded or failed                       *
     **************************************************************************/
    if (pCallerResult != NULL)
    {
        /**********************************************************************
         * Store the result of the initialization operation in the caller     *
         * result.                                                            *
         **********************************************************************/
        pCallerResult->SetStatus(hr);

        /**********************************************************************
         * Notify the caller of the BeginOpen() method that the open is       *
         * complete.                                                          *
         **********************************************************************/
        MFInvokeCallback( pCallerResult);
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    InternalStart
 *  @brief Start playback or seek to the specified location.
 *
 *  @param[in] pCommand : Source command
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::internalStart(ISourceOperation* pCommand)
{
    HRESULT hr = S_OK;
    CComPtr < IMFPresentationDescriptor > pPresentationDescriptor;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * get the presentation descriptor from the start operation               *
     **************************************************************************/
    hr = pCommand->GetPresentationDescriptor(&pPresentationDescriptor);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * activate the streams associated with the presentation descriptor       *
     **************************************************************************/
    hr = selectStreams(pPresentationDescriptor, pCommand->GetData(),
                    pCommand->IsSeek());
    RETURNIFFAILED(hr);

    /**************************************************************************
     * set the start position in the file                                     *
     **************************************************************************/
    hr = mPInputFileParser->setOffset(pCommand->GetData());
    RETURNIFFAILED(hr);

    /**************************************************************************
     * update the internal state variable                                     *
     **************************************************************************/
    mState = SourceStateStarted;

    /**************************************************************************
     * we have just started - which means that none of the streams have hit   *
     * the end of stream indicator yet.  Once all of the streams have ended,  *
     * the source will stop.                                                  *
     **************************************************************************/
    mPendingEndOfStream = 0;

    /**************************************************************************
     * if we got here, then everything succeed.  If this was a seek request,  *
     * queue the result of the seek command.  If this is a start request,     *
     * queue the result of the start command.                                 *
     **************************************************************************/
    if (pCommand->IsSeek())
    {
        hr = mPEventQueue->QueueEventParamVar(MESourceSeeked, // seek result
                        GUID_NULL, // no extended event data
                        S_OK, // succeeded
                        &pCommand->GetData()); // operation object
    }
    else
    {
        hr = mPEventQueue->QueueEventParamVar(MESourceStarted, // start result
                        GUID_NULL, // no extended event data
                        S_OK, // succeeded
                        &pCommand->GetData()); // operation object
    }

    /**************************************************************************
     * if we failed, fire an event indicating status of the operation - there *
     * is no need to fire status if start succeeded, since that would have    *
     * been handled above in the while loop                                   *
     **************************************************************************/
    if (FAILED(hr))
    {
        mPEventQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, hr, NULL);
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    InternalStop
 *  @brief Internal implementation of the stop command - needed to
 *         asynchronously handle stop
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::internalStop(void)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * check every stream, and if it is active, send a stop command to it     *
     **************************************************************************/
    for (DWORD x = 0; x < mMediaStreams.size(); x++)
    {
        CustomStream* pStream = NULL;

        pStream = mMediaStreams[x];
        RETURNIFFAILED(hr);
        if (nullptr == pStream)
        {
            return E_UNEXPECTED;
        }

        if (pStream->isActive())
        {
            hr = pStream->stop();
            RETURNIFFAILED(hr);
        }
    }
    RETURNIFFAILED(hr);

    /**************************************************************************
     * update the internal state variable                                     *
     **************************************************************************/
    mState = SourceStateStopped;

    /**************************************************************************
     * fire an event indicating status of the operation                       *
     **************************************************************************/
    mPEventQueue->QueueEventParamVar(MESourceStopped, GUID_NULL, hr, NULL);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    InternalPause
 *  @brief Internal implementation of the pause command - needed to
 *         asynchronously handle pause
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::internalPause(void)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * if the source is not started, it cannot be paused                      *
     **************************************************************************/
    if (mState != SourceStateStarted)
    {
        return MF_E_INVALID_STATE_TRANSITION;
    }

    /**************************************************************************
     * check every stream, and if it is active, send a pause command to it     *
     **************************************************************************/
    for (DWORD x = 0; x < mMediaStreams.size(); x++)
    {
        CustomStream* pStream = NULL;

        pStream = mMediaStreams[x];
        RETURNIFFAILED(hr);
        if (nullptr == pStream)
        {
            return E_UNEXPECTED;
        }

        if (pStream->isActive())
        {
            hr = pStream->pause();
            RETURNIFFAILED(hr);
        }
    }
    RETURNIFFAILED(hr);

    /**************************************************************************
     * update the internal state variable                                     *
     **************************************************************************/
    mState = SourceStatePaused;

    /**************************************************************************
     * fire an event indicating status of the operation                       *
     **************************************************************************/
    mPEventQueue->QueueEventParamVar(MESourcePaused, GUID_NULL, hr, NULL);

    return hr;
}

/**
 *******************************************************************************
 *  @fn    InternalRequestSample
 *  @brief Check if the current stream needs more data and send the samples to
 *         the stream if required
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::internalRequestSample(void)
{
    bool needMoreData = false;
    HRESULT hr = S_OK;

    /**************************************************************************
     * launch decode thread                                                   *
     **************************************************************************/
    do
    {
        needMoreData = false;

        for (DWORD x = 0; x < mMediaStreams.size(); x++)
        {
            CustomStream* pStream = NULL;

            pStream = mMediaStreams[x];
            RETURNIFFAILED(hr);
            if (nullptr == pStream)
            {
                return E_UNEXPECTED;
            }

            /******************************************************************
             * if the current stream needs more data, process its requests    *
             ******************************************************************/
            if (pStream->needsData())
            {
                /**************************************************************
                 * store a flag indicating that somebody did need data        *
                 **************************************************************/
                needMoreData = true;

                /**************************************************************
                 * call a function to send a sample to the stream             *
                 **************************************************************/
                hr = sendSampleToStream(pStream);
                RETURNIFFAILED(hr);
            }
        }

        RETURNIFFAILED(hr);

        /***********************************************************************
         * loop while some stream needs more data - stop only once none of the *
         * streams are requesting more samples                                 *
         ***********************************************************************/

        // exit criteria
    } while (needMoreData);

    if (FAILED(hr))
    {
        QueueEvent(MEError, GUID_NULL, hr, NULL);
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    SendSampleToStream
 *  @brief Load a sample of the right type from the parser and send it to the
 *          passed-in stream
 *
 * param[in] pStream : pointer to the stream
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::sendSampleToStream(CustomStream* pStream)
{
    HRESULT hr = S_OK;
    CComPtr < IMFSample > pSample;

    hr = mPInputFileParser->getNextVideoSample(&pSample);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * deliver the video sample                                               *
     **************************************************************************/
    hr = pStream->deliverSample(pSample);
    RETURNIFFAILED(hr);

    if (mPInputFileParser->IsVideoEndOfStream())
    {
        hr = pStream->endOfStream();
        RETURNIFFAILED(hr);
    }
    return hr;
}

/**
 *******************************************************************************
 *  @fn    InternalEndOfStream
 *  @brief Handle an end of stream event
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::internalEndOfStream(void)
{
    HRESULT hr = S_OK;

    /**************************************************************************
     * increment the counter which indicates how many streams have signaled   *
     * their end                                                              *
     **************************************************************************/
    mPendingEndOfStream++;

    /**************************************************************************
     * if all of the streams have ended, fire an MEEndOfPresentation event    *
     **************************************************************************/
    if (mPendingEndOfStream == mMediaStreams.size())
    {
        hr = mPEventQueue->QueueEventParamVar(MEEndOfPresentation, GUID_NULL,
                        S_OK, NULL);
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    ParseHeader
 *  @brief Parse the file header of the current file
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::parseHeader(void)
{
    HRESULT hr = S_OK;

    /**************************************************************************
     * Sanity checks against input arguments.                                 *
     **************************************************************************/
    if (nullptr == mPInputFileParser)
    {
        return E_UNEXPECTED;
    }

    hr = mPInputFileParser->parseHeader();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * Create the individual streams from the presentation descriptor         *
     **************************************************************************/
    hr = internalCreatePresentationDescriptor();
    RETURNIFFAILED(hr);

    /**************************************************************************
     * if the header was successfully parsed, and if the streams have been    *
     * created, update the internal state variables                           *
     **************************************************************************/
    if (SUCCEEDED(hr))
    {
        mState = SourceStateStopped;
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    SelectStreams
 *  @brief Activate the streams exposed by the source
 *
 *  param[in]   pPresentationDescriptor : presentation descriptor
 *  param[in]   varStart                : variable for playing the stream
 *  param[in]   isSeek                  : enable/disable seek
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::selectStreams(
                IMFPresentationDescriptor *pPresentationDescriptor,
                const PROPVARIANT varStart, bool isSeek)
{
    HRESULT hr = S_OK;
    BOOL selected = FALSE;
    bool wasSelected = false;
    DWORD streamId = 0;

    CComPtr < IMFStreamDescriptor > pStreamDescriptor;
    CComPtr < IUnknown > pUnkStream;

    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    for (DWORD x = 0; x < mMediaStreams.size(); x++)
    {
        CustomStream* pStream = NULL;

        pStream = mMediaStreams[x];
        RETURNIFFAILED(hr);
        if (nullptr == pStream)
        {
            return E_UNEXPECTED;
        }

        pStreamDescriptor = NULL;

        /**********************************************************************
         * get a stream descriptor for the current stream from the            *
         * presentation descriptor                                            *
         **********************************************************************/
        hr = pPresentationDescriptor->GetStreamDescriptorByIndex(x, &selected,
                        &pStreamDescriptor);
        RETURNIFFAILED(hr);

        /**********************************************************************
         * get the stream ID from the stream descriptor                       *
         **********************************************************************/
        hr = pStreamDescriptor->GetStreamIdentifier(&streamId);
        RETURNIFFAILED(hr);

        if (pStream == NULL)
        {
            hr = E_INVALIDARG;
            return hr;
        }

        /**********************************************************************
         * figure out if the current stream was already selected - IE if it   *
         * was active                                                         *
         **********************************************************************/
        wasSelected = pStream->isActive();

        /**********************************************************************
         * activate the stream                                                *
         **********************************************************************/
        pStream->activate(selected == TRUE);

        /**********************************************************************
         * get the IUnknown pointer for the CustomStream object               *
         **********************************************************************/
        pUnkStream = pStream;

        /**********************************************************************
         * if the stream was selected queue the an event with the result of   *
         * the operation                                                      *
         **********************************************************************/
        if (selected)
        {
            /******************************************************************
             * if the stream was already selected, something must have been   *
             * updated - fire the MEUpdatedStream event                       *
             ******************************************************************/
            if (wasSelected)
            {
                hr = mPEventQueue->QueueEventParamUnk(MEUpdatedStream, // event - the stream was updated
                                GUID_NULL, // basic event - no extension GUID
                                hr, // result of the operation
                                pUnkStream); // pointer to the stream
            }
            else
            {
                hr = mPEventQueue->QueueEventParamUnk(MENewStream, // the new stream was activated
                                GUID_NULL, // basic event - no extension GUID
                                hr, // result of the operation
                                pUnkStream); // pointer to the stream
            }

            /******************************************************************
             * start playing the stream                                       *
             ******************************************************************/
            hr = pStream->start(varStart, isSeek);
            RETURNIFFAILED(hr);
        }
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    InternalCreatePresentationDescriptor
 *  @brief Create the presentation descriptor object
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::internalCreatePresentationDescriptor(void)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    IMFStreamDescriptor* streamDescriptors[2];

    if (nullptr == mPInputFileParser)
    {
        return E_POINTER;
    }

    /**************************************************************************
     * make sure this is a supported file format                              *
     **************************************************************************/
    if (!mPInputFileParser->isSupportedFormat())
    {
        return MF_E_INVALID_FORMAT;
    }

    /**************************************************************************
     * create a video stream descriptor if there is video in the file         *
     **************************************************************************/
    hr = createVideoStream(&(streamDescriptors[mMediaStreams.size()]));
    RETURNIFFAILED(hr);

    /**************************************************************************
     * if we got here, we have successfully created at a stream descriptor a  *
     * video stream (if the file has video). Now create the presentation      *
     * descriptor which will hold the stream descriptors.                     *
     **************************************************************************/
    hr = MFCreatePresentationDescriptor((DWORD) mMediaStreams.size(), // number of streams created
                    streamDescriptors, // array of stream descriptors
                    &mPPresentationDescriptor); // get the presentation descriptor
    RETURNIFFAILED(hr);

    /**************************************************************************
     * activate all of the streams in the beginning - that's their default    *
     * state                                                                  *
     **************************************************************************/
    for (DWORD i = 0; i < mMediaStreams.size(); i++)
    {
        hr = mPPresentationDescriptor->SelectStream(i);
        RETURNIFFAILED(hr);
    }
    RETURNIFFAILED(hr);

    LONGLONG fileDuration = mPInputFileParser->duration();

    /**************************************************************************
     * set the file duration on the presentation descriptor - length of the   *
     * file in 100-ns units                                                   *
     **************************************************************************/
    hr = mPPresentationDescriptor->SetUINT64(MF_PD_DURATION, (UINT64)(
                    fileDuration));

    /**************************************************************************
     * all of the stream descriptors have now been stored in the presentation *
     * descriptor - therefore release all of the stream descriptor pointers   *
     * we have left over                                                      *
     **************************************************************************/
    for (DWORD i = 0; i < mMediaStreams.size(); i++)
    {
        if (streamDescriptors[i])
        {
            streamDescriptors[i]->Release();
            streamDescriptors[i] = NULL;
        }
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    CreateVideoStream
 *  @brief Create the video stream and return the corresponding stream
 *         descriptor
 *
 *  param[out]   ppStreamDescriptor : get the video stream descriptor
 *
 *  @return HRESULT : S_OK if successful; otherwise error code.
 *******************************************************************************
 */
HRESULT CustomSource::createVideoStream(
                IMFStreamDescriptor** ppStreamDescriptor)
{
    HRESULT hr = S_OK;

    IMFMediaType* pMediaType = NULL;
    CustomStream* pAVFStream = NULL;

    /**************************************************************************
     * create an CustomStream object and a stream descriptor for it           *
     **************************************************************************/

    CComPtr < IMFMediaTypeHandler > pHandler;

    /**************************************************************************
     * get the media type for the video stream                                *
     **************************************************************************/
    hr = mPInputFileParser->getVideoMediaType(&pMediaType);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * create the stream descriptor                                           *
     **************************************************************************/
    hr = MFCreateStreamDescriptor((DWORD) mMediaStreams.size() + 1, // stream ID
                    1, // number of media types
                    &pMediaType, // media type for the stream
                    ppStreamDescriptor); // get the descriptor
    RETURNIFFAILED(hr);

    /**************************************************************************
     * get a media type handler for the stream                                *
     **************************************************************************/
    hr = (*ppStreamDescriptor)->GetMediaTypeHandler(&pHandler);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * set current type of the stream visible to source users                 *
     **************************************************************************/
    hr = pHandler->SetCurrentMediaType(pMediaType);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * Create CustomStream object that is implementing the IMFMediaStream     *
     * interface                                                              *
     **************************************************************************/
    hr = CustomStream::createInstance(&pAVFStream, this, *ppStreamDescriptor);
    RETURNIFFAILED(hr);

    /**************************************************************************
     * store the stream in a vector for later reuse                           *
     **************************************************************************/
    EXCEPTIONTOHR(mMediaStreams.push_back(pAVFStream));

    if (pMediaType)
    {
        pMediaType->Release();
        pMediaType = NULL;
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn    CustomSource
 *  @brief Constructor
 *
 *  @param[out]  pHr : HRESULT
 *******************************************************************************
 */
CustomSource::CustomSource(HRESULT* pHr) :
    mCRef(1), mPInputFileParser(NULL), mState(SourceStateUninitialized),
                    mPendingEndOfStream(0)
{
    /**************************************************************************
     * Initialize the event queue that will execute all of the source's       *
     * IMFEventGenerator duties.                                              *
     **************************************************************************/
    *pHr = MFCreateEventQueue(&mPEventQueue);
}

/**
 *******************************************************************************
 *  @fn    ~CustomSource
 *  @brief Destructor
 *
 *******************************************************************************
 */
CustomSource::~CustomSource()
{
    for (DWORD x = 0; x < mMediaStreams.size(); x++)
    {
        CustomStream* pStream = NULL;

        pStream = mMediaStreams[x];

        pStream->shutdown();
        pStream->Release();
    }

    if (NULL != mPInputFileParser)
    {
        delete mPInputFileParser;
        mPInputFileParser = NULL;
    }
}