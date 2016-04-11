#include "platform.h"
#include <ptz_device_info.h>
#include "cnbtec_ptz_controller.h"
#include "http_client.h"

cnbtec_ptz_controller::cnbtec_ptz_controller( void )
	: _is_limits_queried(false)
{
}

cnbtec_ptz_controller::~cnbtec_ptz_controller( void )
{
}

char* cnbtec_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[CNBTEC_ISS2765NW_HTTP_V_1_18C27][VENDOR];
}

char* cnbtec_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[CNBTEC_ISS2765NW_HTTP_V_1_18C27][DEVICE];
}

char* cnbtec_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[CNBTEC_ISS2765NW_HTTP_V_1_18C27][PROTOCOL];
}

char* cnbtec_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[CNBTEC_ISS2765NW_HTTP_V_1_18C27][VERSION];
}

unsigned short cnbtec_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[CNBTEC_ISS2765NW_HTTP_V_1_18C27][VENDOR];
}

unsigned short cnbtec_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[CNBTEC_ISS2765NW_HTTP_V_1_18C27][DEVICE];
}

unsigned short cnbtec_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[CNBTEC_ISS2765NW_HTTP_V_1_18C27][PROTOCOL];
}

unsigned short cnbtec_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[CNBTEC_ISS2765NW_HTTP_V_1_18C27][VERSION];
}

unsigned short cnbtec_ptz_controller::set_host_name( char *host )
{
	if( host && (strlen(host)>0) ) 
	{
		strcpy( _host, host );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short cnbtec_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short cnbtec_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short cnbtec_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short cnbtec_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short cnbtec_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short cnbtec_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short cnbtec_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short cnbtec_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short cnbtec_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short cnbtec_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short cnbtec_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short cnbtec_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	bool need_stop = true;
	
	http_client client( _host, _port_number, "/cnb-cgi/ptzCtl" );

	switch( UINT8(osd) )
	{
		case PTZ_OSE_MENU_OPEN :
		{
			client.put_variable( "cmd", "1224" );
			need_stop = false;
			break;
		}
		case PTZ_OSE_MENU_CLOSE :
		{
			client.put_variable( "cmd", "1225" );
			need_stop = false;
			break;
		}
		case PTZ_OSE_MENU_UP :
		{
			client.put_variable( "cmd", "1227" );
			break;
		}
		case PTZ_OSE_MENU_DOWN :
		{
			client.put_variable( "cmd", "1228" );
			break;
		}
		case PTZ_OSE_MENU_LEFT :
		{
			client.put_variable( "cmd", "1229" );
			break;
		}
		case PTZ_OSE_MENU_RIGHT :
		{
			client.put_variable( "cmd", "1230" );
			break;
		}
		case PTZ_OSE_MENU_SELECT :
		{
			client.put_variable( "cmd", "1231" );
			break;
		}
		case PTZ_OSE_MENU_BACK :
		{
			client.put_variable( "cmd", "1232" );
			break;
		}
	}

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		value =  VMS_PTZ_FAIL;
	}
	else value = VMS_PTZ_SUCCESS;

	client.clear_variable();

	if(need_stop) 
	{
		client.put_variable( "cmd", "1233" );
		if( !client.send_request(_user_id, _user_password) ) 
		{
			client.clear_variable();
			value =  VMS_PTZ_FAIL;
		}
		else value = VMS_PTZ_SUCCESS;

		client.clear_variable();
	}value = stop_move();
	
	return value;
}

unsigned short cnbtec_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short cnbtec_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short cnbtec_ptz_controller::goto_home_position( float speed )
{
	http_client client( _host, _port_number, "/cnb-cgi/ptzCtl" );
	client.put_variable( "cmd", "44" );//1536

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::set_home_position( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::get_preset_list2( int **aliases, int *length )
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

unsigned short cnbtec_ptz_controller::get_preset_list_map( std::map<std::string,std::string> *preset_map )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::add_preset2( int &alias )
{
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;

	http_client client( _host, _port_number, "/cnb-cgi/ptzCtl" );
	client.put_variable( "cmd", "100" );
	client.put_variable( "id", "1" );
	char str_alias[15] = {0,};
	sprintf(str_alias,sizeof(str_alias),"%d",alias);
	client.put_variable( "pr", str_alias );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::remove_preset2( int alias )
{
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;

	http_client client( _host, _port_number, "/cnb-cgi/ptzCtl" );
	client.put_variable( "cmd", "101" );
	client.put_variable( "id", "1" );
	char str_alias[15] = {0,};
	sprintf(str_alias,sizeof(str_alias),"%d",alias);
	client.put_variable( "pr", str_alias );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::goto_preset2( int alias )
{
	if(alias>20) return VMS_PTZ_UNSUPPORTED_COMMAND;

	http_client client( _host, _port_number, "/cnb-cgi/ptzCtl" );
	client.put_variable( "cmd", "102" );
	client.put_variable( "id", "1" );
	char str_alias[15] = {0,};
	sprintf(str_alias,sizeof(str_alias),"%d",alias);
	client.put_variable( "pr", str_alias );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

std::string cnbtec_ptz_controller::get_preset_tour_id( char *tour_name)
{
	return "";
}

unsigned short cnbtec_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;;
}

unsigned short cnbtec_ptz_controller::remove_preset_tour_presets( const char *tour_no )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}


unsigned short cnbtec_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/cnb-cgi/ptzCtl" );
	client.put_variable( "id", "1" );	
	char str_tour_name[20] = {0,};
	sprintf(str_tour_name,sizeof(str_tour_name),"%d", tour_name);

	switch( UINT8(cmd) )
	{
		case PTZ_TOUR_CMD_START  :						
			client.put_variable( "cmd", "1235");	
			client.put_variable( "pr", str_tour_name);
			break;

		case PTZ_TOUR_CMD_STOP :
			client.put_variable( "cmd", "1279");	
			break;

		case PTZ_TOUR_CMD_PAUSE : 
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;

		default :
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
	}

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		value = VMS_PTZ_FAIL;
	}		
	client.clear_variable();		
	value = VMS_PTZ_SUCCESS;
	return value;
}

unsigned short cnbtec_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;		
}

std::string cnbtec_ptz_controller::get_name_by_id(std::map<std::string,std::string> *preset_map, std::string id)
{	
	std::map<std::string,std::string>::iterator it;
	for(it=preset_map->begin();it!=preset_map->end();it++)
	{	
		std::string tmp_str = it->first.substr(11);
		if(strcmp(tmp_str.c_str(), id.c_str())==0) return it->second;
	}
	return "";
	
}

std::string cnbtec_ptz_controller::get_id_by_name(std::map<std::string,std::string> *preset_map, std::string name)
{		
	std::map<std::string,std::string>::iterator it;
	for(it=preset_map->begin();it!=preset_map->end();it++)
	{	
		if(strcmp(it->second.c_str(), name.c_str())==0) 
		{
			return it->first.substr(11);
		}
	}
	return "";
	
}


unsigned short cnbtec_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
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

unsigned short cnbtec_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	unsigned short value = VMS_PTZ_SUCCESS;

	float real_pan_sensitive	= abs(get_continuous_sensitive_value( pan_sensitive ));
	float real_tilt_sensitive	= abs(get_continuous_sensitive_value( tilt_sensitive ));
	float real_zoom_sensitive	= abs(get_continuous_sensitive_value( zoom_sensitive ));
	char cmd[10]	= {0,};
	char pan[10]	= {0,};
	char tilt[10]	= {0,};
	char zoom[10]	= {0,};

	snprintf( pan, sizeof(pan), "%.0f", real_pan_sensitive ); 
	snprintf( tilt, sizeof(tilt), "%.0f", real_tilt_sensitive ); 
	snprintf( zoom, sizeof(zoom), "%.0f", real_zoom_sensitive ); 

	if(pan_sensitive==0 && tilt_sensitive==0&&zoom_sensitive==0) //정지
	{
		value = stop_move();
		return value;
	}
	else if(pan_sensitive!=0 && tilt_sensitive==0)//좌우
	{
		if(pan_sensitive<0) //left
		{
			snprintf( cmd, sizeof(cmd), "%d", 69 ); 
		}
		else //right
		{
			snprintf( cmd, sizeof(cmd), "%d", 71 ); 
		}
	}
	else if(tilt_sensitive!=0&&pan_sensitive==0)//상하
	{
		if(tilt_sensitive>0)
		{
			snprintf( cmd, sizeof(cmd), "%d", 65 ); 
		}
		else
		{
			snprintf( cmd, sizeof(cmd), "%d", 67 ); 
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//좌상
	{
		snprintf( cmd, sizeof(cmd), "%d", 73 ); 
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//우하
	{
		snprintf( cmd, sizeof(cmd), "%d", 79 ); 
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//좌하
	{
		snprintf( cmd, sizeof(cmd), "%d", 77 ); 
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//우상
	{
		snprintf( cmd, sizeof(cmd), "%d", 75 ); 
	}else if(zoom_sensitive>0)//줌인
	{
		snprintf( cmd, sizeof(cmd), "%d", 55 ); 
	}
	else if(zoom_sensitive<0)//줌아웃
	{
		snprintf( cmd, sizeof(cmd), "%d", 53 ); 
	}

	http_client client( _host, _port_number, "/cnb-cgi/ptzCtl" );
	client.put_variable( "id", "1" );
	client.put_variable( "cmd", cmd );
	client.put_variable( "ps", pan );
	client.put_variable( "ts", tilt );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		value = VMS_PTZ_FAIL;
	}
	else 
	{
		client.clear_variable();
		value = VMS_PTZ_SUCCESS;
	}

	return value;
}

unsigned short cnbtec_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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

unsigned short cnbtec_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed  )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short cnbtec_ptz_controller::stop_move( void )
{
	http_client client( _host, _port_number, "/cnb-cgi/ptzCtl");
	client.put_variable( "cmd", "255" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;;
}

unsigned short	cnbtec_ptz_controller::query_limits( void )
{
	if( _is_limits_queried ) return VMS_PTZ_SUCCESS;

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

	_is_limits_queried	= true;
	return VMS_PTZ_SUCCESS;
}

unsigned short cnbtec_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

float cnbtec_ptz_controller::get_continuous_sensitive_value( float sensitive )
{
	query_limits();

	if( _speed_min==_min_cspeed && _speed_max==_max_cspeed )
		return sensitive;

	if( abs(sensitive)>abs(_speed_max-_speed_min) ) 
	{
		if( sensitive<0 )
			return _min_cspeed;
		else
			return _max_cspeed;
	}
	else
	{
		float real_sensitive = float(sensitive*(_max_cspeed-_min_cspeed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

float cnbtec_ptz_controller::get_speed_sensitive_value( float sensitive )
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

float cnbtec_ptz_controller::get_speed_quasi_sensitive_value( float real_sensitive )
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

float cnbtec_ptz_controller::get_rpan_sensitive_value( float sensitive )
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

float cnbtec_ptz_controller::get_rtilt_sensitive_value( float sensitive )
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

float cnbtec_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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

float cnbtec_ptz_controller::get_apan_sensitive_value( float sensitive )
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

float cnbtec_ptz_controller::get_atilt_sensitive_value( float sensitive )
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

float cnbtec_ptz_controller::get_azoom_sensitive_value( float sensitive )
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

float cnbtec_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
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

float cnbtec_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
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

float cnbtec_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
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

void cnbtec_ptz_controller::split2vector( std::string origin, std::string token, std::vector<std::string> *devided )
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

void cnbtec_ptz_controller::split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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

void cnbtec_ptz_controller::split2map2( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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
	return new cnbtec_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	cnbtec_ptz_controller *cnbtec_controller = dynamic_cast<cnbtec_ptz_controller*>( (*ptz_controller) );
	delete cnbtec_controller;
	(*ptz_controller) = 0;
}
