#include "platform.h"
#include <ptz_device_info.h>
#include "axis_ptz_controller.h"
#include "http_client.h"

axis_ptz_controller::axis_ptz_controller( void )
	: _is_limits_queried(false)
{
}

axis_ptz_controller::~axis_ptz_controller( void )
{
}

char* axis_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[AXIS_Q1755_VAPIX_V3_5_02][VENDOR];
}

char* axis_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[AXIS_Q1755_VAPIX_V3_5_02][DEVICE];
}

char* axis_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[AXIS_Q1755_VAPIX_V3_5_02][PROTOCOL];
}

char* axis_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[AXIS_Q1755_VAPIX_V3_5_02][VERSION];
}

unsigned short axis_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[AXIS_Q1755_VAPIX_V3_5_02][VENDOR];
}

unsigned short axis_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[AXIS_Q1755_VAPIX_V3_5_02][DEVICE];
}

unsigned short axis_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[AXIS_Q1755_VAPIX_V3_5_02][PROTOCOL];
}

unsigned short axis_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[AXIS_Q1755_VAPIX_V3_5_02][VERSION];
}

unsigned short axis_ptz_controller::set_host_name( char *host )
{
	if( host && (strlen(host)>0) ) 
	{
		strcpy( _host, host );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short axis_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short axis_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short axis_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short axis_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short axis_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short axis_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short axis_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short axis_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short axis_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short axis_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short axis_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short axis_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short axis_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short axis_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::goto_home_position( float speed )
{
	http_client client( _host, _port_number, "/axis-cgi/com/ptz.cgi" );
	client.put_variable( "gotoserverpresetname", "Home" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::set_home_position( void )
{
	http_client client( _host, _port_number, "/axis-cgi/com/ptzconfig.cgi" );
	client.put_variable( "setserverpresetname", "Home" );
	client.put_variable( "home", "yes" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::get_preset_list( char ***aliases, int *length )
{

	std::map<std::string,std::string> preset_map;
	std::map<std::string,std::string>::iterator it;

	if(VMS_PTZ_FAIL==get_preset_list_map(&preset_map)) return VMS_PTZ_FAIL;

	(*length) = preset_map.size();
	if( (*length)>0 )
	{
		(*aliases) = static_cast<char**>( malloc(sizeof(char**)*(*length)) );
		int index = 0;
		for( it=preset_map.begin(); it!=preset_map.end(); it++, index++ )
		{
			(*aliases)[index] = strdup( it->second.c_str() );
		}
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::get_preset_list_map( std::map<std::string,std::string> *preset_map )
{
	char presets[1000];
	http_client client( _host, _port_number, "/axis-cgi/com/ptz.cgi" );
	client.put_variable( "query", "presetposall" );

	if( !client.send_request(_user_id, _user_password, presets) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	std::vector<std::string> vec;
	std::vector<std::string>::iterator it;	

	std::string str_presets = presets;
    int cut_at= str_presets.find_first_of("\n");
    str_presets = str_presets.substr( cut_at+1 );

	split2vector( str_presets, "\n", &vec );
	for( it=vec.begin(); it!=vec.end(); it++ )
	{
		std::string pair = *it;
		split2map( pair, "=", &(*preset_map));
	}
	
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::add_preset( char *alias )
{
#if defined(WIN32)
	if( (strncasecmp(alias, "Home")==0) ) 
		return VMS_PTZ_PRESET_ALIAS_NAME_IS_NOT_ALLOWED;
#else
	if( (strncasecmp(alias, "Home", strlen("Home"))==0) ) 
		return VMS_PTZ_PRESET_ALIAS_NAME_IS_NOT_ALLOWED;
#endif
	http_client client( _host, _port_number, "/axis-cgi/com/ptzconfig.cgi" );
	client.put_variable( "setserverpresetname", alias );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::remove_preset( char *alias )
{
	http_client client( _host, _port_number, "/axis-cgi/com/ptzconfig.cgi" );
	client.put_variable( "removeserverpresetname", alias );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::goto_preset( char *alias )
{
	http_client client( _host, _port_number, "/axis-cgi/com/ptz.cgi" );
	client.put_variable( "gotoserverpresetname", alias );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{	
    return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	float real_pan_sensitive	= get_continuous_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_continuous_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_continuous_sensitive_value( zoom_sensitive );

	http_client client( _host, _port_number, "/axis-cgi/com/ptz.cgi" );
	char pantilt[100]	= {0,};
	char zoom[100]	= {0,};
	snprintf( pantilt, sizeof(pantilt), "%.2f,%.2f", real_pan_sensitive, real_tilt_sensitive );
	snprintf( zoom, sizeof(zoom), "%.2f", real_zoom_sensitive );
	client.put_variable( "continuouspantiltmove", pantilt );
	client.put_variable( "continuouszoommove", zoom );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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

unsigned short axis_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	// 중간 return 으로 인해 아래 if문(zoom수행) 위로 수정
	char pan[100]	= {0,};
	char tilt[100]	= {0,};
	char zoom[100]	= {0,};
	char speed[100] = {0,};

	float real_pan_sensitive	= get_rpan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_rtilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_rzoom_sensitive_value( zoom_sensitive );
	float real_speed_sensitive	= get_speed_sensitive_value( speed_sensitive );

	http_client client( _host, _port_number, "/axis-cgi/com/ptz.cgi" );
		
	snprintf( pan, sizeof(pan), "%.2f", real_pan_sensitive );
	client.put_variable( "rpan", pan );
	snprintf( tilt, sizeof(tilt), "%.2f", real_tilt_sensitive );
	client.put_variable( "rtilt", tilt );
	snprintf( zoom, sizeof(zoom), "%.2f", real_zoom_sensitive );
	client.put_variable( "rzoom", zoom );

	if( real_speed_sensitive>0 )
	{
		snprintf( speed, sizeof(speed), "%.2f", real_speed_sensitive );
		client.put_variable( "speed", speed );
	}
	else
	{
		snprintf( speed, sizeof(speed), "%.2f", _max_speed );
		client.put_variable( "speed", speed );
	}

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		value = VMS_PTZ_FAIL;
	}
	else
		value = VMS_PTZ_SUCCESS;

	client.clear_variable();
	return value;
}

unsigned short axis_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed  )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(move) )
	{
		case PTZ_RELATIVE_MOVE_HOME :
		{
			//value = goto_home_position( speed );
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_RELATIVE_MOVE_UP :
		{
			value = relative_move( 0, abs(sensitive), 0, speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFT :
		{
			value = relative_move( -abs(sensitive), 0, 0, speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHT :
		{
			value = relative_move( abs(sensitive), 0, 0, speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_DOWN :
		{
			value = relative_move( 0, -abs(sensitive), 0, speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTUP :
		{
			value = relative_move( -abs(sensitive), abs(sensitive), 0, speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTUP :
		{
			value = relative_move( abs(sensitive), abs(sensitive), 0, speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTDOWN :
		{
			value = relative_move( -abs(sensitive), -abs(sensitive), 0, speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTDOWN :
		{
			value = relative_move( abs(sensitive), -abs(sensitive), 0, speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_ZOOMIN :
		{
			value = relative_move( 0, 0, abs(sensitive), speed );
			break;
		}
		case PTZ_RELATIVE_MOVE_ZO0MOUT :
		{
			value = relative_move( 0, 0, -abs(sensitive), speed );
			break;
		}
	}
	return value;
}

unsigned short axis_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short axis_ptz_controller::stop_move( void )
{
	http_client client( _host, _port_number, "/axis-cgi/com/ptz.cgi" );
	client.put_variable( "move", "stop" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	float tmp_pan, tmp_tilt, tmp_zoom;
	query_position( tmp_pan, tmp_tilt, tmp_zoom );
	pan		= 0;
	tilt	= 0;
	zoom	= get_azoom_quasi_sensitive_value( tmp_zoom );
	return VMS_PTZ_SUCCESS;
}

unsigned short	axis_ptz_controller::query_limits( void )
{
	if( _is_limits_queried )
		return VMS_PTZ_SUCCESS;

	char limits[500];
	http_client client( _host, _port_number, "/axis-cgi/param.cgi" );
	client.put_variable( "action", "list" );
	client.put_variable( "group", "PTZ.Limit.L1" );
	if( !client.send_request(_user_id, _user_password, limits) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	std::vector<std::string> vec_limits;
	std::vector<std::string>::iterator vec_limits_iter;
	std::map<std::string,std::string> map_limits;
	std::map<std::string,std::string>::iterator map_limits_iter;
	split2vector( limits, "\n", &vec_limits );
	for( vec_limits_iter=vec_limits.begin(); vec_limits_iter!=vec_limits.end(); vec_limits_iter++ )
	{
		std::string pair = *vec_limits_iter;
		split2map( pair, "=", &map_limits );
	}
	//map_limits_iter = map_limits.find( "root.PTZ.Limit.L1.MinPan" );
	//if( map_limits_iter!=map_limits.end() )
	//	sscanf( map_limits_iter->second.c_str(), "%f", &_min_pan );
	//map_limits_iter = map_limits.find( "root.PTZ.Limit.L1.MaxPan" );
	//if( map_limits_iter!=map_limits.end() )
	//	sscanf( map_limits_iter->second.c_str(), "%f", &_max_pan );
	//map_limits_iter = map_limits.find( "root.PTZ.Limit.L1.MinTilt" );
	//if( map_limits_iter!=map_limits.end() )
	//	sscanf( map_limits_iter->second.c_str(), "%f", &_min_tilt );
	//map_limits_iter = map_limits.find( "root.PTZ.Limit.L1.MaxTilt" );
	//if( map_limits_iter!=map_limits.end() )
	//	sscanf( map_limits_iter->second.c_str(), "%f", &_max_tilt );
	map_limits_iter = map_limits.find( "root.PTZ.Limit.L1.MinZoom" );
	if( map_limits_iter!=map_limits.end() )
		sscanf( map_limits_iter->second.c_str(), "%f", &_min_zoom );
	map_limits_iter = map_limits.find( "root.PTZ.Limit.L1.MaxZoom" );
	if( map_limits_iter!=map_limits.end() )
		sscanf( map_limits_iter->second.c_str(), "%f", &_max_zoom );

	_min_pan			= -19.9;
	_max_pan			= 19.9;
	_min_tilt			= -6.5;
	_max_tilt			= 6.5;//13;
	_min_speed			= 0;
	_max_speed			= 100;
	_min_cspeed			= -100;
	_max_cspeed			= 100;
	_is_limits_queried	= true;
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	char position[200] = {0,};
	http_client client( _host, _port_number, "/axis-cgi/com/ptz.cgi" );
	client.put_variable( "query", "position" );
	if( !client.send_request(_user_id, _user_password, position) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	std::vector<std::string> vec_limits;
	std::vector<std::string>::iterator vec_limits_iter;
	std::map<std::string,std::string> map_limits;
	std::map<std::string,std::string>::iterator map_limits_iter;
	split2vector( position, "\n", &vec_limits );
	for( vec_limits_iter=vec_limits.begin(); vec_limits_iter!=vec_limits.end(); vec_limits_iter++ )
	{
		std::string pair = *vec_limits_iter;
		split2map( pair, "=", &map_limits );
	}
	map_limits_iter = map_limits.find( "pan" );
	if( map_limits_iter!=map_limits.end() )
		sscanf( map_limits_iter->second.c_str(), "%f", &pan );
	map_limits_iter = map_limits.find( "tilt" );
	if( map_limits_iter!=map_limits.end() )
		sscanf( map_limits_iter->second.c_str(), "%f", &tilt );
	map_limits_iter = map_limits.find( "zoom" );
	if( map_limits_iter!=map_limits.end() )
		sscanf( map_limits_iter->second.c_str(), "%f", &zoom );

	//sscanf( position, "pan=%f\ntilt=%f\nzoom=%f\n", &pan, &tilt, &zoom );
	return VMS_PTZ_SUCCESS;
}

float axis_ptz_controller::get_continuous_sensitive_value( float sensitive )
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

float axis_ptz_controller::get_speed_sensitive_value( float sensitive )
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

float axis_ptz_controller::get_rpan_sensitive_value( float sensitive )
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

float axis_ptz_controller::get_rtilt_sensitive_value( float sensitive )
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

float axis_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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

float axis_ptz_controller::get_apan_sensitive_value( float sensitive )
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

float axis_ptz_controller::get_atilt_sensitive_value( float sensitive )
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

float axis_ptz_controller::get_azoom_sensitive_value( float sensitive )
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

float axis_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
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

float axis_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
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

float axis_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
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

void axis_ptz_controller::split2vector( std::string origin, std::string token, std::vector<std::string> *devided )
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

void axis_ptz_controller::split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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

void axis_ptz_controller::split2map2( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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
	return new axis_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	axis_ptz_controller *axis_controller = dynamic_cast<axis_ptz_controller*>( (*ptz_controller) );
	delete axis_controller;
	(*ptz_controller) = 0;
}
