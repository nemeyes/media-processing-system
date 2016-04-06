#include "vod_rtsp_server.h"
#include <liveMedia.hh>
#include <string.h>
#include "media_source_reader.h"
#include "buffered_h264_sms.h"
#include <memory>

vod_rtsp_server * vod_rtsp_server::createNew(UsageEnvironment & env, Port ourPort, UserAuthenticationDatabase * authDatabase, unsigned reclamationTestSeconds)
{
	int ourSocket = setUpOurSocket(env, ourPort);
	if (ourSocket == -1)
		return NULL;
	return new vod_rtsp_server(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

vod_rtsp_server::vod_rtsp_server(UsageEnvironment & env, int ourSocket, Port ourPort, UserAuthenticationDatabase * authDatabase, unsigned reclamationTestSeconds)
	: RTSPServerSupportingHTTPStreaming(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds)
{
}

vod_rtsp_server::~vod_rtsp_server(void)
{
}

static ServerMediaSession * createNewSMS(UsageEnvironment & env, char const * stream_name); // forward

ServerMediaSession * vod_rtsp_server::lookupServerMediaSession(char const * stream_name, Boolean isFirstLookupInSession)
{
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(stream_name);
	Boolean smsExists = sms != NULL;

	if (smsExists && isFirstLookupInSession) 
	{
		removeServerMediaSession(sms);
		sms = NULL;
	}

	if (sms == NULL) 
	{
		sms = createNewSMS(envir(), stream_name);
		addServerMediaSession(sms);
	}

	return sms;
}

#define NEW_SMS(description) do {\
								char const* descStr = description\
								", streamed by the DebugerKing Media Server";\
								sms = ServerMediaSession::createNew(env, stream_name, stream_name, descStr);\
							} while(0)

static ServerMediaSession * createNewSMS(UsageEnvironment & env, char const * stream_name)
{
	if (stream_name == nullptr || strlen(stream_name)<1)
		return nullptr;

	ServerMediaSession* sms = NULL;
	Boolean const reuse_source = False;

	std::shared_ptr<media_source_reader> reader(new media_source_reader);

	media_source_reader::vsubmedia_type video_type = media_source_reader::unknown_video_type;
	media_source_reader::asubmedia_type audio_type = media_source_reader::unknown_audio_type;
	reader->open(stream_name, 0, video_type, audio_type);

	if (video_type == media_source_reader::vsubmedia_type_h264)
	{
		NEW_SMS("H.264 Video");
		OutPacketBuffer::maxSize = 6000000; // allow for some possibly large H.264 frames
		sms->addSubsession(buffered_h264_sms::createNew(env, stream_name, reuse_source, reader));
	}
	return sms;
}