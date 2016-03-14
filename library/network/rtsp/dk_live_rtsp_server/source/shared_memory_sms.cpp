#include "shared_memory_sms.h"

shared_memory_sms::shared_memory_sms(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource)
	: OnDemandServerMediaSubsession(env, reuseFirstSource)
{
	_stream_name = strDup(stream_name);
}

shared_memory_sms::~shared_memory_sms(void)
{
	delete[](char*)_stream_name;
}
