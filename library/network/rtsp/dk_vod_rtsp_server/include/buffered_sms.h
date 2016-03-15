#ifndef _BUFFERED_SERVER_MEDIA_SUBSESSION_H_
#define _BUFFERED_SERVER_MEDIA_SUBSESSION_H_

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#include <memory>
#include "media_file_reader.h"

class buffered_sms : public OnDemandServerMediaSubsession
{
protected: // we're a virtual base class
	buffered_sms(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource, std::shared_ptr<media_file_reader> reader);
	virtual ~buffered_sms(void);

protected:
	char const *	_stream_name;
	std::shared_ptr<media_file_reader> _reader;
};

#endif
