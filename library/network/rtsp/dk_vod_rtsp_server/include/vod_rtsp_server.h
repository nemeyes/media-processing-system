#ifndef _VOD_RTSP_SERVER_H_
#define _VOD_RTSP_SERVER_H_

#ifndef _RTSP_SERVER_SUPPORTING_HTTP_STREAMING_HH
#include "RTSPServerSupportingHTTPStreaming.hh"
#endif

class vod_rtsp_server : public RTSPServerSupportingHTTPStreaming
{
public:
	static vod_rtsp_server * createNew(UsageEnvironment & env, Port ourPort, UserAuthenticationDatabase * authDatabase, unsigned reclamationTestSeconds = 65);

protected:
	vod_rtsp_server(UsageEnvironment & env, int ourSocket, Port ourPort, UserAuthenticationDatabase * authDatabase, unsigned reclamationTestSeconds);
	// called only by createNew();
	virtual ~vod_rtsp_server(void);

protected: // redefined virtual functions
	virtual ServerMediaSession * lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession);
};

#endif