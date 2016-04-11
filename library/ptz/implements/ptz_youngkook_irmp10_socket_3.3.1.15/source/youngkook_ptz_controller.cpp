#if defined(WIN32)
#include <WinSock2.h>
#include <Windows.h>
#include <WinBase.h>
#elif defined(UBUNTU)
#elif defined(ARM)
#endif

#include "platform.h"
#include <ptz_device_info.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include "youngkook_ptz_controller.h"
#include "socket_client.h"

youngkook_ptz_controller::youngkook_ptz_controller( void )
{
	set_port_number(3001);
}

youngkook_ptz_controller::~youngkook_ptz_controller( void )
{

}

char* youngkook_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[YOUNGKOOK_IRMP10_SOCKET_V_3_3_1_15][VENDOR];
}

char* youngkook_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[YOUNGKOOK_IRMP10_SOCKET_V_3_3_1_15][DEVICE];
}

char* youngkook_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[YOUNGKOOK_IRMP10_SOCKET_V_3_3_1_15][PROTOCOL];
}

char* youngkook_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[YOUNGKOOK_IRMP10_SOCKET_V_3_3_1_15][VERSION];
}

unsigned short youngkook_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[YOUNGKOOK_IRMP10_SOCKET_V_3_3_1_15][VENDOR];
}

unsigned short youngkook_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[YOUNGKOOK_IRMP10_SOCKET_V_3_3_1_15][DEVICE];
}

unsigned short youngkook_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[YOUNGKOOK_IRMP10_SOCKET_V_3_3_1_15][PROTOCOL];
}

unsigned short youngkook_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[YOUNGKOOK_IRMP10_SOCKET_V_3_3_1_15][VERSION];
}

unsigned short youngkook_ptz_controller::set_host_name( char *host_name )
{
	if( host_name && (strlen(host_name)>0) ) 
	{
		strcpy( _hostname, host_name );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short youngkook_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short youngkook_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short youngkook_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short	youngkook_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short youngkook_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short youngkook_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short youngkook_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short youngkook_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short youngkook_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short youngkook_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short youngkook_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short youngkook_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short youngkook_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short youngkook_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short youngkook_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short youngkook_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short youngkook_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short youngkook_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	switch( UINT8(osd) )
	{
		case PTZ_OSE_MENU_OPEN :
		{
			value = show_osd(true);
			break;
		}
		case PTZ_OSE_MENU_CLOSE :
		{
			value = show_osd(false);
			break;
		}
		case PTZ_OSE_MENU_UP :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_OSE_MENU_DOWN :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_OSE_MENU_LEFT :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_OSE_MENU_RIGHT :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_OSE_MENU_SELECT :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_OSE_MENU_BACK :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
	}
	return value;
}

unsigned short youngkook_ptz_controller::goto_home_position(float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::set_home_position(void)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	std::map<int,int> preset_map;
	std::map<int,int>::iterator it;
	
	for(int i=1;i<=20;i++)
	{   
		preset_map[i] = i;
	}

	(*length) = preset_map.size();
	if( (*length)>0 )
	{
		(*aliases) = static_cast<int*>( malloc(sizeof(int**)*(*length)) );
		int index = 0;
		for( it=preset_map.begin(); it!=preset_map.end(); it++, index++ )
		{
			(*aliases)[index] = it->second;
		}
	}
	return VMS_PTZ_SUCCESS;
	
}

unsigned short	youngkook_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::add_preset2( int &alias )
{
	unsigned short value = VMS_PTZ_FAIL;
	if(alias>50) return VMS_PTZ_UNSUPPORTED_COMMAND;

	value = send_data("ADD_PRESET", alias);

	return value;
	
}

unsigned short	youngkook_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

unsigned short youngkook_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::goto_preset2( int alias )
{
	unsigned short value = VMS_PTZ_FAIL;
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;

	value = send_data("GOTO_PRESET", alias);

	return VMS_PTZ_SUCCESS;
	
}

unsigned short	youngkook_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	unsigned short value = VMS_PTZ_FAIL;

	int real_pan_sensitive	= int(get_continuous_sensitive_value(abs(pan_sensitive)));
	int real_tilt_sensitive	= int(get_continuous_sensitive_value(abs(tilt_sensitive)));

	if(pan_sensitive==0 && tilt_sensitive==0&&zoom_sensitive==0) //정지
	{
		value = stop_move();
	}
	else if(pan_sensitive!=0 && tilt_sensitive==0)//좌우
	{
		if(pan_sensitive<0) //left
		{
			value = send_data("PTZ_CONTINUOUS_MOVE_LEFT", real_pan_sensitive, 0);
		}
		else //right
		{
			value = send_data( "PTZ_CONTINUOUS_MOVE_RIGHT", real_pan_sensitive, 0);
		}
	}
	else if(tilt_sensitive!=0&&pan_sensitive==0)//상하
	{
		if(tilt_sensitive>0)
		{
			value = send_data( "PTZ_CONTINUOUS_MOVE_UP", 0, real_tilt_sensitive);
		}
		else
		{
			value = send_data( "PTZ_CONTINUOUS_MOVE_DOWN", 0, real_tilt_sensitive);
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//좌상
	{
		value = send_data( "PTZ_CONTINUOUS_MOVE_LEFTUP", real_pan_sensitive, real_tilt_sensitive);
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//우하
	{
		value = send_data( "PTZ_CONTINUOUS_MOVE_RIGHTDOWN", real_pan_sensitive, real_tilt_sensitive);
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//좌하
	{
		value = send_data( "PTZ_CONTINUOUS_MOVE_LEFTDOWN", real_pan_sensitive, real_tilt_sensitive);
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//우상
	{
		value = send_data( "PTZ_CONTINUOUS_MOVE_RIGHTUP", real_pan_sensitive, real_tilt_sensitive);
	}
	else if(zoom_sensitive>0)//줌인
	{
		value = send_data( "PTZ_CONTINUOUS_MOVE_ZOOMIN", 0, 0);
	}
	else if(zoom_sensitive<0)//줌아웃
	{
		value = send_data( "PTZ_CONTINUOUS_MOVE_ZO0MOUT", 0, 0);
	}
	return value;
}

unsigned short youngkook_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(move) )
	{
		case PTZ_CONTINUOUS_MOVE_UP :
		{
			value = continuous_move( 0, speed, 0, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFT :
		{
			value = continuous_move( -speed, 0, 0, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHT :
		{
			value = continuous_move( speed, 0, 0, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_DOWN :
		{
			value = continuous_move( 0, -speed, 0, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTUP :
		{
			value = continuous_move( -speed, speed, 0, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTUP :
		{
			value = continuous_move( speed, speed, 0, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTDOWN :
		{
			value = continuous_move( -speed, -speed, 0, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTDOWN :
		{
			value = continuous_move( speed, -speed, 0, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZOOMIN :
		{
			value = continuous_move( 0, 0, speed, timeout );
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZO0MOUT :
		{
			value = continuous_move( 0, 0, -speed, timeout );
			break;
		}
	}
	return value;
	
}

float youngkook_ptz_controller::get_continuous_sensitive_value( float sensitive )
{
	query_limits();

	if( _speed_min==_min_cspeed && _speed_max==_max_cspeed )
		return sensitive;
	else if( sensitive<_speed_min ) 
		return _min_cspeed;
	else if( sensitive>_speed_max ) 
		return _max_cspeed;
	else
	{
		float real_sensitive = float(sensitive*(_max_cspeed-_min_cspeed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

unsigned short	youngkook_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed)
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	return value;
}

unsigned short	youngkook_ptz_controller::stop_move( void )
{
	return send_data("STOP",0,0);
}

unsigned short	youngkook_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::query_limits( void )
{
	_min_pan = 0.0f;
	_max_pan = 0.0f;
	_min_tilt = 0.0f;
	_max_tilt = 0.0f;
	_min_zoom = 0.0f;;
	_max_zoom = 0.0f;
	_min_speed = 0.0f;
	_max_speed = 0.0f;
	_min_cspeed = 0.0f;
	_max_cspeed = 100.0f;
	return VMS_PTZ_SUCCESS;
}

unsigned short	youngkook_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

// Grouping
unsigned short	youngkook_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short  youngkook_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short youngkook_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	youngkook_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

float youngkook_ptz_controller::get_rpan_sensitive_value( float sensitive )
{	
	return 0;
}

float youngkook_ptz_controller::get_rtilt_sensitive_value( float sensitive )
{
	return 0;
}

float youngkook_ptz_controller::get_rzoom_sensitive_value( float sensitive )
{
	return 0;
}

float youngkook_ptz_controller::get_apan_sensitive_value( float sensitive )
{
	return 0;
}

float youngkook_ptz_controller::get_atilt_sensitive_value( float sensitive )
{
	return 0;
}

float youngkook_ptz_controller::get_azoom_sensitive_value( float sensitive )
{
	return 0;
}

float youngkook_ptz_controller::get_speed_sensitive_value( float sensitive )
{
	return 0;
}

float youngkook_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
{
	return 0;
}

float youngkook_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
{
	return 0;
}

float youngkook_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
{
	return 0;
}

unsigned short youngkook_ptz_controller::send_data(std::string cmd, int data1, int data2)
{
	char send_msg[80] ={0,};
	char str_data1[10] = {0,};
	sprintf(str_data1,"%d",abs(data1));
	char str_data2[10] = {0,};
	sprintf(str_data2,"%d",abs(data2));
	char type[10] = {0,};

	char s_msg1[] = {0xf1,0xf5,0xea,0xf5,0x00,0x00,0x4b,0x00,0x00,0x00
	   	            ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x00};
	char s_msg2[]= {0x09,0x50,0x52,0x4f,0x58,0x59,0x09,0x43,0x4d,0x44
		           ,0x09,0x50,0x54,0x5a,0x5f,0x43,0x4f,0x4e,0x54,0x52
				   ,0x4f,0x4c,0x09,0x30,0x09};
	char dot[] = {0x09};
	char e_msg[] = {0x09, 0x30, 0x0a, 0x0a, 0x0a};
	int pos = 0;

	//type=> 9정지,1상,2하,3좌,4우,5우상,6좌상,7우하,8좌하,25프리셋이동,28프리셋등록,10줌인,11줌아웃
	if(cmd=="PTZ_CONTINUOUS_MOVE_UP")
	{
		sprintf(type,"%d",1);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFT")
	{
		sprintf(type,"%d",3);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHT")
	{
		sprintf(type,"%d",4);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_DOWN")
	{
		sprintf(type,"%d",2);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFTUP")
	{
		sprintf(type,"%d",6);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHTUP")
	{
		sprintf(type,"%d",5);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFTDOWN")
	{
		sprintf(type,"%d",8);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHTDOWN")
	{
		sprintf(type,"%d",7);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_ZOOMIN")
	{
		sprintf(type,"%d",10);
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_ZO0MOUT")
	{
		sprintf(type,"%d",11);
	}
	else if(cmd=="STOP") 
	{
		sprintf(type,"%d",9);
	}
	else if(cmd=="GOTO_PRESET") 
	{
		sprintf(type,"%d",25);
	}
	else if(cmd=="ADD_PRESET") 
	{
		sprintf(type,"%d",28);
	}
	//else if(cmd=="REMOVE_PRESET") 
	//{
	//}

	memcpy(send_msg,s_msg1,sizeof(s_msg1));
	pos = sizeof(s_msg1);
	memcpy(&send_msg[pos],_hostname,strlen(_hostname));
	pos = pos+strlen(_hostname);
	
	
	memcpy(&send_msg[pos],s_msg2,sizeof(s_msg2));
	pos = pos+sizeof(s_msg2);
	//type
	memcpy(&send_msg[pos],type,strlen(type));
	pos=pos+strlen(type);
	memcpy(&send_msg[pos],dot,sizeof(dot));
	pos=pos+sizeof(dot);
	//data1 : pan속도 or 프리셋번호 
	memcpy(&send_msg[pos],str_data1,strlen(str_data1));
	pos=pos+strlen(str_data1);
	memcpy(&send_msg[pos],dot,sizeof(dot));
	pos=pos+sizeof(dot);
	//data2 : tilt속도 
	memcpy(&send_msg[pos],str_data2,strlen(str_data2));
	pos=pos+strlen(str_data2);
	memcpy(&send_msg[pos],e_msg,sizeof(e_msg));


	//전송
	socket_client client( _hostname, _port_number );
	client.send_msg(send_msg,sizeof(send_msg));

	return VMS_PTZ_SUCCESS;
}

unsigned short youngkook_ptz_controller::show_osd(bool show)
{
	char send_msg[73] ={0,};
	char s_msg[] = {0xf1,0xf5,0xea,0xf5,0x00,0x00,0x49,0x00,0x00,0x00
	   	           ,0x00,0x00,0x62,0xd4,0x93,0x80,0x00,0x00,0x35,0x00};
	char channel_msg[]= {0x09,0x50,0x52,0x4f,0x58,0x59,0x09,0x50,0x41,0x52
		                ,0x41,0x53,0x45,0x54,0x09,0x4f,0x53,0x44,0x5f,0x44
				     	,0x49,0x53,0x50,0x4c,0x41,0x59,0x09,0x30,0x09,0x32
					    ,0x09,0x38,0x09,0x30,0x09};
	char date_msg[]   = {0x09,0x50,0x52,0x4f,0x58,0x59,0x09,0x50,0x41,0x52
		                ,0x41,0x53,0x45,0x54,0x09,0x4f,0x53,0x44,0x5f,0x44
					    ,0x49,0x53,0x50,0x4c,0x41,0x59,0x09,0x30,0x09,0x31
					    ,0x09,0x30,0x09,0x30,0x09};
	char on_off[2] = {0,};
	char e_msg[] = {0x0a, 0x0a, 0x0a};
	int pos = 0;
	if(show) sprintf(on_off, "%d", 1);
	else sprintf(on_off, "%d", 0);

	socket_client client( _hostname, _port_number );

	//********channel osd 처리
	memcpy(send_msg,s_msg,sizeof(s_msg));
	pos = sizeof(s_msg);
	memcpy(&send_msg[pos],_hostname,strlen(_hostname));
	pos = pos+strlen(_hostname);

	memcpy(&send_msg[pos],channel_msg,sizeof(channel_msg));
	pos = pos+sizeof(channel_msg);

	memcpy(&send_msg[pos],on_off,strlen(on_off));
	pos = pos+strlen(on_off);

	memcpy(&send_msg[pos],e_msg,sizeof(e_msg));

	//전송
	client.send_msg(send_msg,sizeof(send_msg));

	//********date osd 처리
	memset(send_msg,0x00,sizeof(send_msg));
	memcpy(send_msg,s_msg,sizeof(s_msg));
	pos = sizeof(s_msg);
	memcpy(&send_msg[pos],_hostname,strlen(_hostname));
	pos = pos+strlen(_hostname);

	memcpy(&send_msg[pos],date_msg,sizeof(date_msg));
	pos = pos+sizeof(date_msg);

	memcpy(&send_msg[pos],on_off,strlen(on_off));
	pos = pos+strlen(on_off);

	memcpy(&send_msg[pos],e_msg,sizeof(e_msg));

	//전송
	client.send_msg(send_msg,sizeof(send_msg));

	return VMS_PTZ_SUCCESS;
}

base_ptz_controller* create( void )
{
	return new youngkook_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	youngkook_ptz_controller *youngkook_controller = dynamic_cast<youngkook_ptz_controller*>( (*ptz_controller) );
	delete youngkook_controller;
	(*ptz_controller) = 0;
}
