#ifndef _BUFFERED_H264_SERVER_MEDIA_SUBSESSION_H_
#define _BUFFERED_H264_SERVER_MEDIA_SUBSESSION_H_

#ifndef _BUFFERED_SERVER_MEDIA_SUBSESSION_H_
#include "buffered_sms.h"
#endif

#include <memory>
#include "media_file_reader.h"

class buffered_h264_sms : public buffered_sms
{
public:
	static buffered_h264_sms * createNew(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource, std::shared_ptr<media_file_reader> reader);

	// Used to implement "getAuxSDPLine()":
	void checkForAuxSDPLine1(void);
	void afterPlayingDummy1(void);

protected:
	buffered_h264_sms(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource, std::shared_ptr<media_file_reader> reader);
	virtual ~buffered_h264_sms(void);

	void setDoneFlag() { fDoneFlag = ~0; }

protected: // redefined virtual functions
	virtual char const* getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource);
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned & estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * inputSource);

private:
	char * fAuxSDPLine;
	char fDoneFlag; // used when setting up "fAuxSDPLine"
	RTPSink * fDummyRTPSink; // ditto

	unsigned char _sps[100];
	unsigned char _pps[100];
	int _sps_size;
	int _pps_size;
};

#endif
