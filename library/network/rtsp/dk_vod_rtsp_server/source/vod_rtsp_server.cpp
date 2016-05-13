#include "vod_rtsp_server.h"
#include <liveMedia.hh>
#include <string.h>
#include "media_source_reader.h"
#include "buffered_h264_sms.h"
#include <memory>

debuggerking::vod_rtsp_core * debuggerking::vod_rtsp_core::createNew(UsageEnvironment & env, Port ourPort, UserAuthenticationDatabase * authDatabase, unsigned reclamationTestSeconds)
{
	int ourSocket = setUpOurSocket(env, ourPort);
	if (ourSocket == -1)
		return NULL;
	return new vod_rtsp_core(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

debuggerking::vod_rtsp_core::vod_rtsp_core(UsageEnvironment & env, int ourSocket, Port ourPort, UserAuthenticationDatabase * authDatabase, unsigned reclamationTestSeconds)
	: RTSPServerSupportingHTTPStreaming(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds)
{
}

debuggerking::vod_rtsp_core::~vod_rtsp_core(void)
{
}

static ServerMediaSession * createNewSMS(UsageEnvironment & env, char const * stream_name); // forward

ServerMediaSession * debuggerking::vod_rtsp_core::lookupServerMediaSession(char const * stream_name, Boolean isFirstLookupInSession)
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

	std::shared_ptr<debuggerking::media_source_reader> reader(new debuggerking::media_source_reader);

	int32_t video_type = debuggerking::media_source_reader::video_submedia_type_t::unknown;
	int32_t audio_type = debuggerking::media_source_reader::audio_submedia_type_t::unknown;
	reader->open(stream_name, 0, video_type, audio_type);

	if (video_type == debuggerking::media_source_reader::video_submedia_type_t::h264)
	{
		NEW_SMS("H.264 Video");
		OutPacketBuffer::maxSize = 6000000; // allow for some possibly large H.264 frames
		sms->addSubsession(buffered_h264_sms::createNew(env, stream_name, reuse_source, reader));
	}
	return sms;
}