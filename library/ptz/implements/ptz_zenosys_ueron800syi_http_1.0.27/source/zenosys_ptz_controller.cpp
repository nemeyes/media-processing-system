#include "platform.h"
#include <ptz_device_info.h>
#include "zenosys_ptz_controller.h"
#include "http_client.h"

zenosys_ptz_controller::zenosys_ptz_controller( void )
	: _is_limits_queried(false)
{
}

zenosys_ptz_controller::~zenosys_ptz_controller( void )
{
}

char* zenosys_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[ZENOSYS_UERON800SYI_HTTP_V_1_0_27][VENDOR];
}

char* zenosys_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[ZENOSYS_UERON800SYI_HTTP_V_1_0_27][DEVICE];
}

char* zenosys_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[ZENOSYS_UERON800SYI_HTTP_V_1_0_27][PROTOCOL];
}

char* zenosys_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[ZENOSYS_UERON800SYI_HTTP_V_1_0_27][VERSION];
}

unsigned short zenosys_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[ZENOSYS_UERON800SYI_HTTP_V_1_0_27][VENDOR];
}

unsigned short zenosys_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[ZENOSYS_UERON800SYI_HTTP_V_1_0_27][DEVICE];
}

unsigned short zenosys_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[ZENOSYS_UERON800SYI_HTTP_V_1_0_27][PROTOCOL];
}

unsigned short zenosys_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[ZENOSYS_UERON800SYI_HTTP_V_1_0_27][VERSION];
}

unsigned short zenosys_ptz_controller::set_host_name( char *host )
{
	if( host && (strlen(host)>0) ) 
	{
		strcpy( _host, host );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short zenosys_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short zenosys_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short zenosys_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short zenosys_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short zenosys_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short zenosys_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short zenosys_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short zenosys_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short zenosys_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short zenosys_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short zenosys_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short zenosys_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short zenosys_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short zenosys_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short zenosys_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short zenosys_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short zenosys_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short zenosys_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short zenosys_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	char post_data[14] = {0,};
	int size_of_post_data = 0;
	switch( UINT8(osd) )
	{
		case PTZ_OSE_MENU_OPEN :
		{
			char msg[] = {0x10,0xff,0x01,0x00,0x07,0x00,0x91,0x99};
			size_of_post_data = sizeof(msg);
			memcpy(post_data, msg, size_of_post_data);
			value = VMS_PTZ_TRUE;
			break;
		}
		case PTZ_OSE_MENU_CLOSE :
		{
			char msg[] = {0x10,0xff,0x01,0x00,0x07,0x00,0x92,0x9a};
			size_of_post_data = sizeof(msg);
			memcpy(post_data, msg, size_of_post_data);
			value = VMS_PTZ_TRUE;
			break;
		}
		case PTZ_OSE_MENU_UP :
		{
			char msg[] = {0x10,0xfa,0x01,0x9f,0x00,0x00,0x06,0x81,0x01,0x04,0x07,0x02,0xff,0x34};
			size_of_post_data = sizeof(msg);
			memcpy(post_data, msg, size_of_post_data);
			value = VMS_PTZ_TRUE;
			break;
		}
		case PTZ_OSE_MENU_DOWN :
		{
			char msg[] = {0x10,0xfa,0x01,0x9f,0x00,0x00,0x06,0x81,0x01,0x04,0x07,0x03,0xff,0x35};
			size_of_post_data = sizeof(msg);
			memcpy(post_data, msg, size_of_post_data);
			value = VMS_PTZ_TRUE;
			break;
		}
		case PTZ_OSE_MENU_LEFT :
		{
			char msg[] = {0x10,0xfa,0x01,0x9f,0x00,0x00,0x06,0x81,0x01,0x04,0x08,0x02,0xff,0x35};
			size_of_post_data = sizeof(msg);
			memcpy(post_data, msg, size_of_post_data);
			value = VMS_PTZ_TRUE;
			break;
		}
		case PTZ_OSE_MENU_RIGHT :
		{
			char msg[] = {0x10,0xfa,0x01,0x9f,0x00,0x00,0x06,0x81,0x01,0x04,0x08,0x03,0xff,0x36};
			size_of_post_data = sizeof(msg);
			memcpy(post_data, msg, size_of_post_data);
			value = VMS_PTZ_TRUE;
			break;
		}
		case PTZ_OSE_MENU_SELECT :
		{
			value==VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_OSE_MENU_BACK :
		{
			value==VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
	}
	
	if(value==VMS_PTZ_UNSUPPORTED_COMMAND) return value;

	char *response_data = 0;
	if( !client.post_request(_user_id, _user_password, post_data, size_of_post_data, &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::goto_home_position( float speed )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	char post_data[] = {0x40, 0xff, 0x01};
	char *response_data = 0;
	if( !client.post_request(_user_id, _user_password, post_data, 3, &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::set_home_position( void )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	int alias = 0;//0번을 홈으로 사용 위함
	add_preset2(alias);

	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	//31:ff:01:홈파킹여부:00:00:프리셋id:거주시간
	char post_data[] = {0x31,0xff,0x01,0x00,0x00,0x00,0x00,0x0a};//0번을 홈으로 지정함
	char *response_data = 0;
	if( !client.post_request(_user_id, _user_password, post_data, 8, &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::get_preset_list2( int **aliases, int *length )
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

unsigned short zenosys_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::add_preset2( int &alias )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	//0x32:0xff:0x01:ID(HIGH):ID(LOW):SPD(HIGH):SPD(LOW):TIME(HIGH):TIME(LOW):0x01:PRESETNAME(byte11~byte30)
	//32:ff:01:00:03:00:3f:00:05:01:6e:6f:34:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
	char post_data[30] = {0,};
	char *response_data = 0;
	post_data[0]  = 0x32;
	post_data[1]  = 0xff;
	post_data[2]  = 0x01;
	post_data[3]  = 0x00;//ID(HIGH)
	post_data[4]  = alias;//ID(LOW)
	post_data[5]  = 0x00;//SPEED(HIGH)
	post_data[6]  = 0x3f;//SPEED(LOW)
	post_data[7]  = 0x00;//TIME(HIGH)
	post_data[8]  = 0x05;//TIME(LOW)
	post_data[9]  = 0x01;
	char str_preset_name[20] = {0,};
	if(alias==0)
		strncpy(str_preset_name, "HOME", sizeof(str_preset_name));
	else
		snprintf(str_preset_name, sizeof(str_preset_name), "PRESET%d", alias);

	memcpy(&post_data[10], str_preset_name, strlen(str_preset_name));

	if( !client.post_request(_user_id, _user_password, post_data, sizeof(post_data), &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::remove_preset2( int alias )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	//0x32:0xff:0x01:ID(HIGH):ID(LOW):SPD(HIGH):SPD(LOW):TIME(HIGH):TIME(LOW):0x00:0x00(byte11~byte30)
	char post_data[30] = {0,};
	char *response_data = 0;
	post_data[0]  = 0x32;
	post_data[1]  = 0xff;
	post_data[2]  = 0x01;
	post_data[3]  = 0x00;//ID(HIGH)
	post_data[4]  = alias;//ID(LOW)
	
	if( !client.post_request(_user_id, _user_password, post_data, sizeof(post_data), &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::goto_preset2( int alias )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	//0x41:0xff:0x01:ID(HIGH):ID(LOW)
	char post_data[5] = {0,};
	char *response_data = 0;
	post_data[0]  = 0x41;
	post_data[1]  = 0xff;
	post_data[2]  = 0x01;
	post_data[3]  = 0x00;//ID(HIGH)
	post_data[4]  = alias;//ID(LOW)
	
	if( !client.post_request(_user_id, _user_password, post_data, sizeof(post_data), &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::add_preset_tour( char *tour_name, int size )
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::remove_preset_tour2( int tour_name )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	char post_data[] = {0x34,0xff,0x00,0x00,0x00};
	char *response_data = 0;

	if( !client.post_request(_user_id, _user_password, post_data, 5, &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{	
	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	char post_data[4] = {0,};
	post_data[0] = 0x43;
	post_data[1] = 0xff;
	post_data[2] = 0x01;
	
	switch( UINT8(cmd) )
	{
		case PTZ_TOUR_CMD_START  :						
			post_data[3] = 0x01;
			break;

		case PTZ_TOUR_CMD_STOP :
			post_data[3] = 0x00;
			break;

		case PTZ_TOUR_CMD_PAUSE : 
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;

		default :
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
	}
	char *response_data = 0;
	
	if( !client.post_request(_user_id, _user_password, post_data, sizeof(post_data), &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{	
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	//0x34:0xff:waittime:preset_cnt(H):preset_cnt(L):preset_id(H):preset_id(L):preset_id(H):preset_id(L):...(6byte~514byte까지) 
	char post_data[514] = {0,};
    int idx = 0;
	post_data[idx++] = 0x34;
	post_data[idx++] = 0xff;
	post_data[idx++] = 0x06;//투어재시작 대기시간(10초단위(0~30분))
	post_data[idx++] = 0x00;//preset_cnt(H)
	post_data[idx++] = tour->size_of_tour_spots;//preset_cnt(L)
	
	for(int i=0; i<tour->size_of_tour_spots;i++)
	{
		PTZ_TOUR_SPOT_T* tsp = &(tour->tour_spots[i]);	
		post_data[idx++] = 0x00;
		post_data[idx++] = atoi(tsp->preset_alias);
	}

	char *response_data = 0;

	if( !client.post_request(_user_id, _user_password, post_data, idx, &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;		
}

unsigned short zenosys_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{	
	for(int i=1;i<=1;i++)
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

unsigned short zenosys_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	unsigned short value = VMS_PTZ_SUCCESS;
	int real_pan_sensitive	= int(abs(get_continuous_sensitive_value( pan_sensitive )));
	int real_tilt_sensitive	= int(abs(get_continuous_sensitive_value( tilt_sensitive )));
	//int real_zoom_sensitive	= int(abs(get_continuous_sensitive_value( zoom_sensitive )));

	//0x10:0xff:0x01:0x00:cmd:panspeed:tiltspeed:CS 
	char post_data[8] = {0,};
	post_data[0]  = 0x10;
	post_data[1]  = 0xff;
	post_data[2]  = 0x01;
	post_data[3]  = 0x00;

	if(pan_sensitive==0 && tilt_sensitive==0&&zoom_sensitive==0) //정지
	{
		value = stop_move();
		return value;
	}
	else if(pan_sensitive!=0 && tilt_sensitive==0)//좌우
	{
		if(pan_sensitive<0) //left
		{
			post_data[4]  = 0x04;
		}
		else //right
		{
			post_data[4]  = 0x02;
		}
	}
	else if(tilt_sensitive!=0&&pan_sensitive==0)//상하
	{
		if(tilt_sensitive>0)
		{
			post_data[4]  = 0x08;
		}
		else
		{
			post_data[4]  = 0x10;
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//좌상
	{
		post_data[4]  = 0x0c;
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//우하
	{
		post_data[4]  = 0x12;
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//좌하
	{
		post_data[4]  = 0x14;
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//우상
	{
		post_data[4]  = 0x0a;
	}
	
	if(pan_sensitive!=0 || tilt_sensitive!=0)
	{
		post_data[5]  = real_pan_sensitive;//pan speed	
		post_data[6]  = real_tilt_sensitive;//tilt speed
	}
	else if(zoom_sensitive>0)//줌인
	{
		value = stop_move();
		char ZOOMIN[8] = {0x50,0xff,0x01,0x00,0x20,0x00,0x00,0x21};
		memcpy(&post_data, ZOOMIN, sizeof(ZOOMIN));
	}
	else if(zoom_sensitive<0)//줌아웃
	{
		value = stop_move();
		char ZOOMOUT[8] = {0x50,0xff,0x01,0x00,0x40,0x00,0x00,0x41};
		memcpy(&post_data, ZOOMOUT, sizeof(ZOOMOUT));
	}

	post_data[7] = post_data[2]+post_data[3]+post_data[4]+post_data[5]+post_data[6];

	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	char *response_data = 0;
	if( !client.post_request(_user_id, _user_password, post_data, sizeof(post_data), &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;

}

unsigned short zenosys_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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

float zenosys_ptz_controller::get_max_abs_value( float a, float b )
{
	if(abs(a)>abs(b)) return abs(a);
	else return abs(b);
}

unsigned short zenosys_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed  )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short zenosys_ptz_controller::stop_move( void )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cgi-bin/admin/uartctrl.cgi" );
	char pantilt_post_data[8] = {0x10,0xff,0x01,0x00,0x00,0x00,0x00,0x01};
	//char zoom_post_data[8] = {0x50,0xff,0x01,0x00,0x00,0x00,0xff,0x00};
	char *response_data = 0;
	if( !client.post_request(_user_id, _user_password, pantilt_post_data, sizeof(pantilt_post_data), &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data ) free( response_data );

	return value;
}

unsigned short zenosys_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	float tmp_pan, tmp_tilt, tmp_zoom;
	query_position( tmp_pan, tmp_tilt, tmp_zoom );
	pan		= get_apan_quasi_sensitive_value( tmp_pan );
	tilt	= get_atilt_quasi_sensitive_value( tmp_tilt );
	zoom	= get_azoom_quasi_sensitive_value( tmp_zoom );
	return VMS_PTZ_SUCCESS;
}

unsigned short	zenosys_ptz_controller::query_limits( void )
{
	if( _is_limits_queried )
		return VMS_PTZ_SUCCESS;

	_min_pan = 0.0f;
	_max_pan = 0.0f;
	_min_tilt = 0.0f;
	_max_tilt = 0.0f;
	_min_zoom = 0.0f;;
	_max_zoom = 0.0f;
	_min_speed			= 0;
	_max_speed			= 0;
	_min_cspeed			= 0;
	_max_cspeed			= 63;
	_is_limits_queried	= true;
	return VMS_PTZ_SUCCESS;
}

unsigned short zenosys_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

float zenosys_ptz_controller::get_continuous_sensitive_value( float sensitive )
{
	query_limits();

	if( _speed_min==_min_cspeed && _speed_max==_max_cspeed )
		return sensitive;

	if( abs(sensitive)>abs(_speed_max-_speed_min) ) 
	{
			return _max_cspeed;
	}
	else
	{
		float real_sensitive = float(sensitive*(_max_cspeed-_min_cspeed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

float zenosys_ptz_controller::get_speed_sensitive_value( float sensitive )
{
	query_limits();
	if( _speed_min==_min_speed && _speed_max==_max_speed )
		return sensitive;

	if( sensitive<_speed_min ) 
		return _min_speed;
	else if( sensitive>_speed_max ) 
		return _max_speed;
	else
	{
		float real_sensitive = float(sensitive*(_max_speed-_min_speed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

float zenosys_ptz_controller::get_speed_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

	if( _speed_min==_min_speed && _speed_max==_max_speed )
		return real_sensitive;

	if( real_sensitive<_min_speed ) 
		return _speed_min;
	else if( real_sensitive>_max_speed ) 
		return _speed_max;
	else
	{
		float sensitive = ((real_sensitive-_min_speed)*(_speed_max-_speed_min))/(_max_speed-_min_speed)+_speed_min;
		if( sensitive>_speed_max )
			sensitive = _speed_max;
		if( sensitive<_speed_min )
			sensitive = _speed_min;
		return sensitive;
	}
}

float zenosys_ptz_controller::get_rpan_sensitive_value( float sensitive )
{	
	query_limits();
	
	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return sensitive;

	if( abs(sensitive)>abs(_pan_max-_pan_min) ) 
	{
		if( sensitive<0 )
			return _min_pan;
		else
			return _max_pan;
	}
	else
	{
		float real_sensitive = float(sensitive*(_max_pan-_min_pan))/float(_pan_max-_pan_min);		
		return real_sensitive;
	}		
}

float zenosys_ptz_controller::get_rtilt_sensitive_value( float sensitive )
{
	query_limits();

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return sensitive;
	
    if( abs(sensitive)>abs(_tilt_max-_tilt_min) ) 
	{
		if( sensitive<0 )
			return _min_tilt;
		else
			return _max_tilt;
	}
	else
	{
		float real_sensitive = (sensitive*(_max_tilt-_min_tilt))/(_tilt_max-_tilt_min);
		return real_sensitive;
	}
}

float zenosys_ptz_controller::get_rzoom_sensitive_value( float sensitive )
{
	query_limits();

	if( _zoom_min==_min_zoom && _zoom_max==_max_zoom )
		return sensitive;
	
	if( abs(sensitive)>abs(_zoom_max-_zoom_min) ) 
	{
		if( sensitive<0 )
			return _min_zoom;
		else
			return _max_zoom;
	}
	else
	{
		float real_sensitive = ((sensitive)*(_max_zoom-_min_zoom))/(_zoom_max-_zoom_min);
		return real_sensitive;
	}
}

float zenosys_ptz_controller::get_apan_sensitive_value( float sensitive )
{
	query_limits();	

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return sensitive;

	if( sensitive<_pan_min ) 
		return float(_min_pan);
	else if( sensitive>_pan_max ) 
		return float(_max_pan);
	else
	{
		float real_sensitive = (sensitive-_pan_min)*(_max_pan-_min_pan)/(_pan_max-_pan_min)+_min_pan;
		return real_sensitive;
	}
}

float zenosys_ptz_controller::get_atilt_sensitive_value( float sensitive )
{
	query_limits();

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return sensitive;

	if( sensitive<_tilt_min ) 
		return float(_min_tilt);
	else if( sensitive>_tilt_max ) 
		return float(_max_tilt);
	else
	{
		float real_sensitive = (sensitive-_tilt_min)*(_max_tilt-_min_tilt)/(_tilt_max-_tilt_min)+_min_tilt;
		return real_sensitive;
	}
}

float zenosys_ptz_controller::get_azoom_sensitive_value( float sensitive )
{
	query_limits();

	if( _zoom_min==_min_zoom && _zoom_max==_max_zoom )
		return sensitive;

	if( sensitive<_zoom_min ) 
		return _min_zoom;
	else if( sensitive>_zoom_max ) 
		return _max_zoom;
	else
	{
		float real_sensitive = (sensitive-_zoom_min)*(_max_zoom-_min_zoom)/(_zoom_max-_zoom_min)+_min_zoom;
		return real_sensitive;
	}
}

float zenosys_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
{
	query_limits();	

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return real_sensitive;

	if( real_sensitive<_min_pan ) 
		return _pan_min;
	else if( real_sensitive>_max_pan ) 
		return _pan_max;
	else
	{
		float sensitive = ((real_sensitive-_min_pan)*(_pan_max-_pan_min))/(_max_pan-_min_pan)+_pan_min;
		if( sensitive>_pan_max )
			sensitive = _pan_max;
		if( sensitive<_pan_min )
			sensitive = _pan_min;
		return sensitive;
	}
}

float zenosys_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return real_sensitive;

	if( real_sensitive<_min_tilt ) 
		return _tilt_min;
	else if( real_sensitive>_max_tilt ) 
		return _tilt_max;
	else
	{
		float sensitive = ((real_sensitive-_min_tilt)*(_tilt_max-_tilt_min))/(_max_tilt-_min_tilt)+_tilt_min;
		if( sensitive>_tilt_max )
			sensitive = _tilt_max;
		if( sensitive<_tilt_min )
			sensitive = _tilt_min;
		return sensitive;
	}
}

float zenosys_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

	if( _zoom_min==_min_zoom && _zoom_max==_max_zoom )
		return real_sensitive;

	if( real_sensitive<_min_zoom ) 
		return _zoom_min;
	else if( real_sensitive>_max_zoom ) 
		return _zoom_max;
	else
	{
		float sensitive = ((real_sensitive-_min_zoom)*(_zoom_max-_zoom_min))/(_max_zoom-_min_zoom)+_zoom_min;
		if( sensitive>_zoom_max )
			sensitive = _zoom_max;
		if( sensitive<_zoom_min )
			sensitive = _zoom_min;
		return sensitive;
	}
}

void zenosys_ptz_controller::split2vector( std::string origin, std::string token, std::vector<std::string> *devided )
{
    int cut_at;  // 자르는 위치
    int index = 0;  // 문자열 인덱스
 
    while( (cut_at=origin.find_first_of(token))!=origin.npos )
    {
        if( cut_at>0 )
			devided->push_back( origin.substr(0, cut_at) );
        origin = origin.substr( cut_at+1 );
    }
 
    if( origin.length()>0 )
        devided->push_back( origin.substr(0, cut_at) );
}

void zenosys_ptz_controller::split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided )
{
    int cut_at;  // 자르는 위치
    int index = 0;  // 문자열 인덱스
 
	std::string key;
	std::string value;
    while( (cut_at=origin.find_first_of(token))!=origin.npos )
    {
        if( cut_at>0 )
			key = origin.substr( 0, cut_at );
        origin = origin.substr( cut_at+1 );
    }
 
    if( origin.length()>0 )
	{
        value = origin.substr( 0, cut_at );
        value.erase( remove_if(value.begin(), value.end(), isspace), value.end() );
	}
	devided->insert( std::make_pair(key, value) );
}

void zenosys_ptz_controller::split2map2( std::string origin, std::string token, std::map<std::string,std::string> *devided )
{
    int cut_at;  // 자르는 위치
    int index = 0;  // 문자열 인덱스
 
	std::string key;
	std::string value;
    while( (cut_at=origin.find_first_of(token))!=origin.npos )
    {
        if( cut_at>0 )
			key = origin.substr( 0, cut_at );
        origin = origin.substr( cut_at+1 );
    }
 
    if( origin.length()>0 )
	{
        value = origin.substr( 0, cut_at );
	}
	devided->insert( std::make_pair(key, value) );
}

base_ptz_controller* create( void )
{
	return new zenosys_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	zenosys_ptz_controller *axis_controller = dynamic_cast<zenosys_ptz_controller*>( (*ptz_controller) );
	delete axis_controller;
	(*ptz_controller) = 0;
}
