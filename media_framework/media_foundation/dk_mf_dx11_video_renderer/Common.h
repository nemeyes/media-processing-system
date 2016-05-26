#pragma once

#include <windows.h>
#include <float.h> // for FLT_MAX
#include <tchar.h>
#include <math.h>
#include <strsafe.h>
#include <mmsystem.h>
#include <mfapi.h>
#include <mfidl.h>
#include <evr.h>
#include <mferror.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <wmcodecdsp.h> // for MEDIASUBTYPE_V216
#include "d3d11_video_renderer.h"
#include "linklist.h"
#include "static_async_callback.h"

DEFINE_GUID(CLSID_VideoProcessorMFT, 0x88753b26, 0x5b24, 0x49bd, 0xb2, 0xe7, 0xc, 0x44, 0x5c, 0x78, 0xc9, 0x82);

// MF_XVP_PLAYBACK_MODE
// Data type: UINT32 (treat as BOOL)
// If this attribute is TRUE, the XVP will run in playback mode where it will:
//      1) Allow caller to allocate D3D output samples
//      2) Not perform FRC
//      3) Allow last frame regeneration (repaint)
// This attribute should be set on the transform's attrbiute store prior to setting the input type.
//DEFINE_GUID(MF_XVP_PLAYBACK_MODE, 0x3c5d293f, 0xad67, 0x4e29, 0xaf, 0x12, 0xcf, 0x3e, 0x23, 0x8a, 0xcc, 0xe9);

namespace debuggerking
{
    inline void safe_close_handle(HANDLE& h)
    {
        if (h != NULL)
        {
            CloseHandle(h);
            h = NULL;
        }
    }

    template <class T> inline void safe_delete(T*& pT)
    {
        delete pT;
        pT = NULL;
    }

    template <class T> inline void safe_delete_array(T*& pT)
    {
        delete[] pT;
        pT = NULL;
    }

    template <class T> inline void safe_release(T*& pT)
    {
        if (pT != NULL)
        {
            pT->Release();
            pT = NULL;
        }
    }

    template <class T> inline double ticks2milliseconds(const T& t)
    {
        return t / 10000.0;
    }

    template <class T> inline T milliseconds2ticks(const T& t)
    {
        return t * 10000;
    }

    // returns the greatest common divisor of A and B
    inline int gcd(int A, int B)
    {
        int Temp;

        if (A < B)
        {
            Temp = A;
            A = B;
            B = Temp;
        }

        while (B != 0)
        {
            Temp = A % B;
            A = B;
            B = Temp;
        }

        return A;
    }

    // Convert a fixed-point to a float.
    inline float offset2float(const MFOffset & offset)
    {
        return (float)offset.value + ((float)offset.value / 65536.0f);
    }

    inline RECT video_area2rect(const MFVideoArea area)
    {
		float left = offset2float(area.OffsetX);
		float top = offset2float(area.OffsetY);

        RECT rc =
        {
            int( left + 0.5f ),
            int( top + 0.5f ),
            int( left + area.Area.cx + 0.5f ),
            int( top + area.Area.cy + 0.5f )
        };

        return rc;
    }

    inline MFOffset make_offset(float v)
    {
        MFOffset offset;
        offset.value = short(v);
        offset.fract = WORD(65536 * (v-offset.value));
        return offset;
    }

    inline MFVideoArea make_area(float x, float y, DWORD width, DWORD height)
    {
        MFVideoArea area;
        area.OffsetX = make_offset(x);
		area.OffsetY = make_offset(y);
        area.Area.cx = width;
        area.Area.cy = height;
        return area;
    }

    class mf_base
    {
    public:

        static long get_object_count(void)
        {
			return _object_count;
        }

    protected:

		mf_base(void)
        {
			InterlockedIncrement(&_object_count);
        }

		~mf_base(void)
        {
			InterlockedDecrement(&_object_count);
        }

    private:

        static volatile long _object_count;
    };

    //////////////////////////////////////////////////////////////////////////
    //  CAsyncCallback [template]
    //
    //  Description:
    //  Helper class that routes IMFAsyncCallback::Invoke calls to a class
    //  method on the parent class.
    //
    //  Usage:
    //  Add this class as a member variable. In the parent class constructor,
    //  initialize the CAsyncCallback class like this:
    //      m_cb(this, &CYourClass::OnInvoke)
    //  where
    //      m_cb       = CAsyncCallback object
    //      CYourClass = parent class
    //      OnInvoke   = Method in the parent class to receive Invoke calls.
    //
    //  The parent's OnInvoke method (you can name it anything you like) must
    //  have a signature that matches the InvokeFn typedef below.
    //////////////////////////////////////////////////////////////////////////

    // T: Type of the parent object
    template<class T>
    class async_callback : public IMFAsyncCallback
    {
    public:

        typedef HRESULT (T::*InvokeFn)(IMFAsyncResult * result);

		async_callback(T* parent, InvokeFn fn) :
            _parent(parent),
            _invoke_fn(fn)
        {
        }

        // IUnknown
        STDMETHODIMP_(ULONG) AddRef(void)
        {
            // Delegate to parent class.
            return _parent->AddRef();
        }

        STDMETHODIMP_(ULONG) Release(void)
        {
            // Delegate to parent class.
            return _parent->Release();
        }

        STDMETHODIMP QueryInterface(REFIID iid, __RPC__deref_out _Result_nullonfailure_ void ** ppv)
        {
            if (!ppv)
            {
                return E_POINTER;
            }
            if (iid == __uuidof(IUnknown))
            {
                *ppv = static_cast<IUnknown*>(static_cast<IMFAsyncCallback*>(this));
            }
            else if (iid == __uuidof(IMFAsyncCallback))
            {
                *ppv = static_cast<IMFAsyncCallback*>(this);
            }
            else
            {
                *ppv = NULL;
                return E_NOINTERFACE;
            }
            AddRef();
            return S_OK;
        }

        // IMFAsyncCallback methods
        STDMETHODIMP GetParameters(__RPC__out DWORD* pdwFlags, __RPC__out DWORD* pdwQueue)
        {
            // Implementation of this method is optional.
            return E_NOTIMPL;
        }

        STDMETHODIMP Invoke(__RPC__in_opt IMFAsyncResult* pAsyncResult)
        {
            return (m_pParent->*m_pInvokeFn)(pAsyncResult);
        }

    private:

        T* _parent;
        InvokeFn _invoke_fn;
    };

    //-----------------------------------------------------------------------------
    // ThreadSafeQueue template
    // Thread-safe queue of COM interface pointers.
    //
    // T: COM interface type.
    //
    // This class is used by the scheduler.
    //
    // Note: This class uses a critical section to protect the state of the queue.
    // With a little work, the scheduler could probably use a lock-free queue.
    //-----------------------------------------------------------------------------

    template <class T>
    class thread_safe_queue
    {
    public:
		thread_safe_queue(void)
        {
            InitializeCriticalSection(&_lock);
        }

		virtual ~thread_safe_queue(void)
        {
            DeleteCriticalSection(&_lock);
        }

        HRESULT queue(T* p)
        {
            EnterCriticalSection(&_lock);
            HRESULT hr = _list.InsertBack(p);
            LeaveCriticalSection(&_lock);
            return hr;
        }

        HRESULT dequeue(T** pp)
        {
            EnterCriticalSection(&_lock);
            HRESULT hr = S_OK;
            if (_list.IsEmpty())
            {
                *pp = NULL;
                hr = S_FALSE;
            }
            else
            {
                hr = _list.RemoveFront(pp);
            }
            LeaveCriticalSection(&_lock);
            return hr;
        }

        HRESULT push_back(T* p)
        {
            EnterCriticalSection(&_lock);
            HRESULT hr =  _list.InsertFront(p);
            LeaveCriticalSection(&_lock);
            return hr;
        }

        DWORD get_count(void)
        {
            EnterCriticalSection(&_lock);
            DWORD nCount =  _list.GetCount();
            LeaveCriticalSection(&_lock);
            return nCount;
        }

        void clear(void)
        {
            EnterCriticalSection(&_lock);
            _list.Clear();
            LeaveCriticalSection(&_lock);
        }

    private:
        CRITICAL_SECTION    _lock;
        ComPtrListEx<T>     _list;
    };

	class critical_section
    {
    public:

		critical_section(void)
			: _cs()
        {
            InitializeCriticalSection(&_cs);
        }

		~critical_section(void)
        {
            DeleteCriticalSection(&_cs);
        }

        _Acquires_lock_(this->_cs)
        void lock(void)
        {
            EnterCriticalSection(&_cs);
        }

        _Releases_lock_(this->m_cs)
        void unlock(void)
        {
            LeaveCriticalSection(&_cs);
        }

    private:
        CRITICAL_SECTION _cs;
    };

    class autolock
    {
    public:

		_Acquires_lock_(this->_lock->m_cs)
			autolock(critical_section * lock)
			: _lock(lock)
        {
			_lock->lock();
        }

		_Releases_lock_(this->_lock->m_cs)
			~autolock(void)
        {
			_lock->unlock();
        }

    private:
		critical_section * _lock;
    };
};
