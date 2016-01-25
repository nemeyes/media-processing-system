#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

#ifndef _RTSP_SERVER_SUPPORTING_HTTP_STREAMING_HH
#include "RTSPServerSupportingHTTPStreaming.hh"
#endif

class vmxnet_rtsp_server : public RTSPServerSupportingHTTPStreaming
{
public:
	static vmxnet_rtsp_server* createNew(UsageEnvironment& env, Port port, UserAuthenticationDatabase* auth_db, unsigned reclamationTestSeconds = 65);

protected:
	vmxnet_rtsp_server(UsageEnvironment& env, int socket, Port port, UserAuthenticationDatabase* auth_db, unsigned reclamationTestSeconds);
	virtual ~vmxnet_rtsp_server(void);

protected:
	virtual ServerMediaSession* lookupServerMediaSession(char const* stream_name);
};

#endif
