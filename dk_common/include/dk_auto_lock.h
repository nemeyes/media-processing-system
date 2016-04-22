#ifndef _DK_AUTO_LOCK_H_
#define _DK_AUTO_LOCK_H_

class dk_auto_lock
{
public:
	dk_auto_lock(CRITICAL_SECTION * lock)
		: _cslock(lock)
		, _klock(NULL)
		, _bklock(FALSE)
	{
		::EnterCriticalSection(_cslock);
	}

	dk_auto_lock(HANDLE lock)
		: _klock(lock)
		, _bklock(TRUE)
		, _cslock(NULL)
	{
		::WaitForSingleObject(_klock, INFINITE);
	}

	dk_auto_lock(HANDLE lock, DWORD millisecond)
		: _klock(lock)
		, _bklock(FALSE)
		, _cslock(NULL)
	{
		if (::WaitForSingleObject(_klock, millisecond) == WAIT_OBJECT_0)
			_bklock = TRUE;
	}

	~dk_auto_lock(void)
	{
		if (_cslock)
			::LeaveCriticalSection(_cslock);
		if (_klock && _bklock)
		{
			::SetEvent(_klock);
			_bklock = FALSE;
		}
	}
private:
	CRITICAL_SECTION * _cslock;
	HANDLE _klock;
	BOOL _bklock;
};

#endif