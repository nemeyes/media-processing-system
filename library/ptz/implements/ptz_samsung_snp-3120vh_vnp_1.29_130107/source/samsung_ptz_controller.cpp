#include "platform.h"
#include <ptz_device_info.h>
#include "samsung_ptz_controller.h"
#include "http_client.h"
#include <tinyxml.h>

#define SENSITIVE_MIN	1
#define SENSITIVE_MAX	1000

samsung_ptz_controller::samsung_ptz_controller( void )
	: _is_inverse(false)
{

}

samsung_ptz_controller::~samsung_ptz_controller( void )
{

}

char* samsung_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SAMSUNG_SNP_3120VH_VNP_129_130107][VENDOR];
}

char* samsung_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SAMSUNG_SNP_3120VH_VNP_129_130107][DEVICE];
}

char* samsung_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SAMSUNG_SNP_3120VH_VNP_129_130107][PROTOCOL];
}

char* samsung_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SAMSUNG_SNP_3120VH_VNP_129_130107][VERSION];
}

unsigned short samsung_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[SAMSUNG_SNP_3120VH_VNP_129_130107][VENDOR];
}

unsigned short samsung_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[SAMSUNG_SNP_3120VH_VNP_129_130107][DEVICE];
}

unsigned short samsung_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[SAMSUNG_SNP_3120VH_VNP_129_130107][PROTOCOL];
}

unsigned short samsung_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[SAMSUNG_SNP_3120VH_VNP_129_130107][VERSION];
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

unsigned short samsung_ptz_controller::set_angle_inverse( bool inverse )
{
	_is_inverse = inverse;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_number_place = number_place;
	_pan_min = min;
	_pan_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_number_place = number_place;
	_tilt_min = min;
	_tilt_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_number_place = number_place;
	_zoom_min = min;
	_zoom_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_number_place = number_place;
	_speed_min = min;
	_speed_max = max;
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
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short samsung_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short samsung_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::goto_home_position( float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::set_home_position( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	unsigned short value = VMS_PTZ_FAIL;

	value = get_prest_list();
	if( value!=VMS_PTZ_SUCCESS )
		return value;

	(*length)	= _preset_map.size();
	(*aliases)	= static_cast<char**>( malloc(sizeof(char**)*(*length)) );

	std::map<std::string, int>::iterator iter;
	int index = 0;
	for( iter=_preset_map.begin(); iter!=_preset_map.end(); iter++, index++ )
		(*aliases)[index] = strdup( (*iter).first.c_str() );
	return value;
}

unsigned short samsung_ptz_controller::add_preset( char *alias )
{
	unsigned short value = VMS_PTZ_FAIL;

	value = get_prest_list();
	if( value!=VMS_PTZ_SUCCESS )
		return value;

	int real_index = 0;
	std::map<std::string, int>::iterator iter;
	for( iter=_preset_map.begin(); iter!=_preset_map.end(); iter++ )
	{
		if( real_index<((iter)->second) )
			real_index = ((iter)->second);
	}

	real_index++;
	real_index = real_index%255;

	http_client client( _hostname, _port_number, "/cgi-bin/stw.cgi" );
	char post_data[100] = {0};
	snprintf( post_data, sizeof(post_data), "<AddPTZPreset><Index>%d</Index><Name>%s</Name></AddPTZPreset>", real_index, alias );
	char *response_data = 0;
	if( !client.post_request(_user_id, _user_password, post_data, &response_data) ) 
		value = VMS_PTZ_FAIL;

	if( response_data )
		free( response_data );
	value = VMS_PTZ_SUCCESS;

	if( value==VMS_PTZ_SUCCESS )
		_preset_map.insert( std::make_pair(alias, real_index) );

	return value;
}

unsigned short samsung_ptz_controller::remove_preset( char *alias )
{
	unsigned short value = VMS_PTZ_FAIL;

	value = get_prest_list();
	if( value!=VMS_PTZ_SUCCESS )
		return value;

	std::map<std::string, int>::iterator iter = _preset_map.find( alias );
	if( iter!=_preset_map.end() )
	{
		http_client client( _hostname, _port_number, "/cgi-bin/stw.cgi" );
		char post_data[100] = {0};
		snprintf( post_data, sizeof(post_data), "<RemovePTZPreset><Index>%d</Index></RemovePTZPreset>", (iter->second) );
		char *response_data = 0;
		if( !client.post_request(_user_id, _user_password, post_data, &response_data) ) 
			value = VMS_PTZ_FAIL;

		if( response_data )
			free( response_data );

		value = VMS_PTZ_SUCCESS;
	}
	if( value==VMS_PTZ_SUCCESS )
		_preset_map.erase( alias );
	return value;
}

unsigned short samsung_ptz_controller::goto_preset( char *alias )
{
	unsigned short value = VMS_PTZ_FAIL;

	value = get_prest_list();
	if( value!=VMS_PTZ_SUCCESS )
		return value;

	std::map<std::string, int>::iterator iter = _preset_map.find( alias );
	if( iter!=_preset_map.end() )
	{
		http_client client( _hostname, _port_number, "/cgi-bin/ptz.cgi" );
		char data[10] = {0};
		snprintf( data, sizeof(data), "%d", (iter->second) );
		client.put_variable( "movepresetno", data );
		if( !client.send_request(_user_id, _user_password) ) 
		{
			value = VMS_PTZ_FAIL;
			client.clear_variable();
		}
		value = VMS_PTZ_SUCCESS;
	}
	return value;
}

unsigned short samsung_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}
unsigned short samsung_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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

unsigned short samsung_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short samsung_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _hostname, _port_number, "/cgi-bin/ptz.cgi" );

	switch( UINT8(move) )
	{
		case PTZ_RELATIVE_MOVE_HOME :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_RELATIVE_MOVE_UP :
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "up" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFT :
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "left" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHT :
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "right" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_DOWN :
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "down" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTUP :
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "leftup" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTUP :
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "rightup" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTDOWN :
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "leftdown" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTDOWN :
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "rightdown" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_ZOOMIN :
		{
			client.put_variable( "zoom", "in" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_ZO0MOUT :
		{
			client.put_variable( "zoom", "out" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}

	if( value==VMS_PTZ_SUCCESS )
	{
		if( move!=PTZ_RELATIVE_MOVE_ZOOMIN && move!=PTZ_RELATIVE_MOVE_ZO0MOUT )
			client.put_variable( "duration", "1" );
		if( !client.send_request(_user_id, _user_password) ) 
			value = VMS_PTZ_FAIL;

		client.clear_variable();

		float real_sensitive = get_rzoom_sensitive_value( sensitive );
		sleep_millisecond( real_sensitive );
		if( move!=PTZ_RELATIVE_MOVE_ZOOMIN && move!=PTZ_RELATIVE_MOVE_ZO0MOUT )
		{
			client.put_variable( "mode", "ptz" );
			client.put_variable( "move", "stop" );
			client.put_variable( "duration", "1" );
		}
		else
		{
			client.put_variable( "zoom", "stop" );
		}

		if( !client.send_request(_user_id, _user_password) ) 
			value = VMS_PTZ_FAIL;
		client.clear_variable();
	}
	return value;
}

unsigned short samsung_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	// 중간 return 으로 인해 아래 if문(zoom수행) 위로 수정
	float real_pan_sensitive	= get_apan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_atilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_azoom_sensitive_value( zoom_sensitive );

	http_client client( _hostname, _port_number, "/cgi-bin/ptz.cgi" );

	char pan[100] = {0,};
	char tilt[100] = {0,};
	char zoom[100] = {0,};
	snprintf( pan, sizeof(pan), "%d", real_pan_sensitive );
	snprintf( tilt, sizeof(tilt), "%d", real_tilt_sensitive );
	snprintf( zoom, sizeof(zoom), "%d", real_zoom_sensitive );
	client.put_variable( "move", "absmove" );
	client.put_variable( "pan", pan );
	client.put_variable( "tilt", tilt );
	client.put_variable( "zoom", zoom );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::stop_move( void )
{
	http_client client( _hostname, _port_number, "/cgi-bin/ptz.cgi" );
	client.put_variable( "mode", "ptz" );
	client.put_variable( "move", "stop" );
	client.put_variable( "zoom", "stop" );
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	float pan1, tilt1, zoom1;
	query_position( pan1, tilt1, zoom1 );
	pan		= get_apan_quasi_sensitive_value( pan1 );
	tilt	= get_atilt_quasi_sensitive_value( tilt1 );
	zoom	= get_azoom_quasi_sensitive_value( zoom1 );
	return VMS_PTZ_SUCCESS;
}

/*
<?xml version="1.0" encoding="utf-8" ?>
<GetAllPTZPresetResponse>
	<PTZPreset0>
		<Index>1</Index>
		<Name>test</Name>
	</PTZPreset0>
	<PTZPreset1>
		<Index>2</Index>
		<Name></Name>
	</PTZPreset1>
	<PTZPreset2>
		<Index>3</Index>
		<Name></Name>
	</PTZPreset2>
	<PTZPreset3>
		<Index>4</Index>
		<Name></Name>
	</PTZPreset3>
	<PTZPreset4>
		<Index>5</Index>
		<Name></Name>
	</PTZPreset4>
</GetAllPTZPresetResponse>
*/
//private function

unsigned short	samsung_ptz_controller::query_limits( VOID )
{
	_min_pan	= 0;
	_max_pan	= 359;
	_min_tilt	= -5;
	_max_tilt	= 185;
	_min_zoom	= 1;
	_max_zoom	= 12;
	return VMS_PTZ_SUCCESS;
}

unsigned short samsung_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	char position[200] = {0,};
	http_client client( _hostname, _port_number, "/cgi-bin/ptz.cgi" );
	client.put_variable( "query", "ptz" );
	if( !client.send_request(_user_id, _user_password, position) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	sscanf( position, "pan:%f\ntilt:%f\nzoom:%f\n", &pan, &tilt, &zoom );
	return VMS_PTZ_SUCCESS;
}

float samsung_ptz_controller::get_rpan_sensitive_value( float sensitive )
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

float samsung_ptz_controller::get_rtilt_sensitive_value( float sensitive )
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

float samsung_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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

float samsung_ptz_controller::get_apan_sensitive_value( float sensitive )
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
		float real_sensitive = sensitive*(_max_pan-_min_pan)/(_pan_max-_pan_min)+_min_pan;
		return real_sensitive;
	}
}

float samsung_ptz_controller::get_atilt_sensitive_value( float sensitive )
{
	query_limits();

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
		float real_sensitive =  sensitive*(_max_tilt-_min_tilt)/(_tilt_max-_tilt_min)+_min_tilt;
		return real_sensitive;
	}
}

float samsung_ptz_controller::get_azoom_sensitive_value( float sensitive )
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

float samsung_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
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
		float sensitive = ((real_sensitive-_min_pan)*(_pan_max-_pan_min)-_min_pan)/(_max_pan-_min_pan);
		if( sensitive>_pan_max )
			sensitive = _pan_max;
		if( sensitive<_pan_min )
			sensitive = _pan_min;
		return sensitive;
	}
}

float samsung_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
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
		float sensitive = ((real_sensitive-_min_tilt)*(_tilt_max-_tilt_min)-_min_tilt)/(_max_tilt-_min_tilt);
		if( sensitive>_tilt_max )
			sensitive = _tilt_max;
		if( sensitive<_tilt_min )
			sensitive = _tilt_min;
		return sensitive;
	}
}

float samsung_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
{
	query_limits();
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

unsigned short samsung_ptz_controller::get_prest_list( void )
{
	unsigned short value = VMS_PTZ_FAIL;
	if( _preset_map.size()<1 )
	{
		http_client client( _hostname, _port_number, "/cgi-bin/stw.cgi" );
		char *post_data = "<GetAllPTZPreset></GetAllPTZPreset>";
		char *response_data = 0;
		if( !client.post_request(_user_id, _user_password, post_data, &response_data) ) 
			value = VMS_PTZ_FAIL;

		if( response_data )
		{
			TiXmlDocument	doc;
			TiXmlElement	*elem		= 0;
			TiXmlElement	*elem_index	= 0;
			TiXmlElement	*elem_name	= 0;
			TiXmlNode		*node		= 0;

			doc.Parse( (const char*)response_data, 0 );
			node = doc.FirstChild();
			for( node; node; node=node->NextSibling() )
			{
				elem = node->FirstChildElement();
				for( ;elem; elem=elem->NextSiblingElement() )
				{
					const char *index_key	= 0;
					const char *index_text	= 0;
					const char *name_key	= 0;
					const char *name_text	= 0;
					elem_index = elem->FirstChildElement();
					if( elem_index )
					{
						index_key	= elem_index->Value();
						index_text	= elem_index->GetText();
					}
					elem_name = elem_index->NextSiblingElement();
					if( elem_name )
					{
						name_key	= elem_name->Value();
						name_text	= elem_name->GetText();
					}

					int real_index = 0;
					if( index_text && name_text )
					{
						sscanf( index_text, "%d", &real_index );
						_preset_map.insert( std::make_pair(name_text, real_index) );
					}
					else
						break;
				}
			}
			free( response_data );
		}
		value = VMS_PTZ_SUCCESS;
	}
	else
		value = VMS_PTZ_SUCCESS;

	return value;
}

base_ptz_controller* create( void )
{
	return new samsung_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	samsung_ptz_controller *samsung_controller = dynamic_cast<samsung_ptz_controller*>( (*ptz_controller) );
	delete samsung_controller;
	(*ptz_controller) = 0;
}
