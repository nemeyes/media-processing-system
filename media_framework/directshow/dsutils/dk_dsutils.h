#pragma once

#include <dshow.h>

// SafeRelease Template, for type safety
template <class T> void safe_release(T ** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}