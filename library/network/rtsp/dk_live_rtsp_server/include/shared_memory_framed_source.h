#ifndef _SHARED_MEMORY_FRAMED_SOURCE_H_
#define _SHARED_MEMORY_FRAMED_SOURCE_H_

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

class shared_memory_framed_source : public FramedSource
{
protected:
	shared_memory_framed_source(UsageEnvironment & env, char const * stream_name); // abstract base class
	virtual ~shared_memory_framed_source(void);

	char _stream_name[100];
};

#endif
