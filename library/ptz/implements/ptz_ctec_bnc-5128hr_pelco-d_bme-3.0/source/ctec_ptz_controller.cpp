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
#include "ctec_ptz_controller.h"
#include "socket_client.h"

#include "ptz_command_define.h"


ctec_ptz_controller::ctec_ptz_controller( void )
	: _r_min(01)
	, _r_max(10)
	, _flipped(0)
{

}

ctec_ptz_controller::~ctec_ptz_controller( void )
{

}

char* ctec_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[CTEC_BNC_5128HR_PELCO_D_BME_3_0][VENDOR];
}

char* ctec_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[CTEC_BNC_5128HR_PELCO_D_BME_3_0][DEVICE];
}

char* ctec_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[CTEC_BNC_5128HR_PELCO_D_BME_3_0][PROTOCOL];
}

char* ctec_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[CTEC_BNC_5128HR_PELCO_D_BME_3_0][VERSION];
}

unsigned short ctec_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[CTEC_BNC_5128HR_PELCO_D_BME_3_0][VENDOR];
}

unsigned short ctec_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[CTEC_BNC_5128HR_PELCO_D_BME_3_0][DEVICE];
}

unsigned short ctec_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[CTEC_BNC_5128HR_PELCO_D_BME_3_0][PROTOCOL];
}

unsigned short ctec_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[CTEC_BNC_5128HR_PELCO_D_BME_3_0][VERSION];
}

unsigned short ctec_ptz_controller::set_host_name( char *host_name )
{
	if( host_name && (strlen(host_name)>0) ) 
	{
		strcpy( _hostname, host_name );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short ctec_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short ctec_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short ctec_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short	ctec_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short ctec_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short ctec_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short ctec_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short ctec_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short ctec_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short ctec_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short ctec_ptz_controller::goto_home_position(float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short ctec_ptz_controller::set_home_position(void)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

// List Making Function
std::map< std::string, std::string > ctec_ptz_controller::get_preset_list_mapped( void ) {
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
// Find Alias in List
std::string	ctec_ptz_controller::find_key_by_value( char* value ) {
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

unsigned short	ctec_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; /*

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

	return VMS_PTZ_SUCCESS; */
}

unsigned short	ctec_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; /*
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

	socket_client client( _hostname, _port_number, "/command/presetposition.cgi" );

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

	return VMS_PTZ_SUCCESS; */
}

unsigned short	ctec_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; /*

	std::string key = find_key_by_value( alias );

	socket_client client( _hostname, _port_number, "/command/presetposition.cgi" );

	client.put_variable( "PresetClear", key.c_str() ); 

	if( !client.send_request( _user_id, _user_password ) )
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS; */
}

unsigned short	ctec_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; /*

	std::string key = find_key_by_value( alias );

	if( alias == NULL )
		return VMS_PTZ_FAIL;

	if( strcmp(key.c_str(), "") == 0 )
		return VMS_PTZ_FAIL;

	socket_client client( _hostname, _port_number, "/command/presetposition.cgi" );

	std::string paired_str = key + ",24";
	client.put_variable( "PresetCall", paired_str.c_str() );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS; */
}

unsigned short	ctec_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, unsigned int timeout )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	ctec_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed)
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	ctec_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed)
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	std::string cmd_str;

	switch( UINT8(move) )
	{
		case PTZ_RELATIVE_MOVE_HOME :
		{
			cmd_str = MHO;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_UP :
		{
			cmd_str = PTU;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFT :
		{
			cmd_str = PTL;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHT :
		{
			cmd_str = PTR;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_DOWN :
		{
			cmd_str = PTD;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTUP :
		{
			cmd_str = MLU;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTUP :
		{
			cmd_str = MRU;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFTDOWN :
		{
			cmd_str = MLD;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHTDOWN :
		{
			cmd_str = MRD;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_ZOOMIN :
		{
			cmd_str = ZMT;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_ZO0MOUT :
		{
			cmd_str = ZMW;
			value = VMS_PTZ_SUCCESS;
			break;
		}
		default:
			break;
	}

	std::string prefix
		="CAM_CTRLEX BMSP/0.3\r\nID: channel1\r\nUSER-ID: "
		+ std::string(_user_id)
		+ "\r\nPASSWORD: "
		+ std::string(_user_password)
		+ "\r\nCHANNEL: 1\r\nCOMMAND: "
		;

	char str_sensitive[10] = {0,};
	snprintf( str_sensitive, sizeof(str_sensitive), "%d", (int)speed );
	// Ctec은 최대값 정의가 없음

	std::string postfix
		= "\r\nPARAM_X_SPEED: "
		+ std::string(str_sensitive)
		+ "\r\nPARAM_Y_SPEED: "
		+ std::string(str_sensitive)
		+"\r\nPARAM_Z_SPEED: "
		+ std::string(str_sensitive)
		+ "\r\n\r\n";

	std::string		pelco_str = 
		prefix
		+ cmd_str
		+ postfix;
	std::string		stop_str =
		prefix
		+ STA
		+ postfix;

	socket_client sckt( _hostname, _port_number );
	sckt.send_msg( const_cast<char*>( pelco_str.c_str() ));
	Sleep( sensitive*10 ); // Sensitive로 Timeout설정
	socket_client sckt2( _hostname, _port_number );
	sckt2.send_msg( const_cast<char*>( stop_str.c_str() ));
	
	return value;
}

unsigned short	ctec_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed)
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	return value;
}

unsigned short	ctec_ptz_controller::stop_move( void )
{
	std::string prefix
		="CAM_CTRLEX BMSP/0.3\r\nID: channel1\r\nUSER-ID: "
		+ std::string(_user_id)
		+ "\r\nPASSWORD: "
		+ std::string(_user_password)
		+ "\r\nCHANNEL: 1\r\nCOMMAND: "
		;
	std::string postfix
		= "\r\nPARAM_X_SPEED: 1\r\nPARAM_Y_SPEED: 1\r\nPARAM_Z_SPEED: 1\r\n"
		;

	std::string		stop_str =
		prefix
		+ "STA"
		+ postfix;

	socket_client sckt( _hostname, _port_number );
	sckt.send_msg( const_cast<char*>( stop_str.c_str() ));

	return VMS_PTZ_SUCCESS;
}

unsigned short	ctec_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
	
	// Ctec에서는 지원안함
	float pan1, tilt1, zoom1;
	query_position( pan1, tilt1, zoom1 );
	pan		= get_ab_pan_quasi_sensitive_value( pan1 );
	tilt	= get_ab_tilt_quasi_sensitive_value( tilt1 );
	zoom	= get_ab_zoom_quasi_sensitive_value( zoom1 );
	return VMS_PTZ_SUCCESS;
}



unsigned short	ctec_ptz_controller::query_limits( void )
{
	// 추후 Absolute Move가 존재하는 경우 추가가 되어야 한다.
	// Query 형식이나 절대범위 변수 지정방식을 사용한다.

	return VMS_PTZ_UNSUPPORTED_COMMAND; /*

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
	return VMS_PTZ_SUCCESS; */
}

unsigned short	ctec_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; /*

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
	return VMS_PTZ_SUCCESS; */
}

/* FLIPPING */
unsigned short	ctec_ptz_controller::make_current_cam_information( void *param )
{
		return VMS_PTZ_UNSUPPORTED_COMMAND; /*

	std::map<std::string, std::string>	*current_ptz_informations = static_cast<std::map<std::string, std::string>*>( param );

	char limits[500] = {0};
	socket_client client( _hostname, _port_number, "/command/inquiry.cgi" );
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
	return VMS_PTZ_SUCCESS; */
}

unsigned short	ctec_ptz_controller::make_current_ptz_information( void *param )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; /*
	std::map<std::string, std::string>	*current_ptz_informations = static_cast<std::map<std::string, std::string>*>( param );

	char limits[500] = {0};
	socket_client client( _hostname, _port_number, "/command/inquiry.cgi" );
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
	return VMS_PTZ_SUCCESS; */
}

unsigned short	ctec_ptz_controller::make_current_presetposition_information( void *param )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; /*
	std::map<std::string, std::string>	*current_presetposition_informations = static_cast<std::map<std::string, std::string>*>( param );

	char limits[2000] = {0};
	socket_client client( _hostname, _port_number, "/command/inquiry.cgi" );
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
	return VMS_PTZ_SUCCESS; */
}

unsigned short	ctec_ptz_controller::is_flipped( void )
{
		return VMS_PTZ_UNSUPPORTED_COMMAND; /*

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
	return VMS_PTZ_SUCCESS; */
}

	// Grouping
unsigned short	ctec_ptz_controller::add_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}
unsigned short	ctec_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}
unsigned short  ctec_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}
unsigned short	ctec_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}
unsigned short	ctec_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}
unsigned short	ctec_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

float	ctec_ptz_controller::get_re_sensitive_value( unsigned short sensitive )
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

float	ctec_ptz_controller::get_re_speed_sensitive_value( unsigned short sensitive )
{
	query_limits();
	_min_speed = 1;
	_max_speed = 100;
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

float ctec_ptz_controller::get_ab_pan_sensitive_value( unsigned short sensitive )
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
		//float real_sensitive = banollim( (sensitive*(_max_pan-_min_pan)/(_pantilt_max-_pantilt_min))+_min_pan, _pantilt_number_place );
		return real_sensitive;
	}
}

float ctec_ptz_controller::get_ab_tilt_sensitive_value( unsigned short sensitive )
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
		float real_sensitive =  sensitive*(_max_tilt-_min_tilt)/(_tilt_max-_tilt_min)+_min_tilt;
		//float real_sensitive =  banollim( (sensitive*(_max_tilt-_min_tilt)/(_pantilt_max-_pantilt_min))+_min_tilt, _pantilt_number_place );
		return real_sensitive;
	}
}

float ctec_ptz_controller::get_ab_zoom_sensitive_value(unsigned short sensitive )
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
		float real_sensitive = sensitive*(_max_zoom-_min_zoom)/(_zoom_max-_zoom_min)+_min_zoom;
		//float real_sensitive = banollim( (sensitive*(_max_zoom-_min_zoom)/(_zoom_max-_zoom_min))+_min_zoom, _zoom_number_place );
		return real_sensitive;
	}
}

float ctec_ptz_controller::get_ab_pan_quasi_sensitive_value( float real_sensitive )
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
		//float sensitive = banollim( ((real_sensitive-_min_pan*(_pantilt_max-_pantilt_min)-_min_pan)/(_max_pan-_min_pan)), _pantilt_number_place );
		if( sensitive>_pan_max )
			sensitive = _pan_max;
		if( sensitive<_pan_min )
			sensitive = _pan_min;
		return sensitive;
	}
}

float ctec_ptz_controller::get_ab_tilt_quasi_sensitive_value( float real_sensitive )
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
		//float sensitive = banollim( ((real_sensitive-_min_tilt*(_pantilt_max-_pantilt_min)-_min_tilt)/(_max_tilt-_min_tilt)), _pantilt_number_place );
		if( sensitive>_tilt_max )
			sensitive = _tilt_max;
		if( sensitive<_tilt_min )
			sensitive = _tilt_min;
		return sensitive;
	}
}

float ctec_ptz_controller::get_ab_zoom_quasi_sensitive_value( float real_sensitive )
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
		//float sensitive = banollim( (((real_sensitive-_min_zoom)*(_zoom_max-_zoom_min)-_min_zoom)/(_max_zoom-_min_zoom)), _zoom_number_place+1 );
		float sensitive = ((real_sensitive-_min_zoom)*(_zoom_max-_zoom_min)-_min_zoom)/(_max_zoom-_min_zoom);
		if( sensitive>_zoom_max )
			sensitive = _zoom_max;
		if( sensitive<_zoom_min )
			sensitive = _zoom_min;
		return sensitive;
	}
}


unsigned short ctec_ptz_controller::hexa_string_to_char_string( const char *hexa_string, unsigned char *char_string, int n )
{
    unsigned char d_ch;
    for(int i=0; i<n; i++)
    {
        hexa_to_char( hexa_string+2*i, d_ch );
        char_string[i] = d_ch;
    }
	return VMS_PTZ_SUCCESS;
}

unsigned short ctec_ptz_controller::hexa_to_char( const char *hex, unsigned char &chr )
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
	return new ctec_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	ctec_ptz_controller *ctec_controller = dynamic_cast<ctec_ptz_controller*>( (*ptz_controller) );
	delete ctec_controller;
	(*ptz_controller) = 0;
}
