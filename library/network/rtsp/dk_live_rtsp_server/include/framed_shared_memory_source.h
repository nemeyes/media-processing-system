#ifndef _FRAMED_SHARED_MEMORY_SOURCE_H_
#define _FRAMED_SHARED_MEMORY_SOURCE_H_

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

class framed_shared_memory_source : public FramedSource
{
protected:
	framed_shared_memory_source(UsageEnvironment & env, char const * stream_name); // abstract base class
	virtual ~framed_shared_memory_source(void);

	char _stream_name[100];
};

#endif
