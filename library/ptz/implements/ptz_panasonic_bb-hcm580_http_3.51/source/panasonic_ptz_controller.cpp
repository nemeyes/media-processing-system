#include "platform.h"
#include <ptz_device_info.h>
#include "panasonic_ptz_controller.h"
#include "http_client.h"

panasonic_ptz_controller::panasonic_ptz_controller( void )
	: _is_limits_queried(false)
{

}

panasonic_ptz_controller::~panasonic_ptz_controller( void )
{

}

char* panasonic_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[PANASONIC_BB_HCM580_HTTP_3_51][VENDOR];
}

char* panasonic_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[PANASONIC_BB_HCM580_HTTP_3_51][DEVICE];
}

char* panasonic_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[PANASONIC_BB_HCM580_HTTP_3_51][PROTOCOL];
}

char* panasonic_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[PANASONIC_BB_HCM580_HTTP_3_51][VERSION];
}

unsigned short panasonic_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[PANASONIC_BB_HCM580_HTTP_3_51][VENDOR];
}

unsigned short panasonic_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[PANASONIC_BB_HCM580_HTTP_3_51][DEVICE];
}

unsigned short panasonic_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[PANASONIC_BB_HCM580_HTTP_3_51][PROTOCOL];
}

unsigned short panasonic_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[PANASONIC_BB_HCM580_HTTP_3_51][VERSION];
}

unsigned short panasonic_ptz_controller::set_host_name( char *host )
{
	if( host && (strlen(host)>0) ) 
	{
		strcpy( _host, host );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short panasonic_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short panasonic_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short panasonic_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short panasonic_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short panasonic_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short panasonic_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short panasonic_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short panasonic_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short panasonic_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short panasonic_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short panasonic_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short panasonic_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short panasonic_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short panasonic_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_FALSE;	
}

unsigned short panasonic_ptz_controller::goto_home_position( float speed )
{
	///nphControlCamera?Direction=HomePosition&Data=0
	http_client client( _host, _port_number, "/nphControlCamera" );
	client.put_variable( "Direction", "HomePosition" );
	client.put_variable( "Data", "0" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::set_home_position( void )
{
	//Set?Func=PresetNamePos&Kind=0&Data=Home Position
	http_client client( _host, _port_number, "/Set" );
	client.put_variable( "Func", "PresetNamePos" );
	client.put_variable( "Kind", "0" );
	client.put_variable( "Data", "1" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

std::string panasonic_ptz_controller::get_preset_tour_id( char *tour_name)
{
	std::map<std::string,std::string>::iterator it;		
	std::string str = std::string(tour_name);	   
	it = _tour_list.find(str);
    if(it!=_tour_list.end()) {		
		return it->second;
	}
	return "";
}

std::string panasonic_ptz_controller::get_name_by_id(std::map<std::string,std::string> *preset_map, std::string id)
{	
	std::map<std::string,std::string>::iterator it;
	for(it=preset_map->begin();it!=preset_map->end();it++)
	{	
		std::string tmp_str = it->first.substr(11);
		if(strcmp(tmp_str.c_str(), id.c_str())==0) return it->second;
	}
	return "";
	
}

std::string panasonic_ptz_controller::get_id_by_name(std::map<std::string,std::string> *preset_map, std::string name)
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

unsigned short panasonic_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	std::map<std::string,std::string> preset_map;
	std::map<std::string,std::string>::iterator it;		

	unsigned short value = VMS_PTZ_FAIL;
	value = get_prest_list();
	if( value!=VMS_PTZ_SUCCESS )
		return value;

	//20개 고정
	for(int i=0;i<20;i++)
	{   
		char preset[3] = {0,};
		snprintf( preset, sizeof(preset), "%d", i );
		preset_map[preset] = preset;
	}	

	(*length) = preset_map.size();
	if( (*length)>0 )
	{
		(*aliases) = static_cast<char**>( malloc(sizeof(char**)*(*length)) );
		for(int i=0; i<20; i++)
		{
			char preset[3] = {0,};
			snprintf( preset, sizeof(preset), "%d", i );
			(*aliases)[i] = strdup( preset_map[preset].c_str());
		}		
	}
	return VMS_PTZ_SUCCESS;

	/*std::map<std::string,std::string> preset_map;
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
	return VMS_PTZ_SUCCESS;*/
}
unsigned short panasonic_ptz_controller::get_prest_list( void )
{
	http_client client( _host, _port_number, "/Get" );
	client.put_variable( "Func", "PresetCntPos" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::get_preset_list_map( std::map<std::string,std::string> *preset_map )
{
	char presets[1000] = "";
	http_client client( _host, _port_number, "/Get" );
	client.put_variable( "Func", "PresetCntPos" );

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

unsigned short panasonic_ptz_controller::add_preset( char *alias )
{
	char data[10] = {0};
	snprintf( data, sizeof(data), "%c", *alias+1);
	//Set?Func=PresetNamePos&Kind=1&Data=이름
	http_client client( _host, _port_number, "/Set" );
	client.put_variable( "Func", "PresetNamePos" );
	client.put_variable( "Kind", data );
	//data값이 있어야 저장이됨. 실제 화면에 보여지지 않기 때문에 test로 대체
	client.put_variable( "Data", "test" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::remove_preset( char *alias )
{
	char data[10] = {0};
	snprintf( data, sizeof(data), "%c", *alias + 1 );
	//Set?Func=PresetNamePos&Kind=5&Data=
	http_client client( _host, _port_number, "/Set" );
	client.put_variable( "Func", "PresetNamePos" );
	client.put_variable( "Kind", data );
	//client.put_variable( "PresetName", "test" );
	client.put_variable( "Data", "" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::goto_preset( char *alias )
{
	char data[10] = {0};
	snprintf( data, sizeof(data), "%c", *alias + 1 );
	//nphControlCamera?Direction=Preset&PresetOperation=Move&Data=1
	http_client client( _host, _port_number, "/nphControlCamera" );
	client.put_variable( "Direction", "Preset" );
	client.put_variable( "PresetOperation", "Move" );
	client.put_variable( "Data", data );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;	
}

unsigned short panasonic_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
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
			value = relative_move( PTZ_RELATIVE_MOVE_LEFT, pan_sensitive, 0);
		}
		else //right
		{
			value = relative_move( PTZ_RELATIVE_MOVE_RIGHT, pan_sensitive, 0);
		}
	}
	else if(tilt_sensitive!=0&&pan_sensitive==0)//상하
	{
		if(tilt_sensitive>0)
		{
			value = relative_move( PTZ_RELATIVE_MOVE_UP, tilt_sensitive, 0);
		}
		else
		{
			value = relative_move( PTZ_RELATIVE_MOVE_DOWN, tilt_sensitive, 0);
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//좌상
	{
		if(abs(pan_sensitive) >= abs(tilt_sensitive))
		{
			value = relative_move( PTZ_RELATIVE_MOVE_LEFT, float((pan_sensitive+tilt_sensitive)/2), 0);

		}else{
			value = relative_move( PTZ_RELATIVE_MOVE_UP, float((pan_sensitive+tilt_sensitive)/2), 0);
		}
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//우하
	{
		if(abs(pan_sensitive) >= abs(tilt_sensitive))
		{
			value = relative_move( PTZ_RELATIVE_MOVE_RIGHT, float((pan_sensitive+tilt_sensitive)/2), 0);
		}else{
			value = relative_move( PTZ_RELATIVE_MOVE_DOWN, float((pan_sensitive+tilt_sensitive)/2), 0);
		}		
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//좌하
	{
		if(abs(pan_sensitive) >= abs(tilt_sensitive))
		{
			value = relative_move( PTZ_RELATIVE_MOVE_LEFT, float((pan_sensitive+tilt_sensitive)/2), 0);
		}else{
			value = relative_move( PTZ_RELATIVE_MOVE_DOWN, float((pan_sensitive+tilt_sensitive)/2), 0);
		}
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//우상
	{
		if(abs(pan_sensitive) >= abs(tilt_sensitive))
		{
			value = relative_move( PTZ_RELATIVE_MOVE_RIGHT, float((pan_sensitive+tilt_sensitive)/2), 0);
		}else{
			value = relative_move( PTZ_RELATIVE_MOVE_UP, float((pan_sensitive+tilt_sensitive)/2), 0);
		}
	}
	else if(zoom_sensitive>0)//줌인
	{
		value = relative_move( PTZ_RELATIVE_MOVE_ZOOMIN, zoom_sensitive, 0);
	}
	else if(zoom_sensitive<0)//줌아웃
	{
		value = relative_move( PTZ_RELATIVE_MOVE_ZO0MOUT, zoom_sensitive, 0);
	}
	
	return value;
}

unsigned short panasonic_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed  )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	http_client client( _host, _port_number, "/nphControlCamera" );	
	switch( UINT8(move) )
	{
		case PTZ_RELATIVE_MOVE_UP :
		{
			client.put_variable( "Direction", "TiltUp" );				
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_LEFT :
		{
			client.put_variable( "Direction", "PanLeft" );			
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_RIGHT :
		{
			client.put_variable( "Direction", "PanRight" );			
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_DOWN :
		{
			client.put_variable( "Direction", "TiltDown" );			
			value = VMS_PTZ_SUCCESS;
			break;
		}			
		case PTZ_RELATIVE_MOVE_ZOOMIN :
		{
			client.put_variable( "Direction", "ZoomTele" );	
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_RELATIVE_MOVE_ZO0MOUT :
		{
			client.put_variable( "Direction", "ZoomWide" );	
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}		
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::stop_move( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short panasonic_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

unsigned short	panasonic_ptz_controller::query_limits( void )
{
	//On the ceiling:0 - 9000(0 degrees to +90 degrees)
    //On the table:-12000 - 0 (-120 degrees to 0 degrees)
	_min_pan = -90;
	_max_pan = 90;
	_min_tilt = -90;
	_max_tilt = 90;
	_min_zoom = 0.0f;
	_max_zoom = 0.0f;
	_min_speed = 0.0f;
	_max_speed = 0.0f;
	_min_cspeed =  0.0f;
	_max_cspeed =  0.0f;
	return VMS_PTZ_SUCCESS;
}

unsigned short panasonic_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

float panasonic_ptz_controller::get_continuous_sensitive_value( float sensitive )
{
	query_limits();

	/*if( _pantilt_min==_min_cspeed && _pantilt_max==_max_cspeed )
		return sensitive;

	if( sensitive<_pantilt_min ) 
		return float(_min_cspeed);
	else if( sensitive>_pantilt_max ) 
		return float(_max_cspeed);
	else
	{
		float real_sensitive = banollim( sensitive*(_max_cspeed-_min_cspeed)/(_pantilt_max-_pantilt_min), _pantilt_number_place+1 );
		return real_sensitive;
	}*/

	return 0.0f;
}

float panasonic_ptz_controller::get_speed_sensitive_value( float sensitive )
{
	query_limits();

	if( _speed_min==_min_speed && _speed_max==_max_speed )
		return sensitive;

	if( sensitive<_speed_min ) 
		return _min_tilt;
	else if( sensitive>_speed_max ) 
		return _max_tilt;
	else
	{
		float real_sensitive = float(sensitive*(_max_speed-_min_speed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

float panasonic_ptz_controller::get_rpan_sensitive_value( float sensitive )
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

float panasonic_ptz_controller::get_rtilt_sensitive_value( float sensitive )
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

float panasonic_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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

float panasonic_ptz_controller::get_apan_sensitive_value( float sensitive )
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

float panasonic_ptz_controller::get_atilt_sensitive_value( float sensitive )
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

float panasonic_ptz_controller::get_azoom_sensitive_value( float sensitive )
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

float panasonic_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
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

float panasonic_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
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

float panasonic_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
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

void panasonic_ptz_controller::split2vector( std::string origin, std::string token, std::vector<std::string> *devided )
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

void panasonic_ptz_controller::split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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


base_ptz_controller* create( void )
{
	return new panasonic_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	panasonic_ptz_controller *panasonic_controller = dynamic_cast<panasonic_ptz_controller*>( (*ptz_controller) );
	delete panasonic_controller;
	(*ptz_controller) = 0;
}
