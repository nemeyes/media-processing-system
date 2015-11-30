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
 * @file <CustomStream.h>
 *
 * @brief This file contains implementation of CustomStream class
 *
 ********************************************************************************
 */
#include "CustomStream.h"
#include "CustomSource.h"

/**
 *******************************************************************************
 *  @fn     CreateInstance
 *  @brief  Create an instance of the CustomStream object with the specified
 *          presentation descriptor
 *
 *  @param[out] ppMediaStream      : pointer to the media stream
 *  @param[in]  pMediaSource       : pointer to the media source
 *  @param[in]  pStreamDescriptor  : pointer to the stream descriptor
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT CustomStream::createInstance(CustomStream** ppMediaStream,
                CustomSource *pMediaSource,
                IMFStreamDescriptor *pStreamDescriptor)
{
    HRESULT hr = S_OK;
    CustomStream* pMediaStream = NULL;

    do
    {
        if (nullptr == ppMediaStream)
        {
            return E_POINTER;
        }

        if (nullptr == pMediaSource)
        {
            return E_INVALIDARG;
        }

        if (nullptr == pStreamDescriptor)
        {
            return E_INVALIDARG;
        }

        *ppMediaStream = NULL;

        /**********************************************************************
         * create a new stream object                                         *
         **********************************************************************/
        pMediaStream = new CustomStream();
        if (nullptr == pMediaStream)
        {
            return E_OUTOFMEMORY;
        }

        /**********************************************************************
         * initialize the stream with the stream descriptor                   *
         **********************************************************************/
        hr = pMediaStream->init(pMediaSource, pStreamDescriptor);
        RETURNIFFAILED(hr);

        /**********************************************************************
         * set the output pointer to the new stream                           *
         **********************************************************************/
        *ppMediaStream = pMediaStream;
    } while (false);

    if (FAILED(hr))
    {
        delete pMediaStream;
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  IUnknown interface implementation
 *
 *  @return ULONG : refCount
 *******************************************************************************
 */
ULONG CustomStream::AddRef()
{
    return InterlockedIncrement(&mCRef);
}

/**
 *******************************************************************************
 *  @fn     Release
 *  @brief  IUnknown interface implementation
 *
 *  @return ULONG : refCount
 *******************************************************************************
 */
ULONG CustomStream::Release()
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
 *  @fn
 *  @brief  IUnknown interface implementation
 *
 *  @param[in]  riid : interface identifier
 *  @param[out] ppv : interface pointer requested in the riid
 *
 *  @return HRESULT : S_OK if successful; else E_NONINTERFACE
 *******************************************************************************
 */
HRESULT CustomStream::QueryInterface(REFIID riid, void** ppv)
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
    else if (riid == IID_IMFMediaEventGenerator)
    {
        *ppv = static_cast<IMFMediaEventGenerator*> (this);
    }
    else if (riid == IID_IMFMediaStream)
    {
        *ppv = static_cast<IMFMediaStream*> (this);
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

/**
 *******************************************************************************
 *  @fn     BeginGetEvent
 *  @brief  Begin asynchronous event processing of the next event in the queue
 *          (part of IMFMediaEventGenerator interface implementation)
 *
 *  @param[in]  pCallback : callback of the object interested in events
 *  @param[in]  punkState : some custom state object returned with event
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::BeginGetEvent(IMFAsyncCallback* pCallback,
                IUnknown* punkState)
{
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    return mPEventQueue->BeginGetEvent(pCallback, punkState);
}

/**
 *******************************************************************************
 *  @fn     EndGetEvent
 *  @brief  Complete asynchronous event processing
 *          (part of IMFMediaEventGenerator interface implementation)
 *
 *  @param[in]  pResult : pointer to the IMFAsyncResult interface
 *  @param[out] ppEvent : pointer to the IMFMediaEvent interface
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::EndGetEvent(IMFAsyncResult* pResult,
                IMFMediaEvent** ppEvent)
{
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    return mPEventQueue->EndGetEvent(pResult, ppEvent);
}

/**
 *******************************************************************************
 *  @fn     GetEvent
 *  @brief  Get the next event in the event queue
 *          (part of IMFMediaEventGenerator interface implementation)
 *
 *  @param[in]  dwFlags : specifies one of the following values
 *                        0 - blocks until event generator queues an event
 *                        MF_EVENT_FLAG_NO_WAIT - returns immediately
 *  @param[out] ppEvent : pointer to the IMFMediaEvent interface
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    return mPEventQueue->GetEvent(dwFlags, ppEvent);
}

/**
 *******************************************************************************
 *  @fn     QueueEvent
 *  @brief  Add a new event to the event queue
 *          (part of IMFMediaEventGenerator interface implementation)
 *
 *  @param[in]  met              : Event type
 *  @param[in]  guidExtendedType : Extended type
 *  @param[in]  hrStatus         : status of the event
 *  @param[in]  pvValue          : pointer to the PROPVARIANT that contains
 *                                 event value
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType,
                HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    return mPEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus,
                    pvValue);
}

/**
 *******************************************************************************
 *  @fn     GetMediaSource
 *  @brief  Get the source associated with this stream
 *          (IMFMediaStream interface implementation)
 *
 *  @param[out]  ppMediaSource : pointer to the IMFMediaSource interface
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::GetMediaSource(IMFMediaSource** ppMediaSource)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        if ((nullptr == ppMediaSource) || (nullptr == mPMediaSource))
        {
            return E_POINTER;
        }

        hr = mPMediaSource->QueryInterface(IID_IMFMediaSource,
                        (void**) ppMediaSource);
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     GetStreamDescriptor
 *  @brief  Get the stream descriptor for this stream
 *          (IMFMediaStream interface implementation)
 *
 *  @param[out]  ppStreamDescriptor : pointer to the IMFStreamDescriptor
 *                                    interface
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::GetStreamDescriptor(
                IMFStreamDescriptor** ppStreamDescriptor)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        if (nullptr == ppStreamDescriptor)
        {
            return E_POINTER;
        }
        if (nullptr == mPStreamDescriptor)
        {
            return E_UNEXPECTED;
        }

        hr = mPStreamDescriptor.CopyTo(ppStreamDescriptor);
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     RequestSample
 *  @brief  Request the next sample from the stream.
 *          (IMFMediaStream interface implementation)
 *
 *  @param[in]  pToken : pointer to the IUnknown interface
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::RequestSample(IUnknown* pToken)
{
    HRESULT hr = S_OK;

    do
    {
        CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

        /**********************************************************************
         * make sure the stream is not shut down                              *
         **********************************************************************/
        hr = checkShutdown();
        RETURNIFFAILED(hr);

        /**********************************************************************
         * make sure the stream is not stopped                                *
         **********************************************************************/
        if (mState == SourceStateStopped)
        {
            return MF_E_MEDIA_SOURCE_WRONGSTATE;
        }

        /**********************************************************************
         * check for the end of stream - fire an end of stream event only if  *
         * there are no more samples, and we received an end of stream        *
         * notification                                                       *
         **********************************************************************/
        if (mEndOfStream && mPSampleList.IsEmpty())
        {
            return MF_E_END_OF_STREAM;
        }

        /**********************************************************************
         * Add the token to the CInterfaceList even if it is NULL             *
         **********************************************************************/
        EXCEPTIONTOHR(mPTokenList.AddTail(pToken));
        RETURNIFFAILED(hr);

        /**********************************************************************
         * increment the number of requested samples                          *
         **********************************************************************/
        mNSamplesRequested++;

        /**********************************************************************
         * dispatch the samples                                               *
         **********************************************************************/
        hr = dispatchSamples();
    } while (false);

    /**************************************************************************
     * if something failed and we are not shut down, fire an event indicating *
     * the error                                                              *
     **************************************************************************/
    if (FAILED(hr) && (mState != SourceStateShutdown))
    {
        hr = mPMediaSource->QueueEvent(MEError, GUID_NULL, hr, NULL);
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn     dispatchSamples
 *  @brief  Dispatch samples stored in the stream object, and request samples
 *          if more are needed.
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::dispatchSamples(void)
{
    HRESULT hr = S_OK;

    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        /**********************************************************************
         * if the stream is not started, just exit                            *
         **********************************************************************/
        if (mState != SourceStateStarted)
        {
            return S_OK;
        }

        /**********************************************************************
         * send out the samples                                               *
         **********************************************************************/
        hr = sendSamplesOut();
        RETURNIFFAILED(hr);

        /**********************************************************************
         * if there are no more samples stored in the stream, and if we have  *
         * been notified that this is the end of stream, send the end of      *
         * stream events.  Otherwise, if the stream needs more data, request  *
         * additional data from the source.                                   *
         **********************************************************************/
        if (mPSampleList.IsEmpty() && mEndOfStream)
        {
            /******************************************************************
             * send the end of stream event to anyone listening to this stream*
             ******************************************************************/
            hr = mPEventQueue->QueueEventParamVar(MEEndOfStream, GUID_NULL,
                            S_OK, NULL);
            RETURNIFFAILED(hr);

            /******************************************************************
             * tell the source that the end of stream has been reached        *
             ******************************************************************/
            hr = mPMediaSource->sendOperation(SourceOperationEndOfStream);
            RETURNIFFAILED(hr);
        }
        else if (needsData())
        {
            /******************************************************************
             * send an event to the source indicating that a stream needs     *
             * more data                                                      *
             ******************************************************************/
            hr = mPMediaSource->sendOperation(SourceOperationStreamNeedData);
            RETURNIFFAILED(hr);
        }
    } while (false);

    /**************************************************************************
     * if there was a failure, queue an MEError event                         *
     **************************************************************************/
    if (FAILED(hr) && (mState != SourceStateShutdown))
    {
        mPMediaSource->QueueEvent(MEError, GUID_NULL, hr, NULL);
    }

    return hr;
}

/**
 *******************************************************************************
 *  @fn     sendSamplesOut
 *  @brief  Send out events with samples
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::sendSamplesOut(void)
{
    HRESULT hr = S_OK;
    CComPtr < IUnknown > pUnkSample;
    CComPtr < IMFSample > pSample;
    CComPtr < IUnknown > pToken;

    do
    {
        /**********************************************************************
         * loop while there are samples in the stream object, and while       *
         * samples have been requested                                        *
         **********************************************************************/
        while (!mPSampleList.IsEmpty() && mNSamplesRequested > 0)
        {
            /******************************************************************
             * reset the pUnkSample variable                                  *
             ******************************************************************/
            pUnkSample = NULL;

            /******************************************************************
             * get the next sample and a sample token                         *
             ******************************************************************/
            EXCEPTIONTOHR(pSample = mPSampleList.RemoveHead());
            RETURNIFFAILED(hr);

            /******************************************************************
             * if there are tokens, then get one, associate it with the sample*
             ******************************************************************/
            if (!mPTokenList.IsEmpty())
            {
                EXCEPTIONTOHR(pToken = mPTokenList.RemoveHead());
                RETURNIFFAILED(hr);

                /**************************************************************
                 * if there is a sample token, store it in the sample         *
                 **************************************************************/
                hr = pSample->SetUnknown(MFSampleExtension_Token, pToken);
                RETURNIFFAILED(hr);
            }

            /******************************************************************
             * get the IUnknown pointer for the sample                        *
             ******************************************************************/
            pUnkSample = pSample;

            /******************************************************************
             * queue an event indicating that a new sample is available, and  *
             * pass it a pointer to  the sample                               *
             ******************************************************************/
            hr = mPEventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL,
                            S_OK, pUnkSample);
            RETURNIFFAILED(hr);

            /******************************************************************
             * decrement the counter indicating how many samples have been    *
             * requested                                                      *
             ******************************************************************/
            mNSamplesRequested--;
        }
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     deliverSample
 *  @brief  Receive a sample for the stream, and immediately dispatch it.
 *
 *  @param[in]  pSample : pointer to the IMFSample interface
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::deliverSample(IMFSample *pSample)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        /**********************************************************************
         * store the sample in the sample list                                *
         **********************************************************************/
        EXCEPTIONTOHR(mPSampleList.AddTail(pSample));
        RETURNIFFAILED(hr);

        /**********************************************************************
         * Call the sample dispatching function.                              *
         **********************************************************************/
        hr = dispatchSamples();
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     activate
 *  @brief  Activate or deactivate the stream
 *
 *  @param[in]  active : flag for activating/deactivating the stream
 *
 *  @return
 *******************************************************************************
 */
void CustomStream::activate(bool active)
{
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * if the stream is already active, do nothing                            *
     **************************************************************************/
    if (mActive == active)
    {
        return;
    }

    /**************************************************************************
     * store the activation state of the stream                               *
     **************************************************************************/
    mActive = active;

    /**************************************************************************
     * if the stream has been deactivated, release all samples and tokens     *
     * associated with it                                                     *
     **************************************************************************/

    if (!mActive)
    {
        /**********************************************************************
         * release all samples and tokens                                     *
         **********************************************************************/
        mPSampleList.RemoveAll();
        mPTokenList.RemoveAll();
    }
}

/**
 *******************************************************************************
 *  @fn     start
 *  @brief  Start or seek the stream
 *
 *  @param[out] varStart :
 *  @param[in]  isSeek   : checking whether the request is seek or start
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::start(const PROPVARIANT& varStart, bool isSeek)
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        hr = checkShutdown();
        RETURNIFFAILED(hr);

        /**********************************************************************
         * if the this is a seek request, queue a seek successfull event -    *
         * if it is a start  request queue the start successfull event        *
         **********************************************************************/
        if (isSeek)
        {
            hr = QueueEvent(MEStreamSeeked, GUID_NULL, S_OK, &varStart);
        }
        else
        {
            hr = QueueEvent(MEStreamStarted, GUID_NULL, S_OK, &varStart);
        }
        RETURNIFFAILED(hr);

        /**********************************************************************
         * update the internal state variable                                 *
         **********************************************************************/
        mState = SourceStateStarted;

        /**********************************************************************
         * send the samples stored in the stream                              *
         **********************************************************************/
        hr = dispatchSamples();
        RETURNIFFAILED(hr);
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     pause
 *  @brief  Pause the stream
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::pause()
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        hr = checkShutdown();
        RETURNIFFAILED(hr);

        /**********************************************************************
         * update the internal state variable                                 *
         **********************************************************************/
        mState = SourceStatePaused;

        /**********************************************************************
         * fire a stream paused event                                         *
         **********************************************************************/
        hr = QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL);
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     stop
 *  @brief  Stop the stream
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::stop()
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        hr = checkShutdown();
        RETURNIFFAILED(hr);

        /**********************************************************************
         * release all of the samples associated with the stream              *
         **********************************************************************/
        mPSampleList.RemoveAll();
        mPTokenList.RemoveAll();

        /**********************************************************************
         * update the internal state variable                                 *
         **********************************************************************/
        mState = SourceStateStopped;

        /**********************************************************************
         * queue an event indicating that we stopped successfully             *
         **********************************************************************/
        hr = QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL);
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     endOfStream
 *  @brief  Receive the end of stream notification
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::endOfStream()
{
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * update an internal variable indicating that no new samples will be     *
     * coming                                                                 *
     **************************************************************************/
    mEndOfStream = true;

    /**************************************************************************
     * dispatch any samples still stored in the stream                        *
     **************************************************************************/
    return dispatchSamples();
}

/**
 *******************************************************************************
 *  @fn     shutdown
 *  @brief  shutdown the stream
 *
 *  @return HRESULT: S_OK if succeeded, else returns appropriate error code
 *******************************************************************************
 */
HRESULT CustomStream::shutdown()
{
    HRESULT hr = S_OK;
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    do
    {
        hr = checkShutdown();
        RETURNIFFAILED(hr);

        mState = SourceStateShutdown;

        /**********************************************************************
         * shutdown the event queue                                           *
         **********************************************************************/
        if (mPEventQueue)
        {
            mPEventQueue->Shutdown();
        }

        /**********************************************************************
         * release any samples still in the stream                            *
         ***********************************************************************/
        mPSampleList.RemoveAll();
        mPTokenList.RemoveAll();
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     needsData
 *  @brief  Return true if the stream is active and needs more samples
 *
 *  @return bool : 'true' if succeeded, else 'false'
 *******************************************************************************
 */
bool CustomStream::needsData()
{
    CComCritSecLock < CComAutoCriticalSection > lock(mCritSec);

    /**************************************************************************
     * the stream will indicate that it needs samples if it is active, the    *
     * end of stream has not been reached, and it has internally stored less  *
     * than the maximum number of samples to buffer                           *
     **************************************************************************/
    return (mActive && !mEndOfStream && (mPSampleList.GetCount()
                    < SAMPLE_BUFFER_SIZE));
}

/**
 *******************************************************************************
 *  @fn     CustomStream
 *  @brief  Constructor
 *
 *  @return
 *******************************************************************************
 */
CustomStream::CustomStream() :
    mCRef(1), mPMediaSource(NULL), mState(SourceStateUninitialized),
                    mEndOfStream(false), mActive(true), mNSamplesRequested(0)
{
}

/**
 *******************************************************************************
 *  @fn     init
 *  @brief  Initialise the stream object
 *
 *  @param[in]  pMediaSource : pointer to the media Source
 *  @param[in]  pStreamDescriptor : pointer to the stream Descriptor
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT CustomStream::init(CustomSource *pMediaSource,
                IMFStreamDescriptor *pStreamDescriptor)
{
    HRESULT hr = S_OK;

    do
    {
        /**********************************************************************
         * create the event queue                                             *
         **********************************************************************/
        hr = MFCreateEventQueue(&mPEventQueue);
        RETURNIFFAILED(hr);

        if (nullptr == pMediaSource)
        {
            return E_INVALIDARG;
        }

        if (nullptr == pStreamDescriptor)
        {
            return E_INVALIDARG;
        }

        /**********************************************************************
         * store a reference to the media source                              *
         **********************************************************************/
        mPMediaSource = pMediaSource;
        mPMediaSource->AddRef();

        /**********************************************************************
         * store the passed-in stream descriptor                              *
         **********************************************************************/
        mPStreamDescriptor = pStreamDescriptor;
    } while (false);

    return hr;
}

/**
 *******************************************************************************
 *  @fn     checkShutdown
 *  @brief  Check whether the stream is shut down
 *
 *  @return HRESULT : S_OK if successful; else returns microsofts error codes
 *******************************************************************************
 */
HRESULT CustomStream::checkShutdown()
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
 *  @fn     ~CustomStream
 *  @brief  Destructor
 *
 *  @return
 *******************************************************************************
 */
CustomStream::~CustomStream()
{
    if (mPMediaSource)
    {
        mPMediaSource->Release();
        mPMediaSource = NULL;
    }
}
