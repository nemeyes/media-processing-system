#include "platform.h"
#include <ptz_device_info.h>
#include "hikvision_ptz_controller.h"
#include "http_client.h"
#include <tinyxml.h>

hikvision_ptz_controller::hikvision_ptz_controller( void )
	: _is_inverse(false)
{

}

hikvision_ptz_controller::~hikvision_ptz_controller( void )
{

}

char* hikvision_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HIKVISION_DS_2ZCN3006_REST_V_5_0_2_130917][VENDOR];
}

char* hikvision_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HIKVISION_DS_2ZCN3006_REST_V_5_0_2_130917][DEVICE];
}

char* hikvision_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HIKVISION_DS_2ZCN3006_REST_V_5_0_2_130917][PROTOCOL];
}

char* hikvision_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HIKVISION_DS_2ZCN3006_REST_V_5_0_2_130917][VERSION];
}

unsigned short hikvision_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[HIKVISION_DS_2ZCN3006_REST_V_5_0_2_130917][VENDOR];
}

unsigned short hikvision_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[HIKVISION_DS_2ZCN3006_REST_V_5_0_2_130917][DEVICE];
}

unsigned short hikvision_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[HIKVISION_DS_2ZCN3006_REST_V_5_0_2_130917][PROTOCOL];
}

unsigned short hikvision_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[HIKVISION_DS_2ZCN3006_REST_V_5_0_2_130917][VERSION];
}

unsigned short hikvision_ptz_controller::set_host_name( char *host_name )
{
	if( host_name && (strlen(host_name)>0) ) 
	{
		strcpy( _hostname, host_name );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hikvision_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short hikvision_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hikvision_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hikvision_ptz_controller::set_angle_inverse( bool inverse )
{
	_is_inverse = inverse;
	return VMS_PTZ_SUCCESS;
}

unsigned short hikvision_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_number_place = number_place;
	_pan_min = min;
	_pan_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short hikvision_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_number_place = number_place;
	_tilt_min = min;
	_tilt_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short hikvision_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_number_place = number_place;
	_zoom_min = min;
	_zoom_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short hikvision_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_number_place = number_place;
	_speed_min = min;
	_speed_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short hikvision_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hikvision_ptz_controller::is_enable_home_position( void )
{   
	return VMS_PTZ_FALSE;
}

unsigned short hikvision_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hikvision_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hikvision_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hikvision_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hikvision_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hikvision_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hikvision_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hikvision_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hikvision_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hikvision_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::goto_home_position( float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::set_home_position( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::get_prest_list( void )
{
	unsigned short value = VMS_PTZ_FAIL;

	_preset_map.clear();

	http_client client( _hostname, _port_number, "/PSIA/PTZ/channels/1/presets" );
	char response_data[5000] = {0,};
    if( !client.send_request(_user_id, _user_password, "GET", response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data )
	{
		TiXmlDocument	doc;
		TiXmlElement	*elem		= 0;
		TiXmlElement	*elem_index	= 0;
		TiXmlElement	*elem_name	= 0;

		doc.Parse( (const char*)response_data, 0 );
	   
		for( elem = doc.RootElement()->FirstChildElement("PTZPreset");
                elem != NULL; elem = elem->NextSiblingElement() )
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
			int real_name = 0;
			if( index_text && name_text )
			{
				sscanf( name_text, "%d", &real_name );
				sscanf( index_text, "%d", &real_index );
				_preset_map.insert( std::make_pair(real_name, real_index) );
			}
			else
				continue;
				
		}
	}

	return value;
}

unsigned short hikvision_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	unsigned short value = VMS_PTZ_FAIL;

	value = get_prest_list();
	if( value!=VMS_PTZ_SUCCESS )
		return value;

	(*length)	= _preset_map.size();
	(*aliases)	= static_cast<int*>( malloc(sizeof(char**)*(*length)) );

	std::map<int, int>::iterator iter;
	int index = 0;
	for( iter=_preset_map.begin(); iter!=_preset_map.end(); iter++, index++ )
		(*aliases)[index] = (*iter).first;
	return value;
}

unsigned short hikvision_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::add_preset2( int &alias )
{
	unsigned short value = VMS_PTZ_FAIL;

	if(alias>256 || alias<=0) return VMS_PTZ_FAIL;

	char url_data[50] = {0};
	snprintf( url_data, sizeof(url_data), "/PSIA/PTZ/channels/1/presets/%d", alias );
	char post_data[100] = {0};
	snprintf( post_data, sizeof(post_data), "<PTZPreset><id>%d</id><presetName>%d</presetName></PTZPreset>", alias, alias );
	char *response_data = 0;
	http_client client( _hostname, _port_number, url_data );
	if( !client.send_request_with_data(_user_id, _user_password, "PUT", post_data, &response_data) ) 
	{
		value = VMS_PTZ_FAIL;
		client.clear_variable();
	}
	else value = VMS_PTZ_SUCCESS;

	if( response_data )	free( response_data );

	return value;
}

unsigned short hikvision_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::remove_preset2( int alias )
{
	unsigned short value = VMS_PTZ_FAIL;
	char url_data[50] = {0};
	snprintf( url_data, sizeof(url_data), "/PSIA/PTZ/channels/1/presets/%d", alias );
	http_client client( _hostname, _port_number, url_data );
	if( !client.send_request(_user_id, _user_password, "DELETE") ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	return value;
}

unsigned short hikvision_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::goto_preset2( int alias )
{
	unsigned short value = VMS_PTZ_FAIL;

	char url_data[50] = {0};
	snprintf( url_data, sizeof(url_data), "/PSIA/PTZ/channels/1/presets/%d/goto", alias );
	http_client client( _hostname, _port_number, url_data );
	if( !client.send_request(_user_id, _user_password, "PUT") ) 
	{
		value = VMS_PTZ_FAIL;
		client.clear_variable();
	}
	else value = VMS_PTZ_SUCCESS;

	return value;
}

unsigned short hikvision_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::remove_preset_tour2( int tour_name )
{
	unsigned short value = VMS_PTZ_FAIL;
	char url_data[50] = {0};
	char post_data[500] = {0};
	char *response_data = 0;

    snprintf( url_data, sizeof(url_data), "/PSIA/PTZ/channels/1/patrols/%d", tour_name );
	http_client client( _hostname, _port_number, url_data );
	if( !client.send_request(_user_id, _user_password, "DELETE") ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	snprintf( post_data, sizeof(post_data), "<xml version=\"1.0\" encoding=\"UTF-8\"><PTZPatrol><id>%d</id><patrolName>%d</patrolName><PatrolSequenceList></PatrolSequenceList></PTZPatrol>", tour_name, tour_name );

	if( !client.send_request_with_data(_user_id, _user_password, "PUT", post_data, &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	return value;
}

unsigned short hikvision_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	char url_data[50] = {0};

	switch( UINT8(cmd) )
	{
		case PTZ_TOUR_CMD_START  :		
			snprintf( url_data, sizeof(url_data), "/PSIA/PTZ/channels/1/patrols/%d/start", tour_name );
			break;

		case PTZ_TOUR_CMD_STOP :
			snprintf( url_data, sizeof(url_data), "/PSIA/PTZ/channels/1/patrols/%d/stop", tour_name );
			break;

		case PTZ_TOUR_CMD_PAUSE : 
			break;

		default :
			break;
	}
	
	http_client client( _hostname, _port_number, url_data );
	if( !client.send_request(_user_id, _user_password, "PUT") ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;
	
	return value;
}
unsigned short hikvision_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	if(VMS_PTZ_FAIL==get_prest_list()) return VMS_PTZ_FAIL;	

	unsigned short value = VMS_PTZ_FAIL;
	char url_data[50] = {0};
	snprintf( url_data, sizeof(url_data), "/PSIA/PTZ/channels/1/patrols/%s", tour->tour_name );
	http_client client( _hostname, _port_number, url_data );

	char post_data[5000] = {0,};
	char *response_data = 0;
	char tmp_str[100]= {0,};		
	std::string xml_str = "";
	std::map<std::string,int>::iterator preset_map_it;

	xml_str.append("<PTZPatrol>");
	sprintf(tmp_str, sizeof(tmp_str), "<id>%s</id><patrolName>%s</patrolName>", tour->tour_name, tour->tour_name);	
	xml_str.append(tmp_str);
	xml_str.append("<PatrolSequenceList>");

	for(int i=0; i<tour->size_of_tour_spots;i++)
	{
		PTZ_TOUR_SPOT_T* tsp = &(tour->tour_spots[i]);		
	    xml_str.append("<PatrolSequence>");
		snprintf( tmp_str, sizeof(tmp_str), "<presetID>%s</presetID>", tsp->preset_alias);
		xml_str.append(tmp_str);
		snprintf( tmp_str, sizeof(tmp_str), "<delay>%d</delay>", tsp->stay_time/1000);
		xml_str.append(tmp_str);
		snprintf( tmp_str, sizeof(tmp_str), "<Extensions><selfExt><seqSpeed>%d</seqSpeed></selfExt></Extensions>", int(get_speed_sensitive_value(tsp->speed)));
		xml_str.append(tmp_str);
		xml_str.append("</PatrolSequence>");
	}
	xml_str.append("</PatrolSequenceList>");
	xml_str.append("</PTZPatrol>");
	strncpy(post_data, xml_str.c_str(), sizeof(post_data));

	if( !client.send_request_with_data(_user_id, _user_password, "PUT", post_data, &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	return value;
}

unsigned short hikvision_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	unsigned short value = VMS_PTZ_FAIL;

    char url_data[50] = {0};
	snprintf( url_data, sizeof(url_data), "/PSIA/PTZ/channels/1/patrols/%s", tour->tour_name );
	http_client client( _hostname, _port_number, url_data );

	char response_data[6000] = {0,};

	if( !client.send_request(_user_id, _user_password, "GET", response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	if( response_data )
	{
		TiXmlDocument	doc;
		TiXmlElement	*elem		 = 0;
		TiXmlElement	*elem_spot_list = 0;
		TiXmlElement	*elem_preset = 0;
		TiXmlElement	*elem_speed	 = 0;
		TiXmlElement	*elem_dwt	 = 0;
		TiXmlNode		*node		 = 0;

		doc.Parse( (const char*)response_data, 0 );
		node = doc.FirstChild("PTZPatrol");	
		elem_spot_list = node->FirstChildElement("PatrolSequenceList");
		int tour_spot_cnt = 0;

		for(elem = elem_spot_list->FirstChildElement("PatrolSequence");elem != NULL; elem = elem->NextSiblingElement() )
		{
			const char *preset_text	= 0;
			elem_preset = elem->FirstChildElement("presetID");
			if( elem_preset )
			{
				preset_text	= elem_preset->GetText();
				if(strcmp(preset_text,"0")==0) continue;
				tour_spot_cnt++;
			}
		}

		tour->tour_recurring_time = 0;	
		tour->tour_recurring_duration = 0;	
		tour->tour_direction = PTZ_TOUR_DIRECTION_FORWARD;
		tour->tour_always_start=false;
		tour->size_of_tour_spots = tour_spot_cnt;
		tour->tour_spots = new PTZ_TOUR_SPOT_T[tour_spot_cnt];
		int idx = 0;
		PTZ_TOUR_SPOT_T *tsp = NULL;
		
		for(elem = elem_spot_list->FirstChildElement("PatrolSequence");elem != NULL; elem = elem->NextSiblingElement() )
		{
			const char *preset_key	= 0;
			const char *preset_text	= 0;
			const char *speed_key	= 0;
			const char *speed_text	= 0;
			const char *dwt_key	    = 0;
			const char *dwt_text	= 0;

			tsp = &(tour->tour_spots[idx]);

			elem_preset = elem->FirstChildElement("presetID");
			if( elem_preset )
			{
				preset_key	= elem_preset->Value();
				preset_text	= elem_preset->GetText();
				if(strcmp(preset_text,"0")==0) continue;
				idx++;
				tsp->preset_numberic_alias = std::stoi(preset_text);
			}

			elem_dwt = elem->FirstChildElement("delay");
			if( elem_dwt )
			{
				dwt_key		= elem_dwt->Value();
				dwt_text	= elem_dwt->GetText();
				tsp->stay_time = std::stof(dwt_text,0)*1000;	
			}		

			elem_speed = elem->FirstChildElement("Extensions")->FirstChildElement("selfExt")->FirstChildElement("seqSpeed");
			if( elem_speed )
			{
				speed_key	= elem_speed->Value();
				speed_text	= elem_speed->GetText();
				tsp->speed  = get_speed_quasi_sensitive_value(std::stof(speed_text,0));
			}
		}
	}
	
	return value;
}

unsigned short hikvision_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
   for(int i=1;i<=8;i++)
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

unsigned short hikvision_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _hostname, _port_number, "/PSIA/PTZ/channels/1/continuous" );

	float real_pan_sensitive = get_continuous_sensitive_value(pan_sensitive);
	float real_tilt_sensitive = get_continuous_sensitive_value(tilt_sensitive);
	float real_zoom_sensitive = get_continuous_sensitive_value(zoom_sensitive);

    char xml_data[100] = {0};   
	char *response_data = 0;
	
	if(pan_sensitive == 0 && tilt_sensitive == 0 && zoom_sensitive == 0)
	{
		stop_move();
	}
	else if(pan_sensitive != 0 || tilt_sensitive != 0 )
	{
		snprintf( xml_data, sizeof(xml_data), "<PTZData><pan>%.0f</pan><tilt>%.0f</tilt></PTZData>", real_pan_sensitive, real_tilt_sensitive);
		if( !client.send_request_with_data(_user_id, _user_password, "PUT", xml_data, &response_data) ) return VMS_PTZ_FAIL;
		else value = VMS_PTZ_SUCCESS;
		client.clear_variable();
	}
	else if(zoom_sensitive != 0)
	{
		snprintf( xml_data, sizeof(xml_data), "<PTZData><zoom>%.0f</zoom></PTZData>", real_zoom_sensitive);
		if( !client.send_request_with_data(_user_id, _user_password, "PUT", xml_data, &response_data) ) value = VMS_PTZ_FAIL;
		else value = VMS_PTZ_SUCCESS;
		client.clear_variable();
	}
	return value;
}

unsigned short hikvision_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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

unsigned short hikvision_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hikvision_ptz_controller::stop_move( void )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _hostname, _port_number, "/PSIA/PTZ/channels/1/continuous" );

    char xml_data[100] = {0};   
	char *response_data = 0;
	
	snprintf( xml_data, sizeof(xml_data), "<PTZData><pan>0</pan><tilt>0</tilt></PTZData>");
	if( !client.send_request_with_data(_user_id, _user_password, "PUT", xml_data, &response_data) ) return VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;
	client.clear_variable();

	snprintf( xml_data, sizeof(xml_data), "<PTZData><zoom>0</zoom></PTZData>");
	if( !client.send_request_with_data(_user_id, _user_password, "PUT", xml_data, &response_data) ) value = VMS_PTZ_FAIL;
	else value = VMS_PTZ_SUCCESS;

	client.clear_variable();

	return value;
}

unsigned short hikvision_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hikvision_ptz_controller::query_limits( void )
{
	_min_pan	= 0;
	_max_pan	= 0;
	_min_tilt	= 0;
	_max_tilt	= 0;
	_min_zoom	= 0;
	_max_zoom	= 0;
	_min_rpan	= 0;
	_max_rpan	= 0;
	_min_rtilt	= 0;
	_max_rtilt	= 0;
	_min_speed  = 0;
	_max_speed  = 40;
	_min_cspeed	= 0;
	_max_cspeed	= 100;
	return VMS_PTZ_SUCCESS;
}

unsigned short hikvision_ptz_controller::query_position( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}


float hikvision_ptz_controller::get_continuous_sensitive_value( float sensitive )
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

float hikvision_ptz_controller::get_rpan_sensitive_value( float sensitive )
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
		if( _pan_min==_min_rpan && _pan_max==_max_rpan )
			return sensitive;
		else
		{
			float real_sensitive = float(sensitive*(_max_rpan-_min_rpan))/float(_pan_max-_pan_min);
			if(real_sensitive>0) real_sensitive = ceil(real_sensitive);
	        else real_sensitive = floor(real_sensitive);	
			return real_sensitive;
		}
	}
}

float hikvision_ptz_controller::get_rtilt_sensitive_value( float sensitive )
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
		if( _tilt_min==_min_rtilt && _tilt_max==_max_rtilt )
			return sensitive;
		else
		{
			float real_sensitive = (sensitive*(_max_rtilt-_min_rtilt))/(_tilt_max-_tilt_min);
			if(real_sensitive>0) real_sensitive = ceil(real_sensitive);
	        else real_sensitive = floor(real_sensitive);	
			return real_sensitive;
		}
	}
}

float hikvision_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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

float hikvision_ptz_controller::get_apan_sensitive_value( float sensitive )
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

float hikvision_ptz_controller::get_atilt_sensitive_value( float sensitive )
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
		float real_sensitive = (sensitive-_tilt_min)*(_max_tilt-_min_tilt)/(_tilt_max-_tilt_min)+_min_tilt;
		return real_sensitive;
	}
}

float hikvision_ptz_controller::get_azoom_sensitive_value( float sensitive )
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

float hikvision_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
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

float hikvision_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
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

float hikvision_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
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

float hikvision_ptz_controller::get_speed_sensitive_value( float sensitive )
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
		//float real_sensitive = float(sensitive*(_max_speed-_min_speed))/float(_speed_max-_speed_min);
		float real_sensitive = (sensitive-_speed_min)*(_max_speed-_min_speed)/(_speed_max-_speed_min)+_min_speed;
		//if(real_sensitive>0) real_sensitive = ceil(real_sensitive);
	    //else real_sensitive = floor(real_sensitive);	
		return real_sensitive;
	}
}

float hikvision_ptz_controller::get_speed_quasi_sensitive_value( float real_sensitive )
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
		//float sensitive = ((real_sensitive-_min_speed)*(_speed_max-_speed_min))/(_max_speed-_min_speed)+_speed_min;
		float sensitive = ((real_sensitive-_min_speed)*(_speed_max-_speed_min)-_min_speed)/(_max_speed-_min_speed);
		if( sensitive>_speed_max )
			sensitive = _speed_max;
		if( sensitive<_speed_min )
			sensitive = _speed_min;
		return sensitive;
	}
}

base_ptz_controller* create( void )
{
	return new hikvision_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	hikvision_ptz_controller *samsung_controller = dynamic_cast<hikvision_ptz_controller*>( (*ptz_controller) );
	delete samsung_controller;
	(*ptz_controller) = 0;
}
