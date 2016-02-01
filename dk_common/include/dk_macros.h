#pragma once

#define SAFE_RELEASE(x)	\
	if(x)	\
		x->Release();	\
	x = 0;

#define ZERO_MEMORY(VAR)                    {memset(&VAR, 0, sizeof(VAR));}

#define ALIGN16(value)                      (((value + 15) >> 4) << 4) // round up to a multiple of 16
#define ALIGN32(value)                      (((value + 31) >> 5) << 5) // round up to a multiple of 32
#define ALIGN(value, alignment)             (alignment) * ( (value) / (alignment) + (((value) % (alignment)) ? 1 : 0))
