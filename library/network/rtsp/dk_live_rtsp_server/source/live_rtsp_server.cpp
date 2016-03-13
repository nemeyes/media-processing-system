#include "live_rtsp_server.h"
#include <liveMedia.hh>
#include "log/logger.h"
#include <string.h>
#include "h264_video_shared_memory_sms.h"
#include "buffers/session_queue_manager.h"
#include "byte_stream_shared_memory_source.h"


live_rtsp_server* live_rtsp_server::createNew(UsageEnvironment& env, Port port, UserAuthenticationDatabase* auth_db, unsigned reclamationTestSeconds)
{
	int ourSocket = setUpOurSocket(env, port);
	if (ourSocket == -1)
		return NULL;

	return new live_rtsp_server(env, ourSocket, port, auth_db, reclamationTestSeconds);
}

live_rtsp_server::live_rtsp_server(UsageEnvironment& env, int socket, Port port, UserAuthenticationDatabase* auth_db, unsigned reclamationTestSeconds)
	: RTSPServerSupportingHTTPStreaming(env, socket, port, auth_db, reclamationTestSeconds)
{

}

live_rtsp_server::~live_rtsp_server(void)
{

}

static ServerMediaSession* createNewSMS(UsageEnvironment& env, char const* stream_name);
ServerMediaSession* live_rtsp_server::lookupServerMediaSession(char const* stream_name)
{
	//char tmp_stream_name[100] = {0};
	//_snprintf( tmp_stream_name, sizeof(tmp_stream_name), "Global\\%s", stream_name );
	HANDLE map = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, stream_name);
	Boolean exists = map != NULL;

	// Next, check whether we already have a "ServerMediaSession" for this file:
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(stream_name);
	Boolean smsExists = sms != NULL;

	// Handle the four possibilities for "fileExists" and "smsExists":
	if (!exists)
	{
		char log[MAX_PATH] = { 0 };
		_snprintf(log, sizeof(log), "could not access shared memory");
		logger::instance().make_system_fatal_log(log);

		if (smsExists)
		{
			removeServerMediaSession(sms);
		}
		return NULL;
	}
	else
	{
		if (!smsExists)
		{
			char debug[MAX_PATH] = { 0 };
			logger::instance().make_system_info_log("accept new rtsp client");

			sms = createNewSMS(envir(), stream_name);
			addServerMediaSession(sms);
		}
		::CloseHandle(map);
		return sms;
	}
}

/*
// Special code for handling Matroska files:
struct MatroskaDemuxCreationState
{
MatroskaFileServerDemux* demux;
char watchVariable;
};
static void onMatroskaDemuxCreation(MatroskaFileServerDemux* newDemux, void* clientData) {
MatroskaDemuxCreationState* creationState = (MatroskaDemuxCreationState*)clientData;
creationState->demux = newDemux;
creationState->watchVariable = 1;
}
// END Special code for handling Matroska files:

// Special code for handling Ogg files:
struct OggDemuxCreationState {
OggFileServerDemux* demux;
char watchVariable;
};
static void onOggDemuxCreation(OggFileServerDemux* newDemux, void* clientData) {
OggDemuxCreationState* creationState = (OggDemuxCreationState*)clientData;
creationState->demux = newDemux;
creationState->watchVariable = 1;
}
// END Special code for handling Ogg files:
*/

#define NEW_SMS(description) do {\
								char const* descStr = description\
								", DebugerKing RTSP Media Server";\
								sms = ServerMediaSession::createNew(env, stream_name, stream_name, descStr);\
							} while(0)

//static ServerMediaSession* createNewSMS( UsageEnvironment& env, char const* stream_name, FILE* /*fid*/ ) 
static ServerMediaSession * createNewSMS(UsageEnvironment& env, char const* stream_name)
{
	// Use the file name extension to determine the type of "ServerMediaSession":
	//char const* extension		= strrchr( stream_name, '.' );
	if (stream_name == NULL || strlen(stream_name)<1)
		return NULL;

	ServerMediaSession* sms = NULL;
#if defined(REUSE_FRAMED_SOURCE)
	Boolean const reuse_source = True;
#else
	Boolean const reuse_source = False;
#endif

	NEW_SMS("H.264 video element stream");
#if defined(USE_MERGED_NALUNIT)
	OutPacketBuffer::maxSize = 200000; // allow for some possibly large H.264 frames
#else
	OutPacketBuffer::maxSize = SESSION_VIDEO_FRAME_SIZE;
#endif

	sms->addSubsession(h264_video_shared_memory_sms::createNew(env, stream_name, reuse_source));

	/*
	if( strcmp(extension, ".aac")==0 )
	{
	// Assumed to be an AAC Audio (ADTS format) file:
	NEW_SMS("AAC Audio");
	sms->addSubsession( ADTSAudioFileServerMediaSubsession::createNew(env, stream_name, reuse_source) );
	}
	else if( strcmp(extension, ".amr")==0 )
	{
	// Assumed to be an AMR Audio file:
	NEW_SMS("AMR Audio");
	sms->addSubsession( AMRAudioFileServerMediaSubsession::createNew(env, stream_name, reuse_source) );
	}
	else if( strcmp(extension, ".ac3")==0 )
	{
	// Assumed to be an AC-3 Audio file:
	NEW_SMS("AC-3 Audio");
	sms->addSubsession( AC3AudioFileServerMediaSubsession::createNew(env, stream_name, reuse_source) );
	}
	else if( strcmp(extension, ".m4e")==0 )
	{
	// Assumed to be a MPEG-4 Video Elementary Stream file:
	NEW_SMS("MPEG-4 Video");
	sms->addSubsession( MPEG4VideoFileServerMediaSubsession::createNew(env, stream_name, reuse_source) );
	}
	else if( strcmp(extension, ".264" )==0 )
	{
	// Assumed to be a H.264 Video Elementary Stream file:
	NEW_SMS("H.264 Video");
	OutPacketBuffer::maxSize = 100000; // allow for some possibly large H.264 frames
	sms->addSubsession( H264VideoFileServerMediaSubsession::createNew(env, stream_name, reuse_source) );
	}
	else if( strcmp(extension, ".265")==0 )
	{
	// Assumed to be a H.265 Video Elementary Stream file:
	NEW_SMS("H.265 Video");
	OutPacketBuffer::maxSize = 100000; // allow for some possibly large H.265 frames
	sms->addSubsession( H265VideoFileServerMediaSubsession::createNew(env, stream_name, reuse_source) );
	}
	else if( strcmp(extension, ".mp3")==0 )
	{
	// Assumed to be a MPEG-1 or 2 Audio file:
	NEW_SMS("MPEG-1 or 2 Audio");
	// To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
	//#define STREAM_USING_ADUS 1
	// To also reorder ADUs before streaming, uncomment the following:
	//#define INTERLEAVE_ADUS 1
	// (For more information about ADUs and interleaving,
	//  see <http://www.live555.com/rtp-mp3/>)
	Boolean useADUs = False;
	Interleaving* interleaving = NULL;
	#ifdef STREAM_USING_ADUS
	useADUs = True;
	#ifdef INTERLEAVE_ADUS
	unsigned char interleaveCycle[] = {0,2,1,3}; // or choose your own...
	unsigned const interleaveCycleSize = (sizeof interleaveCycle)/(sizeof (unsigned char));
	interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
	#endif
	#endif
	sms->addSubsession( MP3AudioFileServerMediaSubsession::createNew(env, stream_name, reuse_source, useADUs, interleaving) );
	}
	else if( strcmp(extension, ".mpg")==0 )
	{
	// Assumed to be a MPEG-1 or 2 Program Stream (audio+video) file:
	NEW_SMS("MPEG-1 or 2 Program Stream");
	MPEG1or2FileServerDemux* demux = MPEG1or2FileServerDemux::createNew( env, stream_name, reuse_source );
	sms->addSubsession( demux->newVideoServerMediaSubsession() );
	sms->addSubsession( demux->newAudioServerMediaSubsession() );
	}
	else if( strcmp(extension, ".vob")==0 )
	{
	// Assumed to be a VOB (MPEG-2 Program Stream, with AC-3 audio) file:
	NEW_SMS("VOB (MPEG-2 video with AC-3 audio)");
	MPEG1or2FileServerDemux* demux = MPEG1or2FileServerDemux::createNew( env, stream_name, reuse_source );
	sms->addSubsession( demux->newVideoServerMediaSubsession() );
	sms->addSubsession( demux->newAC3AudioServerMediaSubsession() );
	}
	else if( strcmp(extension, ".ts")==0 )
	{
	// Assumed to be a MPEG Transport Stream file:
	// Use an index file name that's the same as the TS file name, except with ".tsx":
	unsigned indexFileNameLen = strlen(stream_name) + 2; // allow for trailing "x\0"
	char* indexFileName = new char[indexFileNameLen];
	sprintf(indexFileName, "%sx", stream_name);
	NEW_SMS("MPEG Transport Stream");
	sms->addSubsession( MPEG2TransportFileServerMediaSubsession::createNew(env, stream_name, indexFileName, reuse_source) );
	delete[] indexFileName;
	}
	else if( strcmp(extension, ".wav")==0 )
	{
	// Assumed to be a WAV Audio file:
	NEW_SMS("WAV Audio Stream");
	// To convert 16-bit PCM data to 8-bit u-law, prior to streaming,
	// change the following to True:
	Boolean convertToULaw = False;
	sms->addSubsession( WAVAudioFileServerMediaSubsession::createNew( env, stream_name, reuse_source, convertToULaw) );
	}
	else if( strcmp(extension, ".dv")==0 )
	{
	// Assumed to be a DV Video file
	// First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
	OutPacketBuffer::maxSize = 300000;

	NEW_SMS("DV Video");
	sms->addSubsession( DVVideoFileServerMediaSubsession::createNew(env, stream_name, reuse_source) );
	}
	else if( strcmp(extension, ".mkv")==0 || strcmp(extension, ".webm")==0 )
	{
	// Assumed to be a Matroska file (note that WebM ('.webm') files are also Matroska files)
	NEW_SMS("Matroska video+audio+(optional)subtitles");

	// Create a Matroska file server demultiplexor for the specified file.
	// (We enter the event loop to wait for this to complete.)
	MatroskaDemuxCreationState creationState;
	creationState.watchVariable = 0;
	MatroskaFileServerDemux::createNew(env, stream_name, onMatroskaDemuxCreation, &creationState);
	env.taskScheduler().doEventLoop( &creationState.watchVariable );

	ServerMediaSubsession* smss;
	while ((smss = creationState.demux->newServerMediaSubsession()) != NULL)
	{
	sms->addSubsession(smss);
	}
	}
	else if( strcmp(extension, ".ogg")==0 || strcmp(extension, ".ogv")==0 || strcmp(extension, ".opus")==0 )
	{
	// Assumed to be an Ogg file
	NEW_SMS("Ogg video and/or audio");

	// Create a Ogg file server demultiplexor for the specified file.
	// (We enter the event loop to wait for this to complete.)
	OggDemuxCreationState creationState;
	creationState.watchVariable = 0;
	OggFileServerDemux::createNew( env, stream_name, onOggDemuxCreation, &creationState );
	env.taskScheduler().doEventLoop( &creationState.watchVariable );

	ServerMediaSubsession* smss;
	while ( (smss=creationState.demux->newServerMediaSubsession())!=NULL )
	{
	sms->addSubsession(smss);
	}
	}
	*/
	return sms;
}
