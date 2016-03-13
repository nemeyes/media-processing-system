#ifndef _SHARED_MEMORY_SERVER_MEDIA_SUBSESSION_H_
#define _SHARED_MEMORY_SERVER_MEDIA_SUBSESSION_H_

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

class shared_memory_sms : public OnDemandServerMediaSubsession
{
protected: // we're a virtual base class
	shared_memory_sms(UsageEnvironment& env, char const* stream_name, Boolean reuseFirstSource);
	virtual ~shared_memory_sms(void);

protected:
	char const*	_stream_name;
};

#endif
