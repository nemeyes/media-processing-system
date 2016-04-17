#ifndef _SCOPED_LOCK_H_
#define _SCOPED_LOCK_H_

class scoped_lock
{
public:
	scoped_lock(HANDLE lock)
		: _lock(lock)
	{
		if (_lock == NULL || _lock == INVALID_HANDLE_VALUE)
			return;

		::WaitForSingleObject(_lock, INFINITE);
	}

	~scoped_lock(void)
	{
		if (_lock == NULL || _lock == INVALID_HANDLE_VALUE)
			return;

		::SetEvent(_lock);
	}

private:
	HANDLE _lock;

private:
	scoped_lock(void);
	scoped_lock(const scoped_lock & clone);

};











#endif