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
	return VMS_PTZ_DEVICE_INFO[AXIS_P215PTZE_VAPIX_V3_4_49][VENDOR];
}

char* axis_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[AXIS_P215PTZE_VAPIX_V3_4_49][DEVICE];
}

char* axis_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[AXIS_P215PTZE_VAPIX_V3_4_49][PROTOCOL];
}

char* axis_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[AXIS_P215PTZE_VAPIX_V3_4_49][VERSION];
}

unsigned short axis_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[AXIS_P215PTZE_VAPIX_V3_4_49][VENDOR];
}

unsigned short axis_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[AXIS_P215PTZE_VAPIX_V3_4_49][DEVICE];
}

unsigned short axis_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[AXIS_P215PTZE_VAPIX_V3_4_49][PROTOCOL];
}

unsigned short axis_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[AXIS_P215PTZE_VAPIX_V3_4_49][VERSION];
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
	return VMS_PTZ_TRUE;
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
	return VMS_PTZ_FALSE;
}

unsigned short axis_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_FALSE;
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
	get_preset_tour_list();
		
	if(_tour_list.size()>0)
	{
		std::map<std::string, std::string>::iterator it;
		it = _tour_list.find(tour_name);
		if(it != _tour_list.end()) return VMS_PTZ_FAIL;
	}
		
	///axis-cgi/param.cgi?action=add&group=GuardTour&template=guardtour&GuardTour.G.Name=tour8  
	http_client client( _host, _port_number, "/axis-cgi/param.cgi" );
	client.put_variable( "action", "add" );
	client.put_variable( "group", "GuardTour" );
	client.put_variable( "template", "guardtour" );	
	client.put_variable( "GuardTour.G.Name", tour_name );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

std::string axis_ptz_controller::get_preset_tour_id( char *tour_name)
{
	std::map<std::string,std::string>::iterator it;		
	std::string str = std::string(tour_name);	   
	it = _tour_list.find(str);
    if(it!=_tour_list.end()) {		
		return it->second;
	}
	return "";
}

unsigned short axis_ptz_controller::remove_preset_tour( char *tour_name )
{
	///axis-cgi/param.cgi?action=remove&group=GuardTour.G1
	http_client client( _host, _port_number, "/axis-cgi/param.cgi" );
	client.put_variable( "action", "remove" );	
	char str_group[15] = {0,};
	sprintf(str_group,sizeof(str_group),"GuardTour.G%s",get_preset_tour_id(tour_name).c_str());
	client.put_variable( "group", str_group);

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}

	client.clear_variable();
	
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::remove_preset_tour_presets( const char *tour_no )
{
	///axis-cgi/param.cgi?action=remove&group=GuardTour.G1.Tour
	http_client client( _host, _port_number, "/axis-cgi/param.cgi" );
	client.put_variable( "action", "remove" );	
	char str_group[20] = {0,};
	sprintf(str_group,sizeof(str_group),"GuardTour.G%s.Tour", tour_no);
	client.put_variable( "group", str_group);

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}

	client.clear_variable();
	
	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{	
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	///axis-cgi/param.cgi?action=update&GuardTour.G0.Running=no
	http_client client( _host, _port_number, "/axis-cgi/param.cgi" );
	char str_running[25]= {0,};
	snprintf( str_running, sizeof(str_running), "GuardTour.G%s.Running", get_preset_tour_id(tour_name).c_str());
	client.put_variable( "action", "update");	

	switch( UINT8(cmd) )
	{
		case PTZ_TOUR_CMD_START  :						
			client.put_variable( str_running, "yes");				
			break;

		case PTZ_TOUR_CMD_STOP :
			client.put_variable( str_running, "no");				
			break;

		case PTZ_TOUR_CMD_PAUSE : 
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;

		default :
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
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

unsigned short axis_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{	
  ///axis-cgi/param.cgi?action=add&group=GuardTour.G0.Tour&template=tour&GuardTour.G0.Tour.T.PresetNbr=1&GuardTour.G0.Tour.T.Position=2&GuardTour.G0.Tour.T.MoveSpeed=90&GuardTour.G0.Tour.T.WaitTime=10&GuardTour.G0.Tour.T.WaitTimeViewType=Seconds 
	std::map<std::string,std::string> preset_map;
	get_preset_list_map(&preset_map);	

	std::string str_tour_no = get_preset_tour_id(tour->tour_name);
	if(str_tour_no=="") return VMS_PTZ_FAIL;

	if(VMS_PTZ_FAIL==remove_preset_tour_presets(str_tour_no.c_str())) return VMS_PTZ_FAIL;

	for(int i=0; i<tour->size_of_tour_spots;i++)
	{
		PTZ_TOUR_SPOT_T* tsp = &(tour->tour_spots[i]);		
		std::string preset_id = get_id_by_name(&preset_map, tsp->preset_alias);
		if(preset_id=="") return VMS_PTZ_FAIL;
		
		http_client client( _host, _port_number, "/axis-cgi/param.cgi" );			
		char str_group[100]= {0,};						
		char str_key[100]= {0,};
		char str_value[10] = {0,};			
		client.put_variable( "action", "add");				
		snprintf( str_group, sizeof(str_group), "GuardTour.G%s.Tour", str_tour_no.c_str());
		client.put_variable( "group", str_group);	
		client.put_variable( "template", "tour");			
		snprintf( str_key, sizeof(str_key), "GuardTour.G%s.Tour.T.PresetNbr", str_tour_no.c_str());
		client.put_variable( str_key, preset_id.c_str());	
		snprintf( str_key, sizeof(str_key), "GuardTour.G%s.Tour.T.Position", str_tour_no.c_str());
		sprintf(str_value, sizeof(str_value), "%d",i+1);		
		client.put_variable( str_key, str_value);
		snprintf( str_key, sizeof(str_key), "GuardTour.G%s.Tour.T.MoveSpeed", str_tour_no.c_str());			
		sprintf(str_value, sizeof(str_value), "%d", int(get_speed_sensitive_value(tsp->speed)));		
		client.put_variable( str_key, str_value);	
		snprintf( str_key, sizeof(str_key), "GuardTour.G%s.Tour.T.WaitTime", str_tour_no.c_str());
		sprintf(str_value, sizeof(str_value), "%d",tsp->stay_time/1000);		
		client.put_variable( str_key, str_value);
		snprintf( str_key, sizeof(str_key), "GuardTour.G%s.Tour.T.WaitTimeViewType", str_tour_no.c_str());
		client.put_variable( str_key, "Seconds");									
		
		if( !client.send_request(_user_id, _user_password) ) 
		{
			client.clear_variable();
			return VMS_PTZ_FAIL;
		}		
		client.clear_variable();	
	}		

	return VMS_PTZ_SUCCESS;
}

unsigned short axis_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	std::string str_tour_no = get_preset_tour_id(tour->tour_name);
			
    //if(VMS_PTZ_FAIL==get_preset_tour_list()) return VMS_PTZ_FAIL;			

	char recv_tours[5000] = {0,};		
	http_client client( _host, _port_number, "/axis-cgi/param.cgi" );
	client.put_variable( "action", "list" );	
	char str_group[15] = {0,};
	sprintf(str_group,sizeof(str_group),"GuardTour.G%s",str_tour_no.c_str());
	client.put_variable( "group", str_group);

	if( !client.send_request(_user_id, _user_password, recv_tours) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}

	client.clear_variable();

	if(strlen(recv_tours)==0) return VMS_PTZ_FAIL;
	
	int tour_spot_cnt = 0;

	std::vector<std::string> vec;
	std::vector<std::string>::iterator v_it;
	std::map<std::string,std::string> tour_map;
	std::map<std::string,std::string>::iterator tour_map_it;	   	

	split2vector( recv_tours, "\n", &vec);	

	for( v_it=vec.begin(); v_it!=vec.end(); v_it++ )
	{
		std::string pair = *v_it;
		char *tkn0,*tkn1,*tkn2,*tkn3;
		char* tgt = strdup(pair.c_str());

		tkn0 = strtok(tgt,"=");//root.GuardTour.G1.Name
		tkn0 = strtok(tgt,".");//root
		tkn1 = strtok(NULL,".");//GuardTour
		tkn2 = strtok(NULL,".");//G0
		tkn3 = strtok(NULL,".");//Name
		
		split2map2( pair, "=", &tour_map );
		if(strcmp(tkn3,"Tour")==0)
		{ 
			char *tkn4 = strtok(NULL,".");//T0
			tkn4 = strtok(NULL,".");//PresetNbr
			if(strcmp(tkn4,"PresetNbr")==0) tour_spot_cnt++;			
		}
		free(tgt);
	}

	
	tour->tour_recurring_time = 0;	
	tour->tour_recurring_duration = 0;	
	tour->tour_direction = PTZ_TOUR_DIRECTION_FORWARD;
	tour->tour_always_start=false;
	tour->size_of_tour_spots = tour_spot_cnt;
    tour->tour_spots = new PTZ_TOUR_SPOT_T[tour_spot_cnt];		

    int idx = 0;
	PTZ_TOUR_SPOT_T *tsp = NULL;
	std::string sp_chk_str = "";
	std::map<std::string, std::string> preset_map;

	get_preset_list_map(&preset_map);
		
	for(tour_map_it=tour_map.begin(); tour_map_it!=tour_map.end(); ++tour_map_it)   
	{	
		char* key = strdup(tour_map_it->first.c_str());
		
		char *tk = strtok(key,".");//root
		tk = strtok(NULL,".");//GuardTour
		tk = strtok(NULL,".");//G0
		tk = strtok(NULL,".");//tour

		if(strcmp(tk,"Tour")!=0) continue;
		
		char *tk1 = strtok(NULL,".");//T0
		char *tk2 = strtok(NULL,".");//PresetNbr,Position,MoveSpeed,WaitTime,WaitTimeViewType
		
		if((strstr(sp_chk_str.c_str(),tk1)==NULL)) {
			sp_chk_str.append(tk1);
			sp_chk_str.append(",");
			tsp = &(tour->tour_spots[idx++]);
		}
		std::string r_value = tour_map_it->second;

		if(strcmp(tk2,"PresetNbr")==0)
			::strncpy( tsp->preset_alias, get_name_by_id(&preset_map, r_value).c_str(), sizeof(tsp->preset_alias) );
		if(strcmp(tk2,"MoveSpeed")==0)  tsp->speed = get_speed_quasi_sensitive_value(std::stof(r_value,0));

		if(strcmp(tk2,"WaitTime")==0)  {
			char wait_time_type[50] ={0};//root.GuardTour.G0.Tour.T0.WaitTimeViewType=Seconds
			sprintf(wait_time_type,sizeof(wait_time_type),"root.GuardTour.G%s.Tour.%s.WaitTimeViewType",str_tour_no.c_str(),tk1);
			std::string wait_time_type_value = tour_map.find(wait_time_type)->second;
			if(wait_time_type_value=="Seconds"){
				tsp->stay_time = std::stol(r_value,0,10)*1000;	
			}else if(wait_time_type_value=="Minutes"){
				tsp->stay_time = std::stol(r_value,0,10)*60*1000;	
			}
		}
	
		free(key);
	}


	return VMS_PTZ_SUCCESS;		
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

std::string axis_ptz_controller::get_name_by_id(std::map<std::string,std::string> *preset_map, std::string id)
{	
	std::map<std::string,std::string>::iterator it;
	for(it=preset_map->begin();it!=preset_map->end();it++)
	{	
		std::string tmp_str = it->first.substr(11);
		if(strcmp(tmp_str.c_str(), id.c_str())==0) return it->second;
	}
	return "";
	
}

std::string axis_ptz_controller::get_id_by_name(std::map<std::string,std::string> *preset_map, std::string name)
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


unsigned short axis_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{	
	if(VMS_PTZ_FAIL==get_preset_tour_list()) return VMS_PTZ_FAIL;

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

unsigned short axis_ptz_controller::get_preset_tour_list(void)
{
	char recv_tours[5000] = {0,};		
	http_client client( _host, _port_number, "/axis-cgi/param.cgi" );
	client.put_variable( "action", "list" );
	client.put_variable( "group", "GuardTour.*.Name" );

	//결과값:root.GuardTour.G0.Name=Guard 1

	if( !client.send_request(_user_id, _user_password, recv_tours) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	
	client.clear_variable();
	_tour_list.clear();

	if(strlen(recv_tours)==0) return VMS_PTZ_FAIL;

	std::vector<std::string> vec;
	std::vector<std::string>::iterator it;		
   	
	split2vector( recv_tours, "\n", &vec);	
	
	for( it=vec.begin(); it!=vec.end(); it++ )
	{
		std::string pair = *it;		
		int cut_at_name = pair.find_first_of("=");
		if(cut_at_name==-1) return VMS_PTZ_FAIL;
		int cut_at_id = pair.find_last_of(".");
		if(cut_at_id==-1) return VMS_PTZ_FAIL;
		std::string name = (*it).substr(cut_at_name+1);	
		std::string id = pair.substr(0, cut_at_id);
		std::string no = id.substr(id.find_last_of(".")+2);
		_tour_list[name] = no;
	}
	return VMS_PTZ_SUCCESS;
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

	//if( real_tilt_sensitive<0 )	//카메라가 tilt 방향으로 90도 이상 뒤로 넘어가는 것을 방지함.
	//{
	//	float pan = 0.0;
	//	float tilt = 0.0;
	//	float zoom = 0.0;
	//	query_position( pan, tilt, zoom );
	//	//if( tilt+real_tilt_sensitive<(_max_tilt-((_max_tilt-_min_tilt)/2)) )
	//	//	real_tilt_sensitive = 0;
	//	float limit_tilt = (_max_tilt-((_max_tilt-_min_tilt)/2));
	//	if( tilt+real_tilt_sensitive< limit_tilt)	
	//		if(tilt>limit_tilt && tilt+real_tilt_sensitive<limit_tilt) 
	//		{
	//				real_tilt_sensitive = limit_tilt - tilt;
	//		}
	//		else 
	//			{
	//				real_tilt_sensitive = 0;
	//		}
	//}

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

unsigned short axis_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed_sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	// 중간 return 으로 인해 아래 if문(zoom수행) 위로 수정
	char pan[100]	= {0,};
	char tilt[100]	= {0,};
	char zoom[100]	= {0,};
	char speed[100] = {0,};

	float real_pan_sensitive	= get_apan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_atilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_azoom_sensitive_value( zoom_sensitive );
	float real_speed_sensitive	= get_speed_sensitive_value( speed_sensitive );

	http_client client( _host, _port_number, "/axis-cgi/com/ptz.cgi" );
		
	snprintf( pan, sizeof(pan), "%.2f", real_pan_sensitive );
	client.put_variable( "pan", pan );
	snprintf( tilt, sizeof(tilt), "%.2f", real_tilt_sensitive );
	client.put_variable( "tilt", tilt );
	snprintf( zoom, sizeof(zoom), "%.2f", real_zoom_sensitive );
	client.put_variable( "zoom", zoom );

	if( speed_sensitive>_min_speed )
	{
		snprintf( speed, sizeof(speed), "%.2f", real_speed_sensitive );
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
	pan		= get_apan_quasi_sensitive_value( tmp_pan );
	tilt	= get_atilt_quasi_sensitive_value( tmp_tilt );
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
	
	_min_pan			= -60;
	_max_pan			= 60;
	_min_tilt			= -50;
	_max_tilt			= 50;//13;
	_min_zoom           = -5000;
	_max_zoom           = 5000;
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

float axis_ptz_controller::get_speed_quasi_sensitive_value( float real_sensitive )
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
