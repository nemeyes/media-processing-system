#include "platform.h"
#include <ptz_device_info.h>
#include "hitron_ptz_controller.h"
#include "http_client.h"


hitron_ptz_controller::hitron_ptz_controller( void )
	: _is_limits_queried(false)
{
}

hitron_ptz_controller::~hitron_ptz_controller( void )
{

}

char* hitron_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HITRON_HEV0118H_PELCO_D_V_3_0_0][VENDOR];
}

char* hitron_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HITRON_HEV0118H_PELCO_D_V_3_0_0][DEVICE];
}

char* hitron_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HITRON_HEV0118H_PELCO_D_V_3_0_0][PROTOCOL];
}

char* hitron_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[HITRON_HEV0118H_PELCO_D_V_3_0_0][VERSION];
}

unsigned short hitron_ptz_controller::get_vendor_id( void )
{
	return VMS_PTZ_DEVICE_ID[HITRON_HEV0118H_PELCO_D_V_3_0_0][VENDOR];
}

unsigned short hitron_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[HITRON_HEV0118H_PELCO_D_V_3_0_0][DEVICE];
}

unsigned short hitron_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[HITRON_HEV0118H_PELCO_D_V_3_0_0][PROTOCOL];
}

unsigned short hitron_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[HITRON_HEV0118H_PELCO_D_V_3_0_0][VERSION];
}

unsigned short hitron_ptz_controller::set_host_name( char *host )
{
	if( host && (strlen(host)>0) ) 
	{
		strcpy( _host, host );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hitron_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hitron_ptz_controller::set_user_password( char *password )
{
	if( password && (strlen(password)>0) ) 
	{
		strcpy( _user_password, password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short hitron_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_pan_number_place = number_place;
	_pan_min = min;
	_pan_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_tilt_number_place = number_place;
	_tilt_min = min;
	_tilt_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_zoom_number_place = number_place;
	_zoom_min = min;
	_zoom_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_speed_number_place = number_place;
	_speed_min = min;
	_speed_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::set_profile_token( char *token )
{
	if( token && strlen(token)>0 )
	{
		memset( _profile_token, 0x00, MAX_PATH );
		strcpy( _profile_token, token );
		return VMS_PTZ_SUCCESS;
	}
	return VMS_PTZ_FAIL;
}

unsigned short hitron_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hitron_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hitron_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hitron_ptz_controller::is_enable_preset_tour( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hitron_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hitron_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hitron_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hitron_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hitron_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short hitron_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hitron_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short hitron_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	hitron_ptz_controller::goto_home_position( float speed )
{
	// /ptz/control.php?ch=1&move=home
	http_client client( _host, _port_number, "/ptz/control.php");
	//client.put_variable("ch", "1");
	client.put_variable("preset", "1");

	if( !client.send_request( _user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short	hitron_ptz_controller::set_home_position( void )
{	
	http_client client( _host, _port_number, "/ptz/control.php" );
	//client.put_variable( "ch", "1" );	
	client.put_variable( "presetsave", "1" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;

}

unsigned short	hitron_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	std::map<std::string,std::string> preset_map;
	std::map<std::string,std::string>::iterator it;

	//30 개 고정
	for(int i=0;i<20;i++)
	{   
		char preset[3] = {0,};
		//기존 snprintf( preset, sizeof(preset), "%d", i+1 );
		snprintf( preset, sizeof(preset), "%d", i );
		preset_map[preset] = preset;
	}
	(*length) = preset_map.size();

	if( (*length)>0 )
	{
		(*aliases) = static_cast<char**>( malloc(sizeof(char**)*(*length)) );
		int index = 0;
		//정렬이 되지 않아서 itreator 쓰지 않고 for문 사용
		for(int i=0; i<20; i++)
		{
			char preset[3] = {0,};
			//기존 snprintf( preset, sizeof(preset), "%d", i+1 );
			snprintf( preset, sizeof(preset), "%d", i );
			(*aliases)[i] = strdup( preset_map[preset].c_str());
		}		
	}
	return VMS_PTZ_SUCCESS;		
}

unsigned short	hitron_ptz_controller::add_preset( char *alias )
{
	// 수정포인트 snprintf( preset, sizeof(preset), "%d", i );
    char data[10] = {0};
	snprintf( data, sizeof(data), "%c", *alias+1);

	http_client client( _host, _port_number, "/ptz/control.php" );
	//client.put_variable( "ch", "1" );	
	//기존 client.put_variable( "presetsave", alias );
	client.put_variable( "presetsave", data );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short	hitron_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND; 
}

unsigned short	hitron_ptz_controller::goto_preset( char *alias )
{
	char data[10] = {0};
	snprintf( data, sizeof(data), "%c", *alias+1);
	http_client client( _host, _port_number, "/ptz/control.php" );

	//기존 client.put_variable( "preset", alias );
	client.put_variable( "preset", data );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

std::string hitron_ptz_controller::get_preset_name_by_id(char *preset_id)
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

unsigned short hitron_ptz_controller::get_preset_list_map( std::map<std::string,std::string> *preset_map )
{
    ///ptz/preset.php?app=get	
	char recv_str[1000] = {0,};
	http_client client( _host, _port_number, "/ptz/preset.php" );
    //client.put_variable( "ch", "1" );	
	client.put_variable( "app", "get" );

	if( !client.send_request(_user_id, _user_password, recv_str) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	
	std::string str_presets = std::string(recv_str);
	str_presets = str_presets.substr(str_presets.find_last_of("=")+1);
	
	std::vector<std::string> vec;
	std::vector<std::string>::iterator it;	
	std::map<std::string, std::string>::iterator preset_map_it;
	
	//결과값 :res=200&preset_number=0&preset_title=Home&preset_af_mode=1   	
	split2vector(str_presets , ",", &vec);		
		
	for( it=vec.begin(); it!=vec.end(); it++ )
	{		
		std::string preset_id = *it;
		char *id = strdup(preset_id.c_str());
		std::string preset_name = get_preset_name_by_id(id);		
		(*preset_map)[preset_name] = preset_id;		
	}

	return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::add_preset_tour( char *tour_name, int size )
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

unsigned short hitron_ptz_controller::remove_preset_tour( char *tour_name )
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

unsigned short hitron_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
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
unsigned short hitron_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::remove_preset_tour2( int tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
	/*VMS_PTZ_UNSUPPORTED_COMMAND;*/
//	 ///ptz/tour.php?ch=1&app=set&method=create&tour_number=2&tour_title=T2&tour_repeat_time=0&tour_sequence=1&tour_tourlist=0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0,0|0|0|0|0|0 
//	std::map<std::string,std::string> preset_map;
//	std::map<std::string,std::string>::iterator preset_map_it;
//    if(VMS_PTZ_FAIL==get_preset_list_map(&preset_map)) return VMS_PTZ_FAIL;	
//
//	std::string str_tour_no = get_preset_tour_id(tour->tour_name);
//	if(str_tour_no=="") return VMS_PTZ_FAIL;
//
////if(VMS_PTZ_FAIL==remove_preset_tour_presets(str_tour_no.c_str())) return VMS_PTZ_FAIL;
//
//	http_client client( _host, _port_number, "/ptz/tour.php" );			
//	//client.put_variable( "ch", "1");	
//	client.put_variable( "app", "set");	
//	client.put_variable( "method", "create");	
//	client.put_variable( "tour_number", str_tour_no.c_str());
//	client.put_variable( "tour_title", tour->tour_name);	
//	char str_tour_repeat_time[10]= {0,};
//	snprintf( str_tour_repeat_time, sizeof(str_tour_repeat_time), "%d", tour->tour_recurring_time);
//	client.put_variable( "tour_repeat_time", str_tour_repeat_time);
//	if((tour->tour_direction) == PTZ_TOUR_DIRECTION_FORWARD) 
//	{
//		client.put_variable( "tour_sequence", "1");
//	} else
//	{
//		client.put_variable( "tour_sequence", "0");
//	}
//
//	std::string str_tour_list = "";		
//
//	for(int i=0; i<tour->size_of_tour_spots;i++)
//	{
//		PTZ_TOUR_SPOT_T* tsp = &(tour->tour_spots[i]);		
//
//		std::string preset_id = "";	
//   		preset_map_it = preset_map.find(tsp->preset_alias);	
//		if(preset_map_it!=preset_map.end()) preset_id = preset_map_it->second;		
//		if(preset_id=="") return VMS_PTZ_FAIL;		
//				
//		char str_value[10] = {0,};							
//
//		//1|2|12|32|32|8 = 구분(1:프리셋) | 프리셋NO | 거주시간 | 팬(1~32,디폴트32)속도 | 틸트(1~32,디폴트32)속도 | 줌(1~8,디폴트8)속도
//		str_tour_list.append("1|");		
//		str_tour_list.append(preset_id+"|");
//		snprintf( str_value, sizeof(str_value), "%d|", tsp->stay_time/1000);
//		str_tour_list.append(str_value);//stay_time
//		str_tour_list.append("32|");//pan speed		
//		str_tour_list.append("32|");//tilt speed				
//		str_tour_list.append("8");//zoom speed				
//		str_tour_list.append(",");
//	}
//	str_tour_list = str_tour_list.substr(0,str_tour_list.length()-1);
//	client.put_variable( "tour_tourlist", str_tour_list.c_str());
//
//	if( !client.send_request(_user_id, _user_password) ) 
//	{
//		client.clear_variable();
//		return VMS_PTZ_FAIL;
//	}		
//	client.clear_variable();		
//
//	return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
	//std::string str_tour_name = tour->tour_name;
	////std::string str_tour_no = str_tour_name.substr(str_tour_name.size()-1);	
	//std::string str_tour_no = "";
	//std::map<std::string,std::string>::iterator it;
	//it = _tour_list.find(str_tour_name);
	//if(it!=_tour_list.end()) str_tour_no = it->second;

	//char recv_tours[5000] = {0,};		
	//http_client client( _host, _port_number, "/ptz/tour.php" );	
	////client.put_variable( "ch", "1" );	
	//client.put_variable( "app", "get" );	
	//client.put_variable( "method", "get_num" );	
	//client.put_variable( "tour_number", str_tour_no.c_str() );	

	//if( !client.send_request(_user_id, _user_password, recv_tours) ) 
	//{
	//	client.clear_variable();
	//	return VMS_PTZ_FAIL;
	//}

	//client.clear_variable();

	//if(strlen(recv_tours)==0) return VMS_PTZ_FAIL;
	//
	//int tour_spot_cnt = 0;

	//std::vector<std::string> vec;
	//std::vector<std::string>::iterator v_it;
	//std::map<std::string,std::string> tour_map;
	//std::map<std::string,std::string>::iterator tour_map_it;	   	

	//split2vector( recv_tours, "&", &vec);		

	//for( v_it=vec.begin(); v_it!=vec.end(); v_it++ )
	//{
	//	std::string pair = *v_it;				
	//	split2map( pair, "=", &tour_map );	
	//}
	//std::string str_tour_list = "";
	//tour_map_it = tour_map.find("tour_tourlist");
	//if(tour_map_it!=tour_map.end()) str_tour_list = tour_map_it->second;

	//std::vector<std::string> vec_tour_list;
	//std::vector<std::string>::iterator vec_tour_list_it;
	//split2vector( str_tour_list, ",", &vec_tour_list);
	//
	//for( vec_tour_list_it=vec_tour_list.begin(); vec_tour_list_it!=vec_tour_list.end(); vec_tour_list_it++ )
	//{
	//	std::string str =  *vec_tour_list_it;			
	//	char *tmp_str = strdup(str.c_str());
	//	if(strcmp("0|0|0|0|0|0",tmp_str)==0) continue;
	//	tour_spot_cnt++;					
	//}

	//std::string str_tour_recurring_time = "0";
	//tour_map_it = tour_map.find("tour_repeat_time");
	//if(tour_map_it!=tour_map.end()) str_tour_recurring_time = tour_map_it->second;		
	//tour->tour_recurring_time = std::stoi(str_tour_recurring_time,0,10);	
	//tour->tour_recurring_duration = 0;	
	//
	//std::string str_direction = "1";
	//tour_map_it = tour_map.find("tour_sequence");
	//if(tour_map_it!=tour_map.end()) str_direction = tour_map_it->second;
	//if(strcmp(str_direction.c_str(),"1")==0) 
	//{
	//	tour->tour_direction = PTZ_TOUR_DIRECTION_FORWARD;
	//} else
	//{
	//	tour->tour_direction = PTZ_TOUR_DIRECTION_BACKWARD;
	//}
	//	
	//tour->tour_always_start=false;
	//tour->size_of_tour_spots = tour_spot_cnt;
 //   tour->tour_spots = new PTZ_TOUR_SPOT_T[tour_spot_cnt];		

 //   int idx = 0;
	//PTZ_TOUR_SPOT_T *tsp = NULL;

	//for( vec_tour_list_it=vec_tour_list.begin(); vec_tour_list_it!=vec_tour_list.end(); vec_tour_list_it++ )
	//{
	//	std::string str =  *vec_tour_list_it;			
	//	char *tmp_str = strdup(str.c_str());
	//	if(strcmp("0|0|0|0|0|0",tmp_str)==0) continue;
	//	tsp = &(tour->tour_spots[idx++]);
	//	char *dv,*spot_no,*dwell_time,*pan,*tilt,*zoom;
	//	dv = strtok(tmp_str,"|");
	//	spot_no = strtok(NULL,"|");
	//	dwell_time = strtok(NULL,"|");
	//	//pan = strtok(NULL,"|");
	//	//tilt = strtok(NULL,"|");
	//	//zoom = strtok(NULL,"|");
	//	strncpy( tsp->preset_alias, strdup(get_preset_name_by_id(spot_no).c_str()), sizeof(tsp->preset_alias) ); 
	//	//tsp->pan = float(std::stoi(pan,0,10));
	//	//tsp->tilt = float(std::stoi(tilt,0,10));
	//	//tsp->zoom = float(std::stoi(zoom,0,10));
	//	tsp->pan = 0;
	//	tsp->tilt = 0;
	//	tsp->zoom = 0;
	//	tsp->speed = 0;
	//	tsp->stay_time = std::stoi(dwell_time,0,10)*1000;	
	//}	

	//return VMS_PTZ_SUCCESS;		
}

unsigned short hitron_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
	////투어추가를 할수없음으로 투어목록을 생성해논것을 이용함으로 아래 사용불가
	//if(VMS_PTZ_FAIL==get_preset_tour_list()) return VMS_PTZ_FAIL;

	//(*size_of_tours) = _tour_list.size();   
	//*tour = new PTZ_TOUR_T[_tour_list.size()];   
	//
 //   std::map<std::string,std::string>::iterator it;
	//int idx = 0;
 //   for(it=_tour_list.begin(); it!=_tour_list.end(); ++it)   
	//{		
	//	strncpy( (*tour+(idx++))->tour_name, it->first.c_str(), sizeof((*tour+(idx++))->tour_name) );
	//}		

	//return VMS_PTZ_SUCCESS;
}

unsigned short hitron_ptz_controller::get_preset_tour_list(void)
{	
	return VMS_PTZ_UNSUPPORTED_COMMAND;
	/*std::string tour_str = get_preset_tour_list_str();
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
	return VMS_PTZ_SUCCESS;		*/

}

std::string hitron_ptz_controller::get_preset_tour_list_str(void)
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

std::string hitron_ptz_controller::get_preset_tour_name(std::string tour_no)
{
	//char recv_tours[5000] = {0,};		
	//http_client client( _host, _port_number, "/ptz/tour.php" );
	////client.put_variable( "ch", "1" );
	//client.put_variable( "app", "get" );
	//client.put_variable( "method", "get_num" );
	//client.put_variable( "tour_number", tour_no.c_str());
	//
	//if( !client.send_request(_user_id, _user_password, recv_tours) ) 
	//{
	//	client.clear_variable();
	//	return "";
	//}

	//std::vector<std::string> vec;
	//std::vector<std::string>::iterator it;	
	//std::map<std::string, std::string> tour_map;
	//std::map<std::string, std::string>::iterator tour_map_it;
 //  	
	//split2vector(recv_tours , "&", &vec);		
	//	
	//for( it=vec.begin(); it!=vec.end(); it++ )
	//{		
	//	std::string pair = *it;
	//	split2map( pair, "=", &tour_map);				
	//}

	//tour_map_it = tour_map.find("tour_title");

	//if(tour_map_it!=tour_map.end()) return tour_map_it->second;

	return "";

}


unsigned short hitron_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
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
			value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFT, pan_sensitive, 0);
		}
		else //right
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHT, pan_sensitive, 0);
		}
	}
	else if(tilt_sensitive!=0&&pan_sensitive==0)//상하
	{
		if(tilt_sensitive>0)
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_UP, tilt_sensitive, 0);
		}
		else
		{
			value = continuous_move( PTZ_CONTINUOUS_MOVE_DOWN, tilt_sensitive, 0);
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//좌상
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTUP, float((pan_sensitive+tilt_sensitive)/2), 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//우하
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTDOWN, float((pan_sensitive+tilt_sensitive)/2), 0);
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//좌하
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTDOWN, float((pan_sensitive+tilt_sensitive)/2), 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//우상
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTUP, float((pan_sensitive+tilt_sensitive)/2), 0);
	}
	else if(zoom_sensitive>0)//줌인
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_ZOOMIN, zoom_sensitive, 0);
	}
	else if(zoom_sensitive<0)//줌아웃
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_ZO0MOUT, zoom_sensitive, 0);
	}
	
	return value;
}

unsigned short hitron_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	http_client client( _host, _port_number, "/ptz/control.php" );
	float real_speed	= get_continuous_sensitive_value( speed );
    real_speed = ceil(abs(real_speed));

	char spd[100]	= {0,};
	snprintf( spd, sizeof(spd), "%.2f", real_speed );
	
    // /ptz/control.php?move=left&pspd=4&tstp=4
	switch( UINT8(move) )
	{
		case PTZ_CONTINUOUS_MOVE_UP :
		{
			client.put_variable( "move", "up" );
			client.put_variable( "tspd", spd );
			client.put_variable( "pspd", spd );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFT :
		{
			client.put_variable( "move", "left" );
			client.put_variable( "tspd", spd );
			client.put_variable( "pspd", spd );	
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHT :
		{
			client.put_variable( "move", "right" );
			client.put_variable( "pspd", spd );
			client.put_variable( "tstp", spd );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_DOWN :
		{
			client.put_variable( "move", "down" );
			client.put_variable( "pspd", spd );
			client.put_variable( "tstp", spd );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTUP :
		{
			client.put_variable( "move", "upleft" );
			client.put_variable( "pspd", spd );
			client.put_variable( "tstp", spd );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTUP :
		{
			client.put_variable( "move", "upright" );
			client.put_variable( "pspd", spd );
			client.put_variable( "tstp", spd );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTDOWN :
		{
			client.put_variable( "move", "downleft" );
			client.put_variable( "pspd", spd );
			client.put_variable( "tstp", spd );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTDOWN :
		{
			client.put_variable( "move", "downright" );
			client.put_variable( "pspd", spd );
			client.put_variable( "tstp", spd );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZOOMIN :
		{
			client.put_variable( "zoom", "tele" );
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZO0MOUT :
		{
			client.put_variable( "zoom", "wide" );
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

float hitron_ptz_controller::get_continuous_sensitive_value( float sensitive )
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



unsigned short hitron_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short hitron_ptz_controller::stop_move( void )
{
	http_client client( _host, _port_number, "/ptz/control.php" );
	client.put_variable( "move", "stop" );
	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short	hitron_ptz_controller::query_limits( void )
{
	_min_pan	= 0;
	_max_pan	= 359;
	_min_tilt	= -5;
	_max_tilt	= 185;
	_min_zoom	= 1;
	_max_zoom	= 20;	
	_min_speed  = 0;
	_max_speed  = 64;
	_min_cspeed	= 0;
	_max_cspeed	= 8;
	return VMS_PTZ_SUCCESS;
}

float hitron_ptz_controller::get_speed_sensitive_value( float sensitive )
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

float hitron_ptz_controller::get_speed_quasi_sensitive_value( float real_sensitive )
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

float	hitron_ptz_controller::get_rpan_sensitive_value( float sensitive )
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

float	hitron_ptz_controller::get_rtilt_sensitive_value( float sensitive )
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

float	hitron_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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

float hitron_ptz_controller::get_apan_sensitive_value( float sensitive )
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

float hitron_ptz_controller::get_atilt_sensitive_value( float sensitive )
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

float hitron_ptz_controller::get_azoom_sensitive_value( float sensitive )
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

float hitron_ptz_controller::get_apan_quasi_sensitive_value( float real_sensitive )
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

float hitron_ptz_controller::get_atilt_quasi_sensitive_value( float real_sensitive )
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

float hitron_ptz_controller::get_azoom_quasi_sensitive_value( float real_sensitive )
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

void hitron_ptz_controller::split2vector( std::string origin, std::string token, std::vector<std::string> *devided )
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

void hitron_ptz_controller::split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided )
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

std::string hitron_ptz_controller::get_preset_tour_id( char *tour_name)
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
	return new hitron_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	hitron_ptz_controller *hitron_controller = dynamic_cast<hitron_ptz_controller*>( (*ptz_controller) );
	delete hitron_controller;
	(*ptz_controller) = 0;
}