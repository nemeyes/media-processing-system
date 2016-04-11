#include "platform.h"
#include <ptz_device_info.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include "sony_ptz_controller.h"
#include "http_client.h"



sony_ptz_controller::sony_ptz_controller( void )
	: _r_min(01)
	, _r_max(10)
	, _flipped(0)
{

}

sony_ptz_controller::~sony_ptz_controller( void )
{

}

char* sony_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SONY_SNC_ER580_CGI_17800][VENDOR];
}

char* sony_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SONY_SNC_ER580_CGI_17800][DEVICE];
}

char* sony_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SONY_SNC_ER580_CGI_17800][PROTOCOL];
}

char* sony_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SONY_SNC_ER580_CGI_17800][VERSION];
}

unsigned short sony_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[SONY_SNC_ER580_CGI_17800][VENDOR];
}

unsigned short sony_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[SONY_SNC_ER580_CGI_17800][DEVICE];
}

unsigned short sony_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[SONY_SNC_ER580_CGI_17800][PROTOCOL];
}

unsigned short sony_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[SONY_SNC_ER580_CGI_17800][VERSION];
}

unsigned short sony_ptz_controller::set_host_name( char *host_name )
{
	if( host_name && (strlen(host_name)>0) ) 
	{
		strcpy( _hostname, host_name );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short sony_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short sony_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short sony_ptz_controller::set_sensitive_boundary( unsigned int min, unsigned int max )
{
	_min = min;
	_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short	sony_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short  sony_ptz_controller::relative_move( PTZ_TYPE_T move, unsigned short sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(move) )
	{
		case PTZ_HOME :
		{
			http_client client( _hostname, _port_number, "/command/presetposition.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "ptz-recall" );
			client.put_variable( "HomePos", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_UP :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "08%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_LEFT :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "04%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RIGHT :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "06%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_DOWN :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "02%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_LEFTUP :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "07%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RIGHTUP :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "09%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_LEFTDOWN :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "01%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RIGHTDOWN :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "03%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_STOP :
		{
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "stop,motor");
			client.put_variable( "Move", str_sensitive );

			snprintf( str_sensitive, sizeof(str_sensitive), "stop,zoom");
			client.put_variable( "Move", str_sensitive );

			snprintf( str_sensitive, sizeof(str_sensitive), "stop,focus");
			client.put_variable( "Move", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_ZOOMIN :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "11%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_ZO0MOUT :
		{
			float real_sensitive = get_re_sensitive_value( sensitive );
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "10%.2d", (int)real_sensitive );
			client.put_variable( "Relative", str_sensitive );

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

unsigned short  sony_ptz_controller::relative_focus( PTZ_TYPE_T focus, unsigned short sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(focus) )
	{
		case PTZ_FOCUS_NEAR :
		{
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "near,8" );
			client.put_variable( "Move", str_sensitive );

			if( !client.send_request(_user_id, _user_password) ) 
			{
				client.clear_variable();
				return VMS_PTZ_FAIL;
			}
			client.clear_variable();
			value = VMS_PTZ_SUCCESS;
		}
		case PTZ_FOCUS_FAR :
		{
			http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

			char str_sensitive[100] = {0,};
			snprintf( str_sensitive, sizeof(str_sensitive), "far,8" );
			client.put_variable( "Move", str_sensitive );

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

unsigned short  sony_ptz_controller::absolute_move( PTZ_TYPE_T move, unsigned short sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	float ab_pan=0, ab_tilt=0, ab_zoom=0;
	query_position( ab_pan, ab_tilt, ab_zoom );

	unsigned short pan = get_ab_pan_quasi_sensitive_value( ab_pan );
	unsigned short tilt = get_ab_tilt_quasi_sensitive_value( ab_tilt );
	unsigned short zoom = get_ab_zoom_quasi_sensitive_value( ab_zoom );

	switch( UINT8(move) )
	{
		case PTZ_PAN :
		{
			absolute_move2( sensitive, tilt, zoom );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_TILT :
		{
			absolute_move2( pan, sensitive, zoom );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_ZOOM :
		{
			absolute_move2( pan, tilt, sensitive );
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}
	return value;
}

unsigned short	sony_ptz_controller::absolute_move2( int pan_sensitive, int tilt_sensitive, int zoom_sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	float real_pan_sensitive	= get_ab_pan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_ab_tilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_ab_zoom_sensitive_value( zoom_sensitive );

	int int_pan_sensitive = static_cast<int>( real_pan_sensitive );
	std::stringstream int2hex;
	int2hex << std::setfill('0') << std::setw(4);
	int2hex << std::hex << int_pan_sensitive;
	std::string str_pan_sensitive( int2hex.str() );

	int2hex.clear();
	int2hex.str( std::string() );

	int int_tilt_sensitive = static_cast<int>( real_tilt_sensitive );
	int2hex << std::setfill('0') << std::setw(4);
	int2hex << std::hex << int_tilt_sensitive;
	std::string str_tilt_sensitive( int2hex.str() );

	int2hex.clear();
	int2hex.str( std::string() );

	int int_zoom_sensitive = static_cast<int>( real_zoom_sensitive );
	int2hex << std::setfill('0') << std::setw(4);
	int2hex << std::hex << int_zoom_sensitive;
	std::string str_zoom_sensitive( int2hex.str() );

	http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

	char pan_tilt[100] = {0,};
	snprintf( pan_tilt, sizeof(pan_tilt), "%s,%s,%d", str_pan_sensitive.c_str(), str_tilt_sensitive.c_str(), _max_pantilt_velocity );
	client.put_variable( "AbsolutePanTilt", pan_tilt );

	char zoom[100] = {0,};
	snprintf( zoom, sizeof(zoom), "%s", str_zoom_sensitive.c_str() );
	client.put_variable( "AbsoluteZoom", zoom );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	value = VMS_PTZ_SUCCESS;

	return value;
}

unsigned short  sony_ptz_controller::absolute_focus( unsigned short sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	float real_sensitive = get_ab_focus_sensitive_value( sensitive );

	http_client client( _hostname, _port_number, "/command/ptzf.cgi" );

	char zoom[100] = {0,};
	snprintf( zoom, sizeof(zoom), "%.4x", real_sensitive );
	client.put_variable( "AbsoluteFocus", zoom );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::continuous_move( PTZ_TYPE_T move, unsigned short sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	sony_ptz_controller::get_current_absolute_position( unsigned short &pan, unsigned short &tilt, unsigned short &zoom )
{
	float pan1, tilt1, zoom1;
	query_position( pan1, tilt1, zoom1 );
	pan		= get_ab_pan_quasi_sensitive_value( pan1 );
	tilt	= get_ab_tilt_quasi_sensitive_value( tilt1 );
	zoom	= get_ab_zoom_quasi_sensitive_value( zoom1 );
	return VMS_PTZ_SUCCESS;
}

unsigned short	sony_ptz_controller::make_current_presetposition_information( void *param )
{
	std::map<std::string, std::string>	*current_presetposition_informations = static_cast<std::map<std::string, std::string>*>( param );

	char limits[2000] = {0};
	http_client client( _hostname, _port_number, "/command/inquiry.cgi" );
	client.put_variable( "inq", "presetposition" );
	if( !client.send_request(_user_id, _user_password, limits) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	std::string str_limits = limits;

	unsigned start_index = 0;
	unsigned end_index = 0;
	do
	{
		end_index = str_limits.find_first_of( '&', start_index );
		if( end_index>str_limits.length() )
			break;
		std::string key_value = str_limits.substr( start_index, end_index-start_index );
		if( key_value.length()<1 )
			continue;
		start_index = end_index+1;

		{
			unsigned start_value_index = key_value.find_first_of( "=" );
			std::string key		= key_value.substr( 0, start_value_index );

			if( key.length()<1 )
				continue;
			start_value_index	= start_value_index+1;
			std::string value	= key_value.substr( start_value_index, key_value.length()-(start_value_index) );
			if( value.length()<1 )
				continue;

			current_presetposition_informations->insert( std::make_pair(key, value) );
		}
	}
	while(true);
	return VMS_PTZ_SUCCESS;
}





std::map< std::string, std::string > sony_ptz_controller::get_preset_list_mapped( void ) {
	// step 1. get untokenizing string [current_presetposition_information]
	unsigned short value = VMS_PTZ_FAIL;
	std::map<std::string, std::string>	current_presetposition_informations;
	make_current_presetposition_information( &current_presetposition_informations );

	// step 2. tokenize with ','. make result in the form of map [mapped_list_preset]
	std::map<std::string, std::string>::iterator iter;
	iter = current_presetposition_informations.find( "PresetName" );
	std::map< std::string, std::string >	mapped_list_preset;

	if( iter!=current_presetposition_informations.end() )
	{
		std::string raw_list_preset = (*iter).second;

		int token_head_pos = 0;
		int token_tail_pos = 0;
		std::string token_key;
		std::string token_value;

		while( token_tail_pos != std::string::npos ) {
			token_tail_pos = raw_list_preset.find( ',', token_head_pos );
			token_key = raw_list_preset.substr( token_head_pos, token_tail_pos - token_head_pos );

			token_head_pos = token_tail_pos + 1;
			token_tail_pos = raw_list_preset.find( ',', token_head_pos );
			token_value = raw_list_preset.substr( token_head_pos, token_tail_pos - token_head_pos);

			mapped_list_preset.insert( std::pair< std::string, std::string >( token_key, token_value ) );
			token_head_pos = token_tail_pos + 1;
		}
	}

	return mapped_list_preset;
}

unsigned short	sony_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	(*length) = 0;

	std::map< std::string, std::string > mapped_list_preset = get_preset_list_mapped();

	// step 3. convert result in the form of char*** type [aliases]
	(*length) = mapped_list_preset.size();
	(*aliases) = static_cast<char**>( malloc( (*length)*sizeof(char*) ) );
	std::map< std::string, std::string >::iterator iter = mapped_list_preset.begin();
	int index = 0;
	while( iter != mapped_list_preset.end() ) {
		(*aliases)[index] = strdup( (*iter).second.c_str() );
		iter++;
		index++;
	}

	return VMS_PTZ_SUCCESS;
}


std::string	sony_ptz_controller::find_key_by_value( char* value ) {
	bool found = false;
	std::map< std::string, std::string > mapped_list_preset = get_preset_list_mapped();

	std::map< std::string, std::string >::iterator it;
	for( it=mapped_list_preset.begin(); it!=mapped_list_preset.end(); it++ ) {
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


unsigned short	sony_ptz_controller::add_preset( char *alias )
{
	int preset_available = 0;

	// STEP 1: FIND PRESET-NUMBER AVAILABLE
	std::map< std::string, std::string > list = get_preset_list_mapped();
	std::map< std::string, std::string >::iterator it;
	std::stringstream convert_str_int;

	if( list.size() == 256 )
		return VMS_PTZ_FAIL;

	const int MAX_PRESET = 256;
	for( int i=1; i < MAX_PRESET+1; i++ ) {

		bool available = true;
		for( it=list.begin(); it != list.end(); it++ ) {
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

	http_client client( _hostname, _port_number, "/command/presetposition.cgi" );

	std::string paired_str;
	convert_str_int.clear();
	convert_str_int << preset_available;
	convert_str_int >> paired_str;

	paired_str += ",";
	paired_str += alias;
	client.put_variable( "PresetSet", paired_str.c_str() );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS;
}

unsigned short	sony_ptz_controller::remove_preset( char *alias )
{
	std::string key = find_key_by_value( alias );

	http_client client( _hostname, _port_number, "/command/presetposition.cgi" );

	client.put_variable( "PresetClear", key.c_str() ); 

	if( !client.send_request( _user_id, _user_password ) )
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS;
}

unsigned short	sony_ptz_controller::goto_preset( char *alias )
{
	std::string key = find_key_by_value( alias );

	if( alias == NULL )
		return VMS_PTZ_FAIL;

	if( strcmp(key.c_str(), "") == 0 )
		return VMS_PTZ_FAIL;

	http_client client( _hostname, _port_number, "/command/presetposition.cgi" );

	std::string paired_str = key + ",24";
	client.put_variable( "PresetCall", paired_str.c_str() );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS;
}

unsigned short	sony_ptz_controller::query_limits( void )
{
	std::map<std::string, std::string>	current_ptz_informations;
	make_current_ptz_information( &current_ptz_informations );

	std::map<std::string, std::string>::iterator iter;
	iter = current_ptz_informations.find( "PanTiltMaxVelocity" );
	if( iter!=current_ptz_informations.end() )
	{
		std::string max_pantilt_velocity = (*iter).second;
		std::stringstream max_pantilt_velocity_stream( max_pantilt_velocity );
		max_pantilt_velocity_stream >> _max_pantilt_velocity;
	}

	iter = current_ptz_informations.find( "ZoomMaxVelocity" );
	if( iter!=current_ptz_informations.end() )
	{
		std::string max_zoom_velocity = (*iter).second;
		std::stringstream max_zoom_velocity_stream( max_zoom_velocity );
		max_zoom_velocity_stream >> _max_zoom_velocity;
	}

	iter = current_ptz_informations.find( "PanMovementRange" );
	if( iter!=current_ptz_informations.end() )
	{
		std::string	min_max_pan = (*iter).second;
		unsigned start_index = min_max_pan.find( "," );

		std::string str_min_pan = min_max_pan.substr( 0, start_index );
		std::string str_max_pan = min_max_pan.substr( start_index + 1 );

		unsigned short int hex_min_pan_unsigned = 0;
		short hex_min_pan = 0;
		short hex_max_pan = 0;

		std::stringstream str2hex;
		str2hex << str_min_pan;
		str2hex >> std::hex >> hex_min_pan_unsigned;
		hex_min_pan = (signed short int) hex_min_pan_unsigned;

		str2hex.clear();
		str2hex.str( std::string() );

		str2hex << str_max_pan;
		str2hex >> std::hex >> hex_max_pan;

		_min_pan = hex_min_pan;
		_max_pan = hex_max_pan;
	}

	iter = current_ptz_informations.find( "TiltMovementRange" );
	if( iter!=current_ptz_informations.end() )
	{
		std::string	min_max_tilt = (*iter).second;
		unsigned start_index = min_max_tilt.find( "," );

		std::string str_min_tilt = min_max_tilt.substr( 0, start_index );
		std::string str_max_tilt = min_max_tilt.substr( start_index + 1 );

		unsigned short int hex_min_tilt_unsigned = 0;
		short hex_min_tilt = 0;
		short hex_max_tilt = 0;

		std::stringstream str2hex;
		str2hex << str_min_tilt;
		str2hex >> std::hex >> hex_min_tilt_unsigned;
		hex_min_tilt = (signed short int) hex_min_tilt_unsigned;

		str2hex.clear();
		str2hex.str( std::string() );

		str2hex << str_max_tilt;
		str2hex >> std::hex >> hex_max_tilt;

		int sum_range = static_cast<int>(hex_max_tilt + hex_min_tilt);
		short half_tilt = (hex_max_tilt + hex_min_tilt) / 2;

		if( sum_range < 0 )		// means that EFLIP is OFF now.
		{
			_min_tilt = half_tilt;
			_max_tilt = hex_max_tilt;
		}
		else					// means that EFLIP is ON now.
		{
			_min_tilt = hex_min_tilt;
			_max_tilt = half_tilt;
		}
	}

	iter = current_ptz_informations.find( "ZoomMovementRange" );
	if( iter!=current_ptz_informations.end() )
	{
		std::string	min_max_zoom = (*iter).second;
		unsigned start_index = min_max_zoom.find( "," );

		std::string str_min_zoom = min_max_zoom.substr( 0, start_index );
		std::string str_max_zoom = min_max_zoom.substr( start_index + 1 );

		unsigned short hex_min_zoom = 0;
		unsigned short hex_max_zoom = 0;

		std::stringstream str2hex;
		str2hex << str_min_zoom;
		str2hex >> std::hex >> hex_min_zoom;
		
		str2hex.clear();
		str2hex.str( std::string() );

		str2hex << str_max_zoom;
		str2hex >> std::hex >> hex_max_zoom;

		_min_zoom = hex_min_zoom;
		_max_zoom = hex_max_zoom;
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short	sony_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	char position[200] = {0,};
	std::map<std::string, std::string>	current_ptz_informations;
	make_current_ptz_information( &current_ptz_informations );

	std::map<std::string, std::string>::iterator iter;
	iter = current_ptz_informations.find( "AbsolutePTZF" );
	if( iter!=current_ptz_informations.end() )
	{
		std::string current_position = (*iter).second;

		unsigned short current_pan_value = 0;
		unsigned short current_tilt_value = 0;
		unsigned short current_zoom_value = 0;

		unsigned start_index = 0;
		char current_pan[10] = {0};
		strcpy( current_pan, current_position.substr(start_index, 4).c_str() );
		start_index = start_index+4+1;

		char current_tilt[10] = {0};
		strcpy( current_tilt, current_position.substr(start_index, 4).c_str() );
		start_index = start_index+4+1;

		char current_zoom[10] = {0};
		strcpy( current_zoom, current_position.substr(start_index, 4).c_str() );

		std::stringstream str2hex;
		str2hex << current_pan;
		str2hex >> std::hex >> current_pan_value;

		str2hex.clear();
		str2hex.str( std::string() );

		str2hex << current_tilt;
		str2hex >> std::hex >> current_tilt_value;

		str2hex.clear();
		str2hex.str( std::string() );

		str2hex << current_zoom;
		str2hex >> std::hex >> current_zoom_value;

		pan		= static_cast<float>( static_cast<signed short int>(current_pan_value) );
		tilt	= static_cast<float>( static_cast<signed short int>(current_tilt_value) );
		zoom	= static_cast<float>( static_cast<signed short int>(current_zoom_value) );
	}
	return VMS_PTZ_SUCCESS;
}

/* FLIPPING */
unsigned short	sony_ptz_controller::make_current_cam_information( void *param )
{
	std::map<std::string, std::string>	*current_ptz_informations = static_cast<std::map<std::string, std::string>*>( param );

	char limits[500] = {0};
	http_client client( _hostname, _port_number, "/command/inquiry.cgi" );
	client.put_variable( "inq", "camera" );
	if( !client.send_request(_user_id, _user_password, limits) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	std::string str_limits = limits;

	unsigned start_index = 0;
	unsigned end_index = 0;
	do
	{
		end_index = str_limits.find_first_of( '&', start_index );
		if( end_index>str_limits.length() )
			break;
		std::string key_value = str_limits.substr( start_index, end_index-start_index );
		if( key_value.length()<1 )
			continue;
		start_index = end_index+1;

		{
			unsigned start_value_index = key_value.find_first_of( "=" );
			std::string key		= key_value.substr( 0, start_value_index );

			if( key.length()<1 )
				continue;
			start_value_index	= start_value_index+1;
			std::string value	= key_value.substr( start_value_index, key_value.length()-(start_value_index) );
			if( value.length()<1 )
				continue;

			current_ptz_informations->insert( std::make_pair(key, value) );
		}
	}
	while(true);
	return VMS_PTZ_SUCCESS;
}


unsigned short	sony_ptz_controller::is_flipped( void )
{
	/* FLIP OR NOT */
	char position[200] = {0,};

	std::map<std::string, std::string>	current_cam_info;
	make_current_cam_information( &current_cam_info );

	std::map<std::string, std::string>::iterator iter;
	iter = current_cam_info.find( "Eflip" );
	if( iter!=current_cam_info.end() )
	{
		std::string current_position = (*iter).second;

		if( current_position.compare( "on" ) == 0 )
			_flipped = -1;
		else if( current_position.compare( "off" ) == 0 )
			_flipped = +1;
		else
			return VMS_PTZ_FAIL;
	}
	return VMS_PTZ_SUCCESS;
}

float	sony_ptz_controller::get_re_sensitive_value( unsigned short sensitive )
{
	if( sensitive<_min ) 
		return float(_r_min);
	else if( sensitive>_max ) 
		return float(_r_max);
	else
	{
		float real_sensitive = banollim( float((sensitive*(_r_max-_r_min))/float(_max-_min)), 0 );
		return real_sensitive;
	}
}

float	sony_ptz_controller::get_ab_pan_sensitive_value( unsigned short sensitive )
{
	query_limits();
	if( sensitive<_min ) 
		return float(_min_pan);
	else if( sensitive>_max ) 
		return float(_max_pan);
	else
	{
		float real_sensitive = banollim( float((sensitive*(_max_pan-_min_pan))/float(_max-_min))+_min_pan, 1 );
		return real_sensitive;
	}
}

float	sony_ptz_controller::get_ab_tilt_sensitive_value( unsigned short sensitive, bool up )
{
	query_limits();
	if( sensitive<_min ) 
		return float(_min_tilt);
	else if( sensitive>_max ) 
		return float(_max_tilt);
	else
	{
		float real_sensitive =  banollim( float((sensitive*(_max_tilt-_min_tilt))/float(_max-_min))+_min_tilt, 1 );
		return real_sensitive;
	}
}

float	sony_ptz_controller::get_ab_zoom_sensitive_value( unsigned short sensitive )
{
	query_limits();
	if( sensitive<_min ) 
		return _min_zoom;
	else if( sensitive>_max ) 
		return _max_zoom;
	else
	{
		float real_sensitive = banollim( float((sensitive*(_max_zoom-_min_zoom))/(_max-_min))+_min_zoom, 1 );
		return real_sensitive;
	}
}

float	sony_ptz_controller::get_ab_focus_sensitive_value( unsigned short sensitive )
{
	query_limits();
	if( sensitive<_min ) 
		return _min_focus;
	else if( sensitive>_max ) 
		return _max_focus;
	else
	{
		float real_sensitive = banollim( (sensitive*(_max_focus-_min_focus))/(_max-_min)+_min_focus, 1 );
		return real_sensitive;
	}
}

unsigned short	sony_ptz_controller::get_ab_pan_quasi_sensitive_value( float real_sensitive )
{
	query_limits();
	float tmp_max_pan = 0xFFFF;
	float tmp_min_pan = 0x0000;
	if( (real_sensitive<=tmp_max_pan) || (_min_pan<=real_sensitive) )
	{
		tmp_min_pan = _min_pan;
	}
	else if( (real_sensitive<=_max_pan) || (_min_pan<=real_sensitive) )
	{
		tmp_max_pan = _max_pan;
	}



	{
		float sensitive = banollim( (((real_sensitive-_min_pan)*(_max-_min))/(_max_pan-_min_pan)), 1 );
		return unsigned short(sensitive);
	}


	if( real_sensitive<_min_pan ) 
		return _min;
	else if( real_sensitive>_max_pan ) 
		return _max;
	else
	{
		float sensitive = banollim( (((real_sensitive-_min_pan)*(_max-_min))/(_max_pan-_min_pan)), 1 );
		return unsigned short(sensitive);
	}
}

unsigned short	sony_ptz_controller::get_ab_tilt_quasi_sensitive_value( float real_sensitive )
{
	query_limits();
	if( real_sensitive<_min_tilt ) 
		return _min;
	else if( real_sensitive>_max_tilt ) 
		return _max;
	else
	{
		float sensitive = banollim( (((real_sensitive-_min_tilt)*(_max-_min))/(_max_tilt-_min_tilt)), 1 );
		return unsigned short(sensitive);
	}
}

unsigned short	sony_ptz_controller::get_ab_zoom_quasi_sensitive_value( float real_sensitive )
{
	query_limits();
	if( real_sensitive<_min_zoom ) 
		return _min;
	else if( real_sensitive>_max_zoom ) 
		return _max;
	else
	{
		float sensitive = banollim( (((real_sensitive-_min_zoom)*(_max-_min))/(_max_zoom-_min_zoom)), 1 );
		return unsigned short(sensitive);
	}
}

unsigned short	sony_ptz_controller::make_current_ptz_information( void *param )
{
	std::map<std::string, std::string>	*current_ptz_informations = static_cast<std::map<std::string, std::string>*>( param );

	char limits[500] = {0};
	http_client client( _hostname, _port_number, "/command/inquiry.cgi" );
	client.put_variable( "inq", "ptzf" );
	if( !client.send_request(_user_id, _user_password, limits) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	std::string str_limits = limits;

	unsigned start_index = 0;
	unsigned end_index = 0;
	do
	{
		end_index = str_limits.find_first_of( '&', start_index );
		if( end_index>str_limits.length() )
			break;
		std::string key_value = str_limits.substr( start_index, end_index-start_index );
		if( key_value.length()<1 )
			continue;
		start_index = end_index+1;

		{
			unsigned start_value_index = key_value.find_first_of( "=" );
			std::string key		= key_value.substr( 0, start_value_index );

			if( key.length()<1 )
				continue;
			start_value_index	= start_value_index+1;
			std::string value	= key_value.substr( start_value_index, key_value.length()-(start_value_index) );
			if( value.length()<1 )
				continue;

			current_ptz_informations->insert( std::make_pair(key, value) );
		}
	}
	while(true);
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::hexa_string_to_char_string( const char *hexa_string, unsigned char *char_string, int n )
{
    unsigned char d_ch;
    for(int i=0; i<n; i++)
    {
        hexa_to_char( hexa_string+2*i, d_ch );
        char_string[i] = d_ch;
    }
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::hexa_to_char( const char *hex, unsigned char &chr )
{
    chr = 0;
    for(int i=0; i<2; i++)
    {
        if(*(hex + i) >='0' && *(hex + i) <= '9')
            chr = (chr << 4) + (*(hex + i) - '0');
        else if(*(hex + i) >='A' && *(hex + i) <= 'F')
            chr = (chr << 4) + (*(hex + i) - 'A' + 10);
        else
            break;
    }
	return VMS_PTZ_SUCCESS;
}   

base_ptz_controller* create( void )
{
	return new sony_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	sony_ptz_controller *sony_controller = dynamic_cast<sony_ptz_controller*>( (*ptz_controller) );
	delete sony_controller;
	(*ptz_controller) = 0;
}
