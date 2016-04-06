#include "buffered_framed_source.h"

buffered_framed_source::buffered_framed_source(UsageEnvironment & env, char const * stream_name, std::shared_ptr<media_source_reader> reader)
	: FramedSource(env)
	, _reader(reader)
{
	memset(_stream_name, 0x00, sizeof(_stream_name));
	strcpy(_stream_name, stream_name);
}

buffered_framed_source::~buffered_framed_source(void)
{

}