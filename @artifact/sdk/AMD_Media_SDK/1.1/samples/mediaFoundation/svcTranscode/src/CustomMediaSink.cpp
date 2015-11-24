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
 * @file <CustomMediaSink.cpp>                          
 *                                       
 * @brief This file defines class necessary for Media Sink
 *         
 ********************************************************************************
 */
/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/
#include "CustomMediaSink.h"
/** 
 *******************************************************************************
 *  @fn     createInstance
 *  @brief  Creates custom media sink
 *           
 *  @param[in] iid        : Reference Id
 *  @param[out] ppSource  : Pointer to the source 
 *  @param[in] fileName   : Pointer to the output file name to be written
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::createInstance(REFIID iid, void **ppSource,
                LPCWSTR fileName)
{
    if (ppSource == NULL)
    {
        return E_POINTER;
    }
    /**************************************************************************
     * Create Media Sink instance                                              *
     **************************************************************************/
    CComPtr < CustomMediaSink > pMSink(new CustomMediaSink());
    if (pMSink == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    /**************************************************************************
     * Initializes stream sink and stores the output file name                 *
     **************************************************************************/
    HRESULT hr = pMSink->initialize(fileName);
    RETURNIFFAILED(hr);

    hr = pMSink->QueryInterface(iid, ppSource);
    RETURNIFFAILED(hr);

    return S_OK;
}
/** 
 *******************************************************************************
 *  @fn     CustomMediaSink
 *  @brief  Constructor
 *
 *******************************************************************************
 */
CustomMediaSink::CustomMediaSink() :
    mIsShutdown(FALSE), mRefCount(0)
{

}
/** 
 *******************************************************************************
 *  @fn     ~CustomMediaSink
 *  @brief  Destructor
 *
 *******************************************************************************
 */
CustomMediaSink::~CustomMediaSink()
{
    assert( mIsShutdown);
}
/** 
 *******************************************************************************
 *  @fn     initialize
 *  @brief  Initializes the stream sink object & stores the output file name
 *           
 *  @param[in] fileName        : output file name
 *          
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::initialize(LPCWSTR fileName)
{
    mSFileName = std::wstring(fileName);
    return streamSink.initialize(this, fileName);
}

/** 
 *******************************************************************************
 *  @fn     AddRef
 *  @brief  IUnknown methods
 *          
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CustomMediaSink::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}
/** 
 *******************************************************************************
 *  @fn     Release
 *  @brief  IUnknown methods
 *  
 *  @return ULONG 
 *******************************************************************************
 */
ULONG CustomMediaSink::Release()
{
    ULONG uCount = InterlockedDecrement(&mRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    return uCount;
}
/** 
 *******************************************************************************
 *  @fn     QueryInterface
 *  @brief  virtual function
 *  
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*> (static_cast<IMFMediaSink*> (this));
    }
    else if (iid == __uuidof(IMFMediaSink))
    {
        *ppv = static_cast<IMFMediaSink*> (this);
    }
    else if (iid == __uuidof(IMFClockStateSink))
    {
        *ppv = static_cast<IMFClockStateSink*> (this);
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
 *  @fn     GetCharacteristics
 *  @brief  IMFMediaSink methods.Retrieves the characteristics of the media sink.
 *           
 *  @param[out] hWndDlg: Media sink characteristics _MFMEDIASOURCE_CHARACTERISTICS 
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::GetCharacteristics(DWORD *pdwCharacteristics)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (pdwCharacteristics == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = checkShutdown();

    if (SUCCEEDED(hr))
    {
        /***********************************************************************
         * This sink has a fixed number of streams and is rateless.             *
         ***********************************************************************/
        *pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_RATELESS;
    }

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     AddStreamSink
 *  @brief  IMFMediaSink methods.Adds a new stream sink to the media sink.
 *           
 *  @param[in] dwStreamSinkIdentifier   : steam ID
 *  @param[in] pMediaType               : Media type
 *  @param[in] ppStreamSink             : pointer to the stream sink

 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::AddStreamSink(DWORD /*dwStreamSinkIdentifier*/,
                IMFMediaType* /*pMediaType*/, IMFStreamSink** /*ppStreamSink*/)
{
    /***************************************************************************
     * This sink has a fixed number of streams, so this method                  *
     * always returns MF_E_STREAMSINKS_FIXED.                                   *
     ***************************************************************************/
    return MF_E_STREAMSINKS_FIXED;
}
/** 
 *******************************************************************************
 *  @fn     RemoveStreamSink
 *  @brief  Removes a stream sink from the media sink.
 *           
 *  @param[out] dwStreamSinkIdentifier   : steam ID
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::RemoveStreamSink(DWORD /*dwStreamSinkIdentifier*/)
{
    /***************************************************************************
     * This sink has a fixed number of streams, so this method                  *
     * always returns MF_E_STREAMSINKS_FIXED.                                   *
     ***************************************************************************/
    return MF_E_STREAMSINKS_FIXED;
}
/** 
 *******************************************************************************
 *  @fn     GetStreamSinkCount
 *  @brief  returns number of stream sinks in media sink
 *           
 *  @param[out] pcStreamSinkCount   : steam sink count
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::GetStreamSinkCount(DWORD *pcStreamSinkCount)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (pcStreamSinkCount == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * Fixed number of streams.                                                 *
     ***************************************************************************/
    *pcStreamSinkCount = 1;

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     GetStreamSinkByIndex
 *  @brief  returns steam sink pointer depending on index
 *           
 *  @param[in] dwIndex   : index
 *  @param[out] ppStreamSink   : pointer to the stream sink
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::GetStreamSinkByIndex(DWORD dwIndex,
                IMFStreamSink **ppStreamSink)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (ppStreamSink == NULL)
    {
        return E_INVALIDARG;
    }

    /***************************************************************************
     * Fixed stream: Index 0.                                                   *
     ***************************************************************************/
    if (dwIndex > 0)
    {
        return MF_E_INVALIDINDEX;
    }

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);
    /***************************************************************************
     * since only one stream is present return the pointer to the same          *
     ***************************************************************************/
    *ppStreamSink = &streamSink;
    (*ppStreamSink)->AddRef();

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     GetStreamSinkById
 *  @brief  returns steam sink pointer depending on stream id
 *           
 *  @param[in] dwStreamSinkIdentifier   : stream id
 *  @param[out] ppStreamSink            : pointer to the stream sink
 *
 *  @return HRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::GetStreamSinkById(DWORD /*dwStreamSinkIdentifier*/,
                IMFStreamSink **ppStreamSink)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (ppStreamSink == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    *ppStreamSink = &streamSink;
    (*ppStreamSink)->AddRef();

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     SetPresentationClock
 *  @brief  Sets the presentation clock on the media sink
 *           
 *  @param[in] pPresentationClock : Pointer to the IMFPresentationClock 
 *                                  interface of the presentation clock, or NULL. 
 *                                  If the value is NULL, the media sink stops 
 *                                  listening to the presentaton clock that was 
 *                                  previously set, if any
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::SetPresentationClock(
                IMFPresentationClock *pPresentationClock)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * If we already have a clock, remove ourselves from that clock's           *
     * state notifications.                                                     *
     ***************************************************************************/

    if (NULL != mPtrClock)
    {
        hr = mPtrClock->RemoveClockStateSink(this);
        RETURNIFFAILED(hr);
    }

    /***************************************************************************
     * Register ourselves to get state notifications from the new clock.        *
     ***************************************************************************/

    if (NULL != pPresentationClock)
    {
        hr = pPresentationClock->AddClockStateSink(this);
        RETURNIFFAILED(hr);
    }

    /***************************************************************************
     * Release the pointer to the old clock.                                    *
     * Store the pointer to the new clock.                                      *
     ***************************************************************************/
    mPtrClock.Release();
    mPtrClock = pPresentationClock;

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     GetPresentationClock
 *  @brief  Gets the presentation clock that was set on the media sink.
 *           
 *  @param[in] pPresentationClock : Receives a pointer to the presentation 
 *                                  clock's IMFPresentationClock interface. 
 *                                  The caller must release the interface
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::GetPresentationClock(
                IMFPresentationClock **ppPresentationClock)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    if (ppPresentationClock == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    if (NULL == mPtrClock)
    {
        hr = MF_E_NO_CLOCK;
    }
    else
    {
        *ppPresentationClock = mPtrClock;
        (*ppPresentationClock)->AddRef();
    }

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     Shutdown
 *  @brief  Shuts down the media sink and releases the resources it is using.
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::Shutdown()
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    hr = streamSink.shutdown();
    RETURNIFFAILED(hr);

    mPtrClock.Release();

    mIsShutdown = true;

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     OnClockStart
 *  @brief  IMFClockStateSink methods. Called when the presentation clock starts.
 *
 *  @param[in] hnsSystemTime      : The system time when the clock started, in 
 *                                  100-nanosecond units. 
 *  @param[in] llClockStartOffset : The new starting time for the clock
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::OnClockStart(MFTIME /*hnsSystemTime*/,
                LONGLONG llClockStartOffset)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * For an archive sink, we don't care about the system time.                *
     * But we need to cache the value of llClockStartOffset. This gives us the  *
     * earliest time stamp that we archive. If any input samples have an        *
     * earlier time stamp, we discard them.                                     *
     ***************************************************************************/
    hr = streamSink.start(llClockStartOffset);
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     OnClockStart
 *  @brief  IMFClockStateSink methods. Called when the presentation clock stops.
 *
 *  @param[in] hnsSystemTime      : The system time when the clock started, in 
 *                                  100-nanosecond units. 
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::OnClockStop(MFTIME /*hnsSystemTime*/)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * After this method is called, we stop accepting new data.                 *
     ***************************************************************************/
    hr = streamSink.stop();
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     OnClockPause
 *  @brief  Called when the presentation clock is paused.
 *
 *  @param[in] hnsSystemTime      : The system time when the clock started, in 
 *                                  100-nanosecond units. 
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::OnClockPause(MFTIME /*hnsSystemTime*/)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    /***************************************************************************
     * For an archive sink, the paused state is equivalent to the running       *
     * (started) state. We still accept data and archive it.                    *
     ***************************************************************************/
    hr = streamSink.pause();
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     OnClockRestart
 *  @brief  Called when the presentation clock Restart.
 *
 *  @param[in] hnsSystemTime      : The system time when the clock started, in 
 *                                  100-nanosecond units. 
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::OnClockRestart(MFTIME /*hnsSystemTime*/)
{
    CComCritSecLock < CComMultiThreadModel::AutoCriticalSection > lock(
                    mStateLock, true);

    HRESULT hr = checkShutdown();
    RETURNIFFAILED(hr);

    hr = streamSink.restart();
    RETURNIFFAILED(hr);

    return hr;
}
/** 
 *******************************************************************************
 *  @fn     OnClockPause
 *  @brief  Called when the rate changes on the presentation clock
 *
 *  @param[in] hnsSystemTime : The system time when the clock started, in 
 *                             100-nanosecond units. 
 *  @param[in] flRate        : The new rate, as a multiplier of the normal 
 *                             playback rate.
 *
 *  @return LRESULT : S_OK if successful; otherwise returns micorsoft error codes.
 *******************************************************************************
 */
HRESULT CustomMediaSink::OnClockSetRate(MFTIME /*hnsSystemTime*/, float /*flRate*/)
{
    /***************************************************************************
     * For a rateless sink, the clock rate is not important.                    *
     ***************************************************************************/
    return S_OK;
}
