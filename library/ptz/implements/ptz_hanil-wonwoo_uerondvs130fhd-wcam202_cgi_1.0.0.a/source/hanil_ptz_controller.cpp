#include "platform.h"
#include <ptz_device_info.h>
#include "hanil_ptz_controller.h"
#include "http_client.h"

hanil_ptz_controller::hanil_ptz_controller( void )
	: _is_limits_queried(false)
{
}

hanil_ptz_controller::~hanil_ptz_controller( void )
{
}

char* hanil_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HANIL_WONWOO_UERONDVS130FHD_WCAM202_CGI_V_1_0_0_A][VENDOR];
}

char* hanil_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HANIL_WONWOO_UERONDVS130FHD_WCAM202_CGI_V_1_0_0_A][DEVICE];
}

char* hanil_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HANIL_WONWOO_UERONDVS130FHD_WCAM202_CGI_V_1_0_0_A][PROTOCOL];
}

char* hanil_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HANIL_WONWOO_UERONDVS130FHD_WCAM202_CGI_V_1_0_0_A][VERSION];
}

unsigned short hanil_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[HANIL_WONWOO_UERONDVS130FHD_WCAM202_CGI_V_1_0_0_A][VENDOR];
}

unsigned short hanil_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[HANIL_WONWOO_UERONDVS130FHD_WCAM202_CGI_V_1_0_0_A][DEVICE];
}

unsigned short hanil_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[HANIL_WONWOO_UERONDVS130FHD_WCAM202_CGI_V_1_0_0_A][PROTOCOL];
}

unsigned short hanil_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[HANIL_WONWOO_UERONDVS130FHD_WCAM202_CGI_V_1_0_0_A][VERSION];
}

unsigned short hanil_ptz_controller::set_host_name( char *host )
{
	if( host && (strlen(host)>0) ) 
	{
		strcpy( _host, host );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hanil_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hanil_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hanil_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hanil_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hanil_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hanil_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hanil_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::goto_home_position( float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::set_home_position( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;;
}

unsigned short hanil_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::get_preset_list2( int **aliases, int *length )
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

unsigned short hanil_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::add_preset2( int &alias )
{
	///cgi-bin/common/ptz.cgi?CameraID=1&PTZCmd=SetPreset&PTZData=2
	http_client client( _host, _port_number, "/cgi-bin/common/ptz.cgi" );
	client.put_variable( "CameraID", "1" );
	client.put_variable( "PTZCmd", "SetPreset" );
	char str_alias[5] = {0,};
	sprintf(str_alias, "%d", alias);
	client.put_variable( "PTZData", str_alias );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::remove_preset2( int alias )
{
	
	http_client client( _host, _port_number, "/cgi-bin/common/ptz.cgi" );
	client.put_variable( "CameraID", "1" );
	client.put_variable( "PTZCmd", "ClearPreset" );
	char str_alias[5] = {0,};
	sprintf(str_alias, "%d", alias);
	client.put_variable( "PTZData", str_alias );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::goto_preset2( int alias )
{
	http_client client( _host, _port_number, "/cgi-bin/common/ptz.cgi" );
	client.put_variable( "CameraID", "1" );
	client.put_variable( "PTZCmd", "GoPreset" );
	char str_alias[5] = {0,};
	sprintf(str_alias, "%d", alias);
	client.put_variable( "PTZData", str_alias );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::add_preset_tour( char *tour_name, int size )
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{	
	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;		
}

unsigned short hanil_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
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

unsigned short hanil_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	float real_spd = get_continuous_sensitive_value(speed);

	if(VMS_PTZ_FAIL==set_pan_speed(real_spd)) return VMS_PTZ_FAIL;
	if(VMS_PTZ_FAIL==set_tilt_speed(real_spd)) return VMS_PTZ_FAIL;

	std::string cmd = "";
	switch( UINT8(move) )
	{
		case PTZ_CONTINUOUS_MOVE_UP :
		{
			cmd = "TiltUp";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFT :
		{
			cmd = "PanLeft";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHT :
		{
			cmd = "PanRight";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_DOWN :
		{
			cmd = "TiltDown";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTUP :
		{
			cmd = "LeftUp";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTUP :
		{
			cmd = "RightUp";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTDOWN :
		{
			cmd = "LeftDown";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTDOWN :
		{
			cmd = "RightDown";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZOOMIN :
		{
			cmd = "ZoomIn";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZO0MOUT :
		{
			cmd = "ZoomOut";
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}

	http_client client( _host, _port_number, "/cgi-bin/common/ptz.cgi" );
	client.put_variable( "CameraID", "1" );
	client.put_variable( "PTZCmd", cmd.c_str() );
	client.put_variable( "PTZData", "1" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return value;
}

float hanil_ptz_controller::get_max_abs_value( float a, float b )
{
	if(abs(a)>abs(b)) return abs(a);
	else return abs(b);
}

unsigned short hanil_ptz_controller::set_pan_speed( float speed )
{
	char speed_str[10] = {0,};
	snprintf(speed_str, sizeof(speed_str),"%d", int(speed));
	http_client client( _host, _port_number, "/cgi-bin/common/ptz.cgi" );
	client.put_variable( "CameraID", "1" );
	client.put_variable( "PTZCmd", "SetPanSpeed" );
	client.put_variable( "PTZData", speed_str );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::set_tilt_speed( float speed )
{
	char speed_str[10] = {0,};
	snprintf(speed_str, sizeof(speed_str),"%d", int(speed));
	http_client client( _host, _port_number, "/cgi-bin/common/ptz.cgi" );
	client.put_variable( "CameraID", "1" );
	client.put_variable( "PTZCmd", "SetTiltSpeed" );
	client.put_variable( "PTZData", speed_str );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed  )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hanil_ptz_controller::stop_move( void )
{
	http_client client( _host, _port_number, "/cgi-bin/common/ptz.cgi" );
	client.put_variable( "CameraID", "1" );
	client.put_variable( "PTZCmd", "Break" );
	client.put_variable( "PTZData", "0" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}

	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	float tmp_pan, tmp_tilt, tmp_zoom;
	query_position( tmp_pan, tmp_tilt, tmp_zoom );
	pan		= get_apan_quasi_sensitive_value( tmp_pan );
	tilt	= get_atilt_quasi_sensitive_value( tmp_tilt );
	zoom	= get_azoom_quasi_sensitive_value( tmp_zoom );
	return VMS_PTZ_SUCCESS;
}

unsigned short	hanil_ptz_controller::query_limits( void )
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
	_min_cspeed			= 1;
	_max_cspeed			= 255;
	_is_limits_queried	= true;
	return VMS_PTZ_SUCCESS;
}

unsigned short hanil_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

float hanil_ptz_controller::get_continuous_sensitive_value( float sensitive )
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

float hanil_ptz_controller::get_speed_sensitive_value( float sensitive )
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

float hanil_ptz_controller::get_speed_quasi_sensitive_value( float real_sensitive )
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

float hanil_ptz_controller::get_rpan_sensitive_value( float sensitive )
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

float hanil_ptz_controller::get_rtilt_sensitive_value( float sensitive )
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

float hanil_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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

float hanil_ptz_controller::get_apan_sensitive_value( float sensitive )
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

float hanil_ptz_controller::get_atilt_sensitive_value( float sensitive )
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

float hanil_ptz_controller::get_azoom_sensitive_value( float sensitive )
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

float hanil_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
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

float hanil_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
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

float hanil_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
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

void hanil_ptz_controller::split2vector( std::string origin, std::string token, std::vector<std::string> *devided )
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

void hanil_ptz_controller::split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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

void hanil_ptz_controller::split2map2( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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
	return new hanil_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	hanil_ptz_controller *axis_controller = dynamic_cast<hanil_ptz_controller*>( (*ptz_controller) );
	delete axis_controller;
	(*ptz_controller) = 0;
}
