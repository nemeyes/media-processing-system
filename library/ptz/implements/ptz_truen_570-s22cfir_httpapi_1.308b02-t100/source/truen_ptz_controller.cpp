#include "platform.h"
#include <ptz_device_info.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include "truen_ptz_controller.h"
#include "http_client.h"

#define banollim(x,dig) (floor(float(x)*pow(10.0f,float(dig))+0.5f)/pow(10.0f,float(dig)))

truen_ptz_controller::truen_ptz_controller( void )
	:   _is_inverse(false) 
	,_port_number(80)
	,_is_limits_queried(false)
{
	// Inverse => Relative Tilt만 반전
}

truen_ptz_controller::~truen_ptz_controller( void )
{

}

char* truen_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[TRUEN_570_S22CFIR_HTTPAPI_1308B02T100][VENDOR];
}

char* truen_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[TRUEN_570_S22CFIR_HTTPAPI_1308B02T100][DEVICE];
}

char* truen_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[TRUEN_570_S22CFIR_HTTPAPI_1308B02T100][PROTOCOL];
}

char* truen_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[TRUEN_570_S22CFIR_HTTPAPI_1308B02T100][VERSION];
}

unsigned short truen_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[TRUEN_570_S22CFIR_HTTPAPI_1308B02T100][VENDOR];
}

unsigned short truen_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[TRUEN_570_S22CFIR_HTTPAPI_1308B02T100][DEVICE];
}

unsigned short truen_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[TRUEN_570_S22CFIR_HTTPAPI_1308B02T100][PROTOCOL];
}

unsigned short truen_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[TRUEN_570_S22CFIR_HTTPAPI_1308B02T100][VERSION];
}

unsigned short truen_ptz_controller::set_host_name( char *host_name )
{
	if( host_name && (strlen(host_name)>0) ) 
	{
		strcpy( _hostname, host_name );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short truen_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short truen_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short truen_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short	truen_ptz_controller::set_angle_inverse( bool inverse )
{
	_is_inverse = inverse;
	return VMS_PTZ_SUCCESS;
}

unsigned short truen_ptz_controller::set_profile_token( char *token )
{
	if( token && strlen(token)>0 )
	{
		memset( _profile_token, 0x00, 100 );
		strcpy( _profile_token, token );
		return VMS_PTZ_SUCCESS;
	}
	return VMS_PTZ_FAIL;
}

unsigned short truen_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short truen_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short truen_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_number_place = number_place;
	_zoom_min = min;
	_zoom_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short truen_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_number_place = number_place;
	_speed_min = min;
	_speed_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short truen_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short truen_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short truen_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short truen_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short truen_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short truen_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short truen_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short truen_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short truen_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short truen_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short truen_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short truen_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
	/*
	http_client client( _hostname, _port_number, "/axis-cgi/com/ptzconfig.cgi" );

	switch( int(osd) )
	{
	case PTZ_OSE_MENU_OPEN :
		client.put_variable( "osdmenu", "open" );
		break;
	case PTZ_OSE_MENU_CLOSE :
		client.put_variable( "osdmenu", "close" );
		break;
	case PTZ_OSE_MENU_UP :
		client.put_variable( "osdmenu", "up" );
		break;
	case PTZ_OSE_MENU_DOWN :
		client.put_variable( "osdmenu", "down" );
		break;
	case PTZ_OSE_MENU_LEFT :
		client.put_variable( "osdmenu", "left" );
		break;
	case PTZ_OSE_MENU_RIGHT :
		client.put_variable( "osdmenu", "right" );
		break;	
	case PTZ_OSE_MENU_SELECT :
		client.put_variable( "osdmenu", "select" );
		break;
	case PTZ_OSE_MENU_BACK :
		client.put_variable( "osdmenu", "back" );
		break;		
	};

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
	*/
}

unsigned short truen_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;

	float real_pan_sensitive	= get_re_pan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_re_tilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_re_zoom_sensitive_value( zoom_sensitive );
	unsigned int real_time;

	if( timeout>_min_timeout && timeout<=_max_timeout ) // TRUEN의 경우
	{
		real_time = timeout;	
	}
	else
		real_time = 800; // Defualt Value

	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );
	
	char str_sensitive[100] = {0,};
	snprintf( str_sensitive, sizeof(str_sensitive), "%d,%d,%d,-1", (int)real_pan_sensitive,(int)real_tilt_sensitive,(int)real_zoom_sensitive);
			
	client.put_variable( "PTZ_CHANNEL", "1" );
	client.put_variable( "PTZ_RELATIVEPOSITION", str_sensitive ); // Default Speed used
	
	char str_timeout[4] = {0,};
	snprintf( str_timeout, sizeof(str_timeout), "%d", real_time );

	client.put_variable( "PTZ_TIMEOUT", str_timeout ); // Default Speed used

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short truen_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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

// Position Relative
unsigned short  truen_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	float real_pan_sensitive	= get_re_pan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_re_tilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_re_zoom_sensitive_value( zoom_sensitive );
	float real_speed			= get_speed_sensitive_value( speed );

	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );
	
	char str_sensitive[100] = {0,};
	snprintf( str_sensitive, sizeof(str_sensitive), "%d,%d,%d,-1,%d", (int)real_pan_sensitive,(int)real_tilt_sensitive,(int)real_zoom_sensitive,(int)real_speed  );
			
	client.put_variable( "PTZ_CHANNEL", "1" );
	client.put_variable( "PTZ_RELATIVEPOSITIONWITHSPEED", str_sensitive ); // Default Speed used

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

// Relative Move Ver1.0 Using Direction 
unsigned short  truen_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	float real_speed			= get_speed_sensitive_value( speed );
	switch( UINT8(move) )
	{
		case PTZ_RELATIVE_MOVE_HOME  :
		{
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", "home" );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_UP  :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );

			char str_speed[20] = {0,};
			if( _is_inverse )
				snprintf( str_speed, sizeof(str_speed), "down,%d", (int)speed );
			else
				snprintf( str_speed, sizeof(str_speed), "up,%d", (int)speed );
			
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFT  :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );
			
			char str_speed[20] = {0,};
			snprintf( str_speed, sizeof(str_speed), "left,%d", (int)speed );
			
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHT  :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );
			
			char str_speed[20] = {0,};
			snprintf( str_speed, sizeof(str_speed), "right,%d", (int)speed );
			
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_DOWN  :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );
			
			char str_speed[20] = {0,};
			if( _is_inverse )
				snprintf( str_speed, sizeof(str_speed), "up,%d", (int)speed );
			else
				snprintf( str_speed, sizeof(str_speed), "down,%d", (int)speed );
			
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTUP  :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );
			
			char str_speed[20] = {0,};
			
			if( _is_inverse )
				snprintf( str_speed, sizeof(str_speed), "leftdown,%d,%d", (int)speed,(int)speed );
			else
				snprintf( str_speed, sizeof(str_speed), "leftup,%d,%d", (int)speed,(int)speed );
			
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTUP  :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );
			
			char str_speed[20] = {0,};
			if( _is_inverse )
				snprintf( str_speed, sizeof(str_speed), "rightdown,%d,%d", (int)speed,(int)speed );
			else
				snprintf( str_speed, sizeof(str_speed), "rightup,%d,%d", (int)speed,(int)speed );
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTDOWN  :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );
			
			char str_speed[20] = {0,};
			if( _is_inverse )
				snprintf( str_speed, sizeof(str_speed), "leftup,%d,%d", (int)speed,(int)speed );
			else
				snprintf( str_speed, sizeof(str_speed), "leftdown,%d,%d", (int)speed,(int)speed );
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTDOWN  :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );
			
			char str_speed[20] = {0,};
			if( _is_inverse )
				snprintf( str_speed, sizeof(str_speed), "rightup,%d,%d", (int)speed,(int)speed );	
			else
				snprintf( str_speed, sizeof(str_speed), "rightdown,%d,%d", (int)speed,(int)speed );
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_ZOOMIN  :
		{
			float real_sensitive = get_re_zoom_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );

			char str_speed[20] = {0,};
			snprintf( str_speed, sizeof(str_speed), "zoomin,%d", (int)speed);
			
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_ZO0MOUT  :
		{
			float real_sensitive = get_re_zoom_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

			char str_sensitive[10] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)real_sensitive );
			
			char str_speed[20] = {0,};
			snprintf( str_speed, sizeof(str_speed), "zoomout,%d", (int)speed);
			
			client.put_variable( "PTZ_CHANNEL", "1" );
			client.put_variable( "PTZ_MOVE", str_speed ); // Default Speed used
			client.put_variable( "PTZ_TIMEOUT", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}
	return value;
}

unsigned short	truen_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	// Absolute의 경우 Speed사용하면 화면이 보이지 않음
	int real_pan_sensitive	= get_ab_pan_sensitive_value( pan_sensitive );
	int real_tilt_sensitive = get_ab_tilt_sensitive_value( tilt_sensitive );
	int real_zoom_sensitive = get_ab_zoom_sensitive_value( zoom_sensitive );
	//float real_speed			= get_speed_sensitive_value( speed );

	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );
	
	char str_sensitive[100] = {0,};
	char pan[10] = {0,};
	char tilt[10] = {0,};
	char zoom[10] = {0,};
	snprintf( str_sensitive, sizeof(str_sensitive), "%d,%d,%d,-1", real_pan_sensitive, real_tilt_sensitive, real_zoom_sensitive );
	
	client.put_variable( "PTZ_ABSOLUTEPOSITION", str_sensitive );
	
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short	truen_ptz_controller::get_status( float &pan, float &tilt, float &zoom  )
{
	float pan1, tilt1, zoom1;
	query_status( pan1, tilt1, zoom1 );
	pan		= get_ab_pan_quasi_sensitive_value( pan1 );
	tilt	= get_ab_tilt_quasi_sensitive_value( tilt1 );
	zoom	= get_ab_zoom_quasi_sensitive_value( zoom1 );
	return VMS_PTZ_SUCCESS;
}

// New Version Firmware After V1.308B02_T100
void truen_ptz_controller::get_preset_list_mapped( void ) 
{
	// If Program doesn't have list
	if (_preset_list.size() < 1)
	{
		char position[4000] = {0,};
		_preset_list.clear();

		http_client client( _hostname, _port_number, "/httpapi/ReadParam?action=readpage" );
			
			memset( position, 0x00, sizeof(position));
		
			client.put_variable( "page", "preset" );
	
			if( !client.send_request(_user_id, _user_password, position) ) 
			{
				client.clear_variable();
				return ;
			}
			else
				client.clear_variable();

		char* tkn;
		char line[2] = { 10,0 };

		tkn = strtok(position,line);
		for( int i=1; i < 129; i++ ) {	

			char alias[20] = {0,}; // 별칭 20 Byte 가정
			int trash;
			
			sscanf( tkn, "PTZ_PRESET%d=%s",&trash,alias);
			
			if(strcmp(alias,"")!=0)
			{
				char str_sensitive[4] = {0,};
				snprintf( str_sensitive, sizeof(str_sensitive), "%d", i );
				_preset_list[str_sensitive] = alias;
			}
			
			tkn = strtok(NULL,line);
		
		}

	}
	return ;
}

/*
// Old Version if doesn't have preset list page
void truen_ptz_controller::get_preset_list_mapped( void ) 
{
	// If Program doesn't have list
	if (_preset_list.size() < 1)
	{
		char position[200] = {0,};
		_preset_list.clear();

		for( int i=1; i < 129; i++ ) {
			char alias[20] = {0,};
			http_client client( _hostname, _port_number, "/httpapi/ReadParam?action=readparam" );
			char str_sensitive[15] = {0,};
			memset( position, 0x00, sizeof(position));
		
			snprintf( str_sensitive, sizeof(str_sensitive), "PTZ_PRESET%03d", i );
	
			client.put_variable( str_sensitive, "0" );
	
			if( !client.send_request(_user_id, _user_password, position) ) 
			{
				client.clear_variable();
				break;
			}
			client.clear_variable();
		
			int trash;
			sscanf( position, "PTZ_PRESET%d=%s", &trash, alias );
			if(strcmp(alias,"")!=0)
			{
				char str_sensitive[4] = {0,};
				snprintf( str_sensitive, sizeof(str_sensitive), "%d", i );
				_preset_list[str_sensitive] = alias;
			}
		
		}

	}
	

}
*/

unsigned short	truen_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	get_preset_list_mapped();

	(*length) = 0;
	(*length) = _preset_list.size();
	(*aliases) = static_cast<char**>( malloc( (*length)*sizeof(char**) ) );
	std::map< std::string, std::string >::iterator iter = _preset_list.begin();
	int index = 0;
	while( iter != _preset_list.end() ) {
		//(*aliases)[index] = strdup( (*iter).second.c_str() );
		(*aliases)[index] = strdup( iter->second.c_str() );
		iter++;
		index++;
	}

	/*

	(*length) = map_limits.size();
	if( (*length)>0 )
	{
		(*aliases) = static_cast<char**>( malloc(sizeof(char**)*(*length)) );
		int index = 0;
		for( map_limits_iter=map_limits.begin(); map_limits_iter!=map_limits.end(); map_limits_iter++, index++ )
		{
			(*aliases)[index] = strdup( map_limits_iter->second.c_str() );
		}
	}
	return VMS_PTZ_SUCCESS;
	*/
	return VMS_PTZ_SUCCESS;
}

std::string	truen_ptz_controller::find_key_by_value( char* value ) {
	get_preset_list_mapped();

	bool found = false;

	std::map< std::string, std::string >::iterator it;
	for( it=_preset_list.begin(); it!=_preset_list.end(); it++ ) {
		if( strcmp( value, (*it).second.c_str() ) == 0 ) // if string is identical after comparing,
		{
			found = true;
			break;
		}
	}

	if( found ) {
		return (*it).first;
	} else {
		return "";
	}
}

unsigned short	truen_ptz_controller::add_preset( char *alias )
{
	get_preset_list_mapped();
	int preset_available = 0;

	// STEP 1: FIND PRESET-NUMBER AVAILABLE
	std::map< std::string, std::string >::iterator it;
	std::stringstream convert_str_int;

	if( _preset_list.size() == 128 )
		return VMS_PTZ_FAIL;

	const int MAX_PRESET = 128;
	for( int i=1; i < MAX_PRESET+1; i++ ) {

		bool available = true;
		for( it=_preset_list.begin(); it != _preset_list.end(); it++ ) {
			convert_str_int.clear();
			convert_str_int.str( std::string() );

			convert_str_int << (*it).first;
			int temp;
			convert_str_int >> temp;

			if (temp == i) {
				available = false;
				break;
			}
		}

		if( available ) {
			preset_available = i;
			break;
		}
	}

	// STEP 2: SEND CGI QUERY TO CAMERA
	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );
	
	std::string paired_str;
	convert_str_int.clear();
	convert_str_int << preset_available;
	convert_str_int >> paired_str;

	client.put_variable( "PTZ_PRESETSET", paired_str.c_str() );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	else
	{
		client.clear_variable();
		// Alias추가
		http_client client2( _hostname, _port_number, "/httpapi/WriteParam?action=writeparam" );
	
		char str_sensitive[15] = {0,};
		snprintf( str_sensitive, sizeof(str_sensitive), "PTZ_PRESET%03d", preset_available );

		client2.put_variable( str_sensitive, alias );

		if( !client2.send_request(_user_id, _user_password) ) 
		{
			client2.clear_variable();
			return VMS_PTZ_FAIL;
		}
		client2.clear_variable();

		// Map 구조체 수정
		_preset_list[paired_str]=alias;

	}

	return VMS_PTZ_SUCCESS;
}

unsigned short	truen_ptz_controller::remove_preset( char *alias )
{
	std::stringstream convert_str_int;
	int preset_available = 0;

	std::string key = find_key_by_value( alias );

	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

	client.put_variable( "PTZ_PRESETCLR", key.c_str() ); 

	if( !client.send_request( _user_id, _user_password ) )
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	else
	{
		client.clear_variable();

		// Camera Alias 삭제 
		http_client client2( _hostname, _port_number, "/httpapi/WriteParam?action=writeparam" );

		convert_str_int.clear();
		convert_str_int.str( std::string() );		
		convert_str_int << key;		
		convert_str_int >> preset_available;
		
		char str_sensitive[15] = {0,};
		snprintf( str_sensitive, sizeof(str_sensitive), "PTZ_PRESET%03d", preset_available );


		client2.put_variable( str_sensitive, "" ); 

		if( !client2.send_request( _user_id, _user_password ) )
		{
			client2.clear_variable();
			return VMS_PTZ_FAIL;
		}
		client2.clear_variable();
		
		// Map 삭제 
		_preset_list.erase(key);
	}
	
	return VMS_PTZ_SUCCESS;
}

unsigned short	truen_ptz_controller::goto_preset( char *alias )
{
	std::string key = find_key_by_value( alias );

	if( alias == NULL )
		return VMS_PTZ_FAIL;

	if( strcmp(key.c_str(), "") == 0 )
		return VMS_PTZ_FAIL;

	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

	client.put_variable( "PTZ_PRESETGOTO", key.c_str() );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS;
}

unsigned short truen_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short truen_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short truen_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short truen_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	truen_ptz_controller::query_limits( VOID )
{
	_min_pan = 0x0000;
	_max_pan = 0x8C9F; //360o
	_min_tilt = 0x0000;  
	_max_tilt = 0x2328; //9000
	_min_zoom = 0x0000;
	_max_zoom = 0xFFFF; //65535
	_min_speed = 0x0001; //1부터 움직임 
	_max_speed = 0x00FF; //255
	_min_timeout = 0x000A; // 10부터 
	_max_timeout = 0x1388; // 5000
	return VMS_PTZ_SUCCESS;
}

unsigned short	truen_ptz_controller::query_status( float &pan, float &tilt, float &zoom )
{
	char position[200] = {0,};
	char trash[100] = {0,};
	char *pre_char = "PTZ_GETPOSITION";
	
	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );
	
	client.put_variable( "PTZ_GETPOSITION", "0" );
	if( !client.send_request(_user_id, _user_password, position) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	
	//ptr = strstr(position,pre_char);
	sscanf( position, "PTZ_CODE_GET_POSITION=%f,%f,%f", &pan, &tilt, &zoom );
	
	return VMS_PTZ_SUCCESS;
}

unsigned short	truen_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	// Touring의 Label을 입력할수 없음
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	truen_ptz_controller::remove_preset_tour( char *tour_name )
{
	// Touring의 삭제도 존재하지 않음 
	// 일일이 변수값 삭제는 가능하나 너무 오래 걸림 (성능상 이슈존재)
	// 불필요한경우 밑에 주석만 삭제
	// return VMS_PTZ_UNSUPPORTED_COMMAND;

	// tourname이 group0~7로 들어온다고 가정함.
	if(strncmp(tour_name,"group",5)==0 && strlen(tour_name)>=6)
	{
		int group_num=-1;
		sscanf( tour_name, "group%d",&group_num );
		if(group_num>=0 && group_num <= 7) // 최대 Group 갯수
		{
			// Action = Preset 연결정보 초기화
			for (int j=0 ; j<4 ; j++) // 80번의 Request 발생 
			{
				for (int i=0 ; i<20 ; i++)
				{
					http_client client( _hostname, _port_number, "/httpapi/WriteParam?action=writeparam" );
	
					char str_sensitive[30] = {0,};

					if (j==0)
						snprintf( str_sensitive, sizeof(str_sensitive), "PTZ_GROUP%02dACTION%02d", group_num,i );
					else if (j==1)
						snprintf( str_sensitive, sizeof(str_sensitive), "PTZ_GROUP%02dDWELL%02d", group_num,i );
					else if (j==2)
						snprintf( str_sensitive, sizeof(str_sensitive), "PTZ_GROUP%02dOPTION%02d", group_num,i );
					else
						snprintf( str_sensitive, sizeof(str_sensitive), "PTZ_GROUP%02dENABLE%02d", group_num,i );

					client.put_variable( str_sensitive, "0" );

					if( !client.send_request(_user_id, _user_password) ) 
					{	
						client.clear_variable();
						return VMS_PTZ_FAIL;
					}
					client.clear_variable();
				}
			}

		}
		return VMS_PTZ_SUCCESS;
	}
	return VMS_PTZ_FAIL;
}


unsigned short  truen_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	if(strncmp(tour_name,"group",5)==0 && strlen(tour_name)>=6)
	{
		int group_num=-1;
		
		sscanf( tour_name, "group%d",&group_num);
		if(group_num>=0 && group_num < 8) // 최대 Group 갯수
		{
			http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );
			char str_groupnum[5] = {0,};

			switch( UINT8(cmd) )
			{
				case PTZ_TOUR_CMD_START  :
					
					snprintf( str_groupnum, sizeof(str_groupnum), "%d", group_num+151); // Group 151 ~ 158
					client.put_variable( "PTZ_PRESETGOTO", str_groupnum );

					if( !client.send_request(_user_id, _user_password) ) 
					{
						client.clear_variable();
						return VMS_PTZ_FAIL;
					}
					client.clear_variable();
					value = VMS_PTZ_SUCCESS;
					break;

				case PTZ_TOUR_CMD_STOP :
					value = stop_move();
					break;

				case PTZ_TOUR_CMD_PAUSE : 
					value = VMS_PTZ_UNSUPPORTED_COMMAND;
					break;

				default :
					value = VMS_PTZ_UNSUPPORTED_COMMAND;
					break;
			}
		}
	}
	return value;

}

unsigned short truen_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short truen_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short truen_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	truen_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	if(strncmp(tour->tour_name,"group",5)==0 && strlen(tour->tour_name)>=6)
	{
		int group_num=-1;
		sscanf( tour->tour_name, "group%d",&group_num);
		if(group_num>=0 && group_num < 8) // 최대 Group 갯수
		{
			for (int i=0 ; i<tour->size_of_tour_spots; i++) // Point 별로
			{
				PTZ_TOUR_SPOT_T* tour_sp = &(tour->tour_spots[i]);
				std::string key = find_key_by_value(tour_sp->preset_alias);

				if( strcmp(key.c_str(), "") == 0 )
					return VMS_PTZ_FAIL;

				for (int j=0 ; j<4; j++) // 4가지 값 설정
				{
					http_client client( _hostname, _port_number, "/httpapi/WriteParam?action=writeparam" );
	
					char str_group_msg[30] = {0,};
					if (j==0)
					{
						snprintf( str_group_msg, sizeof(str_group_msg), "PTZ_GROUP%02dACTION%02d", group_num, i );
						client.put_variable( str_group_msg, key.c_str() ); // Preset Number 설정
					}
					else if (j==1)
					{
						snprintf( str_group_msg, sizeof(str_group_msg), "PTZ_GROUP%02dDWELL%02d", group_num,i );
						char str_number[5] = {0,};
						snprintf( str_number, sizeof(str_number), "%d", (int) tour_sp->stay_time );
						client.put_variable( str_group_msg, str_number ); // 시간설정
					}
					else if (j==2)
					{
						snprintf( str_group_msg, sizeof(str_group_msg), "PTZ_GROUP%02dOPTION%02d", group_num,i );
						char str_number[5] = {0,};
						snprintf( str_number, sizeof(str_number), "%d", (int)tour_sp->speed );
						client.put_variable( str_group_msg, str_number ); // 속도설정
					}
					else
					{
						snprintf( str_group_msg, sizeof(str_group_msg), "PTZ_GROUP%02dENABLE%02d", group_num,i );
						client.put_variable( str_group_msg, "1" ); // 사용가능여부 무조건 OK
					}
					
					if( !client.send_request(_user_id, _user_password) ) 
					{	
						client.clear_variable();
						return VMS_PTZ_FAIL;
					}
					client.clear_variable();
				}
			}
		}
		return VMS_PTZ_SUCCESS;
	}
	
	return VMS_PTZ_FAIL; // 이름이 group이 아닌경우
}

unsigned short	truen_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	get_preset_list_mapped();
	if(strncmp(tour->tour_name,"group",5)==0 && strlen(tour->tour_name)>=6)
	{
		int group_num=-1;
		sscanf( tour->tour_name, "group%d",&group_num);
		if(group_num>=0 && group_num<8) // 최대 Group 갯수
		{
			char position[4000] = {0,};
			http_client client( _hostname, _port_number, "/httpapi/ReadParam?action=readpage" );
			memset( position, 0x00, sizeof(position));
		
			char str_group_msg[10];
			snprintf( str_group_msg, sizeof(str_group_msg), "group-%d", group_num);
			client.put_variable( "page", str_group_msg );
			if( !client.send_request(_user_id, _user_password, position) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			else
				client.clear_variable();


			char* tkn;
			char line[2] = { 10,0 };

			tkn = strtok(position,line);

			tour->size_of_tour_spots = 20;

			tour->tour_spots = new PTZ_TOUR_SPOT_T[tour->size_of_tour_spots];

			for (int i=0 ; i<tour->size_of_tour_spots; i++) // Point 별로
			{
				PTZ_TOUR_SPOT_T* tour_sp = &(tour->tour_spots[i]);
				int trash,trash2;
				int value; // 값이 들어가는 부분
				//char alias[20] = {0,}; // 별칭 20 Byte 가정
				std::string alias; 
				sscanf( tkn, "PTZ_GROUP%dACTION%d=%d",&trash,&trash2,&value);
			
				char str_number[5];
				snprintf( str_number, sizeof(str_number), "%d", value);
				
				alias=_preset_list[str_number];
				if(strcmp(alias.c_str(),"")==0)
				{
					if(i!=0)
					{
					//	tour->size_of_tour_spots  = i;
						break;
					}
					else
						return VMS_PTZ_FAIL;
				}
				//tour_sp->preset_alias = strdup(alias.c_str() );//new char[ alias.length() ];
				strcpy( tour_sp->preset_alias, alias.c_str() );
				//strcpy(tour_sp->preset_alias,alias.c_str());
				
				// 시간받기
				tkn = strtok(NULL,line);
				sscanf( tkn, "PTZ_GROUP%dDWELL%d=%d",&trash,&trash2,&value);
				tour_sp->stay_time = (unsigned int) value;

				// 속력받기
				tkn = strtok(NULL,line);
				sscanf( tkn, "PTZ_GROUP%dOPTION%d=%d",&trash,&trash2,&value);
				tour_sp->speed = (float) value;

				tkn = strtok(NULL,line);
				sscanf( tkn, "PTZ_GROUP%dENABLE%d=%d",&trash,&trash2,&value);
				tkn = strtok(NULL,line);

				if ( value == 0 )
				{
					//tour->size_of_tour_spots  = i;
					break;
				}
		
			}
		}
		return VMS_PTZ_SUCCESS;
	}
	return VMS_PTZ_FAIL;
}

//수정중
unsigned short	truen_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	// 총 8개의 List 전달
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	*size_of_tours = 8;
		
	*tour = new PTZ_TOUR_T [8];
	
	for (int i=0 ; i<*size_of_tours; i++)
	{
		PTZ_TOUR_T * tour1 = *tour+i;
		char str_group_msg[10];
		snprintf( str_group_msg, sizeof(str_group_msg), "group%d", i);
		//tour1->tour_name = strdup( str_group_msg ); //new char[strlen(str_group_msg) ];
		strcpy(tour1->tour_name,str_group_msg);
	}
	return VMS_PTZ_SUCCESS;
}


float truen_ptz_controller::get_speed_sensitive_value( float sensitive )
{
	query_limits();
	if( _speed_min==_min_speed && _speed_max==_max_speed )
		return sensitive;

	if( sensitive<_speed_min ) 
		return _min_speed;
	else if( sensitive>_speed_max ) 
		return _max_speed;
	else if( sensitive==_speed_min) // 속도가 min속도랑 같으면 기본속도 평균으로 이동
		return (_min_speed+_max_speed)/2 ;
	else
	{
		float real_sensitive = float(sensitive*(_max_speed-_min_speed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

float truen_ptz_controller::get_re_pan_sensitive_value( float sensitive )
{
	query_limits();
	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return sensitive;

	// + 최대 범위 , - 최대범위
	if( sensitive<_pan_min && sensitive>= 0 )
		return _min_pan;
	else if( sensitive>_pan_max && sensitive>= 0 ) 
		return _max_pan;
	else if( -sensitive<_pan_min && sensitive< 0 )
		return -_min_pan;
	else if( -sensitive>_pan_max && sensitive< 0 ) 
		return -_max_pan;

	else
	{
		float real_sensitive = float(sensitive*(_max_pan-_min_pan))/float(_pan_max-_pan_min);
		if (real_sensitive < 0)
			real_sensitive = real_sensitive - float(_min_pan);
		else
			real_sensitive = real_sensitive + float(_min_pan);
		return real_sensitive;
	}
}

float truen_ptz_controller::get_re_tilt_sensitive_value( float sensitive )
{
	query_limits();
	
	if( _is_inverse )
		sensitive = -sensitive;

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return sensitive;

	// + 최대 범위 , - 최대범위
	if( sensitive<_tilt_min && sensitive>= 0 ) 
		return _min_tilt;
	else if( sensitive>_tilt_max && sensitive>= 0 ) 
		return _max_tilt;
	else if( -sensitive<_tilt_min && sensitive < 0 ) 
		return -_min_tilt;
	else if( -sensitive>_tilt_max && sensitive < 0 ) 
		return -_max_tilt;
	else
	{
		float real_sensitive = (sensitive*(_max_tilt-_min_tilt))/(_tilt_max-_tilt_min);
		if (real_sensitive < 0)
			real_sensitive = real_sensitive - float(_min_tilt);
		else
			real_sensitive = real_sensitive + float(_min_tilt);
		return real_sensitive;
	}
}

float truen_ptz_controller::get_re_zoom_sensitive_value( float sensitive )
{
	float pan,tilt,zoom;

	query_limits();
	query_status(pan,tilt,zoom); //현 범위 확인
	if( _zoom_min==_min_zoom && _zoom_max==_max_zoom )
		return sensitive;

	float real_sensitive = (sensitive*(_max_zoom-_min_zoom))/(_zoom_max-_zoom_min);

	if (real_sensitive < 0)
		real_sensitive = real_sensitive - float(_min_zoom);
	else
		real_sensitive = real_sensitive + float(_min_zoom);

	// Truen은 Zoom 범위 넘어가는경우 처리가 없음 AP단 처리
	if (real_sensitive < 0)
	{
		if (zoom + real_sensitive < 0) // Sensitive가 더 클경우
			real_sensitive = - zoom;
	}
	else if (real_sensitive + zoom >= _max_zoom) // Max 줌보다 클경우
		real_sensitive = _max_zoom - zoom - 1; // Max 값으로 이동
	
	return real_sensitive;

}

// Timeout Sensitive 이용구현
float truen_ptz_controller::get_re_sensitive_value( float sensitive )
{
	query_limits();
	float timeout_max = 100;
	float timeout_min = 0;
	// 0 ~ 100 User 입력 Time Sensitive 가정
	if( sensitive < timeout_min ) 
		return _min_zoom;
	else if( sensitive > timeout_max ) 
		return _max_zoom;
	else
	{
		float real_sensitive = (abs(sensitive)*(_max_timeout-_min_timeout))/(timeout_max-timeout_min)+_min_timeout;
		if( sensitive<0 )
			real_sensitive = -(real_sensitive);
		else
			real_sensitive = real_sensitive;
		return real_sensitive;
	}
}

float truen_ptz_controller::get_ab_pan_sensitive_value( float sensitive )
{
	query_limits();

//	if( _is_inverse )
//		sensitive = _pantilt_max+_pantilt_min-sensitive;

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return sensitive;

	if( sensitive<_pan_min ) 
		return float(_min_pan);
	else if( sensitive>_pan_max ) 
		return float(_max_pan);
	else
	{
		float real_sensitive = float((sensitive*(_max_pan-_min_pan)))/float(_pan_max-_pan_min)+float(_min_pan);
		return real_sensitive;
	}
}

float truen_ptz_controller::get_ab_tilt_sensitive_value( float sensitive )
{
	query_limits();

	//if( _is_inverse )
	//	sensitive = _pantilt_max+_pantilt_min-sensitive;

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return sensitive;

	//TILT 값을 임의로 조정해서 뒤로 넘어가는 것을 방지함
	//if( sensitive<((_max-_min)/2) )
		//sensitive = (_max-_min)/2;

	if( sensitive<_tilt_min ) 
		return float(_min_tilt);
	else if( sensitive>_tilt_max ) 
		return float(_max_tilt);
	else
	{
		float real_sensitive =  float((sensitive*(_max_tilt-_min_tilt)))/float(_tilt_max-_tilt_min)+float(_min_tilt);
		return real_sensitive;
	}
}

float truen_ptz_controller::get_ab_zoom_sensitive_value( float sensitive )
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
		float real_sensitive = float((sensitive*(_max_zoom-_min_zoom)))/float(_zoom_max-_zoom_min)+float(_min_zoom);
		return real_sensitive;
	}
}

float truen_ptz_controller::get_ab_pan_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

	//if( _is_inverse )
	//	real_sensitive = _max_pan+_min_pan-real_sensitive;

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return real_sensitive;

	if( real_sensitive<_min_pan ) 
		return _pan_min;
	else if( real_sensitive>_max_pan ) 
		return _pan_max;
	else
	{
		float sensitive = ((real_sensitive-_min_pan)*(_pan_max-_pan_min)-_min_pan)/(_max_pan-_min_pan);
		if( sensitive>_pan_max )
			sensitive = _pan_max;
		if( sensitive<_pan_min )
			sensitive = _pan_min;
		return sensitive;
	}
}

float truen_ptz_controller::get_ab_tilt_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

//	if( _is_inverse )
		//real_sensitive = _max_tilt+_min_tilt-real_sensitive;

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return real_sensitive;

	if( real_sensitive<_min_tilt ) 
		return _tilt_min;
	else if( real_sensitive>_max_tilt ) 
		return _tilt_max;
	else
	{
		float sensitive = ((real_sensitive-_min_tilt)*(_tilt_max-_tilt_min)-_min_tilt)/(_max_tilt-_min_tilt);
		if( sensitive>_tilt_max )
			sensitive = _tilt_max;
		if( sensitive<_tilt_min )
			sensitive = _tilt_min;
		return sensitive;
	}
}

float truen_ptz_controller::get_ab_zoom_quasi_sensitive_value( float real_sensitive )
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
		float sensitive = ((real_sensitive-_min_zoom)*(_zoom_max-_zoom_min)-_min_zoom)/(_max_zoom-_min_zoom);
		if( sensitive>_zoom_max )
			sensitive = _zoom_max;
		if( sensitive<_zoom_min )
			sensitive = _zoom_min;
		return sensitive;
	}
}


unsigned short	truen_ptz_controller::goto_home_position( float speed )
{
	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

	client.put_variable( "PTZ_CHANNEL", "1" );
	client.put_variable( "PTZ_MOVE", "home" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
	
}

unsigned short	truen_ptz_controller::set_home_position( void )
{
	// PRESET 1번을 HOME으로 사용되는 Case이다.
	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );
	
	client.put_variable( "PTZ_PRESETSET", "1" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	else
	{
		client.clear_variable();
		// Alias추가
		http_client client2( _hostname, _port_number, "/httpapi/WriteParam?action=writeparam" );
	
		client2.put_variable( "PTZ_PRESET001", "HOME" );

		if( !client2.send_request(_user_id, _user_password) ) 
		{
			client2.clear_variable();
			return VMS_PTZ_FAIL;
		}
		client2.clear_variable();

		// Map 구조체 수정
		_preset_list["1"]="HOME";

	}

	return VMS_PTZ_SUCCESS;
	
}

unsigned short truen_ptz_controller::stop_move( void )
{
	http_client client( _hostname, _port_number, "/httpapi/SendPTZ?action=sendptz" );

	client.put_variable( "PTZ_CHANNEL", "1" );
	client.put_variable( "PTZ_MOVE", "stop" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
	
}

base_ptz_controller* create( void )
{
	return new truen_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	truen_ptz_controller *truen_controller = dynamic_cast<truen_ptz_controller*>( (*ptz_controller) );
	delete truen_controller;
	(*ptz_controller) = 0;
}
