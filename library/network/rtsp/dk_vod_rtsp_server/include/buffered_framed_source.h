#ifndef _BUFFERED_FRAMED_SOURCE_H_
#define _BUFFERED_FRAMED_SOURCE_H_

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include <memory>
#include "media_source_reader.h"

class buffered_framed_source : public FramedSource
{
protected:
	buffered_framed_source(UsageEnvironment & env, char const * stream_name, std::shared_ptr<debuggerking::media_source_reader> reader); // abstract base class
	virtual ~buffered_framed_source(void);

	char _stream_name[100];
	std::shared_ptr<debuggerking::media_source_reader> _reader;
};

#endif
