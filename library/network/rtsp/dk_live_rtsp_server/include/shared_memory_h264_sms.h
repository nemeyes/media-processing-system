#ifndef _SHARED_MEMORY_H264_SERVER_MEDIA_SUBSESSION_H_
#define _SHARED_MEMORY_H264_SERVER_MEDIA_SUBSESSION_H_

#ifndef _SHARED_MEMORY_SERVER_MEDIA_SUBSESSION_H_
#include "shared_memory_sms.h"
#endif

class shared_memory_h264_sms : public shared_memory_sms
{
public: 
	static shared_memory_h264_sms * createNew(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource);

	// Used to implement "getAuxSDPLine()":
	void checkForAuxSDPLine1(void);
	void afterPlayingDummy1(void);

protected:
	shared_memory_h264_sms(UsageEnvironment & env, char const * stream_name, Boolean reuseFirstSource);
	virtual ~shared_memory_h264_sms(void);

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
