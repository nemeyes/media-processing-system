#pragma once

#include "common.h"

namespace debuggerking
{
    //-----------------------------------------------------------------------------
    // SchedulerCallback
    //
    // Defines the callback method to present samples.
    //-----------------------------------------------------------------------------
    struct scheduler_callback
    {
        virtual HRESULT present_frame(void) = 0;
    };

    //-----------------------------------------------------------------------------
    // Scheduler class
    //
    // Schedules when a sample should be displayed.
    //
    // Note: Presentation of each sample is performed by another object which
    // must implement SchedulerCallback::PresentSample.
    //
    // General design:
    // The scheduler generally receives samples before their presentation time. It
    // puts the samples on a queue and presents them in FIFO order on a worker
    // thread. The scheduler communicates with the worker thread by posting thread
    // messages.
    //
    // The caller has the option of presenting samples immediately (for example,
    // for repaints).
    //-----------------------------------------------------------------------------

    class scheduler : public IUnknown, private mf_base
    {
    public:

		scheduler(critical_section & cs);
		virtual ~scheduler(void);

        // IUnknown
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void** ppv);

        void set_callback(scheduler_callback * pcb)
        {
            _cb = pcb;
        }

        void set_frame_rate(const MFRatio & fps);
		void set_clock_rate(float rate) { _rate = rate; }

		const LONGLONG & last_sample_time(void) const { return _last_sample_time; }
		const LONGLONG & frame_duration(void) const { return _per_frame_interval; }

        HRESULT start_scheduler(IMFClock * clock);
        HRESULT stop_scheduler(void);

        HRESULT schedule_sample(IMFSample * sample, BOOL present_now);
        HRESULT process_samples_in_queue(LONG * next_sleep);
		HRESULT process_sample(IMFSample * sample, LONG * next_sleep);
        HRESULT flush(void);

        DWORD get_count(void) { return _scheduled_samples.get_count(); }

    private:

        HRESULT start_process_sample(void);
        HRESULT timer_callback(__RPC__in_opt IMFAsyncResult * result);
		METHODASYNCCALLBACKEX(timer_callback, scheduler, 0, MFASYNC_CALLBACK_QUEUE_MULTITHREADED);

        long							_ref_count;
        critical_section &				_cs;          // critical section for thread safety
		scheduler_callback *			_cb;              // Weak reference; do not delete.
        thread_safe_queue<IMFSample>	_scheduled_samples; // Samples waiting to be presented.
        IMFClock*						_clock;           // Presentation clock. Can be NULL.
        float							_rate;            // Playback rate.
        HANDLE							_wait_timer;       // Wait Timer after which frame is presented.
        MFTIME							_last_sample_time;   // Most recent sample time.
        MFTIME							_per_frame_interval; // Duration of each frame.
        LONGLONG						_per_frame_1_4th;   // 1/4th of the frame duration.
        MFWORKITEM_KEY					_key_timer;
    };
}
