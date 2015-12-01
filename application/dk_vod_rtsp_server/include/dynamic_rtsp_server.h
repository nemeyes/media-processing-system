#ifndef _DYNAMIC_RTSP_SERVER_H_
#define _DYNAMIC_RTSP_SERVER_H_

#ifndef _RTSP_SERVER_SUPPORTING_HTTP_STREAMING_HH
#include "RTSPServerSupportingHTTPStreaming.hh"
#endif

class dynamic_rtsp_server : public RTSPServerSupportingHTTPStreaming
{
public:
	static dynamic_rtsp_server * createNew(UsageEnvironment & env, Port ourPort, UserAuthenticationDatabase * authDatabase, unsigned reclamationTestSeconds = 65);

protected:
	dynamic_rtsp_server(UsageEnvironment & env, int ourSocket, Port ourPort, UserAuthenticationDatabase * authDatabase, unsigned reclamationTestSeconds);
	// called only by createNew();
	virtual ~dynamic_rtsp_server();

protected: // redefined virtual functions
	virtual ServerMediaSession * lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession);
};

#endif