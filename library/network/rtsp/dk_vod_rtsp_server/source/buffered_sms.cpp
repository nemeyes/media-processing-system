#include "buffered_sms.h"
#include "buffered_byte_stream_source.h"

buffered_sms::buffered_sms(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource, std::shared_ptr<debuggerking::media_source_reader> reader)
	: OnDemandServerMediaSubsession(env, reuseFirstSource)
	, _reader(reader)
{
	_stream_name = strDup(stream_name);
}

buffered_sms::~buffered_sms(void)
{
	delete[](char*)_stream_name;
}


void buffered_sms::testScaleFactor(float& scale)
{
	if (scale > 8.f)
		scale = 8.f;
	if (scale < 0.0f)
		scale = 0.0f;
}

void buffered_sms::setStreamSourceScale(FramedSource* inputSource, float scale)
{
	_reader->set_scale(scale);
}