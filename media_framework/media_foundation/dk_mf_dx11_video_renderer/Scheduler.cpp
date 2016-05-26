#include "Scheduler.h"

debuggerking::scheduler::scheduler(critical_section & cs)
	: _ref_count(1)
	, _cs(cs)
	, _cb(NULL)
	, _scheduled_samples()
	, _clock(NULL)
	, _rate(1.0f)
	, _wait_timer(NULL)
	, _last_sample_time(0)
	, _per_frame_interval(0)
	, _per_frame_1_4th(0)
	, _key_timer(0)
{}

debuggerking::scheduler::~scheduler(void)
{
	if (_wait_timer != NULL)
    {
		CloseHandle(_wait_timer);
		_wait_timer = NULL;
    }
    // Discard samples.
	_scheduled_samples.clear();
    safe_release(_clock);
}

// IUnknown methods
ULONG debuggerking::scheduler::AddRef()
{
    return InterlockedIncrement(&_ref_count);
}

ULONG  debuggerking::scheduler::Release()
{
    ULONG count = InterlockedDecrement(&_ref_count);
	if (count == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
	return count;
}

HRESULT debuggerking::scheduler::QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void ** ppv)
{
    if (!ppv)
        return E_POINTER;
    
	if (iid == IID_IUnknown)
        *ppv = static_cast<IUnknown*>(this);
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

//-----------------------------------------------------------------------------
// SetFrameRate
// Specifies the frame rate of the video, in frames per second.
//-----------------------------------------------------------------------------
void debuggerking::scheduler::set_frame_rate(const MFRatio & fps)
{
    UINT64 AvgTimePerFrame = 0;
    // Convert to a duration.
    MFFrameRateToAverageTimePerFrame(fps.Numerator, fps.Denominator, &AvgTimePerFrame);
    _per_frame_interval = (MFTIME)AvgTimePerFrame;
    // Calculate 1/4th of this value, because we use it frequently.
	_per_frame_1_4th = _per_frame_interval / 4;
}

//-----------------------------------------------------------------------------
// StartScheduler
// Starts the scheduler's worker thread.
//
// IMFClock: Pointer to the DX11VideoRenderer's presentation clock. Can be NULL.
//-----------------------------------------------------------------------------
HRESULT debuggerking::scheduler::start_scheduler(IMFClock * clock)
{
    HRESULT hr = S_OK;
	safe_release(_clock);
	_clock = clock;
	if (_clock != NULL)
		_clock->AddRef();
    // Set a high the timer resolution (ie, short timer period).
    timeBeginPeriod(1);

    // create the waitable timer
    _wait_timer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (_wait_timer == NULL)
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

//-----------------------------------------------------------------------------
// StopScheduler
//
// Stops the scheduler's worker thread.
//-----------------------------------------------------------------------------
HRESULT debuggerking::scheduler::stop_scheduler(void)
{
	if (_wait_timer != NULL)
    {
		CloseHandle(_wait_timer);
		_wait_timer = NULL;
    }
    // Discard samples.
    _scheduled_samples.clear();
    // Restore the timer resolution.
    timeEndPeriod(1);
    safe_release(_clock);
    return S_OK;
}

//-----------------------------------------------------------------------------
// Flush
//
// Flushes all samples that are queued for presentation.
//
// Note: This method is synchronous; ie., it waits for the flush operation to
// complete on the worker thread.
//-----------------------------------------------------------------------------
HRESULT debuggerking::scheduler::flush(void)
{
    autolock lock(&_cs);
    // Flushing: Clear the sample queue and set the event.
    m_ScheduledSamples.Clear();

    // Cancel timer callback
    if (_key_timer != 0)
    {
        (void)MFCancelWorkItem(m_keyTimer);
        m_keyTimer = 0;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// ScheduleSample
//
// Schedules a new sample for presentation.
//
// pSample:     Pointer to the sample.
// bPresentNow: If TRUE, the sample is presented immediately. Otherwise, the
//              sample's time stamp is used to schedule the sample.
//-----------------------------------------------------------------------------

HRESULT debuggerking::scheduler::ScheduleSample(IMFSample* pSample, BOOL bPresentNow)
{
    if (m_pCB == NULL)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;

    if (bPresentNow || (m_pClock == NULL))
    {
        // Present the sample immediately.
        hr = m_pCB->PresentFrame();
    }
    else
    {
        // Queue the sample and ask the scheduler thread to wake up.
        hr = m_ScheduledSamples.Queue(pSample);

        if (SUCCEEDED(hr))
        {
            // process the frame asynchronously
            hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, &m_xOnTimer, nullptr);
        }
    }

    return hr;
}

//-----------------------------------------------------------------------------
// ProcessSamplesInQueue
//
// Processes all the samples in the queue.
//
// plNextSleep: Receives the length of time the scheduler thread should sleep
//              before it calls ProcessSamplesInQueue again.
//-----------------------------------------------------------------------------

HRESULT debuggerking::scheduler::ProcessSamplesInQueue(LONG* plNextSleep)
{
    HRESULT hr = S_OK;
    LONG lWait = 0;
    IMFSample* pSample = NULL;

    // Process samples until the queue is empty or until the wait time > 0.

    // Note: Dequeue returns S_FALSE when the queue is empty.

    while (m_ScheduledSamples.Dequeue(&pSample) == S_OK)
    {
        // Process the next sample in the queue. If the sample is not ready
        // for presentation. the value returned in lWait is > 0, which
        // means the scheduler should sleep for that amount of time.

        hr = ProcessSample(pSample, &lWait);
        SafeRelease(pSample);

        if (FAILED(hr) || lWait > 0)
        {
            break;
        }
    }

    // If the wait time is zero, it means we stopped because the queue is
    // empty (or an error occurred). Set the wait time to infinite; this will
    // make the scheduler thread sleep until it gets another thread message.
    if (lWait == 0)
    {
        lWait = INFINITE;
    }

    *plNextSleep = lWait;
    return hr;
}


//-----------------------------------------------------------------------------
// ProcessSample
//
// Processes a sample.
//
// plNextSleep: Receives the length of time the scheduler thread should sleep.
//-----------------------------------------------------------------------------


HRESULT debuggerking::scheduler::ProcessSample(IMFSample* pSample, LONG* plNextSleep)
{
    HRESULT hr = S_OK;

    LONGLONG hnsPresentationTime = 0;
    LONGLONG hnsTimeNow = 0;
    MFTIME   hnsSystemTime = 0;
    LONGLONG hnsDelta = 0;

    BOOL bPresentNow = TRUE;
    LONG lNextSleep = 0;

    if (m_pClock)
    {
        // Get the sample's time stamp. It is valid for a sample to
        // have no time stamp.
        hr = pSample->GetSampleTime(&hnsPresentationTime);

        // Get the clock time. (But if the sample does not have a time stamp,
        // we don't need the clock time.)
        if (SUCCEEDED(hr))
        {
            hr = m_pClock->GetCorrelatedTime(0, &hnsTimeNow, &hnsSystemTime);
        }

        if (SUCCEEDED(hr))
        {
            // Calculate the time until the sample's presentation time.
            // A negative value means the sample is late.
            hnsDelta = hnsPresentationTime - hnsTimeNow;
            if (m_fRate < 0)
            {
                // For reverse playback, the clock runs backward. Therefore, the
                // delta is reversed.
                hnsDelta = - hnsDelta;
            }

            if (hnsDelta < - m_PerFrame_1_4th)
            {
                // This sample is late.
                bPresentNow = TRUE;
            }
            else if (hnsDelta > (3 * m_PerFrame_1_4th))
            {
                // This sample is still too early. Go to sleep.
                lNextSleep = static_cast<LONG>(TicksToMilliseconds(hnsDelta - (3 * m_PerFrame_1_4th)));

                // Adjust the sleep time for the clock rate. (The presentation clock runs
                // at m_fRate, but sleeping uses the system clock.)
                lNextSleep = static_cast<LONG>(lNextSleep / fabsf(m_fRate));

                // Don't present yet.
                bPresentNow = FALSE;
            }
        }
    }

    if (bPresentNow)
    {
        hr = m_pCB->PresentFrame();
    }
    else
    {
        // The sample is not ready yet. Return it to the queue.
        hr = m_ScheduledSamples.PutBack(pSample);
    }

    *plNextSleep = lNextSleep;

    return hr;
}

//-----------------------------------------------------------------------------
// StartProcessSample
//
// Synopsis:   Main entrypoint for the frame processing
//-----------------------------------------------------------------------------

HRESULT debuggerking::scheduler::StartProcessSample()
{
    HRESULT hr = S_OK;

    LONG    lWait = INFINITE;
    BOOL    bExitThread = FALSE;
    IMFAsyncResult *pAsyncResult = NULL;

    hr = ProcessSamplesInQueue(&lWait);

    if(SUCCEEDED(hr))
    {
        if(INFINITE != lWait && 0 < lWait)
        {
            // not time to process the frame yet, wait until the right time
            LARGE_INTEGER llDueTime;
            llDueTime.QuadPart = -1 * MillisecondsToTicks(lWait);
            if (SetWaitableTimer(m_hWaitTimer, &llDueTime, 0, NULL, NULL, FALSE) == 0)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

            if(SUCCEEDED(hr))
            {
                // queue a waititem to wait for timer completion
                hr = MFCreateAsyncResult(nullptr, &m_xOnTimer, nullptr, &pAsyncResult);
                if(SUCCEEDED(hr))
                {
                    hr = MFPutWaitingWorkItem(m_hWaitTimer, 0, pAsyncResult, &m_keyTimer);
                }
            }
        }
    }

    SafeRelease(pAsyncResult);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   OnTimer
//
//  Synopsis:   Callback fired when the timer expires. Used as entrypoint to
//              check whether a frame should now be presented
//
//--------------------------------------------------------------------------
HRESULT debuggerking::scheduler::OnTimer(__RPC__in_opt IMFAsyncResult *pResult)
{
    HRESULT hr = S_OK;

    autolock lock(&m_critSec);

    m_keyTimer = 0;

    // if we have a pending frame, process it
    // it's possible that we don't have a frame at this point if the pending frame was cancelled
    hr = StartProcessSample();

    return hr;
}