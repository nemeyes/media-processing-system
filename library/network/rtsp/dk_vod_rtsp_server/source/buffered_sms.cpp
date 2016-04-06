#include "buffered_sms.h"

buffered_sms::buffered_sms(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource, std::shared_ptr<media_source_reader> reader)
	: OnDemandServerMediaSubsession(env, reuseFirstSource)
	, _reader(reader)
{
	_stream_name = strDup(stream_name);
}

buffered_sms::~buffered_sms(void)
{
	delete[](char*)_stream_name;
}
