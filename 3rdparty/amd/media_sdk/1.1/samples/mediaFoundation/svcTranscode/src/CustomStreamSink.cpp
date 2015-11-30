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
 * @file <CustomStreamSink.cpp>                          
 *                                       
 * @brief This file defines class necessary for Stream Sink
 *         
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include "CustomStreamSink.h"
#include "CustomMediaSink.h"
/** 
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  IUnknown methods
 *          
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CustomStreamSink::AddRef()
{
    /*long uCount = */
    InterlockedIncrement(&mNRefCount);

    return mPtrSink->AddRef();
}
/** 
 *******************************************************************************
 *  @fn     Release
 *  @brief  IUnknown methods
 *  
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CustomStreamSink::Release()
{
    /*long uCount = */
    InterlockedDecrement(&mNRefCount);

    return mPtrSink->Release();
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
HRESULT CustomStreamSink::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*> (static_cast<IMFStreamSink*> (this));
    }
    else if (iid == __uuidof(IMFStreamSink))
    {
        *ppv = static_cast<IMFStreamSink *> (this);
    }
    else if (iid == __uuidof(IMFMediaEventGenerator))
    {
        *ppv = static_cast<IMFMediaEventGenerator*> (this);
    }
    else if (iid == __uuidof(IMFMediaTypeHandler))
    {
        *ppv = static_cast<IMFMediaTypeHandler*> (this);
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
 *  @fn     BeginGetEvent
 *  @brief  IMFMediaEventGenerator methods.Begins an asynchronous request for 
 *          the next event in the queue.
 *           
 *  @param[in] pCallback : Pointer to the IMFAsyncCallback interface of a 
 *                         callback object. 
 *  @param[in] punkState : Pointer to the IUnknown interface of a state object,
 *                         defined by the caller. This parameter can be NULL.
 *                         You can use this object to hold state information. 
 *                         The object is returned to the caller when the 
 *                         callback is invoked
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::BeginGetEvent(IMFAsyncCallback* pCallback,
                IUnknown* punkState)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    hr = checkShutdown();
    RETURNIFFAILED(hr);

    hr = mPtrEventQueue->BeginGetEvent(pCallback, punkState);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     EndGetEvent
 *  @brief  Completes an asynchronous request for the next event in the queue. 
 *           
 *  @param[in] pResult : Pointer to the IMFAsyncResult interface. 
 *  @param[out] ppEvent: Receives a pointer to the IMFMediaEvent interface.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::EndGetEvent(IMFAsyncResult* pResult,
                IMFMediaEvent** ppEvent)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    hr = checkShutdown();
    RETURNIFFAILED(hr);

    hr = mPtrEventQueue->EndGetEvent(pResult, ppEvent);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetEvent
 *  @brief  Retrieves the next event in the queue. This method is synchronous. 
 *           
 *  @param[in] dwFlags : 0 -The method blocks until the event generator 
 *                          queues an event.
 *                       MF_EVENT_FLAG_NO_WAIT - The method returns immediately
 *  @param[out] ppEvent : Receives a pointer to the IMFMediaEvent interface
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    /***************************************************************************
     * GetEvent can block indefinitely, so we don't hold the lock.              *
     * This requires some juggling with the event queue pointer.                *
     ***************************************************************************/

    HRESULT hr = S_OK;
    CComPtr < IMFMediaEventQueue > ptrQueue = NULL;

    {// scope for lock
        CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                        mStateLock, true);

        hr = checkShutdown();
        RETURNIFFAILED(hr);

        ptrQueue = mPtrEventQueue;

    }// release lock

    hr = ptrQueue->GetEvent(dwFlags, ppEvent);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     QueueEvent
 *  @brief  Puts a new event in the object's queue.
 *           
 *  @param[in] met              : Specifies event type
 *  @param[in] guidExtendedType : The extended type. If the event does not 
 *                                 have an extended type, use the value GUID_NULL
 *  @param[in] hrStatus         : status of the event 
 *  @param[in] pvValue          : Pointer to a PROPVARIANT that contains the event value.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::QueueEvent(MediaEventType met,
                REFGUID guidExtendedType, HRESULT hrStatus,
                const PROPVARIANT* pvValue)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    hr = checkShutdown();
    RETURNIFFAILED(hr);

    hr = mPtrEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus,
                    pvValue);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetMediaSink
 *  @brief  IMFStreamSink methods. Retrieves the media sink that owns this 
 *          stream sink..
 *           
 *  @param[out] ppMediaSink : Receives a pointer to the media sink's IMFMediaSink 
 *                           interface.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::GetMediaSink(IMFMediaSink **ppMediaSink)
{
    if (ppMediaSink == NULL)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    *ppMediaSink = mPtrSink;
    (*ppMediaSink)->AddRef();

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetIdentifier
 *  @brief  Retrieves the stream identifier for this stream sink.
 *           
 *  @param[out] ppMediaSink : Receives the stream identifier.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::GetIdentifier(DWORD *pdwIdentifier)
{
    if (pdwIdentifier == NULL)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * Return 1, because we have just one stream                                *
     ***************************************************************************/
    *pdwIdentifier = 1;

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetMediaTypeHandler
 *  @brief  Retrieves the media type handler for the stream sink. You can use 
 *          the media type handler to find which formats the stream supports, 
 *          and to set the media type on the stream.
 *           
 *  @param[out] ppHandler : Receives a pointer to the IMFMediaTypeHandler interface
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::GetMediaTypeHandler(IMFMediaTypeHandler **ppHandler)
{
    if (ppHandler == NULL)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * This stream object acts as its own type handler, so we QI ourselves.     *
     ***************************************************************************/
    hr = this->QueryInterface(IID_IMFMediaTypeHandler, (void**) ppHandler);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     ProcessSample
 *  @brief  Delivers a sample to the stream. The media sink processes the sample.
 *           
 *  @param[out] pSample : Pointer to the IMFSample interface of a sample that
 *                        contains valid data for the stream.
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::ProcessSample(IMFSample *pSample)
{
    if (pSample == NULL)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    hr = checkShutdown();
    RETURNIFFAILED(hr);

    hr = mSampleQueue.InsertBack(pSample);
    RETURNIFFAILED(hr);

    /****************************************************************************
     * Unless we are paused, start an async operation to dispatch the next sample*.
     ****************************************************************************/

    if (mState != StatePaused)
    {
        hr = queueAsyncOperation(OpProcessSample);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     PlaceMarker
 *  @brief  Places a marker in the stream.
 *           
 *  @param[in] eMarkerType     : Specifies the marker type, as a member of the 
 *                               MFSTREAMSINK_MARKER_TYPE enumeration.
 *  @param[in] pvarMarkerValue : Optional pointer to a PROPVARIANT that contains 
 *                               additional information related to the marker
 *  @param[in] pvarContextValue: Optional pointer to a PROPVARIANT that is 
 *                               attached to the MEStreamSinkMarker event
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType,
                const PROPVARIANT *pvarMarkerValue,
                const PROPVARIANT *pvarContextValue)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr;

    hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * Create a marker object and put it on the sample queue.                   *
     ***************************************************************************/

    CComPtr < IMarker > ptrMarker;
    hr = CMarker::create(eMarkerType, pvarMarkerValue, pvarContextValue,
                    &ptrMarker);
    RETURNIFFAILED(hr);

    hr = mSampleQueue.InsertBack(ptrMarker);
    RETURNIFFAILED(hr);

    /****************************************************************************
     *Unless we are paused, start an async operation to dispatch the next marker *
     ****************************************************************************/

    if (mState != StatePaused)
    {
        hr = queueAsyncOperation(OpPlaceMarker);
        RETURNIFFAILED(hr);
    }

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     Flush
 *  @brief  Causes the stream sink to drop any samples that it has received 
 *          and has not rendered yet
 *           
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::Flush()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * Even though we are flushing data, we still need to send                  *
     * any marker events that were queued.                                      *
     ***************************************************************************/
    hr = processSamplesFromQueue(DropSamples);
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     IsMediaTypeSupported
 *  @brief  Queries whether the object supports a specified media type.
 *           
 *  @param[in] pMediaType  : Pointer to the IMFMediaType interface
 *  @param[out] ppMediaType : Pointer to the IMFMediatype,closest match.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::IsMediaTypeSupported(IMFMediaType *pMediaType,
                IMFMediaType **ppMediaType)
{
    if (nullptr == pMediaType)
    {
        return E_POINTER;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * We support all possible video types.                                     *
     ***************************************************************************/

    GUID majorType = GUID_NULL;
    hr = pMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
    RETURNIFFAILED(hr);

    if (majorType != MFMediaType_Video)
    {
        return MF_E_INVALIDTYPE;
    }

    /***************************************************************************
     * We don't return any "close match" types.                                 *
     ***************************************************************************/
    if (ppMediaType)
    {
        *ppMediaType = nullptr;
    }

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     GetMediaTypeCount
 *  @brief  Retrieves the number of media types in the object's list of 
 *          supported media types.
 *           
 *  @param[out] pdwTypeCount  : Number of media types in the list 
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::GetMediaTypeCount(DWORD *pdwTypeCount)
{
    if (pdwTypeCount == NULL)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * App must provide the media type. To test what kind of type is supported  *
     * call IsMediaTypeSupported.                                               *
     ***************************************************************************/
    *pdwTypeCount = 0;

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     GetMediaTypeByIndex
 *  @brief  Retrieves a media type from the object's list of supported media
 *          types.
 *           
 *  @param[in] dwIndex  : index
 *  @param[out] ppType  : Media type corresponding to the index
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::GetMediaTypeByIndex(DWORD /*dwIndex*/, IMFMediaType** /*ppType*/)
{
    /************************************************************************
     * App must provide the media type. To test what kind of type is supported
     * call IsMediaTypeSupported.
     *************************************************************************/
    return E_NOTIMPL;
}
/** 
 *******************************************************************************
 *  @fn     SetCurrentMediaType
 *  @brief  Sets the object's media type.
 *           
 *  @param[in] pMediaType  : Pointer to the IMFMediaType interface of the new
 *                           media type.
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::SetCurrentMediaType(IMFMediaType *pMediaType)
{
    if (pMediaType == NULL)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    hr = IsMediaTypeSupported(pMediaType, NULL);
    RETURNIFFAILED(hr);

    CComPtr < IMFMediaType > newMediaType;
    hr = MFCreateMediaType(&newMediaType);
    RETURNIFFAILED(hr);

    hr = pMediaType->CopyAllItems(newMediaType);
    RETURNIFFAILED(hr);

    mPtrCurrentType.Release();
    mPtrCurrentType = newMediaType;

    mState = StateReady;

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     GetCurrentMediaType
 *  @brief  Retrieves the current media type of the object.
 *           
 *  @param[out] ppMediaType  : Pointer to the IMFMediaType interface
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::GetCurrentMediaType(IMFMediaType **ppMediaType)
{
    if (ppMediaType == NULL)
    {
        return E_INVALIDARG;
    }

    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    if (mPtrCurrentType == NULL)
    {
        return MF_E_NOT_INITIALIZED;
    }

    CComPtr < IMFMediaType > mediaType;
    hr = MFCreateMediaType(&mediaType);
    RETURNIFFAILED(hr);

    mPtrCurrentType->CopyAllItems(mediaType);
    RETURNIFFAILED(hr);

    *ppMediaType = mediaType.Detach();

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     GetMajorType
 *  @brief  Gets the major media type of the object.
 *           
 *  @param[out] pdwTypeCount  :Receives a GUID that identifies the major type
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::GetMajorType(GUID *pguidMajorType)
{
    if (pguidMajorType == NULL)
    {
        return E_INVALIDARG;
    }

    *pguidMajorType = MFMediaType_Video;

    return S_OK;
}

/** 
 *******************************************************************************
 *  @fn     initialize
 *  @brief  Initializes the stream sink object & stores the output file name
 *           
 *  @param[in] pParent        : pointer to the media sink object
 *  @param[in] fileName       : output file name
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::initialize(CustomMediaSink *pParent, LPCWSTR fileName)
{
    if (nullptr == pParent)
    {
        return E_POINTER;
    }

    assert(pParent != NULL);

    HRESULT hr = S_OK;

    hr = MFCreateEventQueue(&mPtrEventQueue);
    RETURNIFFAILED(hr);

    hr = MFAllocateWorkQueue(&mWorkQueueId);
    RETURNIFFAILED(hr);

    mPtrSink = pParent;

    mSFileName = std::string(CW2A(fileName));

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     start
 *  @brief  queues the async operation 
 *           
 *  @param[in] start        : MFTime
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::start(MFTIME start)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    if (start != PRESENTATION_CURRENT_POSITION)
    {
        mStartTime = start;
    }

    mState = StateStarted;
    hr = queueAsyncOperation(OpStart);
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     stop
 *  @brief  stops
 *           
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::stop()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    mState = StateStopped;
    hr = queueAsyncOperation(OpStop);
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     pause
 *  @brief  pause
 *           
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::pause()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    mState = StatePaused;
    hr = queueAsyncOperation(OpPause);
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     restart
 *  @brief  restart
 *           
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::restart()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    hr = queueAsyncOperation(OpRestart);
    RETURNIFFAILED(hr);

    mState = StateStarted;

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     shutdown
 *  @brief  shutdown
 *           
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::shutdown()
{
    assert(!mIsShutdown);
    HRESULT hr = S_OK;

    if (NULL != mPtrEventQueue)
    {
        hr = mPtrEventQueue->Shutdown();
        RETURNIFFAILED(hr);
    }

    hr = MFUnlockWorkQueue(mWorkQueueId);
    RETURNIFFAILED(hr);

    mSampleQueue.Clear();

    mPtrEventQueue.Release();
    mPtrCurrentType.Release();

    mIsShutdown = TRUE;

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     queueAsyncOperation
 *  @brief  queues the async operation 
 *           
 *  @param[in] op        : enum for operation        
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::queueAsyncOperation(StreamOperation op)
{
    HRESULT hr = S_OK;
    CAsyncOperation* pOp = new CAsyncOperation(op);
    CComPtr < CAsyncOperation > ptrOp(pOp);

    if (ptrOp == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = MFPutWorkItem(mWorkQueueId, &mWorkQueueCb, ptrOp);
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     onDispatchWorkItem
 *  @brief  process the workitem depending on the operation 
 *           
 *  @param[in] pAsyncResult : Provides information about the result of an 
 *                            asynchronous operation.
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::onDispatchWorkItem(IMFAsyncResult* pAsyncResult)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = S_OK;

    CComPtr < IUnknown > pState;
    hr = pAsyncResult->GetState(&pState);
    RETURNIFFAILED(hr);

    /***************************************************************************
     * The state object is a CAsncOperation object.                             *
     ***************************************************************************/
    CAsyncOperation *pOp = (CAsyncOperation*) (pState.p);
    StreamOperation op = pOp->mOp;

    switch (op)
    {
    case OpStart:
    case OpRestart:
        hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL);
        RETURNIFFAILED(hr);

        hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL);
        RETURNIFFAILED(hr);

        hr = processSamplesFromQueue(WriteSamples);
        RETURNIFFAILED(hr);
        break;

    case OpStop:
        hr = processSamplesFromQueue(DropSamples);
        RETURNIFFAILED(hr);

        hr = QueueEvent(MEStreamSinkStopped, GUID_NULL, hr, NULL);
        RETURNIFFAILED(hr);
        break;

    case OpPause:
        hr = QueueEvent(MEStreamSinkPaused, GUID_NULL, hr, NULL);
        RETURNIFFAILED(hr);
        break;

    case OpProcessSample:
        hr = processSamplesFromQueue(WriteSamples);
        RETURNIFFAILED(hr);

        hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, S_OK, NULL);
        RETURNIFFAILED(hr);
        break;
    case OpPlaceMarker:
        /***********************************************************************
         * Check specific type(sample or marker) of sample inside.              *
         ***********************************************************************/
        hr = processSamplesFromQueue(WriteSamples);
        RETURNIFFAILED(hr);
        break;
    }

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     processSamplesFromQueue
 *  @brief  process the workitem depending on the operation 
 *
 *  @param[in] bFlushData : flag for drop samples or write sample to the file
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::processSamplesFromQueue(FlushState bFlushData)
{
    ComPtrList<IUnknown>::POSITION pos = mSampleQueue.FrontPosition();

    HRESULT hr = S_OK;

    while (pos != mSampleQueue.EndPosition())
    {
        CComPtr < IUnknown > ptrUnk;
        CComPtr < IMarker > ptrMarker;
        CComPtr < IMFSample > ptrSample;

        hr = mSampleQueue.GetItemPos(pos, &ptrUnk);

        /***********************************************************************
         * Figure out if this is a marker or a sample.                          *
         ***********************************************************************/
        if (SUCCEEDED(hr))
        {
            hr = ptrUnk->QueryInterface(&ptrMarker);
            if (hr == E_NOINTERFACE)
            {
                hr = ptrUnk->QueryInterface(&ptrSample);
            }
        }

        if (SUCCEEDED(hr))
        {
            if (ptrMarker)
            {
                hr = sendMarkerEvent(ptrMarker, bFlushData);
            }
            else
            {
                assert(ptrSample != NULL);

                if (bFlushData == WriteSamples)
                {
                    hr = writeSampleToFile(ptrSample);
                }
            }
        }

        if (FAILED(hr))
        {
            break;
        }

        pos = mSampleQueue.Next(pos);

    }

    /***************************************************************************
     * We process all of the samples. Clear queue.                              *
     ***************************************************************************/
    mSampleQueue.Clear();

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     writeSampleToFile
 *  @brief  Writes the sample to the file
 *
 *  @param[in] pSample : pointer to the IMFSample interface
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::writeSampleToFile(IMFSample *pSample)
{
    HRESULT hr;

    LONGLONG time = 0;
    hr = pSample->GetSampleTime(&time);
    RETURNIFFAILED(hr);

    /***************************************************************************
     * If the time stamp is too early, just discard this sample.                *
     ***************************************************************************/
    if (time <= mStartTime)
    {
        return S_OK;
    }

    DWORD cBufferCount = 0;
    hr = pSample->GetBufferCount(&cBufferCount);
    RETURNIFFAILED(hr);

    FILE* f = NULL;
    for (DWORD iBuffer = 0; iBuffer < cBufferCount; ++iBuffer)
    {
        CComPtr < IMFMediaBuffer > ptrBuffer;
        hr = pSample->GetBufferByIndex(iBuffer, &ptrBuffer);
        RETURNIFFAILED(hr);

        BYTE *pData = NULL;
        DWORD cbData = 0;
        hr = ptrBuffer->Lock(&pData, NULL, &cbData);
        RETURNIFFAILED(hr);

        /***************************************************************************
         * Write bitstream to the file as is.                                       *
         ***************************************************************************/
        /*errno_t err = */
        (!mCbDataWritten) ? fopen_s(&f, mSFileName.c_str(), "wb") : fopen_s(&f,
                        mSFileName.c_str(), "ab");
        fwrite(pData, cbData, 1, f);
        fclose(f);
        mCbDataWritten += cbData;
    }

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     sendMarkerEvent
 *  @brief  Writes the sample to the file
 *
 *  @param[in] pMarker : pointer to the IMarker interface
 *  @param[in] bFlushData : flag for drop samples or write sample to the file
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomStreamSink::sendMarkerEvent(IMarker *pMarker,
                FlushState bFlushData)
{
    HRESULT hr = S_OK;
    HRESULT hrStatus = S_OK;

    if (bFlushData == DropSamples)
    {
        hrStatus = E_ABORT;
    }

    PROPVARIANT var;
    PropVariantInit(&var);

    hr = pMarker->GetContext(&var);
    RETURNIFFAILED(hr);

    hr = QueueEvent(MEStreamSinkMarker, GUID_NULL, hrStatus, &var);

    PropVariantClear(&var);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     CAsyncOperation
 *  @brief  CAsyncOperation Methods.Constructor
 *
 *          
 *******************************************************************************
 */
CustomStreamSink::CAsyncOperation::CAsyncOperation(StreamOperation op) :
    mNRefCount(0), mOp(op)
{
}
/** 
 *******************************************************************************
 *  @fn     CAsyncOperation
 *  @brief  CAsyncOperation Methods.Constructor
 *
 *          
 *******************************************************************************
 */

CustomStreamSink::CAsyncOperation::~CAsyncOperation()
{
    assert(mNRefCount == 0);
}
/** 
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  IUnknown methods
 *          
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CustomStreamSink::CAsyncOperation::AddRef()
{
    return InterlockedIncrement(&mNRefCount);
}
/** 
 *******************************************************************************
 *  @fn     Release
 *  @brief  IUnknown methods
 *          
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CustomStreamSink::CAsyncOperation::Release()
{
    ULONG uCount = InterlockedDecrement(&mNRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    return uCount;
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
HRESULT CustomStreamSink::CAsyncOperation::QueryInterface(REFIID iid,
                void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*> (this);
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
 *  @fn     CustomStreamSink
 *  @brief  CustomStreamSink .Constructor
 *
 *          
 *******************************************************************************
 */
CustomStreamSink::CustomStreamSink() :
    mPtrSink(nullptr), mNRefCount(0), mState(StateTypeNotSet), mIsShutdown(
                    FALSE), mStartTime(0), mCbDataWritten(0), mWorkQueueId(0),
                    mWorkQueueCb(this, &CustomStreamSink::onDispatchWorkItem)
{
    mSFileName = std::string("dump.bin");
}
/** 
 *******************************************************************************
 *  @fn     CustomStreamSink
 *  @brief  CustomStreamSink  Destructor
 *
 *          
 *******************************************************************************
 */
CustomStreamSink::~CustomStreamSink()
{
}
/** 
 *******************************************************************************
 *  @fn     CMarker
 *  @brief  CMarker Constsructor.
 *
 *  
 *******************************************************************************
 */
CMarker::CMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType) :
    mNRefCount(1), mEMarkerType(eMarkerType)
{
    PropVariantInit(&mVarMarkerValue);
    PropVariantInit(&mVarContextValue);
}
/** 
 *******************************************************************************
 *  @fn     CustomStreamSink
 *  @brief  CustomStreamSink  Destructor
 *
 *          
 *******************************************************************************
 */
CMarker::~CMarker()
{
    assert(mNRefCount == 0);

    PropVariantClear(&mVarMarkerValue);
    PropVariantClear(&mVarContextValue);
}
/** 
 *******************************************************************************
 *  @fn     create
 *  @brief  Creates marker instance 
 *
 *  @param[in] eMarkerType :  Marker type
 *  @param[in] pvarMarkerValue : Marker value
 *  @param[in] pvarContextValue : Context value 
 *  @param[out] pvarMarkerValue : Pointer to the marker created
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CMarker::create(MFSTREAMSINK_MARKER_TYPE eMarkerType,
                const PROPVARIANT* pvarMarkerValue,
                const PROPVARIANT* pvarContextValue, IMarker **ppMarker)
{
    if (ppMarker == NULL)
    {
        return E_POINTER;
    }

    CComPtr < CMarker > ptrMarker(new CMarker(eMarkerType));
    if (ptrMarker == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = S_OK;

    if (pvarMarkerValue)
    {
        hr = PropVariantCopy(&(ptrMarker->mVarMarkerValue), pvarMarkerValue);
        RETURNIFFAILED(hr);
    }

    if (pvarContextValue)
    {
        hr = PropVariantCopy(&(ptrMarker->mVarContextValue), pvarContextValue);
        RETURNIFFAILED(hr);
    }

    *ppMarker = ptrMarker.Detach();

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
ULONG CMarker::AddRef()
{
    return InterlockedIncrement(&mNRefCount);
}
/** 
 *******************************************************************************
 *  @fn     Release
 *  @brief  IUnknown methods
 *          
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CMarker::Release()
{
    ULONG uCount = InterlockedDecrement(&mNRefCount);
    if (uCount == 0)
    {
        delete this;
    }

    return uCount;
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
HRESULT CMarker::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*> (this);
    }
    else if (iid == __uuidof(IMarker))
    {
        *ppv = static_cast<IMarker*> (this);
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
 *  @fn     GetMarkerType
 *  @brief  IMarker Methods. Returns the marker type
 *
 *  @param[out] pType :  Marker type
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CMarker::GetMarkerType(MFSTREAMSINK_MARKER_TYPE *pType)
{
    if (pType == NULL)
    {
        return E_POINTER;
    }

    *pType = mEMarkerType;

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     GetMarkerValue
 *  @brief  IMarker Methods. Returns the marker value
 *
 *  @param[out] pvar :  Marker value
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CMarker::GetMarkerValue(PROPVARIANT *pvar)
{
    if (pvar == NULL)
    {
        return E_POINTER;
    }

    return PropVariantCopy(pvar, &mVarMarkerValue);
}
/** 
 *******************************************************************************
 *  @fn     GetContext
 *  @brief  IMarker Methods. Returns the marker context
 *
 *  @param[out] pvar :  Marker context
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CMarker::GetContext(PROPVARIANT *pvar)
{
    if (pvar == NULL)
    {
        return E_POINTER;
    }

    return PropVariantCopy(pvar, &mVarContextValue);
}
