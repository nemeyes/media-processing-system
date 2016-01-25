#include "framed_shared_memory_source.h"

framed_shared_memory_source::framed_shared_memory_source(UsageEnvironment& env, char const* stream_name)
	: FramedSource(env)
{
	memset(_stream_name, 0x00, sizeof(_stream_name));
	strcpy(_stream_name, stream_name);
}

framed_shared_memory_source::~framed_shared_memory_source(void)
{

}