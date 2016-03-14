#ifndef _LIVE_RTSP_SERVER_H_
#define _LIVE_RTSP_SERVER_H_

#ifndef _RTSP_SERVER_SUPPORTING_HTTP_STREAMING_HH
#include "RTSPServerSupportingHTTPStreaming.hh"
#endif

class live_rtsp_server : public RTSPServerSupportingHTTPStreaming
{
public:
	static live_rtsp_server * createNew(UsageEnvironment & env, Port port, UserAuthenticationDatabase * auth_db, unsigned reclamationTestSeconds = 65);

protected:
	live_rtsp_server(UsageEnvironment & env, int socket, Port port, UserAuthenticationDatabase * auth_db, unsigned reclamationTestSeconds);
	virtual ~live_rtsp_server(void);

protected:
	virtual ServerMediaSession * lookupServerMediaSession(char const * stream_name);
};

#endif
