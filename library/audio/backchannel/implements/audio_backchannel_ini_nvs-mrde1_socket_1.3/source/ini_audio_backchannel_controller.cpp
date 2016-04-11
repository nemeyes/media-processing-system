#include "platform.h"
#include "ini_audio_backchannel_controller.h"
#include <audio_backchannel_device_info.h>
#include <tchar.h>
#if defined(_USE_EXIST_LIBRARY)
# include "NVSNetLib.h"
# pragma comment( lib, "NVSNetLib.lib" )
void create_audio_backchannel_render_buffer( LPINI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T audiobc_buffer, int size, unsigned char *source )
{
	audiobc_buffer->buffer_size = size+4;
	audiobc_buffer->buffer = static_cast<unsigned char*>( malloc(audiobc_buffer->buffer_size) );

	audiobc_buffer->buffer[0] = 0x00;
	audiobc_buffer->buffer[1] = 0x01;
	audiobc_buffer->buffer[2] = size/2;
	audiobc_buffer->buffer[3] = 0x00;
	memcpy( &(audiobc_buffer->buffer[4]), source, audiobc_buffer->buffer_size-4 );
}

void destroy_audio_backchannel_render_buffer( LPINI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T audiobc_buffer )
{
	if( audiobc_buffer->buffer )
	{
		free( audiobc_buffer->buffer );
		audiobc_buffer->buffer = 0;
	}
	audiobc_buffer->buffer_size = 0;
}
#endif

ini_audio_backchannel_controller::ini_audio_backchannel_controller( void )
	: _port_number(1852)
#if defined(_EXPORT_FILE)
	, _file(NULL)
#endif
#if defined(_USE_EXIST_LIBRARY)
	, _net_session(NULL)
	, _run_process_message(false)
	, _run_send_audio(false)
#endif
{
#if defined(_USE_EXIST_LIBRARY)
	pthread_mutex_init( &_send_audio_mutex, 0 );
#endif
}

ini_audio_backchannel_controller::~ini_audio_backchannel_controller( void )
{
	pthread_mutex_destroy( &_send_audio_mutex );
}

char* ini_audio_backchannel_controller::get_vendor_name( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_INFO[AUDIO_BC_INI_NVS_MRDE1_SOCKET_1_3][AUDIO_BC_VENDOR];
}

char* ini_audio_backchannel_controller::get_vendor_device_name( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_INFO[AUDIO_BC_INI_NVS_MRDE1_SOCKET_1_3][AUDIO_BC_DEVICE];
}

char* ini_audio_backchannel_controller::get_vendor_device_protocol_name( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_INFO[AUDIO_BC_INI_NVS_MRDE1_SOCKET_1_3][AUDIO_BC_PROTOCOL];
}

char* ini_audio_backchannel_controller::get_vendor_device_version_name( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_INFO[AUDIO_BC_INI_NVS_MRDE1_SOCKET_1_3][AUDIO_BC_VERSION];
}

unsigned short ini_audio_backchannel_controller::get_vendor_id( void )
{

	return VMS_AUDIO_BACKCHANNEL_DEVICE_ID[AUDIO_BC_INI_NVS_MRDE1_SOCKET_1_3][AUDIO_BC_VENDOR];
}

unsigned short ini_audio_backchannel_controller::get_vendor_device_id( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_ID[AUDIO_BC_INI_NVS_MRDE1_SOCKET_1_3][AUDIO_BC_DEVICE];
}

unsigned short ini_audio_backchannel_controller::get_vendor_device_protocol_id( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_ID[AUDIO_BC_INI_NVS_MRDE1_SOCKET_1_3][AUDIO_BC_PROTOCOL];
}

unsigned short ini_audio_backchannel_controller::get_vendor_device_version_id( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_ID[AUDIO_BC_INI_NVS_MRDE1_SOCKET_1_3][AUDIO_BC_VERSION];
}

unsigned short ini_audio_backchannel_controller::set_host_name( char *hostname )
{
	if( hostname && (strlen(hostname)>0) ) 
	{
		strcpy( _hostname, hostname );
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
	}
	else
		return VMS_AUDIO_BACKCHANNEL_FAIL;
}

unsigned short ini_audio_backchannel_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short ini_audio_backchannel_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
	}
	else
		return VMS_AUDIO_BACKCHANNEL_FAIL;
}

unsigned short ini_audio_backchannel_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
	}
	else
		return VMS_AUDIO_BACKCHANNEL_FAIL;
}

unsigned short ini_audio_backchannel_controller::connect( void )
{
#if defined(_EXPORT_FILE)
	_file = CreateFile( _T("audio_backchannel.au"), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	DWORD written = 0;
	DWORD magic_number = 0x2e736e64;
	WriteFile( _file, (char*)&magic_number, (DWORD)sizeof(magic_number), &written, NULL );
	DWORD data_offset = 32;
	WriteFile( _file, (char*)&data_offset, (DWORD)sizeof(data_offset), &written, NULL );
	DWORD data_size = 0xffffffff; //unknown data size
	WriteFile( _file, (char*)&data_size, (DWORD)sizeof(data_size), &written, NULL );
	DWORD encoding = 0x01;//8-bit G.711 mu-law
	WriteFile( _file, (char*)&encoding, (DWORD)sizeof(encoding), &written, NULL );
	DWORD sample_rate = 8000; //8khz
	WriteFile( _file, (char*)&sample_rate, (DWORD)sizeof(sample_rate), &written, NULL );
	DWORD channels = 1; //mono
	WriteFile( _file, (char*)&channels, (DWORD)sizeof(channels), &written, NULL );
#endif

#if defined(_USE_EXIST_LIBRARY)
	pthread_create( &_tid, nullptr, (void* (*)(void*))ini_audio_backchannel_controller::process_message, (void*)this );
	for( int index=0; index<300 && !_run_send_audio; index++ )
		Sleep( 10 );
	if( _run_send_audio )	
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
	else
		return VMS_AUDIO_BACKCHANNEL_FAIL;
#else
	return static_cast<socket_client*>( this )->connect( _hostname, _port_number );
#endif
}

unsigned short ini_audio_backchannel_controller::disconnect( void )
{
#if defined(_USE_EXIST_LIBRARY)
	unsigned short value = VMS_AUDIO_BACKCHANNEL_FAIL;
	if( _run_process_message )
	{
		_run_process_message = false;
		pthread_join( _tid, nullptr );
	}
	value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
#else
	unsigned short value = static_cast<socket_client*>( this )->disconnect();
#endif
#if defined(_EXPORT_FILE)
	CloseHandle( _file );
#endif
	return value;
}

unsigned short ini_audio_backchannel_controller::get_codec_type( AUDIO_BACKCHANNEL_CODEC_TYPE_T &codec_type, unsigned short &bit_depth, unsigned long &sample_rate )
{
	codec_type = AUDIO_BACKCHANNEL_CODEC_TYPE_G726_16;
	bit_depth	= 16;
	sample_rate	= 8000;
	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short ini_audio_backchannel_controller::get_duration( float &duration )
{
	duration	= 0.35;
	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short ini_audio_backchannel_controller::process( void *input, int input_size, void *output, int &output_size )
{
#if defined(_USE_EXIST_LIBRARY)
	if( _net_session )
	{
		if( _render_buffers.size()<30 )
		{
			LPINI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T buffer_render = static_cast<LPINI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T>( malloc(sizeof(INI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T)) );
			create_audio_backchannel_render_buffer( buffer_render, input_size, reinterpret_cast<unsigned char*>(input) );
			pthread_mutex_lock( &_send_audio_mutex );
			_render_buffers.push_back( buffer_render );
			pthread_mutex_unlock( &_send_audio_mutex );
		}
	}
	unsigned short value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
#else
	unsigned short value = post_send_message( tmp_input, tmp_input_size );
#endif
#if defined(_EXPORT_FILE)
	while( true )
	{
		DWORD written = 0;
		WriteFile( _file, (char*)tmp_input, (DWORD)tmp_input_size, &written, NULL );
		tmp_input_size -= written;
		tmp_input += written;
		if( tmp_input_size<1 )
			break;
	}
#endif
	return value;
}

void* ini_audio_backchannel_controller::process_message( void *param )
{
	ini_audio_backchannel_controller* self = static_cast<ini_audio_backchannel_controller*>( param );
	if( self->_net_session )
	{
		self->_net_session->release( self->_net_session );
		self->_net_session = nullptr;
	}

	BYTE connect_type	= /*NETSESSION_LOGIN_TYPE_VIDEO |*/ NETSESSION_LOGIN_TYPE_AUDIO_PLAYBACK;
	self->_net_session	= (CNetSession*)NETSESSION_InitEx( self->_hostname, self->_port_number, 1, NULL, self->_user_id, self->_user_password, connect_type );
	if( self->_net_session==nullptr )
		return nullptr;
	self->_run_process_message = true;
	unsigned int	status;
	bool			login = false;
	while( self->_run_process_message )
	{
		__try
		{
			self->_net_session->select( self->_net_session, 0, 100000 );
			status = self->_net_session->doStateMachine( self->_net_session );
			if( status==NETSESSION_RET_ADNORMAL )
				break;

			if( (status & NETSESSION_RET_LOGIN_OK_MASK)!=0 )
			{
				switch( status & NETSESSION_RET_LOGIN_OK_MASK )
				{
					case NETSESSION_RET_LOGIN_SUCCESS:
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), 
								  "[NETSESSION_RET_LOGIN_SUCCESS] : isVideo = %d, isAudio = %d, isPlaybackAudio : %d\r\n", 
								  self->_net_session->isVideo, self->_net_session->isAudio, self->_net_session->isPlayBackAudio );
						OutputDebugStringA( debug_message );
#endif
						login = true;
						break;
					}
					case NETSESSION_RET_PARTIAL_LOGIN_SUCCESS:
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), 
								  "[NETSESSION_RET_PARTIAL_LOGIN_SUCCESS] : isVideo = %d, isAudio = %d, isPlaybackAudio : %d\r\n", 
								  self->_net_session->isVideo, self->_net_session->isAudio, self->_net_session->isPlayBackAudio );
						OutputDebugStringA( debug_message );
#endif
						login = true;
						break;
					}
					case NETSESSION_RET_GET_VIDEO:
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						if( status & NETSESSION_RET_EVENT_DINPUT )	// V20A & V24A model
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_EVENT_DINPUT]\r\n" );
							OutputDebugStringA( debug_message );
						}
						if( status & NETSESSION_RET_EVENT_DOUTPUT )	// V20A & V24A model
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_EVENT_DOUTPUT]\r\n" );
							OutputDebugStringA( debug_message );
						}
						if( status & NETSESSION_RET_EVENT_VIDEOSIGNAL )
						{

						}

						if ((status & NETSESSION_RET_VIDEO_ENCODING_CHANGED) || login)
						{
							snprintf( debug_message, sizeof(debug_message), "NETSESSION_RET_VIDEO_ENCODING_CHANGED] : encodingType = " );
							OutputDebugStringA( debug_message );
							switch( self->_net_session->serverInfo.videoInfo.encodingType )
							{
							case FCC_MP4S :
								snprintf( debug_message, sizeof(debug_message), "MP4S\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_DX50 :
								snprintf( debug_message, sizeof(debug_message), "FCC_DX50\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_MJPG :
								snprintf( debug_message, sizeof(debug_message), "FCC_MJPG\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_MPG2 :
								snprintf( debug_message, sizeof(debug_message), "FCC_MPG2\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_MPG1 :
								snprintf( debug_message, sizeof(debug_message), "FCC_MPG1\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_H264 :
								snprintf( debug_message, sizeof(debug_message), "FCC_H264\r\n" );
								OutputDebugStringA( debug_message );
								break;
							default :		// never reache here.
								snprintf( debug_message, sizeof(debug_message), "UNKNOWN\r\n" );
								OutputDebugStringA( debug_message );
								break;
							}
							self->_run_process_message = false;
						}
						if( status & NETSESSION_RET_VIDEO_FRAMEMODE_CHANGED )
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_VIDEO_FRAMEMODE_CHANGED] : frame mode = " );
							OutputDebugStringA( debug_message );
							switch( self->_net_session->serverInfo.videoInfo.frameMode )
							{
							case VIFM_IONLY :
								snprintf( debug_message, sizeof(debug_message), "I Only\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case VIFM_IPFRAME :
								snprintf( debug_message, sizeof(debug_message), "IP Only\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case VIFM_IPB :
								snprintf( debug_message, sizeof(debug_message), "IPB\r\n" );
								OutputDebugStringA( debug_message );
								break;
							default :		// never reache here.
								snprintf( debug_message, sizeof(debug_message), "UNKNOWN\r\n" );
								OutputDebugStringA( debug_message );
								break;
							}
						}
						if( status & NETSESSION_RET_VIDEO_VBR_CHANGED )
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_VIDEO_VBR_CHANGED] : " );
							OutputDebugStringA( debug_message );
							if( self->_net_session->serverInfo.videoInfo.vbr==0 )
							{
								snprintf( debug_message, sizeof(debug_message), "CBR\r\n" );
								OutputDebugStringA( debug_message );
							}
							else
							{
								snprintf( debug_message, sizeof(debug_message), "VBR, Q = %d\r\n", self->_net_session->serverInfo.videoInfo.vbr );
								OutputDebugStringA( debug_message );
							}
						}
						if( (status & NETSESSION_RET_VIDEO_BITRATE_CHANGED) || login )
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_VIDEO_BITRATE_CHANGED] : bitrate = %d\r\n", 
									  self->_net_session->serverInfo.videoInfo.bitrate );
							OutputDebugStringA( debug_message );
							self->_run_process_message = false;
						}
						if( (status & NETSESSION_RET_VIDEO_WIDTH_CHANGED) || login )
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_VIDEO_WIDTH_CHANGED] : width = %d\r\n", 
									  self->_net_session->serverInfo.videoInfo.videoWidth );
							OutputDebugStringA( debug_message );
							self->_run_process_message = false;
						}
						if( status & NETSESSION_RET_VIDEO_HEIGHT_CHANGED )
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_VIDEO_HEIGHT_CHANGED] : height = %d\r\n", 
									  self->_net_session->serverInfo.videoInfo.videoHeight );
							OutputDebugStringA( debug_message );
						}
						if (status & NETSESSION_RET_VIDEO_GOPSIZE_CHANGED)
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_VIDEO_GOPSIZE_CHANGED] : group size = %d\r\n", 
									  self->_net_session->serverInfo.videoInfo.groupSize );
							OutputDebugStringA( debug_message );
						}
						if (status & NETSESSION_RET_VIDEO_FPS_CHANGED)
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_VIDEO_FPS_CHANGED] : fps = %d\r\n", 
									  self->_net_session->serverInfo.videoInfo.fps );
							OutputDebugStringA( debug_message );
						}
						if( login )
							login = false;
#endif
						break;
					}
					case NETSESSION_RET_GET_AUDIO :
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						if (status & NETSESSION_RET_AUDIO_ENCODING_CHANGED)
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_AUDIO_ENCODING_CHANGED] : encodingType = " );
							OutputDebugStringA( debug_message );
							switch( self->_net_session->serverInfo.audioInfo.encodingType )
							{
							case FCC_IMAACPCM:
								snprintf( debug_message, sizeof(debug_message), "IMA ADPCM\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_MSADPCM :
								snprintf( debug_message, sizeof(debug_message), "MS ADPCM\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_PCM:
								snprintf( debug_message, sizeof(debug_message), "PCM\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_G726:
								snprintf( debug_message, sizeof(debug_message), "G726\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_G711_A:
								snprintf( debug_message, sizeof(debug_message), "G711 a law\r\n" );
								OutputDebugStringA( debug_message );
								break;
							case FCC_G711_U:
								snprintf( debug_message, sizeof(debug_message), "G711 u law\r\n" );
								OutputDebugStringA( debug_message );
								break;
							default:		// never reache here.
								snprintf( debug_message, sizeof(debug_message), "UNKNOWN\r\n" );
								OutputDebugStringA( debug_message );
								break;
							}
						}

						if( status & NETSESSION_RET_AUDIO_CHANNEL_CHANGED )
						{	
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_AUDIO_CHANNEL_CHANGED] : channel = %d\r\n", self->_net_session->serverInfo.audioInfo.channel );
							OutputDebugStringA( debug_message );
						}
						if( status & NETSESSION_RET_AUDIO_BPS_CHANGED )
						{	
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_AUDIO_BPS_CHANGED] : bit per sample = %d\r\n", self->_net_session->serverInfo.audioInfo.bitPerSample );
							OutputDebugStringA( debug_message );
						}
						if( status & NETSESSION_RET_AUDIO_SAMPLERATE_CHANGED )
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_AUDIO_SAMPLERATE_CHANGED] : sampling rate = %d\r\n", self->_net_session->serverInfo.audioInfo.samplingRate );
							OutputDebugStringA( debug_message );
						}

						if( self->_net_session->isAudio )
						{

						}
#endif
						break;
					}
					case NETSESSION_RET_TYPE_CHANGED :
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_TYPE_CHANGED] : isVideo = %d, isAudio = %d, isPlaybackAudio : %d\r\n", 
							      self->_net_session->isVideo, self->_net_session->isAudio, self->_net_session->isPlayBackAudio );
						OutputDebugStringA( debug_message );
#endif
						break;					
					}
					case NETSESSION_RET_GET_ACK	:			
					{
#if defined(_DEBUG)
						// NETSESSION_LOGIN_TYPE_EVENT or NETSESSION_LOGIN_TYPE_EVENT_VIDEO mode ONLY !!	
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_GET_ACK]\r\n" );
						OutputDebugStringA( debug_message );
#endif
						break;					
					}
					case NETSESSION_RET_GET_EVENT :
					{
						// NETSESSION_LOGIN_TYPE_EVENT mode ONLY !!	
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_GET_EVENT]\r\n" );
						OutputDebugStringA( debug_message );
						if( status & NETSESSION_RET_EVENT_MOTION1 )
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_EVENT_MOTION1]\r\n" );
							OutputDebugStringA( debug_message );
						}
						if( status & NETSESSION_RET_EVENT_DINPUT )	// V20A & V24A model
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_EVENT_DINPUT]\r\n" );
							OutputDebugStringA( debug_message );
						}
						if( status & NETSESSION_RET_EVENT_DOUTPUT )	// V20A & V24A model
						{
							snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_EVENT_DOUTPUT]\r\n" );
							OutputDebugStringA( debug_message );
						}
						if( status & NETSESSION_RET_EVENT_VIDEOSIGNAL )	// V20A & V24A model
						{

						}
#endif
						break;					
					}
					case NETSESSION_RET_GET_EVENT_START :
					{
#if defined(_DEBUG)
						// NETSESSION_LOGIN_TYPE_EVENT_VIDEO mode ONLY !!	
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_GET_EVENT_START]\r\n" );
						OutputDebugStringA( debug_message );
#endif
						break;					
					}
					case NETSESSION_RET_GET_EVENT_STOP :				
					{
#if defined(_DEBUG)
						// NETSESSION_LOGIN_TYPE_EVENT_VIDEO mode ONLY !!				
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_GET_EVENT_STOP]\r\n" );
						OutputDebugStringA( debug_message );
#endif
						break;					
					}
					case NETSESSION_RET_GET_CONTROL :
					{
						break;
					}
				} // switch (status & NETSESSION_RET_LOGIN_OK_MASK)
			}
			else if( (status & NETSESSION_RET_LOGIN_FAIL_MASK)!=0 )	// error
			{
				switch (status & NETSESSION_RET_LOGIN_FAIL_MASK)
				{
					case NETSESSION_RET_LOGIN_FAIL_MAX :
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_LOGIN_FAIL_MAX]\r\n" );
						OutputDebugStringA( debug_message );
#endif
						self->_run_process_message = false;
						break;
					}
					case NETSESSION_RET_LOGIN_FAIL_AUTH :
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_LOGIN_FAIL_AUTH]- Please check ID and password !!!\r\n" );
						OutputDebugStringA( debug_message );
#endif
						self->_run_process_message = false;
						break;
					}
					case NETSESSION_RET_CONNECT_ERROR :
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_CONNECT_ERROR]\r\n" );
						OutputDebugStringA( debug_message );
#endif
						break;
					}
					case NETSESSION_RET_CONNECTION_TIMEOUT :
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_CONNECTION_TIMEOUT]\r\n" );
						OutputDebugStringA( debug_message );
#endif
						break;
					}
					case NETSESSION_RET_NETWORK_ERROR :
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_NETWORK_ERROR]\r\n" );
						OutputDebugStringA( debug_message );
#endif
						break;
					}
					default :		// Never happened.
					{
#if defined(_DEBUG)
						char debug_message[200] = {0};
						snprintf( debug_message, sizeof(debug_message), "[NETSESSION_RET_LOGIN_FAIL_MASK] - Default : error !!!\r\n" );
						OutputDebugStringA( debug_message );
#endif
						break;
					}
				}
				Sleep(3);
			}

			// Send audio to Server.
			if( self->_net_session->status==NETSESSION_STAT_RECV/*IsNETSESSION_STAT_RECV(self->_net_session)*/ )
			{
				if( self->_net_session->isPlayBackAudio )
				{
					// Create Send Audio process thread...
					if( !self->_run_send_audio )
					{
						pthread_create( &self->_send_tid, nullptr, (void* (*)(void*))ini_audio_backchannel_controller::process_send_audio, (void*)self );
						for( int index=0; index<100 || !self->_run_send_audio; index++ )
							Sleep( 10 );
					}
				}
			}
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{

		}
	}

	if( self->_run_send_audio )
	{
		self->_run_send_audio = false;
		pthread_join( self->_send_tid, nullptr );
	}

	if( self->_net_session )
		self->_net_session->release( self->_net_session );
	self->_net_session = nullptr;

	return nullptr;
}

void* ini_audio_backchannel_controller::process_send_audio( void *param )
{
	ini_audio_backchannel_controller* self = static_cast<ini_audio_backchannel_controller*>( param );
	if( !strcmp((char*)self->_net_session->serverInfo.serverName, "EARTH") || 
		!strcmp((char*)self->_net_session->serverInfo.serverName, "MARS") )
	{
		memset( self->_send_buffer, 0x00, sizeof(self->_send_buffer) );
		self->_offset			= 0;
		self->_run_send_audio	= true;
		while( self->_run_send_audio )
		{
			if( self->_render_buffers.size()<1 )
			{
				Sleep( 10 );
				continue;
			}
			pthread_mutex_lock( &self->_send_audio_mutex );
			INI_AUDIO_BACKCHANNEL_RENDER_BUFFER_T *buffer_render = self->_render_buffers.front();
			if( buffer_render )
			{
				memcpy( &self->_send_buffer[self->_offset], buffer_render->buffer, buffer_render->buffer_size );
				self->_offset += buffer_render->buffer_size;

				if( self->_offset>=900 )
				{
					self->_net_session->sendAudioEx( self->_net_session,
													 self->_net_session->serverInfo.audioInfo.encodingType,
													 self->_net_session->serverInfo.audioInfo.channel,		
													 self->_net_session->serverInfo.audioInfo.bitPerSample,	
													 self->_net_session->serverInfo.audioInfo.samplingRate,	
													 self->_send_buffer, 
													 self->_offset );
					self->_offset = 0;
					memset( self->_send_buffer, 0x00, sizeof(self->_send_buffer) );
				}
				destroy_audio_backchannel_render_buffer( buffer_render );
				free( buffer_render );
				buffer_render = 0;
				self->_render_buffers.erase( self->_render_buffers.begin() );
			}
			pthread_mutex_unlock( &self->_send_audio_mutex );
			Sleep( 10 );
		}
		//self->_run_send_audio	= false;
	}

	return nullptr;
}

base_audio_backchannel_controller* create( void )
{
	return new ini_audio_backchannel_controller();
}

void destroy( base_audio_backchannel_controller **audio_backchannel_controller )
{
	ini_audio_backchannel_controller *controller = dynamic_cast<ini_audio_backchannel_controller*>( (*audio_backchannel_controller) );
	delete controller;
	(*audio_backchannel_controller) = 0;
}