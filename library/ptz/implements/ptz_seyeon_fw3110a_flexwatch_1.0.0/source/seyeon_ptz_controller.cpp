#include "platform.h"
#include <ptz_device_info.h>
#include "seyeon_ptz_controller.h"
#include "http_client.h"

seyeon_ptz_controller::seyeon_ptz_controller( void )
{

}

seyeon_ptz_controller::~seyeon_ptz_controller( void )
{

}

char* seyeon_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SEYEONTECH_FW3110A_10][VENDOR];
}

char* seyeon_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SEYEONTECH_FW3110A_10][DEVICE];
}

char* seyeon_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SEYEONTECH_FW3110A_10][PROTOCOL];
}

char* seyeon_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SEYEONTECH_FW3110A_10][VERSION];
}

unsigned short seyeon_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[SEYEONTECH_FW3110A_10][VENDOR];
}

unsigned short seyeon_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[SEYEONTECH_FW3110A_10][DEVICE];
}

unsigned short seyeon_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[SEYEONTECH_FW3110A_10][PROTOCOL];
}

unsigned short seyeon_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[SEYEONTECH_FW3110A_10][VERSION];
}

unsigned short seyeon_ptz_controller::set_host_name( char *hostname )
{
	if( hostname && (strlen(hostname)>0) ) 
	{
		strcpy( _hostname, hostname );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short seyeon_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short seyeon_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short seyeon_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short seyeon_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_number_place = number_place;
	_pan_min = min;
	_pan_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short seyeon_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_number_place = number_place;
	_tilt_min = min;
	_tilt_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short seyeon_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_number_place = number_place;
	_zoom_min = min;
	_zoom_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short seyeon_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_number_place = number_place;
	_speed_min = min;
	_speed_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short seyeon_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short seyeon_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short seyeon_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short seyeon_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short seyeon_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short seyeon_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short seyeon_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short seyeon_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short seyeon_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short seyeon_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short seyeon_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short seyeon_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::goto_home_position( float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::set_home_position( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	seyeon_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	(*length) = 0;
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	seyeon_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	seyeon_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	seyeon_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}
unsigned short seyeon_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	(*size_of_tours) = 0;
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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

unsigned short seyeon_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(move) )
	{
		case PTZ_RELATIVE_MOVE_HOME :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_RELATIVE_MOVE_UP :
		{
			value = move_up( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFT :
		{
			value = move_left( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHT :
		{
			value = move_right( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_DOWN :
		{
			value = move_down( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTUP :
		{
			value = move_leftup( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTUP :
		{
			value = move_riightup( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTDOWN :
		{
			value = move_leftdown( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTDOWN :
		{
			value = move_rightdown( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_ZOOMIN :
		{
			value = zoom_in( sensitive );
			break;
		}
		case PTZ_RELATIVE_MOVE_ZO0MOUT :
		{
			value = zoom_out( sensitive );
			break;
		}
	}
	return value;
}

unsigned short seyeon_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short seyeon_ptz_controller::stop_move( void )
{
	return move_stop();
}

unsigned short seyeon_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}


//private function
unsigned short  seyeon_ptz_controller::move_home( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = 15;
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "260" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short  seyeon_ptz_controller::move_up( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rtilt_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "263" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short  seyeon_ptz_controller::move_left( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rpan_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "259" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short  seyeon_ptz_controller::move_right( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rpan_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "261" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short  seyeon_ptz_controller::move_down( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rtilt_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "257" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}


unsigned short  seyeon_ptz_controller::move_leftup( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rpan_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "262" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short  seyeon_ptz_controller::move_riightup( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rpan_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "264" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short  seyeon_ptz_controller::move_leftdown( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rpan_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "256" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short  seyeon_ptz_controller::move_rightdown( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rpan_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "257" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short	seyeon_ptz_controller::move_stop( VOID )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short  seyeon_ptz_controller::zoom_in( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rzoom_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "267" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short  seyeon_ptz_controller::zoom_out( unsigned short sensitive )
{
	http_client client( _hostname, _port_number, "/cgi-bin/fwptzctr.cgi" );

	unsigned int real_sensitive_value = get_rzoom_sensitive_value( sensitive );
	char str_real_sensitive_value[10] = {0};
	snprintf( str_real_sensitive_value, sizeof(str_real_sensitive_value), "%d", real_sensitive_value );
	client.put_variable( "FwModId", "0" );
	client.put_variable( "PortId", "0" );
	client.put_variable( "PtzCode", "268" );
	client.put_variable( "PtzParm", str_real_sensitive_value );
	client.put_variable( "RcvData", "NO" );
	client.put_variable( "FwCgiVer", "0x0001" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}


unsigned short seyeon_ptz_controller::query_limits( void )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	_min_pan = 1.0f;
	_max_pan = 15.0f;
	_min_tilt = 1.0f;
	_max_tilt = 15.0f;
	_min_zoom = 1.0f;
	_max_zoom = 15.0f;
	_min_speed = 1.0f;
	_max_speed = 15.0f;
	return VMS_PTZ_SUCCESS;
}

float seyeon_ptz_controller::get_speed_sensitive_value( float sensitive )
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

float seyeon_ptz_controller::get_rpan_sensitive_value( float sensitive )
{
	query_limits();

	if( abs(sensitive)>abs(_pan_max-_pan_min) ) 
	{
		if( sensitive<0 )
			return float(-abs(_pan_max-_pan_min));
		else
			return float(abs(_pan_max-_pan_min));
	}
	else
	{
		if( _pan_min==_min_pan && _pan_max==_max_pan )
			return sensitive;
		else
		{
			float real_sensitive = float(sensitive*(_max_pan-_min_pan))/float(_pan_max-_pan_min);
			return real_sensitive;
		}
	}
}

float seyeon_ptz_controller::get_rtilt_sensitive_value( float sensitive )
{
	query_limits();

	if( abs(sensitive)>abs(_tilt_max-_tilt_min) ) 
	{
		if( sensitive<0 )
			return float(-abs(_tilt_max-_tilt_min));
		else
			return float(abs(_tilt_max-_tilt_min));
	}
	else
	{
		if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
			return sensitive;
		else
		{
			float real_sensitive = (sensitive*(_max_tilt-_min_tilt))/(_tilt_max-_tilt_min);
			return real_sensitive;
		}
	}
}

float seyeon_ptz_controller::get_rzoom_sensitive_value( float sensitive )
{
	query_limits();

	if( abs(sensitive)>abs(_zoom_max-_zoom_min) ) 
	{
		if( sensitive<0 )
			return float(-abs(_zoom_max-_zoom_min));
		else
			return float(abs(_zoom_max-_zoom_min));
	}
	else
	{
		if( _zoom_min==_min_zoom && _zoom_max==_max_zoom )
			return sensitive;
		else
		{
			float real_sensitive = (abs(sensitive)*(_max_zoom-_min_zoom))/(_zoom_max-_zoom_min);
			return real_sensitive;
		}
	}
}

base_ptz_controller* create( void )
{
	return new seyeon_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	seyeon_ptz_controller *seyeon_controller = dynamic_cast<seyeon_ptz_controller*>( (*ptz_controller) );
	delete seyeon_controller;
	(*ptz_controller) = 0;
}
