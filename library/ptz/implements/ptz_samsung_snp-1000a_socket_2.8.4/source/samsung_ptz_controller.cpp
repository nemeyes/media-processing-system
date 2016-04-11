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
#include "samsung_ptz_controller.h"
#include "socket_client.h"
#include "http_client.h"

samsung_ptz_controller::samsung_ptz_controller( void )
{
	set_port_number(4000);
}

samsung_ptz_controller::~samsung_ptz_controller( void )
{

}

char* samsung_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SAMSUNG_SNP_1000A_SOCKET_V_2_8_4][VENDOR];
}

char* samsung_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SAMSUNG_SNP_1000A_SOCKET_V_2_8_4][DEVICE];
}

char* samsung_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SAMSUNG_SNP_1000A_SOCKET_V_2_8_4][PROTOCOL];
}

char* samsung_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SAMSUNG_SNP_1000A_SOCKET_V_2_8_4][VERSION];
}

unsigned short samsung_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[SAMSUNG_SNP_1000A_SOCKET_V_2_8_4][VENDOR];
}

unsigned short samsung_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[SAMSUNG_SNP_1000A_SOCKET_V_2_8_4][DEVICE];
}

unsigned short samsung_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[SAMSUNG_SNP_1000A_SOCKET_V_2_8_4][PROTOCOL];
}

unsigned short samsung_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[SAMSUNG_SNP_1000A_SOCKET_V_2_8_4][VERSION];
}

unsigned short samsung_ptz_controller::set_host_name( char *host_name )
{
	if( host_name && (strlen(host_name)>0) ) 
	{
		strcpy( _hostname, host_name );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short samsung_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short samsung_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short	samsung_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	char msg[22] = {0,};

	switch( UINT8(osd) )
	{
		case PTZ_OSE_MENU_OPEN :
		{
			char tmp_msg[] = {0x53,0x44,0x56,0x52,0x00,0x10,0x0c,0x00,0x11,0x35,0x00,0x06,0x0a,0x00,0x00,0x00,0xa0,0x07,0x00,0x00,0x00,0x00};
			memcpy(msg, tmp_msg, sizeof(msg));
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_OSE_MENU_CLOSE :
		{
			char tmp_msg[] = {0x53,0x44,0x56,0x52,0x00,0x10,0x0c,0x00,0x11,0x35,0x00,0x06,0x0a,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x00};
			memcpy(msg, tmp_msg, sizeof(msg));
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_OSE_MENU_UP :
		{
			char tmp_msg[] = {0x53,0x44,0x56,0x52,0x00,0x10,0x0c,0x00,0x11,0x35,0x00,0x07,0x0a,0x00,0x00,0x00,0xa0,0x02,0x00,0x00,0x00,0x00}; 
			memcpy(msg, tmp_msg, sizeof(msg));
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_OSE_MENU_DOWN :
		{
			char tmp_msg[] = {0x53,0x44,0x56,0x52,0x00,0x10,0x0c,0x00,0x11,0x35,0x00,0x07,0x0a,0x00,0x00,0x00,0xa0,0x03,0x00,0x00,0x00,0x00};
			memcpy(msg, tmp_msg, sizeof(msg));
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_OSE_MENU_LEFT :
		{
			char tmp_msg[] = {0x53,0x44,0x56,0x52,0x00,0x10,0x0c,0x00,0x11,0x35,0x00,0x07,0x0a,0x00,0x00,0x00,0xa0,0x04,0x00,0x00,0x00,0x00};
			memcpy(msg, tmp_msg, sizeof(msg));
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_OSE_MENU_RIGHT :
		{
			char tmp_msg[] = {0x53,0x44,0x56,0x52,0x00,0x10,0x0c,0x00,0x11,0x35,0x00,0x07,0x0a,0x00,0x00,0x00,0xa0,0x05,0x00,0x00,0x00,0x00};
			memcpy(msg, tmp_msg, sizeof(msg));
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_OSE_MENU_SELECT :
		{
			char tmp_msg[] = {0x53,0x44,0x56,0x52,0x00,0x10,0x0c,0x00,0x11,0x35,0x00,0x07,0x0a,0x00,0x00,0x00,0xa0,0x01,0x00,0x00,0x00,0x00};
			memcpy(msg, tmp_msg, sizeof(msg));
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_OSE_MENU_BACK :
		{
			char tmp_msg[] = {0x53,0x44,0x56,0x52,0x00,0x10,0x0c,0x00,0x11,0x35,0x00,0x07,0x0a,0x00,0x00,0x00,0xa0,0x08,0x00,0x00,0x00,0x00};
			memcpy(msg, tmp_msg, sizeof(msg));
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}
	
	if( !_connected )
	{
		_step = 0;
		connect( _hostname, _port_number );
		login();
		post_recv_message( 16 );
		for( int index=0; _step<3; index++ )
			Sleep( 10 );
	}

	post_send_message(msg, sizeof(msg));
	

	return value;
}

unsigned short samsung_ptz_controller::goto_home_position(float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::set_home_position(void)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	samsung_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::get_preset_list2( int **aliases, int *length )
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

unsigned short	samsung_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::add_preset2( int &alias )
{
	if( !_connected )
	{
		_step = 0;
		connect( _hostname, _port_number );
		login();
		post_recv_message( 16 );
		for( int index=0; _step<3; index++ )
			Sleep( 10 );
	}

	CTRL con;
	init_ctrl(&con);
	if(alias>20) 
		return VMS_PTZ_UNSUPPORTED_COMMAND;

	set_cmd_data(&con, "ADD_PRESET", alias);
	post_send_message((char*)(&con), sizeof(con));

	return VMS_PTZ_SUCCESS;
}

unsigned short	samsung_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::remove_preset2( int alias )
{
	if( !_connected )
	{
		_step = 0;
		connect( _hostname, _port_number );
		login();
		post_recv_message( 16 );
		for( int index=0; _step<3; index++ )
			Sleep( 10 );
	}

	CTRL con;
	init_ctrl(&con);
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;

	set_cmd_data(&con, "REMOVE_PRESET", alias);
	post_send_message((char*)(&con), sizeof(con));

	return VMS_PTZ_SUCCESS;
}

unsigned short	samsung_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::goto_preset2( int alias )
{
	if( !_connected )
	{
		_step = 0;
		connect( _hostname, _port_number );
		login();
		post_recv_message( 16 );
		for( int index=0; _step<3; index++ )
			Sleep( 10 );
	}

	CTRL con;
	init_ctrl(&con);
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;

	set_cmd_data(&con, "GOTO_PRESET", alias);
	post_send_message((char*)(&con), sizeof(con));

	return VMS_PTZ_SUCCESS;
}

unsigned short	samsung_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	unsigned short value = VMS_PTZ_SUCCESS;

	if(pan_sensitive==0 && tilt_sensitive==0&&zoom_sensitive==0) //정지
	{
		value = stop_move();
	}
	else if(pan_sensitive!=0 && tilt_sensitive==0)//좌우
	{
		if(pan_sensitive<0) //left
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFT, pan_sensitive, 0);
		}
		else //right
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHT, pan_sensitive, 0);
		}
	}
	else if(tilt_sensitive!=0&&pan_sensitive==0)//상하
	{
		if(tilt_sensitive>0)
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_UP, tilt_sensitive, 0);
		}
		else
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_DOWN, tilt_sensitive, 0);
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//좌상
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTUP, get_max_abs_value(pan_sensitive, tilt_sensitive), 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//우하
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTDOWN, get_max_abs_value(pan_sensitive, tilt_sensitive), 0);
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//좌하
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTDOWN, get_max_abs_value(pan_sensitive, tilt_sensitive), 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//우상
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTUP, get_max_abs_value(pan_sensitive, tilt_sensitive), 0);
	}
	else if(zoom_sensitive>0)//줌인
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_ZOOMIN, zoom_sensitive, 0);
	}
	else if(zoom_sensitive<0)//줌아웃
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_ZO0MOUT, zoom_sensitive, 0);
	}
	
	return value;
}

float samsung_ptz_controller::get_max_abs_value( float a, float b )
{
	if(abs(a)>abs(b)) 
		return abs(a);
	else 
		return abs(b);
}

unsigned short samsung_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	if( !_connected )
	{
		_step = 0;
		connect( _hostname, _port_number );
		login();
		post_recv_message( 16 );
		for( int index=0; _step<3 /*&& index<100*/; index++ )
			Sleep( 10 );
	}

	CTRL con;
	init_ctrl(&con);
	float real_spd = get_continuous_sensitive_value(speed);
	int spd = (int)real_spd;

	switch( UINT8(move) )
	{
		case PTZ_CONTINUOUS_MOVE_UP :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_UP", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFT :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_LEFT", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHT :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_RIGHT", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_DOWN :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_DOWN", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTUP :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_LEFTUP", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTUP :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_RIGHTUP", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTDOWN :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_LEFTDOWN", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTDOWN :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_RIGHTDOWN", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZOOMIN :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_ZOOMIN", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZO0MOUT :
		{
			set_cmd_data(&con, "PTZ_CONTINUOUS_MOVE_ZO0MOUT", spd);
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}

	if(value == VMS_PTZ_UNSUPPORTED_COMMAND) return value;

	post_send_message( (char*)(&con), sizeof(con) );

	return value;
}

float samsung_ptz_controller::get_continuous_sensitive_value( float sensitive )
{
	query_limits();
	if( _speed_min==_min_cspeed && _speed_max==_max_cspeed )
		return sensitive;

	if( sensitive<_speed_min ) 
		return _min_cspeed;
	else if( sensitive>_speed_max ) 
		return _max_cspeed;
	else
	{
		float real_sensitive = float(sensitive*(_max_cspeed-_min_cspeed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

unsigned short	samsung_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	samsung_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	samsung_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed)
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	return value;
}

unsigned short	samsung_ptz_controller::stop_move( void )
{
	if( !_connected )
	{
		_step = 0;
		connect( _hostname, _port_number );
		login();
		post_recv_message( 16 );
		for( int index=0; _step<3 /*&& index<100*/; index++ )
			Sleep( 10 );
	}

	CTRL con;
	init_ctrl( &con );
	set_cmd_data( &con, "STOP", 0 );
	post_send_message( (char*)(&con), sizeof(con) );
	return VMS_PTZ_SUCCESS;
}

unsigned short	samsung_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	samsung_ptz_controller::query_limits( void )
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
	_max_cspeed = 64.0f;
	return VMS_PTZ_SUCCESS;
}

unsigned short	samsung_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

// Grouping
unsigned short	samsung_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	samsung_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::remove_preset_tour2( int tour_name )
{
	unsigned short value = VMS_PTZ_FAIL;

	http_client client( _hostname, 80, "/group_ok.cgi" );
	char post_data[5000] = {0,};
	char *response_data = 0;
	char tmp_str[100]= {0,};
	std::string data_str = "";
	sprintf(tmp_str, "GROUP_SAVE=1&GROUP_NO_SELECT=%d&GROUP_LIST=CH 1 GROUP %d", tour_name, tour_name);
	data_str.append(tmp_str);
	data_str.append("%0D%0A");
	data_str.append("Seq No.	Preset No.		Speed	Dwell Time%0D%0A");
	data_str.append("=====================================%0D%0A");
	data_str.append("&GROUP_PRESETNO=1&GROUP_SPEED=64&GROUP_TIME=3");
	strncpy(post_data, data_str.c_str(), data_str.size());

	if( !client.post_request(_user_id, _user_password, post_data, &response_data) ) 
		value = VMS_PTZ_FAIL;
	else 
		value = VMS_PTZ_SUCCESS;

	return value;
}

unsigned short  samsung_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	if( !_connected )
	{
		_step = 0;
		connect( _hostname, _port_number );
		login();
		post_recv_message( 16 );
		for( int index=0; _step<3; index++ )
			Sleep( 10 );
	}

	CTRL con;
	init_ctrl(&con);

	switch( UINT8(cmd) )
	{
		case PTZ_TOUR_CMD_START  :						
			set_cmd_data(&con, "START_TOUR", tour_name);				
			break;

		case PTZ_TOUR_CMD_STOP :
			set_cmd_data(&con, "STOP", 0);				
			break;

		case PTZ_TOUR_CMD_PAUSE : 
			return VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
	}

	post_send_message((char*)(&con), sizeof(con));

	return VMS_PTZ_SUCCESS;
}

unsigned short	samsung_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	unsigned short value = VMS_PTZ_FAIL;

	http_client client( _hostname, 80, "/group_ok.cgi" );
	char post_data[5000] = {0,};
	char *response_data = 0;
	char tmp_str[100]= {0,};
	std::string data_str = "";
	sprintf(tmp_str, "GROUP_SAVE=1&GROUP_NO_SELECT=%s&GROUP_LIST=CH 1 GROUP %s", tour->tour_name, tour->tour_name);
	data_str.append(tmp_str);
	data_str.append("%0D%0A");
	data_str.append("Seq No.	Preset No.		Speed	Dwell Time%0D%0A");
	data_str.append("=====================================%0D%0A");
	int preset_no = 0;
	int speed = 0;
	int staytime = 0;
	for(int i=0; i<tour->size_of_tour_spots;i++)
	{
		PTZ_TOUR_SPOT_T* tsp = &(tour->tour_spots[i]);	
		preset_no = atoi(tsp->preset_alias);
		speed = int(get_continuous_sensitive_value(tsp->speed));
		staytime = tsp->stay_time/1000;
		//sprintf(tmp_str, "[%d]	%d		%d	%d", i+1, preset_no, speed, staytime);
		sprintf(tmp_str, "[%d] %d  %d %d", i+1, preset_no, speed, staytime);
		data_str.append(tmp_str);
		data_str.append("%0D%0A");
	}
	sprintf(tmp_str, "&GROUP_PRESETNO=%d&GROUP_SPEED=%d&GROUP_TIME=%d", preset_no, speed, staytime);
	data_str.append(tmp_str);
	
	
	strncpy(post_data, data_str.c_str(), data_str.size());

	if( !client.post_request(_user_id, _user_password, post_data, &response_data) ) 
		value = VMS_PTZ_FAIL;
	else 
		value = VMS_PTZ_SUCCESS;

	return value;
}

unsigned short	samsung_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	samsung_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	for(int i=1;i<=6;i++)
	{
		_tour_list[i] = i;
	}

	(*size_of_tours) = _tour_list.size();   
	*tour = new PTZ_TOUR_T[_tour_list.size()];   
	
    std::map<int,int>::iterator it;
	int idx = 0;
    for(it=_tour_list.begin(); it!=_tour_list.end(); ++it, idx++)   
	{	char tmp[3] = {0,};	
		itoa(it->first, tmp, 10);
		strncpy( (*tour+idx)->tour_name, tmp, sizeof((*tour+idx)->tour_name) );
	}		
	
	return VMS_PTZ_SUCCESS;
}

float samsung_ptz_controller::get_speed_sensitive_value( float sensitive )
{
	query_limits();

	if( _speed_min==_min_speed && _speed_max==_max_speed )
		return sensitive;

	if( abs(sensitive)>abs(_speed_max-_speed_min) ) 
	{
		if( sensitive<0 )
			return _min_speed;
		else
			return _max_speed;
	}
	else
	{
		float real_sensitive = (sensitive-_speed_min)*(_max_speed-_min_speed)/(_speed_max-_speed_min)+_min_speed;
		
		return real_sensitive;


	}
}

void samsung_ptz_controller::init_ctrl(CTRL* con)
{
	memset(con->msg,0x00,sizeof(con->msg));	
	con->ptz_cont.common[0] = 0x53;
	con->ptz_cont.common[1] = 0x44;
	con->ptz_cont.common[2] = 0x56;
	con->ptz_cont.common[3] = 0x52;
	con->ptz_cont.common[4] = 0x00;
	con->ptz_cont.common[5] = 0x10;
	con->ptz_cont.common[6] = 0x0c;
	con->ptz_cont.common[7] = 0x00;
	con->ptz_cont.common[8] = 0x11;
	con->ptz_cont.common[9] = 0x61;
	con->ptz_cont.common[10] = 0x00;
	con->ptz_cont.common[11] = 0x00;
	con->ptz_cont.common[12] = 0x0e;
	con->ptz_cont.common[13] = 0x00;
	con->ptz_cont.common[14] = 0x01;
	con->ptz_cont.common[15] = 0x00;
	con->ptz_cont.end[0]= 0x00;
	con->ptz_cont.end[1]= 0x00;
	con->ptz_cont.end[2]= 0xfd;
	con->ptz_cont.end[3]= 0xff;
	con->ptz_cont.end[4]= 0x3f;

}

void samsung_ptz_controller::set_cmd_data(CTRL* con, std::string cmd, int data)
{
	if(cmd=="PTZ_CONTINUOUS_MOVE_UP")
	{
		con->ptz_cont.cmd[0] = 0x02;//ptz구분
	    con->ptz_cont.cmd[1] = 0x00;//pan방향
	    con->ptz_cont.cmd[2] = 0x08;//tilt방향
		con->ptz_cont.cmd[3] = 0x00;//pan속도
		con->ptz_cont.cmd[4] = data;//tilt속도
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFT")
	{
		con->ptz_cont.cmd[0] = 0x02;//ptz구분
	    con->ptz_cont.cmd[1] = 0x04;//pan방향
	    con->ptz_cont.cmd[2] = 0x00;//tilt방향
		con->ptz_cont.cmd[3] = data;//pan속도
		con->ptz_cont.cmd[4] = 0x00;//tilt속도

	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHT")
	{
		con->ptz_cont.cmd[0] = 0x02;//ptz구분
	    con->ptz_cont.cmd[1] = 0x02;//pan방향
	    con->ptz_cont.cmd[2] = 0x00;//tilt방향
		con->ptz_cont.cmd[3] = data;//pan속도
		con->ptz_cont.cmd[4] = 0x00;//tilt속도
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_DOWN")
	{
		con->ptz_cont.cmd[0] = 0x02;//ptz구분
	    con->ptz_cont.cmd[1] = 0x00;//pan방향
	    con->ptz_cont.cmd[2] = 0x10;//tilt방향
		con->ptz_cont.cmd[3] = 0x00;//pan속도
		con->ptz_cont.cmd[4] = data;//tilt속도
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFTUP")
	{
		con->ptz_cont.cmd[0] = 0x02;//ptz구분
	    con->ptz_cont.cmd[1] = 0x04;//pan방향
	    con->ptz_cont.cmd[2] = 0x08;//tilt방향
		con->ptz_cont.cmd[3] = data;//pan속도
		con->ptz_cont.cmd[4] = data;//tilt속도
	
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHTUP")
	{
		con->ptz_cont.cmd[0] = 0x02;//ptz구분
	    con->ptz_cont.cmd[1] = 0x02;//pan방향
	    con->ptz_cont.cmd[2] = 0x08;//tilt방향
		con->ptz_cont.cmd[3] = data;//pan속도
		con->ptz_cont.cmd[4] = data;//tilt속도
		
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_LEFTDOWN")
	{
		con->ptz_cont.cmd[0] = 0x02;//ptz구분
	    con->ptz_cont.cmd[1] = 0x04;//pan방향
	    con->ptz_cont.cmd[2] = 0x10;//tilt방향
		con->ptz_cont.cmd[3] = data;//pan속도
		con->ptz_cont.cmd[4] = data;//tilt속도
		
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_RIGHTDOWN")
	{
		con->ptz_cont.cmd[0] = 0x02;//ptz구분
	    con->ptz_cont.cmd[1] = 0x02;//pan방향
	    con->ptz_cont.cmd[2] = 0x10;//tilt방향
		con->ptz_cont.cmd[3] = data;//pan속도
		con->ptz_cont.cmd[4] = data;//tilt속도
		
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_ZOOMIN")
	{
		con->ptz_cont.cmd[0] = 0x33;//ptz구분
	    con->ptz_cont.cmd[1] = 0x00;
	    con->ptz_cont.cmd[2] = data;//zoom속도;
		con->ptz_cont.cmd[3] = 0x00;
		con->ptz_cont.cmd[4] = 0x00;
	}
	else if(cmd=="PTZ_CONTINUOUS_MOVE_ZO0MOUT")
	{
		con->ptz_cont.cmd[0] = 0x34;//ptz구분 focusfar35 focusnear36
	    con->ptz_cont.cmd[1] = 0x00;
	    con->ptz_cont.cmd[2] = data;//zoom속도
		con->ptz_cont.cmd[3] = 0x00;
		con->ptz_cont.cmd[4] = 0x00;
	}
	else if(cmd=="STOP") 
	{
		con->ptz_cont.cmd[0] = 0x01;
	    con->ptz_cont.cmd[1] = 0x00;
	    con->ptz_cont.cmd[2] = 0x00;
		con->ptz_cont.cmd[3] = 0x00;
		con->ptz_cont.cmd[4] = 0x00;
	}
	else if(cmd=="GOTO_PRESET") 
	{
		con->ptz_cont.cmd[0] = 0x0f;
	    con->ptz_cont.cmd[1] = data;
	    con->ptz_cont.cmd[2] = 0x00;
		con->ptz_cont.cmd[3] = 0x00;
		con->ptz_cont.cmd[4] = 0x00;
	}
	else if(cmd=="ADD_PRESET") 
	{
		con->ptz_cont.cmd[0] = 0x3d;
	    con->ptz_cont.cmd[1] = data;
	    con->ptz_cont.cmd[2] = 0x00;
		con->ptz_cont.cmd[3] = 0x00;
		con->ptz_cont.cmd[4] = 0x00;
	}
	else if(cmd=="REMOVE_PRESET") 
	{
		con->ptz_cont.cmd[0] = 0x65;
	    con->ptz_cont.cmd[1] = data;
	    con->ptz_cont.cmd[2] = 0x00;
		con->ptz_cont.cmd[3] = 0x00;
		con->ptz_cont.cmd[4] = 0x00;
	}
	else if(cmd=="START_TOUR") 
	{
		con->ptz_cont.cmd[0] = 0x26;
	    con->ptz_cont.cmd[1] = data;
	    con->ptz_cont.cmd[2] = 0x00;
		con->ptz_cont.cmd[3] = 0x00;
		con->ptz_cont.cmd[4] = 0x00;
	}
	else if(cmd=="END") 
	{
		con->ptz_cont.cmd[0] = 0x02;
	    con->ptz_cont.cmd[1] = 0x00;
	    con->ptz_cont.cmd[2] = 0x00;
		con->ptz_cont.cmd[3] = 0x00;
		con->ptz_cont.cmd[4] = 0x00;
		con->ptz_cont.end[0]= 0x00;
		con->ptz_cont.end[1]= 0x00;
		con->ptz_cont.end[2]= 0x00;
		con->ptz_cont.end[3]= 0x00;
		con->ptz_cont.end[4]= 0xff;
	}
}

void samsung_ptz_controller::login( void )
{
	char message[32];
	message[0] = 0x53;
	message[1] = 0x44;
	message[2] = 0x56;
	message[3] = 0x52;
	message[4] = 0x00;
	message[5] = 0x20;
	message[6] = 0x0c;
	message[7] = 0x00;
	message[8] = 0x11;
	message[9] = 0x71;
	message[10] = 0x00;
	message[11] = 0x00;
	message[12] = 0x14;
	message[13] = 0x00;
	message[14] = 0x00;
	message[15] = 0x01;
	message[16] = _user_password[0];
	message[17] = _user_password[1];
	message[18] = _user_password[2];
	message[19] = _user_password[3];
	message[20] = _user_password[4];
	message[21] = _user_password[5];
	message[22] = _user_password[6];
	message[23] = _user_password[7];
	message[24] = _user_id[0];
	message[25] = _user_id[1];
	message[26] = _user_id[2];
	message[27] = _user_id[3];
	message[28] = _user_id[4];
	message[29] = _user_id[5];
	message[30] = _user_id[6];
	message[31] = _user_id[7];

	post_send_message( message, 32 );
}

void samsung_ptz_controller::step_phase_1( void )
{
	char message[12];
	message[0] = 0x53;
	message[1] = 0x44;
	message[2] = 0x56;
	message[3] = 0x52;
	message[4] = 0x00;
	message[5] = 0x10;
	message[6] = 0x0c;
	message[7] = 0x00;
	message[8] = 0x10;
	message[9] = 0x73; 
	message[10] = 0x00;
	message[11] = 0x02;
	post_send_message( message, 12 );
}

void  samsung_ptz_controller::step_phase_2( void )
{
	char message[26];
	message[0] = 0x53;
	message[1] = 0x44;
	message[2] = 0x56;
	message[3] = 0x52;
	message[4] = 0x00;
	message[5] = 0x10;
	message[6] = 0x0c;
	message[7] = 0x00;
	message[8] = 0x11;
	message[9] = 0x5d;
	message[10] = 0x00;
	message[11] = 0x02;
	message[12] = 0x0e;
	message[13] = 0x00;
	message[14] = 0x00;
	message[15] = 0x00;
	message[16] = 0x02;
	message[17] = 0x00;
	message[18] = 0x00;
	message[19] = 0x00;
	message[20] = 0x00;
	message[21] = 0x00;
	message[22] = 0x00;
	message[23] = 0x00;
	message[24] = 0x00;
	message[25] = 0xff;
	post_send_message( message, 26 );
}

void samsung_ptz_controller::heart_beat( void )
{
	char message[16];
	message[0] = 0x53;
	message[1] = 0x44;
	message[2] = 0x56;
	message[3] = 0x52;
	message[4] = 0x00;
	message[5] = 0x10;
	message[6] = 0x0c;
	message[7] = 0x00;
	message[8] = 0x19;
	message[9] = 0x7d; 
	message[10] = 0x00;
	message[11] = 0x02;
	message[12] = 0x04;
	message[13] = 0x00; 
	message[14] = 0x00;
	message[15] = 0x00;
	post_send_message( message, 16, 1000 ); //1초후 전송
}

void samsung_ptz_controller::on_recv_message( char *msg, int size )
{
	/*
	CHAR message[500] = {0};
	_snprintf( message, 500, "%.2x", msg ); 
	OutputDebugStringA( message );
	*/
	if( _step==0 ) //53:44:56:52:00:11:0c:00:19:72:00:02:04:00:00:02
	{
		post_recv_message( 314 );
		step_phase_1();
		_step++;
	}
	else if( _step==1 || _step==2 )
	{
		if( _step==1 )
		{
			post_recv_message( 26 );
			step_phase_2();
		}
		_step++;
	}
	else
	{
		post_recv_message( 16 );
		heart_beat();
		_step++;
	}
}

base_ptz_controller* create( void )
{
	return new samsung_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	samsung_ptz_controller *ini_controller = dynamic_cast<samsung_ptz_controller*>( (*ptz_controller) );
	delete ini_controller;
	(*ptz_controller) = 0;
}
