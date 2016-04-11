#include "platform.h"
#include <ptz_device_info.h>
#include "lge_ptz_controller.h"

#define SENSITIVE_MIN	1
#define SENSITIVE_MAX	100

lge_ptz_controller::lge_ptz_controller( void )
	: _port_number(80)
{

}

lge_ptz_controller::~lge_ptz_controller( void )
{

}

char* lge_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LNP2810_SOAP_API_1889001310250][VENDOR];
}

char* lge_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LNP2810_SOAP_API_1889001310250][DEVICE];
}

char* lge_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LNP2810_SOAP_API_1889001310250][PROTOCOL];
}

char* lge_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LNP2810_SOAP_API_1889001310250][VERSION];
}

unsigned short lge_ptz_controller::get_vendor_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LNP2810_SOAP_API_1889001310250][VENDOR];
}

unsigned short lge_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LNP2810_SOAP_API_1889001310250][DEVICE];
}

unsigned short lge_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LNP2810_SOAP_API_1889001310250][PROTOCOL];
}

unsigned short lge_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LNP2810_SOAP_API_1889001310250][VERSION];
}

unsigned short	lge_ptz_controller::set_host_name( char *hostname )
{
	if( hostname && (strlen(hostname)>0) ) 
	{
		strcpy( _hostname, hostname );
		char xaddress[300] = {0};
		snprintf( xaddress, sizeof(xaddress), "%s:%d", _hostname, _port_number );
		set_xaddress( xaddress );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short	lge_ptz_controller::set_port_number( unsigned short port_number )
{
	_port_number = port_number;
	if( strlen(_hostname)>0 )
	{
		char xaddress[300] = {0};
		snprintf( xaddress, sizeof(xaddress), "%s:%d", _hostname, _port_number );
		set_xaddress( xaddress );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short	lge_ptz_controller::set_user_id( char *user_id )
{
	if( user_id && (strlen(user_id)>0) )		
	{
		strcpy( _user_id, user_id );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short lge_ptz_controller::set_user_password( char *user_password )
{
	if( user_password && (strlen(user_password)>0) ) 
	{
		strcpy( _user_password, user_password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short lge_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	_min = min;
	_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short lge_ptz_controller::set_profile_token( char *token )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::is_enable_osd_menu( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::is_enable_home_position( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short lge_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short lge_ptz_controller::is_enable_preset_tour( void )
{
	//get_preset_tour을 가져오기를 할수없어서 막음
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::is_enable_continuous_move( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short lge_ptz_controller::is_enable_relative_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::is_enable_absolute_move( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::is_preset_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short lge_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short lge_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_TRUE;
}

unsigned short lge_ptz_controller::is_preset_tour_name_changable( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::goto_home_position( float speed )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	PTZSoap proxy;
	_ns1__GotoPreset request;
	_ns1__GotoPresetResponse response;
	request.channel = 0;
	request.alias	= soap_strdup( proxy.soap, "*HOME*" );

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__GotoPreset( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	else
	{
		return VMS_PTZ_SUCCESS;
	}
}

unsigned short lge_ptz_controller::set_home_position( void )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	int sequence = 0;
	{
		PTZSoap proxy;
		_ns1__GetPresetList request;
		request.soap_default( proxy.soap );
		_ns1__GetPresetListResponse response;
		response.soap_default( proxy.soap );
		request.channel = 0;

		int ret;
		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
		proxy.endpoint = szPTZUrl;
		proxy.soap->userid = soap_strdup( proxy.soap, _username );
		proxy.soap->passwd = soap_strdup( proxy.soap, _password );
		ret = proxy.__ns2__GetPresetList( &request, &response );
		if( ret!=SOAP_OK )
		{
			full_fault_code( proxy.soap );
			return VMS_PTZ_FAIL;
		}
		else
		{
			if( response.GetPresetListResult )
			{
				std::string preset_list = "";
				for( int index=0; index<response.GetPresetListResult->__sizePreset_USCORET; index++ )
				{
					if( response.GetPresetListResult->Preset_USCORET[index]->sequence>sequence )
						sequence = response.GetPresetListResult->Preset_USCORET[index]->sequence;
				}
			}
		}
	}


	{
		sequence++;
		sequence = sequence%256;
		PTZSoap proxy;
	
		_ns1__AddPreset request;
		_ns1__AddPresetResponse response;
		request.channel = 0;
		request.preset				= soap_new_ns1__Preset_USCORET( proxy.soap, -1 );
		request.preset->alias		= soap_strdup( proxy.soap, "*HOME*0" );
		request.preset->sequence	= sequence;
		request.preset->presetSpeed	= 127;
		request.preset->parkTime	= 3;

		int ret;
		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
		proxy.endpoint = szPTZUrl;
		proxy.soap->userid = soap_strdup( proxy.soap, _username );
		proxy.soap->passwd = soap_strdup( proxy.soap, _password );
		ret = proxy.__ns2__AddPreset( &request, &response );
		if( ret!=SOAP_OK )
		{
			full_fault_code( proxy.soap );
			return VMS_PTZ_FAIL;
		}
		else
		{
			return VMS_PTZ_SUCCESS;
		}
	}
}


unsigned short lge_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::add_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::remove_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::goto_preset( char *alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::get_preset_list2( int **aliases, int *length )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );


	PTZSoap proxy;
	
	_ns1__GetPresetList request;
	request.soap_default( proxy.soap );
	_ns1__GetPresetListResponse response;
	response.soap_default( proxy.soap );

	request.channel = 0;

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__GetPresetList( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	else
	{
		if( response.GetPresetListResult )
		{
			if( response.GetPresetListResult->__sizePreset_USCORET>0 )
			{
				(*length) = response.GetPresetListResult->__sizePreset_USCORET;
				(*aliases) = static_cast<int*>( malloc(sizeof(int*)*(*length)) );
				int r_idx = 0;
				for( int index=0; index<response.GetPresetListResult->__sizePreset_USCORET; index++ )
				{
					std::string tmp = response.GetPresetListResult->Preset_USCORET[index]->alias;
					if(tmp.find("*HOME*") != -1) continue;					
					int i_alias = atoi(tmp.c_str());					
					(*aliases)[r_idx++] = i_alias;
				}
				(*length) = r_idx;
				return VMS_PTZ_SUCCESS;
			}
			else
			{
				(*length) = 0;
				return VMS_PTZ_SUCCESS;
			}
		}
		else
			return VMS_PTZ_FAIL;
	}
}

unsigned short lge_ptz_controller::add_preset2( int &alias )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	int sequence = 0;
	{
		PTZSoap proxy;
		_ns1__GetPresetList request;
		request.soap_default( proxy.soap );
		_ns1__GetPresetListResponse response;
		response.soap_default( proxy.soap );
		request.channel = 0;

		int ret;
		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
		proxy.endpoint = szPTZUrl;
		proxy.soap->userid = soap_strdup( proxy.soap, _username );
		proxy.soap->passwd = soap_strdup( proxy.soap, _password );
		ret = proxy.__ns2__GetPresetList( &request, &response );
		if( ret!=SOAP_OK )
		{
			full_fault_code( proxy.soap );
			return VMS_PTZ_FAIL;
		}
		else
		{
			if( response.GetPresetListResult )
			{
				for( int index=0; index<response.GetPresetListResult->__sizePreset_USCORET; index++ )
				{
					if( response.GetPresetListResult->Preset_USCORET[index]->sequence>sequence )
						sequence = response.GetPresetListResult->Preset_USCORET[index]->sequence;
				}
			}
		}
	}


	{
		sequence++;
		sequence = sequence%256;
		PTZSoap proxy;
	
		_ns1__AddPreset request;
		_ns1__AddPresetResponse response;
		request.channel = 0;
		request.preset				= soap_new_ns1__Preset_USCORET( proxy.soap, -1 );
		char str_alias[10] = {0,};
		sprintf(str_alias, "%d", alias);
		request.preset->alias		= soap_strdup( proxy.soap, str_alias );
		request.preset->sequence	= sequence;
		request.preset->presetSpeed	= 127;
		request.preset->parkTime	= 3;

		int ret;
		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
		proxy.endpoint = szPTZUrl;
		proxy.soap->userid = soap_strdup( proxy.soap, _username );
		proxy.soap->passwd = soap_strdup( proxy.soap, _password );
		ret = proxy.__ns2__AddPreset( &request, &response );
		if( ret!=SOAP_OK )
		{
			full_fault_code( proxy.soap );
			return VMS_PTZ_FAIL;
		}
		else
		{
			return VMS_PTZ_SUCCESS;
		}
	}
}

unsigned short lge_ptz_controller::remove_preset2( int alias )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	PTZSoap proxy;
	
	_ns1__RemovePreset request;
	_ns1__RemovePresetResponse response;
	request.channel = 0;
	char str_alias[10] = {0,};
	sprintf(str_alias, "%d", alias);
	request.alias	= soap_strdup( proxy.soap, str_alias );

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__RemovePreset( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	else
	{
		return VMS_PTZ_SUCCESS;
	}
}

unsigned short lge_ptz_controller::goto_preset2( int alias )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	PTZSoap proxy;
	_ns1__GotoPreset request;
	_ns1__GotoPresetResponse response;
	request.channel = 0;
	char str_alias[10] = {0,};
	sprintf(str_alias, "%d", alias);
	request.alias	= soap_strdup( proxy.soap, str_alias);

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__GotoPreset( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	else
	{
		return VMS_PTZ_SUCCESS;
	}
}

unsigned short lge_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::remove_preset_tour( char *tour_name )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::remove_preset_tour2( int tour_name )
{
	char str_cmd[20] = {0,};
	PTZSoap proxy;
	_ns1__ControlPTZ request;
	_ns1__ControlPTZResponse response;

	request.channel = 0;
	request.param = soap_new_ns1__ArrayOfParameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(proxy.soap, sizeof(ns1__Parameter_USCORET*)*2) );
	request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[0]->key = soap_strdup( proxy.soap, "PresetGroup" );		
	sprintf(str_cmd, "Clear,%d" ,tour_name);
	request.param->Parameter_USCORET[0]->value = soap_strdup( proxy.soap, str_cmd);
	request.param->__sizeParameter_USCORET = 1;

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__ControlPTZ( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short lge_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	char str_cmd[20] = {0,};

	switch( UINT8(cmd) )
	{
		case PTZ_TOUR_CMD_START  :						
			sprintf(str_cmd, "Tour,%d" ,tour_name);
			break;

		case PTZ_TOUR_CMD_STOP :
			sprintf(str_cmd, "TourStop,%d" ,tour_name);
			break;

		case PTZ_TOUR_CMD_PAUSE : 
			return VMS_PTZ_UNSUPPORTED_COMMAND;
			break;

		default :
			return  VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
	}

	PTZSoap proxy;
	_ns1__ControlPTZ request;
	_ns1__ControlPTZResponse response;

	request.channel = 0;
	request.param = soap_new_ns1__ArrayOfParameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(proxy.soap, sizeof(ns1__Parameter_USCORET*)*2) );
	request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[0]->key = soap_strdup( proxy.soap, "PresetGroup" );	
	request.param->Parameter_USCORET[0]->value = soap_strdup( proxy.soap, str_cmd );
	request.param->__sizeParameter_USCORET = 1;

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__ControlPTZ( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short lge_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{

	remove_preset_tour2(atoi(tour->tour_name));

	char str_cmd[20] = {0,};
	PTZSoap proxy;
	_ns1__ControlPTZ request;
	_ns1__ControlPTZResponse response;

	request.channel = 0;
	request.param = soap_new_ns1__ArrayOfParameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(proxy.soap, sizeof(ns1__Parameter_USCORET*)*2) );

    if(tour->size_of_tour_spots == 0) return VMS_PTZ_FAIL;	

	for(int i=0; i<tour->size_of_tour_spots;i++)
	{
		PTZ_TOUR_SPOT_T* tsp = &(tour->tour_spots[i]);		
		request.param->Parameter_USCORET[i] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
		request.param->Parameter_USCORET[i]->key = soap_strdup( proxy.soap, "PresetGroup" );		
		sprintf(str_cmd, "Add,%s,%s" ,tour->tour_name, tsp->preset_alias);
		request.param->Parameter_USCORET[i]->value = soap_strdup( proxy.soap, str_cmd);
	}

	request.param->__sizeParameter_USCORET = tour->size_of_tour_spots;

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__ControlPTZ( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short lge_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;	
}

unsigned short lge_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	 for(int i=1;i<=6;i++)
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

unsigned short lge_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
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
			value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFT, abs(pan_sensitive), 0);
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
			value = continuous_move( PTZ_CONTINUOUS_MOVE_DOWN, abs(tilt_sensitive), 0);
		}
	}
	else if(pan_sensitive<0&&tilt_sensitive>0)//좌상
	{		
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTUP,  (abs(pan_sensitive)+abs(tilt_sensitive))/2, 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//우하
	{		
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTDOWN,  (abs(pan_sensitive)+abs(tilt_sensitive))/2, 0);
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//좌하
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTDOWN,  (abs(pan_sensitive)+abs(tilt_sensitive))/2, 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//우상
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTUP,  (abs(pan_sensitive)+abs(tilt_sensitive))/2, 0);
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

unsigned short lge_ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{	
	std::string d_value = "";

	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
	{
		set_wsse_token( _user_id, _user_password );
	}

	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(move) )
	{
		case PTZ_CONTINUOUS_MOVE_UP :
		{
			_moveType = "Move";
			_moveSpeedType = "movespeed";
			d_value   = "Up";		
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFT :
		{
			_moveType = "Move";
			_moveSpeedType = "movespeed";
			d_value   = "Left";	
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHT :
		{
			_moveType = "Move";
			_moveSpeedType = "movespeed";
			d_value   = "Right";		
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_DOWN :
		{
			_moveType = "Move";
			_moveSpeedType = "movespeed";
			d_value   = "Down";		
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTUP :
		{
			_moveType = "Move";
			_moveSpeedType = "movespeed";
			d_value   = "LeftUp";		
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTUP :
		{
			_moveType = "Move";
			_moveSpeedType = "movespeed";
			d_value   = "RightUp";	
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTDOWN :
		{
			_moveType = "Move";
			_moveSpeedType = "movespeed";
			d_value   = "LeftDown";	
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTDOWN :
		{
			_moveType = "Move";
			_moveSpeedType = "movespeed";
			d_value   = "RightDown";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZOOMIN :
		{
			_moveType = "Zoom";
			_moveSpeedType = "zoomspeed";
			d_value   = "tele";			
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZO0MOUT :
		{
			_moveType = "Zoom";
			_moveSpeedType = "zoomspeed";
			d_value   = "wide";			
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}
	continuous_move_direction(d_value, abs(speed));			

	return value;
}

unsigned short  lge_ptz_controller::continuous_move_direction( std::string value, float sensitive )
{
	PTZSoap proxy;
	_ns1__ControlPTZ request;
	_ns1__ControlPTZResponse response;

	request.channel = 0;
	request.param = soap_new_ns1__ArrayOfParameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(proxy.soap, sizeof(ns1__Parameter_USCORET*)*2) );
	request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[0]->key = soap_strdup( proxy.soap, _moveSpeedType.c_str() );

	int real_sensitive = (int)get_sensitive_value( sensitive );
	char str_sensitive[50] = {0};
	snprintf( str_sensitive, sizeof(str_sensitive), "%d", real_sensitive );
	request.param->Parameter_USCORET[0]->value = soap_strdup( proxy.soap, str_sensitive );

	request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );	
	request.param->Parameter_USCORET[1]->key = soap_strdup( proxy.soap, _moveType.c_str() );
	request.param->Parameter_USCORET[1]->value = soap_strdup( proxy.soap, value.c_str() );

	request.param->__sizeParameter_USCORET = 2;

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__ControlPTZ( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short lge_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::stop_move( void )
{
	PTZSoap proxy;
	
	_ns1__Stop request;
	_ns1__StopResponse response;
	request.channel = 0;
	request.param	= soap_new_ns1__ArrayOfParameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(proxy.soap, sizeof(ns1__Parameter_USCORET*)*6) );
	request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[0]->key = soap_strdup( proxy.soap, "stop" );
	request.param->Parameter_USCORET[0]->value = soap_strdup( proxy.soap, "pan" );

	request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[1]->key = soap_strdup( proxy.soap, "stop" );
	request.param->Parameter_USCORET[1]->value = soap_strdup( proxy.soap, "tilt" );

	request.param->Parameter_USCORET[2] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[2]->key = soap_strdup( proxy.soap, "stop" );
	request.param->Parameter_USCORET[2]->value = soap_strdup( proxy.soap, "zoom" );

	request.param->Parameter_USCORET[3] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[3]->key = soap_strdup( proxy.soap, "stop" );
	request.param->Parameter_USCORET[3]->value = soap_strdup( proxy.soap, "focus" );

	request.param->Parameter_USCORET[4] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[4]->key = soap_strdup( proxy.soap, "stop" );
	request.param->Parameter_USCORET[4]->value = soap_strdup( proxy.soap, "iris" );

/*
	request.param->Parameter_USCORET[5] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[5]->key = soap_strdup( proxy.soap, "stop" );
	request.param->Parameter_USCORET[5]->value = soap_strdup( proxy.soap, "move" );
*/

	request.param->__sizeParameter_USCORET = 5;

	int ret;
	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", _xaddr );
	proxy.endpoint = szPTZUrl;
	proxy.soap->userid = soap_strdup( proxy.soap, _username );
	proxy.soap->passwd = soap_strdup( proxy.soap, _password );
	ret = proxy.__ns2__Stop( &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( proxy.soap );
		return VMS_PTZ_FAIL;
	}
	else
	{
		return VMS_PTZ_SUCCESS;
	}
}

unsigned short lge_ptz_controller::get_status( float &pan, float &tilt, float &zoom )
{
	pan = 0;
	tilt = 0;
	zoom = 0;
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}


unsigned int lge_ptz_controller::get_sensitive_value( float sensitive )
{
	if( sensitive<_min ) 
		return SENSITIVE_MIN;
	else if( sensitive>_max ) 
		return SENSITIVE_MAX;
	else
	{
		float real_sensitive = (float(SENSITIVE_MAX-SENSITIVE_MIN)/float(_max-_min))*float(sensitive);
		return real_sensitive;
	}
}


base_ptz_controller* create( void )
{
	return new lge_ptz_controller();
}

void destroy( base_ptz_controller **ptz_controller )
{
	lge_ptz_controller *lge_controller = dynamic_cast<lge_ptz_controller*>( (*ptz_controller) );
	delete lge_controller;
	(*ptz_controller) = 0;
}

