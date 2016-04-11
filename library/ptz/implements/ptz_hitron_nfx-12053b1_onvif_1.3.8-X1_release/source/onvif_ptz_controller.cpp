#include "platform.h"
#include <ptz_device_info.h>
#include "onvif_ptz_controller.h"
#include "http_client.h"

onvif_ptz_controller::onvif_ptz_controller( void )
	: _is_inverse(false)
	, _port_number(80)
	, _is_limits_queried(false)
{
	strcpy( _profile_token, "Profile1" );
}


onvif_ptz_controller::~onvif_ptz_controller( void )
{

}

char* onvif_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HITRON_NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE][VENDOR];
}

char* onvif_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HITRON_NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE][DEVICE];
}

char* onvif_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HITRON_NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE][PROTOCOL];
}

char* onvif_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HITRON_NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE][VERSION];
}

unsigned short onvif_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[HITRON_NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE][VENDOR];
}

unsigned short onvif_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[HITRON_NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE][DEVICE];
}

unsigned short onvif_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[HITRON_NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE][PROTOCOL];
}

unsigned short onvif_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[HITRON_NFX_12053B1_ONVIF_V_1_3_8_X1_RELEASE][VERSION];
}

unsigned short onvif_ptz_controller::set_host_name( char *host )
{
	if( host && (strlen(host)>0) ) 
	{
		strcpy( _host, host );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::set_angle_inverse( bool inverse )
{
	_is_inverse = inverse;
	return VMS_PTZ_SUCCESS;
}


unsigned short onvif_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_number_place = number_place;
	_pan_min = min;
	_pan_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_number_place = number_place;
	_tilt_min = min;
	_tilt_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_number_place = number_place;
	_zoom_min = min;
	_zoom_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_number_place = number_place;
	_speed_min = min;
	_speed_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::set_profile_token( char *token )
{
	if( token && strlen(token)>0 )
	{
		memset( _profile_token, 0x00, MAX_PATH );
		strcpy( _profile_token, token );
		return VMS_PTZ_SUCCESS;
	}
	return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short onvif_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short onvif_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short onvif_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short onvif_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short onvif_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short onvif_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short onvif_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short onvif_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short onvif_ptz_controller::goto_home_position( float speed )
{
	float real_speed			= get_speed_sensitive_value( speed );

	PTZBindingProxy proxy;
	_tptz__GotoHomePosition request;
	_tptz__GotoHomePositionResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	if( speed>_speed_min )
	{
		request.Speed						= soap_new_tt__PTZSpeed( &proxy, -1 );
		request.Speed->PanTilt				= soap_new_tt__Vector2D( &proxy, -1 );
		request.Speed->PanTilt->x			= real_speed;
		request.Speed->PanTilt->y			= real_speed;
		request.Speed->Zoom					= soap_new_tt__Vector1D( &proxy, -1 );
		request.Speed->Zoom->x				= real_speed;
	}

	int result = fnGotoHomePosition( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::set_home_position( void )
{
	/*PTZBindingProxy proxy;
	_tptz__SetHomePosition request;
	_tptz__SetHomePositionResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnSetHomePosition( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;*/
	
	//onvif방식으로 홈포지션이 지정이 되긴하나 정상작동이 안되 http방식사용
	///ptz/preset.php?ch=1&app=set&method=create&preset_number=0
	http_client client( _host, _port_number, "/ptz/preset.php" );
	//client.put_variable( "ch", "1" );
	client.put_variable( "app", "set" );
	client.put_variable( "method", "create" );
	client.put_variable( "preset_number", "0" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short	onvif_ptz_controller::get_preset_list( char ***aliases, int *length )
{	
	(*length)	= 0;
	(*aliases)	= 0;

	PTZBindingProxy proxy;
	_tptz__GetPresets request;
	_tptz__GetPresetsResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetPresets( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
	{
		(*length) = response.__sizePreset+1;
		if( (*length)>0 )
		{
			(*aliases) = static_cast<char**>( malloc(sizeof(char**)*(*length)) );
			(*aliases)[0] = strdup("Home");
			for( int index=1; index<(*length); index++ )
				(*aliases)[index] = strdup( response.Preset[index-1]->Name );
		}
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short	onvif_ptz_controller::add_preset( char *alias )
{
	unsigned short value = VMS_PTZ_FAIL;

	//1-1. 사용할 id 찾기
	value = get_preset_list_map();
	if( value!=VMS_PTZ_SUCCESS )
		return value;
	bool idx[240] ={0,}; 

	int real_index = -1;

	std::map<std::string, std::string>::iterator iter;
	iter = _preset_map.find(alias); 
    if( iter!=_preset_map.end() ) 
	{
		real_index = std::stoi(((iter)->second));
		//1-2. 찾은건 삭제(덮어쓰기가 안되므로 지우고 다시씀)
		value = remove_preset( alias );
		if( value!=VMS_PTZ_SUCCESS )
			return value;
	}
	else
	{
		for( iter=_preset_map.begin(); iter!=_preset_map.end(); iter++ )
		{
			idx[std::stoi((iter)->second)] = true;
		}

		for(int i=1;i<=sizeof(idx);i++)//1~240 사용함
		{
			if(idx[i]== false) 
			{
					real_index = i;
				    break;
			}
		}
		if(real_index==-1) real_index = 1;
	}

    char str_index[4] = {0};
	snprintf( str_index, sizeof(str_index), "%d", real_index);

	//2. 저장
	http_client client( _host, _port_number, "/ptz/preset.php" );
	//client.put_variable( "ch", "1" );
	client.put_variable( "app", "set" );
	client.put_variable( "method", "create" );
	client.put_variable( "preset_number", str_index );
	client.put_variable( "preset_title", alias );
	//client.put_variable( "preset_af_mode", "1" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short	onvif_ptz_controller::remove_preset( char *alias )
{
	PTZBindingProxy proxy;
	_tptz__GetPresets request;
	_tptz__GetPresetsResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetPresets( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
	{
		for( int index=0; index<response.__sizePreset; index++ )
		{
#if defined(WIN32)
			if( strncasecmp(response.Preset[index]->Name, alias)==0 )
#else
			if( strncasecmp(response.Preset[index]->Name, alias,strlen(alias))==0 )
#endif
			{
				_tptz__RemovePreset request1;
				_tptz__RemovePresetResponse response1;
				request1.ProfileToken	= soap_strdup( &proxy, _profile_token );
				request1.PresetToken	= soap_strdup( &proxy, response.Preset[index]->token );

				result = fnRemovePreset( this, &proxy, "http://%s:%d/onvif/device_service", &request1, &response1 );
				if( result==SOAP_OK )
					return VMS_PTZ_SUCCESS;
				else
					return VMS_PTZ_FAIL;
			}
		}
	}
	return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::goto_preset( char *alias )
{
#if defined(WIN32)
	if( (strncasecmp(alias, "Home")==0) ) 
		return goto_home_position(0);
#else
	if( (strncasecmp(alias, "Home", strlen("Home"))==0) ) 
		return goto_home_position(0);
#endif

	PTZBindingProxy proxy;
	_tptz__GetPresets request;
	_tptz__GetPresetsResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetPresets( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
	{
		for( int index=0; index<response.__sizePreset; index++ )
		{
#if defined(WIN32)
			if( strncasecmp(response.Preset[index]->Name, alias)==0 )
#else
			if( strncasecmp(response.Preset[index]->Name, alias,strlen(alias))==0 )
#endif
			{
				_tptz__GotoPreset request1;
				_tptz__GotoPresetResponse response1;
				request1.ProfileToken	= soap_strdup( &proxy, _profile_token );
				request1.PresetToken	= soap_strdup( &proxy, response.Preset[index]->token );

				result = fnGotoPreset( this, &proxy, "http://%s:%d/onvif/device_service", &request1, &response1 );
				if( result==SOAP_OK )
					return VMS_PTZ_SUCCESS;
				else
					return VMS_PTZ_FAIL;
			}
		}
	}
	return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short onvif_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short onvif_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short onvif_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

std::string onvif_ptz_controller::get_preset_name_by_id(char *preset_id)
{	
	 ///ptz/preset.php?app=get&method=get_num&preset_number=5	
	char recv_str[100] = {0,};	
	http_client client( _host, _port_number, "/ptz/preset.php" );
    //client.put_variable( "ch", "1" );	
	client.put_variable( "app", "get" );
	client.put_variable( "method", "get_num" );
	client.put_variable( "preset_number", preset_id);

	if( !client.send_request(_user_id, _user_password, recv_str) ) 
	{
		client.clear_variable();
		return "";
	}
	client.clear_variable();
	
	std::vector<std::string> vec;
	std::vector<std::string>::iterator it;	
	std::map<std::string, std::string> preset_map;
	std::map<std::string, std::string>::iterator preset_map_it;

	//결과값 :res=200&preset_number=0&preset_title=Home&preset_af_mode=1   	
	split2vector(recv_str , "&", &vec);		
		
	for( it=vec.begin(); it!=vec.end(); it++ )
	{		
		std::string pair = *it;
		split2map( pair, "=", &preset_map);				
	}

	preset_map_it = preset_map.find("preset_title");

	if(preset_map_it!=preset_map.end()) return preset_map_it->second;

	return "";
}


unsigned short onvif_ptz_controller::get_preset_list_map()
{
	_preset_map.clear();
    ///ptz/preset.php?app=get	
	char recv_str[1000] = {0,};
	http_client client( _host, _port_number, "/ptz/preset.php" );
    //client.put_variable( "ch", "1" );	
	client.put_variable( "app", "get" );
	//결과값 :res=200&http_port=80&https_port=443&preset_home=0&preset_af_mode_type=3&def_preset_af_mode=1&preset_list=0,1&def_preset_speed=16&min_preset_speed=1&max_preset_speed=32   

	if( !client.send_request(_user_id, _user_password, recv_str) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	std::string str_presets = std::string(recv_str);

	std::vector<std::string> tmp_vec;
	std::vector<std::string>::iterator v_it;	
	std::map<std::string,std::string> tmp_map;
	std::map<std::string, std::string>::iterator tmp_map_it;

	split2vector(str_presets , "&", &tmp_vec);		

	for( v_it=tmp_vec.begin(); v_it!=tmp_vec.end(); v_it++ )
	{
		std::string pair = *v_it;				
		split2map( pair, "=", &tmp_map );	
	}
	tmp_map_it = tmp_map.find("preset_list");
	if(tmp_map_it!=tmp_map.end()) str_presets = tmp_map_it->second;
	else return VMS_PTZ_FAIL;

	std::vector<std::string> vec;
	std::vector<std::string>::iterator it;	
	std::map<std::string, std::string>::iterator preset_map_it;

	split2vector(str_presets , ",", &vec);		
		
	for( it=vec.begin(); it!=vec.end(); it++ )
	{		
		std::string preset_id = *it;
		char *id = strdup(preset_id.c_str());
		std::string preset_name = get_preset_name_by_id(id);		
		_preset_map[preset_name] =  preset_id;	
	}

	return VMS_PTZ_SUCCESS;
}


unsigned short onvif_ptz_controller::add_preset_tour( char *tour_name, int size )
{			
	int tour_no = -1;

	std::string tour_str = get_preset_tour_list_str();
	if(tour_str!="") 
	{			
		std::vector<std::string> vec;
		std::vector<std::string>::iterator it;		
   	
		split2vector(tour_str.c_str() , ",", &vec);	

		bool found = false;

		for(int i=1;i<9;i++)
		{
			for( it=vec.begin(); it!=vec.end(); ++it )
			{
				int tour_list_value = stoi(*it);
				if(i == tour_list_value) 
				{		
					found = true;
					break;				
				}
			}
			if(found) 
			{
				found = false;			
			}else
			{
				tour_no = i;
				break;
			}
		}

		if(tour_no == -1) return VMS_PTZ_FAIL;
	}
	else
	{
		tour_no = 1;
	}
	
	http_client client( _host, _port_number, "/ptz/tour.php" );
	//client.put_variable( "ch", "1" );
	client.put_variable( "app", "set" );
	client.put_variable( "method", "create" );			
	char str_tour_no[2] ={0,};
	sprintf(str_tour_no,"%d",tour_no);
	client.put_variable( "tour_number", str_tour_no);	
	client.put_variable( "tour_title", tour_name );	
	client.put_variable( "tour_repeat_time", "0" );
	client.put_variable( "tour_sequence", "1" );//1:foward,0:backward
	client.put_variable( "tour_tourlist", "1|0|1|32|32|8");//구분(1:프리셋) | 프리셋NO | 거주시간 | 팬(1~32,디폴트32)속도 | 틸트(1~32,디폴트32)속도 | 줌(1~8,디폴트8)속도

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::remove_preset_tour( char *tour_name )
{
	http_client client( _host, _port_number, "/ptz/tour.php" );
    //client.put_variable( "ch", "1" );	
	client.put_variable( "app", "set" );
	client.put_variable( "method", "delete" );
	client.put_variable( "tour_number", get_preset_tour_id(tour_name).c_str());

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}

	client.clear_variable();
	
	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	http_client client( _host, _port_number, "/ptz/control.php" );		
	//client.put_variable( "ch", "1");		

	switch( UINT8(cmd) )
	{
		case PTZ_TOUR_CMD_START  :						
			client.put_variable( "tour", get_preset_tour_id(tour_name).c_str());	
			break;

		case PTZ_TOUR_CMD_STOP :
			client.put_variable( "move", "stop");				
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

unsigned short onvif_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short onvif_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short onvif_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short onvif_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	 ///ptz/tour.php?ch=1&app=set&method=create&tour_number=2&tour_title=T2&tour_repeat_time=0&tour_sequence=1&tour_tourlist=0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0 
	std::map<std::string,std::string>::iterator preset_map_it;
    if(VMS_PTZ_FAIL==get_preset_list_map()) return VMS_PTZ_FAIL;	

	std::string str_tour_no = get_preset_tour_id(tour->tour_name);
	if(str_tour_no=="") return VMS_PTZ_FAIL;

//if(VMS_PTZ_FAIL==remove_preset_tour_presets(str_tour_no.c_str())) return VMS_PTZ_FAIL;

	http_client client( _host, _port_number, "/ptz/tour.php" );			
	//client.put_variable( "ch", "1");	
	client.put_variable( "app", "set");	
	client.put_variable( "method", "create");	
	client.put_variable( "tour_number", str_tour_no.c_str());
	client.put_variable( "tour_title", tour->tour_name);	
	char str_tour_repeat_time[10]= {0,};
	snprintf( str_tour_repeat_time, sizeof(str_tour_repeat_time), "%d", tour->tour_recurring_time);
	client.put_variable( "tour_repeat_time", str_tour_repeat_time);
	if((tour->tour_direction) == PTZ_TOUR_DIRECTION_FORWARD) 
	{
		client.put_variable( "tour_sequence", "1");
	} else
	{
		client.put_variable( "tour_sequence", "0");
	}

	std::string str_tour_list = "";		

	for(int i=0; i<tour->size_of_tour_spots;i++)
	{
		PTZ_TOUR_SPOT_T* tsp = &(tour->tour_spots[i]);		

		std::string preset_id = "";	
   		preset_map_it = _preset_map.find(tsp->preset_alias);	
		if(preset_map_it!=_preset_map.end()) preset_id = preset_map_it->second;		
		if(preset_id=="") return VMS_PTZ_FAIL;		
				
		char str_value[10] = {0,};							

		//1|2|12|32|32|8 = 구분(1:프리셋) | 프리셋NO | 거주시간 | 팬(1~32,디폴트32)속도 | 틸트(1~32,디폴트32)속도 | 줌(1~8,디폴트8)속도
		str_tour_list.append("1|");		
		str_tour_list.append(preset_id+"|");
		snprintf( str_value, sizeof(str_value), "%d|", tsp->stay_time/1000);
		str_tour_list.append(str_value);//stay_time
		str_tour_list.append("32|");//pan speed	
		str_tour_list.append("32|");//tilt speed	
		str_tour_list.append("8");//zoom speed	
		//float pan_speed, tilt_speed, zoom_speed;
		//get_tour_speed(tsp->speed, pan_speed, tilt_speed, zoom_speed);
		//snprintf( str_value, sizeof(str_value), "%d|", int(pan_speed));
		//str_tour_list.append(str_value);//pan speed		
		//snprintf( str_value, sizeof(str_value), "%d|", int(tilt_speed));
		//str_tour_list.append(str_value);//tilt speed	
		//snprintf( str_value, sizeof(str_value), "%d", int(zoom_speed));
		//str_tour_list.append(str_value);//zoom speed				
		str_tour_list.append(",");
	}

	for(int j=tour->size_of_tour_spots; j<100; j++)
	{
		str_tour_list.append("0|0|0|0|0|0,");
	}

	str_tour_list = str_tour_list.substr(0,str_tour_list.length()-1);
	client.put_variable( "tour_tourlist", str_tour_list.c_str());

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}		
	client.clear_variable();		

	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	std::string str_tour_name = tour->tour_name;
	//std::string str_tour_no = str_tour_name.substr(str_tour_name.size()-1);	
	std::string str_tour_no = "";
	std::map<std::string,std::string>::iterator it;
	it = _tour_list.find(str_tour_name);
	if(it!=_tour_list.end()) str_tour_no = it->second;

	char recv_tours[5000] = {0,};		
	http_client client( _host, _port_number, "/ptz/tour.php" );	
	//client.put_variable( "ch", "1" );	
	client.put_variable( "app", "get" );	
	client.put_variable( "method", "get_num" );	
	client.put_variable( "tour_number", str_tour_no.c_str() );	

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

	split2vector( recv_tours, "&", &vec);		

	for( v_it=vec.begin(); v_it!=vec.end(); v_it++ )
	{
		std::string pair = *v_it;				
		split2map( pair, "=", &tour_map );	
	}
	std::string str_tour_list = "";
	tour_map_it = tour_map.find("tour_tourlist");
	if(tour_map_it!=tour_map.end()) str_tour_list = tour_map_it->second;

	std::vector<std::string> vec_tour_list;
	std::vector<std::string>::iterator vec_tour_list_it;
	split2vector( str_tour_list, ",", &vec_tour_list);
	
	for( vec_tour_list_it=vec_tour_list.begin(); vec_tour_list_it!=vec_tour_list.end(); vec_tour_list_it++ )
	{
		std::string str =  *vec_tour_list_it;			
		char *tmp_str = strdup(str.c_str());
		if(strcmp("0|0|0|0|0|0",tmp_str)==0) continue;
		tour_spot_cnt++;					
	}

	std::string str_tour_recurring_time = "0";
	tour_map_it = tour_map.find("tour_repeat_time");
	if(tour_map_it!=tour_map.end()) str_tour_recurring_time = tour_map_it->second;		
	tour->tour_recurring_time = std::stoi(str_tour_recurring_time,0,10);	
	tour->tour_recurring_duration = 0;	
	
	std::string str_direction = "1";
	tour_map_it = tour_map.find("tour_sequence");
	if(tour_map_it!=tour_map.end()) str_direction = tour_map_it->second;
	if(strcmp(str_direction.c_str(),"1")==0) 
	{
		tour->tour_direction = PTZ_TOUR_DIRECTION_FORWARD;
	} else
	{
		tour->tour_direction = PTZ_TOUR_DIRECTION_BACKWARD;
	}
		
	tour->tour_always_start=false;
	tour->size_of_tour_spots = tour_spot_cnt;
    tour->tour_spots = new PTZ_TOUR_SPOT_T[tour_spot_cnt];		

    int idx = 0;
	PTZ_TOUR_SPOT_T *tsp = NULL;

	for( vec_tour_list_it=vec_tour_list.begin(); vec_tour_list_it!=vec_tour_list.end(); vec_tour_list_it++ )
	{
		std::string str =  *vec_tour_list_it;			
		char *tmp_str = strdup(str.c_str());
		if(strcmp("0|0|0|0|0|0",tmp_str)==0) continue;
		tsp = &(tour->tour_spots[idx++]);
		char *dv,*spot_no,*dwell_time,*pan,*tilt,*zoom;
		dv = strtok(tmp_str,"|");
		spot_no = strtok(NULL,"|");
		dwell_time = strtok(NULL,"|");
		pan = strtok(NULL,"|");
		tilt = strtok(NULL,"|");
		zoom = strtok(NULL,"|");
		strncpy( tsp->preset_alias, strdup(get_preset_name_by_id(spot_no).c_str()), sizeof(tsp->preset_alias) ); 
		//tsp->pan = float(std::stoi(pan,0,10));
		//tsp->tilt = float(std::stoi(tilt,0,10));
		//tsp->zoom = float(std::stoi(zoom,0,10));
		tsp->pan = 0;
		tsp->tilt = 0;
		tsp->zoom = 0;
		//float t_speed;
		//get_tour_quasi_speed(std::stof(pan),std::stof(tilt),std::stof(zoom),t_speed);
		tsp->speed = _speed_max;//t_speed;
		
		tsp->stay_time = std::stoi(dwell_time,0,10)*1000;	
	}	

	return VMS_PTZ_SUCCESS;		
}


unsigned short onvif_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{		
	//투어추가를 할수없음으로 투어목록을 생성해논것을 이용함으로 아래 사용불가
	if(VMS_PTZ_FAIL==get_preset_tour_list()) return VMS_PTZ_FAIL;

	(*size_of_tours) = _tour_list.size();   
	*tour = new PTZ_TOUR_T[_tour_list.size()];   
	
    std::map<std::string,std::string>::iterator it;
	int idx = 0;
    for(it=_tour_list.begin(); it!=_tour_list.end(); ++it)   
	{		
		strncpy( (*tour+(idx++))->tour_name, it->first.c_str(), sizeof((*tour+(idx++))->tour_name) );
	}		

	return VMS_PTZ_SUCCESS;
}

unsigned short onvif_ptz_controller::get_preset_tour_list(void)
{	
	std::string tour_str = get_preset_tour_list_str();
	if(tour_str=="") return VMS_PTZ_FAIL;
	
	std::vector<std::string> vec;
	std::vector<std::string>::iterator it;		
   	
	split2vector(tour_str.c_str() , ",", &vec);		

	_tour_list.clear();		
		
	for( it=vec.begin(); it!=vec.end(); it++ )
	{
		std::string tour_no = *it;
		std::string name = get_preset_tour_name(tour_no);
		_tour_list[name] = tour_no;
	}
	return VMS_PTZ_SUCCESS;		

}

std::string onvif_ptz_controller::get_preset_tour_list_str(void)
{
	char recv_tours[5000] = {0,};		

	http_client client( _host, _port_number, "/ptz/tour.php" );
	//client.put_variable( "ch", "1" );
	client.put_variable( "app", "get" );
	client.put_variable( "method", "get_tour" );

	//결과값: res=200&tour_list=1,2
	if( !client.send_request(_user_id, _user_password, recv_tours) ) 
	{
		client.clear_variable();
		return "";
	}
	
	client.clear_variable();
	
	char *chk = &recv_tours[strlen(recv_tours)-1];	

	if(*chk == '=') return "";
	
	std::string recv_str = std::string(recv_tours);
	std::string tour_str = recv_str.substr(recv_str.find_last_of("=")+1);	

	return tour_str;		
}

std::string onvif_ptz_controller::get_preset_tour_name(std::string tour_no)
{
	char recv_tours[5000] = {0,};		
	http_client client( _host, _port_number, "/ptz/tour.php" );
	//client.put_variable( "ch", "1" );
	client.put_variable( "app", "get" );
	client.put_variable( "method", "get_num" );
	client.put_variable( "tour_number", tour_no.c_str());
	
	if( !client.send_request(_user_id, _user_password, recv_tours) ) 
	{
		client.clear_variable();
		return "";
	}

	std::vector<std::string> vec;
	std::vector<std::string>::iterator it;	
	std::map<std::string, std::string> tour_map;
	std::map<std::string, std::string>::iterator tour_map_it;
   	
	split2vector(recv_tours , "&", &vec);		
		
	for( it=vec.begin(); it!=vec.end(); it++ )
	{		
		std::string pair = *it;
		split2map( pair, "=", &tour_map);				
	}

	tour_map_it = tour_map.find("tour_title");

	if(tour_map_it!=tour_map.end()) return tour_map_it->second;

	return "";

}

unsigned short onvif_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	float real_pan_sensitive	= get_continuous_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_continuous_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_continuous_sensitive_value( zoom_sensitive );

	PTZBindingProxy proxy;
	_tptz__ContinuousMove request;
	_tptz__ContinuousMoveResponse response;

	request.ProfileToken				= soap_strdup( &proxy, _profile_token );
	request.Velocity					= soap_new_tt__PTZSpeed( &proxy, -1 );
	request.Velocity->PanTilt			= soap_new_tt__Vector2D( &proxy, -1 );
	request.Velocity->PanTilt->x		= real_pan_sensitive;
	request.Velocity->PanTilt->y		= real_tilt_sensitive;
	request.Velocity->PanTilt->space	= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace" );
	request.Velocity->Zoom				= soap_new_tt__Vector1D( &proxy, -1 );
	request.Velocity->Zoom->x			= real_zoom_sensitive;
	request.Velocity->Zoom->space		= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace" );

	if( timeout>0 && timeout<=60 )
	{
		char stimeout[50] = {0};
		snprintf( stimeout, sizeof(stimeout), "PT%dS", timeout );
		request.Timeout					= soap_strdup( &proxy, stimeout );
	}

	int result = fnContinuousMove( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
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


float onvif_ptz_controller::get_continuous_sensitive_value( float sensitive )
{
	query_limits();

	if( _speed_min==_min_cspeed && _speed_max==_max_cspeed )
		return sensitive;

	else if( abs(sensitive)>abs(_speed_max-_speed_min) ) 
	{
		if( sensitive<0 )
			return _min_cspeed;
		else
			return _max_cspeed;
	}
	else
	{
		float real_sensitive = float(sensitive*(_max_cspeed-_min_cspeed))/float(_speed_max-_speed_min)/2;
		return real_sensitive;
	}
}

unsigned short onvif_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	float real_pan_sensitive	= get_rpan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_rtilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_rzoom_sensitive_value( zoom_sensitive );
	float real_speed			= get_speed_sensitive_value( speed );

	//if( real_tilt_sensitive<0 )	//카메라가 tilt 방향으로 90도 이상 뒤로 넘어가는 것을 방지함.
	//{
	//	float pan = 0.0;
	//	float tilt = 0.0;
	//	float zoom = 0.0;
	//	query_status( pan, tilt, zoom );
	//	if( tilt+real_tilt_sensitive<(_max_tilt-((_max_tilt-_min_tilt)/2)) )
	//		real_tilt_sensitive = 0;
	//}

	PTZBindingProxy proxy;
	_tptz__RelativeMove request;
	_tptz__RelativeMoveResponse response;

	request.ProfileToken				= soap_strdup( &proxy, _profile_token );
	request.Translation					= soap_new_tt__PTZVector( &proxy, -1 );
	request.Translation->PanTilt		= soap_new_tt__Vector2D( &proxy, -1 );
	request.Translation->PanTilt->x		= real_pan_sensitive;
	request.Translation->PanTilt->y		= real_tilt_sensitive;		                                                             
	request.Translation->PanTilt->space = soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace" );
	request.Translation->Zoom			= soap_new_tt__Vector1D( &proxy, -1 );
	request.Translation->Zoom->x		= real_zoom_sensitive;
	request.Translation->Zoom->space	= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace" );

	if( speed>0 )
	{
		request.Speed						= soap_new_tt__PTZSpeed( &proxy, -1 );
		request.Speed->PanTilt				= soap_new_tt__Vector2D( &proxy, -1 );
		request.Speed->PanTilt->x			= real_speed;
		request.Speed->PanTilt->y			= real_speed;
		request.Speed->Zoom					= soap_new_tt__Vector1D( &proxy, -1 );
		request.Speed->Zoom->x				= real_speed;
	}


	int result = fnRelativeMove( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
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

unsigned short onvif_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	int result = SOAP_FAULT;

	// 중간 return 으로 인해 아래 if문(zoom수행) 위로 수정
	query_limits();
	float realPanSensitive	= get_apan_sensitive_value( pan_sensitive );
	float realTiltSensitive = get_atilt_sensitive_value( tilt_sensitive );
	float realZoomSensitive = get_azoom_sensitive_value( zoom_sensitive );
	float real_speed		= get_speed_sensitive_value( speed );

	PTZBindingProxy proxy;
	_tptz__AbsoluteMove request;
	_tptz__AbsoluteMoveResponse response;

	request.ProfileToken				= soap_strdup( &proxy, _profile_token );
	request.Position					= soap_new_tt__PTZVector( &proxy, -1 );
	request.Position->PanTilt			= soap_new_tt__Vector2D( &proxy, -1 );
	request.Position->PanTilt->y		= realTiltSensitive;
	request.Position->PanTilt->x		= realPanSensitive;
	request.Position->PanTilt->space	= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace" );
	request.Position->Zoom				= soap_new_tt__Vector1D( &proxy, -1 );
	request.Position->Zoom->x			= realZoomSensitive;
	request.Position->Zoom->space		= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace" );

	if( speed>_speed_min )
	{
		request.Speed						= soap_new_tt__PTZSpeed( &proxy, -1 );
		request.Speed->PanTilt				= soap_new_tt__Vector2D( &proxy, -1 );
		request.Speed->PanTilt->x			= real_speed;
		request.Speed->PanTilt->y			= real_speed;
		request.Speed->Zoom					= soap_new_tt__Vector1D( &proxy, -1 );
		request.Speed->Zoom->x				= real_speed;
	}

	result = fnAbsoluteMove( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
		value = VMS_PTZ_SUCCESS;
	else
		value = VMS_PTZ_FAIL;
	return value;
}

unsigned short onvif_ptz_controller::stop_move( void )
{
	PTZBindingProxy proxy;
	_tptz__Stop request;
	_tptz__StopResponse response;

	request.ProfileToken	= soap_strdup( &proxy, _profile_token );
	request.PanTilt			= static_cast<bool*>( soap_malloc(&proxy, sizeof(bool)) );
	(*request.PanTilt)		= true;
	request.Zoom			= static_cast<bool*>( soap_malloc(&proxy, sizeof(bool)) );
	(*request.Zoom)			= true;

	int result = fnStop( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	float tmp_pan	= 0.0f;
	float tmp_tilt	= 0.0f;
	float tmp_zoom	= 0.0f;
	if( query_status(tmp_pan, tmp_tilt, tmp_zoom)==VMS_PTZ_SUCCESS )
	{
		pan		= get_apan_quasi_sensitive_value( tmp_pan );
		tilt	= get_atilt_quasi_sensitive_value( tmp_tilt );
		zoom	= get_azoom_quasi_sensitive_value( tmp_zoom );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}


unsigned short	onvif_ptz_controller::query_limits( void )
{
	if( _is_limits_queried ) return VMS_PTZ_SUCCESS;

	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	PTZBindingProxy proxy;
	_tptz__GetConfiguration request;
	_tptz__GetConfigurationResponse response;
	_min_cspeed			= -1.0f;
	_max_cspeed			= 1.0f;
	_min_tour_pantilt_speed = 1.0f;
	_max_tour_pantilt_speed = 32.0f;
	_min_tour_zoom_speed = 1.0f;
	_max_tour_zoom_speed = 8.0f;

	//request.PTZConfigurationToken	= soap_strdup( &proxy, _profile_token );
	std::string str = _profile_token;
	request.PTZConfigurationToken	= soap_strdup( &proxy, str.substr(7).c_str() );
	int result = fnGetConfiguration( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );

	if( result==SOAP_OK )
	{
		if( response.PTZConfiguration )
		{
			if( response.PTZConfiguration->PanTiltLimits && response.PTZConfiguration->PanTiltLimits->Range )
			{
				if( response.PTZConfiguration->PanTiltLimits->Range->XRange )
				{
					_min_pan = response.PTZConfiguration->PanTiltLimits->Range->XRange->Min;
					_max_pan = response.PTZConfiguration->PanTiltLimits->Range->XRange->Max;
				}
				else
				{
					_min_pan = -1.0f;
					_max_pan = 1.0f;
				}

				if( response.PTZConfiguration->PanTiltLimits->Range->YRange )
				{
					_min_tilt = response.PTZConfiguration->PanTiltLimits->Range->YRange->Min;
					_max_tilt = response.PTZConfiguration->PanTiltLimits->Range->YRange->Max;
				}
				else
				{
					_min_tilt = -1.0f;
					_max_tilt = 1.0f;
				}
			}

			if( response.PTZConfiguration->ZoomLimits && response.PTZConfiguration->ZoomLimits->Range )
			{
				if( response.PTZConfiguration->ZoomLimits->Range->XRange )
				{
					_min_zoom = response.PTZConfiguration->ZoomLimits->Range->XRange->Min;
					_max_zoom = response.PTZConfiguration->ZoomLimits->Range->XRange->Max;
				}
				else
				{
					_min_zoom = -1.0f;
					_max_zoom = 1.0f;
				}
			}

			_min_speed = 0.0f;
			_max_speed = 1.0f;

			_is_limits_queried = true;
			return VMS_PTZ_SUCCESS;
		}
		else
			return VMS_PTZ_HOST_IS_NOT_CONNECTABLE;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::query_status( float &pan, float &tilt, float &zoom )
{
	char position[200] = {0,};
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	PTZBindingProxy proxy;
	_tptz__GetStatus request;
	_tptz__GetStatusResponse response;
	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetStatus( this, &proxy, "http://%s:%d/onvif/device_service", &request, &response );
	if( result==SOAP_OK )
	{
		if( response.PTZStatus )
		{
			pan		= response.PTZStatus->Position->PanTilt->x;
			tilt	= response.PTZStatus->Position->PanTilt->y;
			zoom	= response.PTZStatus->Position->Zoom->x;
			return VMS_PTZ_SUCCESS;
		}
		else
			return VMS_PTZ_FAIL;
	}
	else
		return VMS_PTZ_FAIL;
}


float onvif_ptz_controller::get_speed_sensitive_value( float sensitive )
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

float onvif_ptz_controller::get_speed_quasi_sensitive_value( float real_sensitive )
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

float	onvif_ptz_controller::get_rpan_sensitive_value( float sensitive )
{
	query_limits();

	if( !_is_inverse )
		sensitive = -sensitive;

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
		float real_sensitive = float(sensitive*(_max_pan-_min_pan))/float(_pan_max-_pan_min)/2;		
		return real_sensitive;
	}				
}

float	onvif_ptz_controller::get_rtilt_sensitive_value( float sensitive )
{
	query_limits();

	if( !_is_inverse )
		sensitive = sensitive*-1;

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
		float real_sensitive = (sensitive*(_max_tilt-_min_tilt))/(_tilt_max-_tilt_min)/2;
		return real_sensitive;
	}
}

float	onvif_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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
		float real_sensitive = ((sensitive)*(_max_zoom-_min_zoom))/(_zoom_max-_zoom_min)/2;
		return real_sensitive;
	}
}

float onvif_ptz_controller::get_apan_sensitive_value( float sensitive )
{
	query_limits();

	if( _is_inverse )
		sensitive = _pan_max+_pan_min-sensitive;

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return sensitive;

	if( sensitive<_pan_min ) 
		return float(_min_pan);
	else if( sensitive>_pan_max ) 
		return float(_max_pan);
	else
	{
		//float real_sensitive = float(sensitive*(_max_pan-_min_pan))/float(_pan_max-_pan_min)+float(_min_pan);
		float real_sensitive = (sensitive-_pan_min)*(_max_pan-_min_pan)/(_pan_max-_pan_min)+_min_pan;
		return real_sensitive;
	}
}

float onvif_ptz_controller::get_atilt_sensitive_value( float sensitive )
{
	query_limits();

	if( _is_inverse )
		sensitive = _tilt_max+_tilt_min-sensitive;

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
		//float real_sensitive =  float((sensitive*(_max_tilt-_min_tilt)))/float(_tilt_max-_tilt_min)+float(_min_tilt);	
		float real_sensitive = (sensitive-_tilt_min)*(_max_tilt-_min_tilt)/(_tilt_max-_tilt_min)+_min_tilt;
		return real_sensitive;
	}
}

float onvif_ptz_controller::get_azoom_sensitive_value( float sensitive )
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
		//float real_sensitive = float((sensitive*(_max_zoom-_min_zoom)))/float(_zoom_max-_zoom_min)+float(_min_zoom);
		float real_sensitive = (sensitive-_zoom_min)*(_max_zoom-_min_zoom)/(_zoom_max-_zoom_min)+_min_zoom;
		return real_sensitive;
	}
}

float onvif_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

	if( _is_inverse )
		real_sensitive = _max_pan+_min_pan-real_sensitive;

	if( _pan_min==_min_pan && _pan_max==_max_pan )
		return real_sensitive;

	if( real_sensitive<_min_pan ) 
		return _pan_min;
	else if( real_sensitive>_max_pan ) 
		return _pan_max;
	else
	{
		//float sensitive = ((real_sensitive-_min_pan)*(_pan_max-_pan_min)-_min_pan)/(_max_pan-_min_pan);		
		float sensitive = ((real_sensitive-_min_pan)*(_pan_max-_pan_min))/(_max_pan-_min_pan)+_pan_min;

		if( sensitive>_pan_max )
			sensitive = _pan_max;
		if( sensitive<_pan_min )
			sensitive = _pan_min;
		return sensitive;
	}
}

float onvif_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
{
	query_limits();

	if( _is_inverse )
		real_sensitive = _max_tilt+_min_tilt-real_sensitive;

	if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
		return real_sensitive;

	if( real_sensitive<_min_tilt ) 
		return _tilt_min;
	else if( real_sensitive>_max_tilt ) 
		return _tilt_max;
	else
	{
		//float sensitive = ((real_sensitive-_min_tilt)*(_tilt_max-_tilt_min)-_min_tilt)/(_max_tilt-_min_tilt);	 
		float sensitive = ((real_sensitive-_min_tilt)*(_tilt_max-_tilt_min))/(_max_tilt-_min_tilt)+_tilt_min;
		if( sensitive>_tilt_max )
			sensitive = _tilt_max;
		if( sensitive<_tilt_min )
			sensitive = _tilt_min;
		return sensitive;
	}
}

float onvif_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
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
		//float sensitive = ((real_sensitive-_min_zoom)*(_zoom_max-_zoom_min)-_min_zoom)/(_max_zoom-_min_zoom);
		float sensitive = ((real_sensitive-_min_zoom)*(_zoom_max-_zoom_min))/(_max_zoom-_min_zoom)+_zoom_min;
		if( sensitive>_zoom_max )
			sensitive = _zoom_max;
		if( sensitive<_zoom_min )
			sensitive = _zoom_min;
		return sensitive;
	}
}

void onvif_ptz_controller::split2vector( std::string origin, std::string token, std::vector<std::string> *devided )
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

void onvif_ptz_controller::get_tour_speed( float speed, float &pan_speed, float &tilt_speed, float &zoom_speed )
{
	query_limits();

	if( speed<_speed_min )
	{
		pan_speed = _min_tour_pantilt_speed;
		tilt_speed = _min_tour_pantilt_speed;
		zoom_speed = _min_tour_zoom_speed;
	}
    else if( speed>_speed_max )
	{
		pan_speed = _max_tour_pantilt_speed;
		tilt_speed = _max_tour_pantilt_speed;
		zoom_speed = _max_tour_zoom_speed;
	}	
	else
	{
		pan_speed = tilt_speed = speed*(_max_tour_pantilt_speed-_min_tour_pantilt_speed)/(_speed_max-_speed_min);
		zoom_speed = speed*(_max_tour_zoom_speed-_min_tour_zoom_speed)/(_speed_max-_speed_min);;
	}
}

void onvif_ptz_controller::get_tour_quasi_speed( float pan_speed, float tilt_speed, float zoom_speed, float &speed )
{
	query_limits();

	float pan = ((pan_speed-_min_tour_pantilt_speed)*(_speed_max-_speed_min))/(_max_tour_pantilt_speed-_min_tour_pantilt_speed)+_speed_min;
	float tilt = ((tilt_speed-_min_tour_pantilt_speed)*(_speed_max-_speed_min))/(_max_tour_pantilt_speed-_min_tour_pantilt_speed)+_speed_min;
	float zoom = ((zoom_speed-_min_tour_zoom_speed)*(_speed_max-_speed_min))/(_max_tour_zoom_speed-_min_tour_zoom_speed)+_speed_min;

	float avg_speed = (pan+tilt+zoom)/3;

	if( avg_speed<_speed_min )
	{
		speed = _speed_min;
	}
	else if( avg_speed>_speed_max )
	{
		speed = _speed_max;
	}
	else
	{
		speed = avg_speed;
	}				
}

void onvif_ptz_controller::split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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

std::string onvif_ptz_controller::get_preset_tour_id( char *tour_name)
{
	std::map<std::string,std::string>::iterator it;		
	std::string str = std::string(tour_name);	   
	it = _tour_list.find(str);
    if(it!=_tour_list.end()) {		
		return it->second;
	}
	return "";
}



base_ptz_controller* create( void )
{
	return new onvif_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	onvif_ptz_controller *onvif_controller = dynamic_cast<onvif_ptz_controller*>( (*ptz_controller) );
	delete onvif_controller;
	(*ptz_controller) = 0;
}