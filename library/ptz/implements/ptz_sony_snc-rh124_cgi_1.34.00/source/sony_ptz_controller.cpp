#include "platform.h"
#include <ptz_device_info.h>
#include "sony_ptz_controller.h"
#include "http_client.h"
#include <iostream>
#include <sstream>
#include <iomanip>

sony_ptz_controller::sony_ptz_controller( void )
	: _is_limits_queried(false)
{
}

sony_ptz_controller::~sony_ptz_controller( void )
{
}

char* sony_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SONY_SNC_RH124_CGI_1_34_00][VENDOR];
}

char* sony_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SONY_SNC_RH124_CGI_1_34_00][DEVICE];
}

char* sony_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SONY_SNC_RH124_CGI_1_34_00][PROTOCOL];
}

char* sony_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[SONY_SNC_RH124_CGI_1_34_00][VERSION];
}

unsigned short sony_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[SONY_SNC_RH124_CGI_1_34_00][VENDOR];
}

unsigned short sony_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[SONY_SNC_RH124_CGI_1_34_00][DEVICE];
}

unsigned short sony_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[SONY_SNC_RH124_CGI_1_34_00][PROTOCOL];
}

unsigned short sony_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[SONY_SNC_RH124_CGI_1_34_00][VERSION];
}

unsigned short sony_ptz_controller::set_host_name( char *host )
{
	if( host && (strlen(host)>0) ) 
	{
		strcpy( _host, host );
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

unsigned short sony_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_min = min;
	_pan_max = max;
	_pan_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_min = min;
	_tilt_max = max;
	_tilt_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_min = min;
	_zoom_max = max;
	_zoom_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_min = min;
	_speed_max = max;
	_speed_number_place = number_place;
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short sony_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short sony_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short sony_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short sony_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short sony_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short sony_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short sony_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short sony_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short sony_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short sony_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short sony_ptz_controller::goto_home_position( float speed )
{
	http_client client( _host, _port_number, "/command/presetposition.cgi" );
	client.put_variable( "HomePos", "ptz-recall" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::set_home_position( void )
{
	http_client client( _host, _port_number, "/command/presetposition.cgi" );
	client.put_variable( "HomePos", "set" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	std::map<std::string,std::string> preset_map = get_preset_list_map();
	std::map<std::string,std::string>::iterator it;

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

unsigned short sony_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

std::map< std::string, std::string > sony_ptz_controller::get_preset_list_map()
{
	// step 1. get untokenizing string [current_presetposition_information]
	unsigned short value = VMS_PTZ_FAIL;
	std::map<std::string, std::string>	current_presetposition_informations;
	make_current_presetposition_information( &current_presetposition_informations );

	// step 2. tokenize with ','. make result in the form of map [mapped_list_preset]
	std::map<std::string, std::string>::iterator iter;
	iter = current_presetposition_informations.find( "PresetName" );
	std::map< std::string, std::string >	preset_map;

	if( iter!=current_presetposition_informations.end() )
	{
		preset_map = get_preset_list_map_by_string(iter->second);
	}

	return preset_map;
}

std::map< std::string, std::string > sony_ptz_controller::get_preset_list_map_by_string(std::string raw_list_preset)
{
	std::map< std::string, std::string >	preset_map;
	int token_head_pos = 0;
	int token_tail_pos = 0;
	std::string token_key;
	std::string token_value;

	while( token_tail_pos != std::string::npos ) 
	{
		token_tail_pos = raw_list_preset.find( ',', token_head_pos );
		token_key = raw_list_preset.substr( token_head_pos, token_tail_pos - token_head_pos );

		token_head_pos = token_tail_pos + 1;
		token_tail_pos = raw_list_preset.find( ',', token_head_pos );
		token_value = raw_list_preset.substr( token_head_pos, token_tail_pos - token_head_pos);

		preset_map.insert( std::pair< std::string, std::string >( token_key, token_value ) );
		token_head_pos = token_tail_pos + 1;
	}

	return preset_map;
}

unsigned short	sony_ptz_controller::make_current_presetposition_information( std::map<std::string, std::string> *preset_map )
{
	char limits[5000] = {0};
	http_client client( _host, _port_number, "/command/inquiry.cgi" );
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

			preset_map->insert( std::make_pair(key, value) );
		}
	}
	while(true);
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::add_preset( char *alias )
{
int preset_available = 0;

	// STEP 1: FIND PRESET-NUMBER AVAILABLE
	std::map< std::string, std::string > list = get_preset_list_map();
	std::map< std::string, std::string >::iterator it;
	std::stringstream convert_str_int;

	if( list.size() == 256 )
		return VMS_PTZ_FAIL;

	const int MAX_PRESET = 256;
	
	for( it=list.begin(); it != list.end(); it++ ) {
		if(strcmp(it->second.c_str(), alias) == 0 ) 
		{
			convert_str_int.clear();
			convert_str_int.str( std::string() );
			convert_str_int << (*it).first;
			convert_str_int >> preset_available;
			break;
		}
	}

	if(preset_available == 0)
	{
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
	}

	// STEP 2: SEND CGI QUERY TO CAMERA

	http_client client( _host, _port_number, "/command/presetposition.cgi" );

	std::string paired_str;
	convert_str_int.clear();
	convert_str_int << preset_available;
	convert_str_int >> paired_str;
	paired_str += ",";
	paired_str += alias;
	paired_str += ",on";
	client.put_variable( "PresetSet", paired_str.c_str() );
	//client.put_variable( "PresetThumbnailClear", paired_str.c_str() );//썸네일 추가안할시 추가

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::remove_preset( char *alias )
{
	std::map< std::string, std::string > preset_map = get_preset_list_map();
	std::string preset_id = get_id_by_name( &preset_map, alias );

	http_client client( _host, _port_number, "/command/presetposition.cgi" );
	client.put_variable( "PresetClear", preset_id.c_str() ); 
	client.put_variable( "PresetThumbnailClear", preset_id.c_str() ); 
	
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::goto_preset( char *alias )
{
	std::map< std::string, std::string > preset_map = get_preset_list_map();
	std::string preset_id = get_id_by_name( &preset_map, alias );
	if(preset_id=="") return VMS_PTZ_FAIL;

	http_client client( _host, _port_number, "/command/presetposition.cgi" );
	std::string paired_str = preset_id + ",24";
	client.put_variable( "PresetCall", paired_str.c_str() );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

std::string sony_ptz_controller::get_preset_tour_id( char *tour_name)
{
	std::map<std::string,std::string>::iterator it;		
	std::string str = std::string(tour_name);	   
	it = _tour_list.find(str);
    if(it!=_tour_list.end()) {		
		return it->second;
	}
	return "";
}

unsigned short sony_ptz_controller::remove_preset_tour( char *tour_name )
{
	std::map< std::string, std::string > preset_map = get_preset_list_map();
	char tmp_str[100]= {0,};
	http_client client( _host, _port_number, "/command/presetposition.cgi" );	
	snprintf( tmp_str, sizeof(tmp_str), "Tour%s", tour_name);
	client.put_variable( tmp_str, "off");
	snprintf( tmp_str, sizeof(tmp_str), "Tour%sSequence", tour_name);
	client.put_variable( tmp_str, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
	snprintf( tmp_str, sizeof(tmp_str), "Tour%sStayTime", tour_name);
	client.put_variable( tmp_str, "10");
	snprintf( tmp_str, sizeof(tmp_str), "Tour%sPeriod", tour_name);
	client.put_variable( tmp_str, "always");
	snprintf( tmp_str, sizeof(tmp_str), "Tour%sSpeed", tour_name);
	client.put_variable( tmp_str, "24");
	client.put_variable( "reload", "referer");

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}		
	client.clear_variable();	

	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::remove_preset_tour_presets( const char *tour_no )
{
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{	
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	////command/presetposition.cgi&Tour=on&TourResume=off&TourA=off&reload=referer
	http_client client( _host, _port_number, "/command/presetposition.cgi" );
	char str_tour_name[10]= {0,};
	snprintf( str_tour_name, sizeof(str_tour_name), "Tour%s", tour_name);
	client.put_variable( "Tour", "on");	
	client.put_variable( "reload", "referer");	

	switch( UINT8(cmd) )
	{
		case PTZ_TOUR_CMD_START  :
			client.put_variable( "TourResume", "on");	
			client.put_variable( str_tour_name, "on");				
			break;

		case PTZ_TOUR_CMD_STOP :
			//client.put_variable( "TourResume", "off");	
			client.put_variable( str_tour_name, "off");				
			break;

		case PTZ_TOUR_CMD_PAUSE : 
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;

		default :
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
	}

	if(UINT8(cmd) ==PTZ_TOUR_CMD_START)
	{
		std::map< std::string, std::string > ::iterator it;
		for(it= _tour_list.begin();it!= _tour_list.end();it++)
		{
			operate_preset_tour(const_cast<char*>(it->second.c_str()), PTZ_TOUR_CMD_STOP);
		}
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

unsigned short sony_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short sony_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{	
	//Tour=on&TourRsmTime=180&TourResume=on&TourB=on&TourBSpeed=24&TourBStaytime=2&TourBPeriod=always&TourBSequence=6,256,8,0,0,0,0,0,0,0,0,0,0,0,0,0&reload=referer
	//속도:1~24, 정착시간: 1~3600초
	//각 프리셋에 정해진속도가 아니라 투어에서 공통으로 속도와 시간을 하나로 관리함으로 평균값으로처리하게 해놓음
	std::map< std::string, std::string > preset_map = get_preset_list_map();
	
	char tmp_str[100]= {0,};
	char tmp_value[10]= {0,};
	http_client client( _host, _port_number, "/command/presetposition.cgi" );	
	client.put_variable( "Tour", "on");	
	snprintf( tmp_str, sizeof(tmp_str), "Tour%s", tour->tour_name);
	client.put_variable( tmp_str, "off");
	client.put_variable( "TourRsmTime", "180");
	snprintf( tmp_str, sizeof(tmp_str), "Tour%sPeriod", tour->tour_name);
	client.put_variable( tmp_str, "always");
	
	std::string str_tour_no = tour->tour_name;
	//std::string str_tour_no = get_preset_tour_id(tour->tour_name);
	//if(str_tour_no=="") return VMS_PTZ_FAIL;

	std::string preset_seq = "";
	int sum_speed = 0;
	int sum_staytime = 0;
	
	for(int i=0; i<tour->size_of_tour_spots;i++)
	{
		PTZ_TOUR_SPOT_T* tsp = &(tour->tour_spots[i]);		
		std::string preset_id = get_id_by_name( &preset_map, tsp->preset_alias );
		if(preset_id=="") return VMS_PTZ_FAIL;
		preset_seq.append(preset_id+",");
		sum_speed    += int(get_speed_sensitive_value(tsp->speed));
		sum_staytime += tsp->stay_time/1000;
	}	

	snprintf( tmp_str, sizeof(tmp_str), "Tour%sSpeed", tour->tour_name);
	snprintf( tmp_value, sizeof(tmp_value), "%d", sum_speed/tour->size_of_tour_spots);
	client.put_variable( tmp_str, tmp_value);
	snprintf( tmp_str, sizeof(tmp_str), "Tour%sStayTime", tour->tour_name);
	snprintf( tmp_value, sizeof(tmp_value), "%d", sum_staytime/tour->size_of_tour_spots);
	client.put_variable( tmp_str, tmp_value);

	for(int i=0; i<16-tour->size_of_tour_spots; i++)
	{
		preset_seq.append("0,");
	}
	preset_seq = preset_seq.substr(0, preset_seq.length()-1);

	snprintf( tmp_str, sizeof(tmp_str), "Tour%sSequence", tour->tour_name);
	client.put_variable( tmp_str, preset_seq.c_str());
	client.put_variable( "reload", "referer");

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}		
	client.clear_variable();	

	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	unsigned short value = VMS_PTZ_FAIL;
	std::map<std::string, std::string>	current_presetposition_informations;
	make_current_presetposition_information( &current_presetposition_informations );

	char tmp_str_key[100]= {0,};
	long staytime = 0;
	float speed = 0.0;
	std::map<std::string, std::string>::iterator iter;

	snprintf( tmp_str_key, sizeof(tmp_str_key), "Tour%sStaytime", tour->tour_name);
	iter = current_presetposition_informations.find( tmp_str_key );
	if( iter!=current_presetposition_informations.end() ) 
		staytime = std::stol(iter->second,0,10)*1000;	

	snprintf( tmp_str_key, sizeof(tmp_str_key), "Tour%sSpeed", tour->tour_name);
	iter = current_presetposition_informations.find( tmp_str_key );
	if( iter!=current_presetposition_informations.end() )
		speed = get_speed_quasi_sensitive_value(std::stof(iter->second,0));

	iter = current_presetposition_informations.find( "PresetName" );
	std::map< std::string, std::string >	preset_map;
	if( iter!=current_presetposition_informations.end() )
		preset_map = get_preset_list_map_by_string(iter->second);

	std::vector<std::string> vec_preset_name;
	snprintf( tmp_str_key, sizeof(tmp_str_key), "Tour%sSequence", tour->tour_name);
	if( iter!=current_presetposition_informations.end() )
		iter = current_presetposition_informations.find( tmp_str_key );
	if( iter!=current_presetposition_informations.end() )
	{
		std::string preset_id_strs = iter->second;
		int token_head_pos = 0;
		int token_tail_pos = 0;
		std::string preset_id;
		std::string token_value;

		while( token_tail_pos != std::string::npos ) {
			token_tail_pos = preset_id_strs.find( ',', token_head_pos );
			preset_id = preset_id_strs.substr( token_head_pos, token_tail_pos - token_head_pos );
			if(preset_id == "0") break;
			vec_preset_name.push_back(get_name_by_id(&preset_map, preset_id));
			token_head_pos = token_tail_pos + 1;
		}
	}
	tour->tour_spots = new PTZ_TOUR_SPOT_T[vec_preset_name.size()];	

	int idx = 0;
	std::vector<std::string>::iterator vec_it;
	for(vec_it = vec_preset_name.begin(); vec_it != vec_preset_name.end(); vec_it++)
	{
		PTZ_TOUR_SPOT_T *tsp = &(tour->tour_spots[idx++]);
		strncpy( tsp->preset_alias, vec_it->c_str(), sizeof(tsp->preset_alias) );
		tsp->stay_time = staytime;
		tsp->speed = speed;
	}

	tour->size_of_tour_spots = vec_preset_name.size();
	tour->tour_recurring_time = 0;	
	tour->tour_recurring_duration = 0;	
	tour->tour_direction = PTZ_TOUR_DIRECTION_FORWARD;

	snprintf( tmp_str_key, sizeof(tmp_str_key), "Tour%sPeriod", tour->tour_name);
	iter = current_presetposition_informations.find( tmp_str_key );
	if(iter->second == "always") tour->tour_always_start=true;
	else tour->tour_always_start=false;
	
	return VMS_PTZ_SUCCESS;			
}

std::string sony_ptz_controller::get_name_by_id(std::map<std::string,std::string> *preset_map, std::string id)
{	
	std::map<std::string,std::string>::iterator it;
	for(it=preset_map->begin();it!=preset_map->end();it++)
	{	
		if(it->first == id) return it->second;
	}
	return "";
	
}

std::string sony_ptz_controller::get_id_by_name(std::map<std::string,std::string> *preset_map, std::string name)
{		
	std::map<std::string,std::string>::iterator it;
	for(it=preset_map->begin();it!=preset_map->end();it++)
	{	
		if(it->second == name) 
		{
			return it->first;
		}
	}
	return "";
	
}


unsigned short sony_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{	
	char tour_arr[] = {'A','B','C','D','E'};
	std::string tmp_str = "";
	_tour_list.clear();
	for(int i=0; i<sizeof(tour_arr); i++)
	{
		tmp_str = tour_arr[i];
		_tour_list.insert( std::pair< std::string, std::string >( tmp_str, tmp_str ) );
	}

	(*size_of_tours) = _tour_list.size();   
	*tour = new PTZ_TOUR_T[_tour_list.size()];   
	
    std::map<std::string,std::string>::iterator it;
	int idx = 0;
    for(it=_tour_list.begin(); it!=_tour_list.end(); ++it, idx++)   
	{		
		strncpy( (*tour+idx)->tour_name, it->first.c_str(), sizeof((*tour+idx)->tour_name) );
	}		
	
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	int real_pan_sensitive	= int(get_continuous_sensitive_value( pan_sensitive ));
	int real_tilt_sensitive	= int(get_continuous_sensitive_value( tilt_sensitive ));
	int real_zoom_sensitive	= int(get_continuous_sensitive_value( zoom_sensitive ));

	http_client client( _host, _port_number, "/command/ptzf.cgi" );
	char ptz[50]	= {0,};
	snprintf( ptz, sizeof(ptz), "%d,%d,%d,0", real_pan_sensitive, real_tilt_sensitive, real_zoom_sensitive );
	client.put_variable( "ContinuousPanTiltZoom", ptz );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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

unsigned short sony_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	char str_cmd[20] = {0,}; 

	int real_pan_sensitive	= abs(int(get_rpan_sensitive_value( pan_sensitive )));
	int real_tilt_sensitive	= abs(int(get_rtilt_sensitive_value( tilt_sensitive )));
	int real_zoom_sensitive	= abs(int(get_rzoom_sensitive_value( zoom_sensitive )));

	if( real_tilt_sensitive<0 )	//카메라가 tilt 방향으로 90도 이상 뒤로 넘어가는 것을 방지함.
	{
		float pan = 0.0;
		float tilt = 0.0;
		float zoom = 0.0;
		query_position( pan, tilt, zoom );
		//if( tilt+real_tilt_sensitive<(_max_tilt-((_max_tilt-_min_tilt)/2)) )
		//	real_tilt_sensitive = 0;
		float limit_tilt = (_max_tilt-((_max_tilt-_min_tilt)/2));
		if( tilt+real_tilt_sensitive< limit_tilt)	
			if(tilt>limit_tilt && tilt+real_tilt_sensitive<limit_tilt) 
			{
					real_tilt_sensitive = limit_tilt - tilt;
			}
			else 
				{
					real_tilt_sensitive = 0;
			}
	}

	if(pan_sensitive==0 && tilt_sensitive==0&&zoom_sensitive==0) //정지
	{
		value = stop_move();
		return value;
	}
	else if(pan_sensitive!=0 && tilt_sensitive==0)//좌우
	{
		if(pan_sensitive<0) //left
		{
			snprintf(str_cmd, sizeof(str_cmd), "04%.2d", real_pan_sensitive);
		}
		else //right
		{
			snprintf(str_cmd, sizeof(str_cmd), "06%.2d", real_pan_sensitive);
		}
	}
	else if(tilt_sensitive!=0&&pan_sensitive==0)//상하
	{
		if(tilt_sensitive>0)
		{
			snprintf(str_cmd, sizeof(str_cmd), "08%.2d", real_tilt_sensitive);
		}
		else
		{
			snprintf(str_cmd, sizeof(str_cmd), "02%.2d", real_tilt_sensitive);
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//좌상
	{
		snprintf(str_cmd, sizeof(str_cmd), "07%.2d", get_max_abs_value(real_pan_sensitive, real_tilt_sensitive));
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//우하
	{
		snprintf(str_cmd, sizeof(str_cmd), "03%.2d", get_max_abs_value(real_pan_sensitive, real_tilt_sensitive));
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//좌하
	{
		snprintf(str_cmd, sizeof(str_cmd), "01%.2d", get_max_abs_value(real_pan_sensitive, real_tilt_sensitive));
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//우상
	{
		snprintf(str_cmd, sizeof(str_cmd), "09%.2d", get_max_abs_value(real_pan_sensitive, real_tilt_sensitive));
	}
	else if(zoom_sensitive>0)//줌인
	{
		snprintf(str_cmd, sizeof(str_cmd), "11%.2d", real_zoom_sensitive);
	}
	else if(zoom_sensitive<0)//줌아웃
	{
		snprintf(str_cmd, sizeof(str_cmd), "10%.2d", real_zoom_sensitive);
	}

	http_client client( _host, _port_number, "/command/ptzf.cgi" );
	client.put_variable( "Relative", str_cmd );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed  )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(move) )
	{
		case PTZ_RELATIVE_MOVE_HOME :
		{
			value = goto_home_position( speed );
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

unsigned short sony_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	// 중간 return 으로 인해 아래 if문(zoom수행) 위로 수정
	//char pan[100]	= {0,};
	//char tilt[100]	= {0,};
	//char zoom[100]	= {0,};
	//char speed[100] = {0,};

	float real_pan_sensitive	= get_apan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_atilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_azoom_sensitive_value( zoom_sensitive );
	float real_speed_sensitive	= get_speed_sensitive_value( speed_sensitive );

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


	http_client client( _host, _port_number, "/command/ptzf.cgi" );
	
	char pan_tilt[100] = {0,};
	snprintf( pan_tilt, sizeof(pan_tilt), "%s,%s,%d", str_pan_sensitive.c_str(), str_tilt_sensitive.c_str(), int(real_speed_sensitive) );
	client.put_variable( "AbsolutePanTilt", pan_tilt );

	char zoom[100] = {0,};
	snprintf( zoom, sizeof(zoom), "%s", str_zoom_sensitive.c_str() );
	client.put_variable( "AbsoluteZoom", zoom );


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

unsigned short sony_ptz_controller::stop_move( void )
{
	http_client client( _host, _port_number, "/command/ptzf.cgi" );
	//client.put_variable( "Move", "stop,motor" );
	//client.put_variable( "Move", "stop,zoom" );
	client.put_variable( "ContinuousPanTiltZoom", "0,0,0,0" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}

	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	float tmp_pan, tmp_tilt, tmp_zoom;
	query_position( tmp_pan, tmp_tilt, tmp_zoom );
	pan		= get_apan_quasi_sensitive_value( tmp_pan );
	tilt	= get_atilt_quasi_sensitive_value( tmp_tilt );
	zoom	= get_azoom_quasi_sensitive_value( tmp_zoom );
	return VMS_PTZ_SUCCESS;
}

unsigned short	sony_ptz_controller::query_limits( void )
{
	if( _is_limits_queried )
		return VMS_PTZ_SUCCESS;

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

    _min_rpan = 0;
	_max_rpan = 10;
	_min_rtilt = 0;
	_max_rtilt = 10;
	_min_rzoom = 0;
	_max_rzoom = 10;
	_min_cspeed			= 0;//-100;
	_max_cspeed			= 100;
	_min_speed			= 0;
	_max_speed			= 24;
	_is_limits_queried	= true;

	return VMS_PTZ_SUCCESS;
}

unsigned short sony_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
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

unsigned short	sony_ptz_controller::make_current_ptz_information( void *param )
{
	std::map<std::string, std::string>	*current_ptz_informations = static_cast<std::map<std::string, std::string>*>( param );

	char limits[500] = {0};
	http_client client( _host, _port_number, "/command/inquiry.cgi" );
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

float sony_ptz_controller::get_continuous_sensitive_value( float sensitive )
{
	query_limits();

	if( abs(sensitive)>abs(_speed_max-_speed_min) ) 
	{
		if( sensitive<0 )
			return -_max_cspeed;
		else
			return _max_cspeed;
	}

	if( _speed_min==_min_cspeed && _speed_max==_max_cspeed )
		return sensitive;
	else
	{
		float real_sensitive = float(sensitive*(_max_cspeed-_min_cspeed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

float sony_ptz_controller::get_speed_sensitive_value( float sensitive )
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

float sony_ptz_controller::get_speed_quasi_sensitive_value( float real_sensitive )
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

float sony_ptz_controller::get_rpan_sensitive_value( float sensitive )
{	
	query_limits();

	if( _pan_min==_min_rpan && _pan_max==_max_rpan )
		return sensitive;
	
	if( abs(sensitive)>abs(_pan_max-_pan_min) ) 
	{
		if( sensitive<0 )
			return -_max_rpan;
		else
			return _max_rpan;
	}
	else
	{
		float real_sensitive = float(sensitive*(_max_rpan-_min_rpan))/float(_pan_max-_pan_min);		
		return real_sensitive;
	}		
}

float sony_ptz_controller::get_rtilt_sensitive_value( float sensitive )
{
	query_limits();

	if( _tilt_min==_min_rtilt && _tilt_max==_max_rtilt )
		return sensitive;
	
	if( abs(sensitive)>abs(_tilt_max-_tilt_min) ) 
	{
		if( sensitive<0 )
			return -_max_rtilt;
		else
			return _max_rtilt;
	}
	else
	{
		float real_sensitive = (sensitive*(_max_rtilt-_min_rtilt))/(_tilt_max-_tilt_min);
		return real_sensitive;
	}
}

float sony_ptz_controller::get_rzoom_sensitive_value( float sensitive )
{
	query_limits();

	if( _zoom_min==_min_rzoom && _zoom_max==_max_rzoom )
		return sensitive;

	if( abs(sensitive)>abs(_zoom_max-_zoom_min) ) 
	{
		if( sensitive<0 )
			return -_max_rzoom;
		else
			return _max_rzoom;
	}
	else
	{
		float real_sensitive = ((sensitive)*(_max_rzoom-_min_rzoom))/(_zoom_max-_zoom_min);
		return real_sensitive;
	}
}

float sony_ptz_controller::get_apan_sensitive_value( float sensitive )
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

float sony_ptz_controller::get_atilt_sensitive_value( float sensitive )
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

float sony_ptz_controller::get_azoom_sensitive_value( float sensitive )
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

float sony_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
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

float sony_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
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

float sony_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
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

void sony_ptz_controller::split2vector( std::string origin, std::string token, std::vector<std::string> *devided )
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

void sony_ptz_controller::split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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

void sony_ptz_controller::split2map2( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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

int sony_ptz_controller::get_max_abs_value( int a, int b )
{
	if(abs(a)>abs(b)) return abs(a);
	else return abs(b);
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
