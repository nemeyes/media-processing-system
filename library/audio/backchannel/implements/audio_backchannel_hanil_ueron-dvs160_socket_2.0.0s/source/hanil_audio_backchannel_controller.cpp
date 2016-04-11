#include "platform.h"
#include "hanil_audio_backchannel_controller.h"
#include <audio_backchannel_device_info.h>
#include <tchar.h>
# include "UeronSendAudio_API.h"
# pragma comment( lib, "UeronSendAudio.lib" )

hanil_audio_backchannel_controller::hanil_audio_backchannel_controller( void )
	: _port_number(2002)
	, _sender_id(-1)
#if defined(_EXPORT_FILE)
	, _file(NULL)
#endif
{

}

hanil_audio_backchannel_controller::~hanil_audio_backchannel_controller( void )
{

}

char* hanil_audio_backchannel_controller::get_vendor_name( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_INFO[AUDIO_BC_HANIL_UREON_DVS160_SOCKET_2_0_0S][AUDIO_BC_VENDOR];
}

char* hanil_audio_backchannel_controller::get_vendor_device_name( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_INFO[AUDIO_BC_HANIL_UREON_DVS160_SOCKET_2_0_0S][AUDIO_BC_DEVICE];
}

char* hanil_audio_backchannel_controller::get_vendor_device_protocol_name( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_INFO[AUDIO_BC_HANIL_UREON_DVS160_SOCKET_2_0_0S][AUDIO_BC_PROTOCOL];
}

char* hanil_audio_backchannel_controller::get_vendor_device_version_name( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_INFO[AUDIO_BC_HANIL_UREON_DVS160_SOCKET_2_0_0S][AUDIO_BC_VERSION];
}

unsigned short hanil_audio_backchannel_controller::get_vendor_id( void )
{

	return VMS_AUDIO_BACKCHANNEL_DEVICE_ID[AUDIO_BC_HANIL_UREON_DVS160_SOCKET_2_0_0S][AUDIO_BC_VENDOR];
}

unsigned short hanil_audio_backchannel_controller::get_vendor_device_id( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_ID[AUDIO_BC_HANIL_UREON_DVS160_SOCKET_2_0_0S][AUDIO_BC_DEVICE];
}

unsigned short hanil_audio_backchannel_controller::get_vendor_device_protocol_id( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_ID[AUDIO_BC_HANIL_UREON_DVS160_SOCKET_2_0_0S][AUDIO_BC_PROTOCOL];
}

unsigned short hanil_audio_backchannel_controller::get_vendor_device_version_id( void )
{
	return VMS_AUDIO_BACKCHANNEL_DEVICE_ID[AUDIO_BC_HANIL_UREON_DVS160_SOCKET_2_0_0S][AUDIO_BC_VERSION];
}

unsigned short hanil_audio_backchannel_controller::set_host_name( char *hostname )
{
	if( hostname && (strlen(hostname)>0) ) 
	{
		strcpy( _hostname, hostname );
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
	}
	else
		return VMS_AUDIO_BACKCHANNEL_FAIL;
}

unsigned short hanil_audio_backchannel_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short hanil_audio_backchannel_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
	}
	else
		return VMS_AUDIO_BACKCHANNEL_FAIL;
}

unsigned short hanil_audio_backchannel_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_AUDIO_BACKCHANNEL_SUCCESS;
	}
	else
		return VMS_AUDIO_BACKCHANNEL_FAIL;
}

unsigned short hanil_audio_backchannel_controller::connect( void )
{
#if defined(_EXPORT_FILE)
	_file = CreateFile( _T("D:\\00.vms\\00.intelli-vms\\Multicaster_v2.02\\Player2xx\\win32\\bin\\audio_backchannel.au"), 
						GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

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
	__try
	{
		UeronSendAudio_Create( &_sender_id );
		if( _sender_id<=0 )
		{
			return VMS_AUDIO_BACKCHANNEL_FAIL;
		}

		int result = UeronSendAudio_SetConnectionInfo( _sender_id, _hostname, _port_number, _user_id, _user_password, 5*1000 );
		if ( result!=UERON_SUCCESS )
		{
			return VMS_AUDIO_BACKCHANNEL_FAIL;
		}

		result = UeronSendAudio_ConnectServer(_sender_id);
		if ( result!=UERON_SUCCESS )
			{
				return VMS_AUDIO_BACKCHANNEL_FAIL;
			}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return VMS_AUDIO_BACKCHANNEL_FAIL;
	}
	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short hanil_audio_backchannel_controller::disconnect( void )
{
	unsigned short value = VMS_AUDIO_BACKCHANNEL_FAIL;
	__try
	{
		UeronSendAudio_DisconnectServer(_sender_id);
		UeronSendAudio_Destroy( _sender_id );
		value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		value = VMS_AUDIO_BACKCHANNEL_FAIL;
	}

#if defined(_EXPORT_FILE)
	CloseHandle( _file );
#endif
	return value;
}

unsigned short hanil_audio_backchannel_controller::get_codec_type( AUDIO_BACKCHANNEL_CODEC_TYPE_T &codec_type, unsigned short &bit_depth, unsigned long &sample_rate  )
{
	codec_type	= AUDIO_BACKCHANNEL_CODEC_TYPE_PCM;
	bit_depth	= 16;
	sample_rate	= 8000;
	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short hanil_audio_backchannel_controller::get_duration( float &duration )
{
	duration	= 0.5;
	return VMS_AUDIO_BACKCHANNEL_SUCCESS;
}

unsigned short hanil_audio_backchannel_controller::process( void *input, int input_size, void *output, int &output_size )
{
	int value = VMS_AUDIO_BACKCHANNEL_SUCCESS;
	int result;
	__try
	{
#if defined(_EXPORT_FILE)
		char *tmp_input		= (char*)input;
		int tmp_input_size	= input_size;
		while( true )
		{
			DWORD written = 0;
			WriteFile( _file, (char*)tmp_input, (DWORD)tmp_input_size, &written, NULL );
			tmp_input_size	-= written;
			tmp_input		+= written;
			if( tmp_input_size<1 )
				break;
		}
#endif

		result = UeronSendAudio_SendStreamBuffer( _sender_id, input_size, (unsigned char*)input );
		//for(int i=0;i<input_size;i=i+642)
		//{
		//	if(i>input_size) i = input_size;
		//	result = UeronSendAudio_SendStreamBuffer( _sender_id, 642, (unsigned char*)input+i );
		//	//Sleep(10);
		//}

		if( result!=UERON_SUCCESS )
		{
			value = VMS_AUDIO_BACKCHANNEL_FAIL;
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		value = VMS_AUDIO_BACKCHANNEL_FAIL;
	}

	return value;
}

base_audio_backchannel_controller* create( void )
{
	return new hanil_audio_backchannel_controller();
}

void destroy( base_audio_backchannel_controller **audio_backchannel_controller )
{
	hanil_audio_backchannel_controller *controller = dynamic_cast<hanil_audio_backchannel_controller*>( (*audio_backchannel_controller) );
	delete controller;
	(*audio_backchannel_controller) = 0;
}