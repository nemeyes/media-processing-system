#ifndef _DK_AUTO_LOCK_H_
#define _DK_AUTO_LOCK_H_

class dk_auto_lock
{
public:
	dk_auto_lock(CRITICAL_SECTION * lock)
		: _lock(lock)
	{
		::EnterCriticalSection(_lock);
	}

	~dk_auto_lock(void)
	{
		::LeaveCriticalSection(_lock);
	}
private:
	CRITICAL_SECTION * _lock;
};

#endif