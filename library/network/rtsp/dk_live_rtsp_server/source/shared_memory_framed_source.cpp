#include "shared_memory_framed_source.h"

shared_memory_framed_source::shared_memory_framed_source(UsageEnvironment & env, char const * stream_name)
	: FramedSource(env)
{
	memset(_stream_name, 0x00, sizeof(_stream_name));
	strcpy(_stream_name, stream_name);
}

shared_memory_framed_source::~shared_memory_framed_source(void)
{

}