#include "platform.h"
#include <ptz_device_info.h>
#include "lge_ptz_controller.h"

#define SENSITIVE_MIN	1
#define SENSITIVE_MAX	1000

lge_ptz_controller::lge_ptz_controller( void )
	: _port_number(80)
{

}

lge_ptz_controller::~lge_ptz_controller( void )
{
}

char* lge_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LW9424_SOAP_API_461501120420][VENDOR];
}

char* lge_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LW9424_SOAP_API_461501120420][DEVICE];
}

char* lge_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LW9424_SOAP_API_461501120420][PROTOCOL];
}

char* lge_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LW9424_SOAP_API_461501120420][VERSION];
}

unsigned short lge_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[LGE_LW9424_SOAP_API_461501120420][VENDOR];
}

unsigned short lge_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LW9424_SOAP_API_461501120420][DEVICE];
}

unsigned short lge_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LW9424_SOAP_API_461501120420][PROTOCOL];
}

unsigned short lge_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LW9424_SOAP_API_461501120420][VERSION];
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

unsigned short	lge_ptz_controller::set_user_password( char *user_password )
{
	if( user_password && (strlen(user_password)>0) ) 
	{
		strcpy( _user_password, user_password );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short	lge_ptz_controller::set_sensitive_boundary( unsigned int min, unsigned int max )
{
	_min = min;
	_max = max;
	return VMS_PTZ_SUCCESS;
}

unsigned short lge_ptz_controller::set_angle_inverse( bool inverse )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::relative_move( PTZ_TYPE_T move, unsigned short sensitive )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
	{
		set_wsse_token( _user_id, _user_password );
	}

	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(move) )
	{
		case PTZ_HOME :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_UP :
		{
			value = move_up( sensitive );
			break;
		}
		case PTZ_LEFT :
		{
			value = move_left( sensitive );
			break;
		}
		case PTZ_RIGHT :
		{
			value = move_right( sensitive );
			break;
		}
		case PTZ_DOWN :
		{
			value = move_down( sensitive );
			break;
		}
		case PTZ_LEFTUP :
		{
			value = move_left_up( sensitive );
			break;
		}
		case PTZ_RIGHTUP :
		{
			value = move_right_up( sensitive );
			break;
		}
		case PTZ_LEFTDOWN :
		{
			value = move_left_down( sensitive );
			break;
		}
		case PTZ_RIGHTDOWN :
		{
			value = move_right_down( sensitive );
			break;
		}
		case PTZ_STOP :
		{
			value = move_stop();
			break;
		}
		case PTZ_ZOOMIN :
		{
			value = zoom_in( sensitive );
			break;
		}
		case PTZ_ZO0MOUT :
		{
			value = zoom_out( sensitive );
			break;
		}
	}
	return value;
}

unsigned short lge_ptz_controller::relative_focus( PTZ_TYPE_T focus, unsigned short sensitive )
{
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(focus) )
	{
		case PTZ_FOCUS_NEAR :
		{
			value = focus_near( sensitive );
			break;
		}
		case PTZ_FOCUS_FAR :
		{
			value = focus_far( sensitive );
			break;
		}
	}
	return value;
}

unsigned short lge_ptz_controller::absolute_move( PTZ_TYPE_T move, unsigned short sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::absolute_move2( int pan_sensitive, int tilt_sensitive, int zoom_sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::absolute_focus( unsigned short sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::continuous_move( PTZ_TYPE_T move, unsigned short sensitive )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
	{
		set_wsse_token( _user_id, _user_password );
	}

	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;
	switch( UINT8(move) )
	{
		case PTZ_HOME :
		{
			value = VMS_PTZ_UNSUPPORTED_COMMAND;
			break;
		}
		case PTZ_UP :
		{
			value = continuous_move_up( sensitive );
			break;
		}
		case PTZ_LEFT :
		{
			value = continuous_move_left( sensitive );
			break;
		}
		case PTZ_RIGHT :
		{
			value = continuous_move_right( sensitive );
			break;
		}
		case PTZ_DOWN :
		{
			value = continuous_move_down( sensitive );
			break;
		}
		case PTZ_LEFTUP :
		{
			value = continuous_move_left_up( sensitive );
			break;
		}
		case PTZ_RIGHTUP :
		{
			value = continuous_move_right_up( sensitive );
			break;
		}
		case PTZ_LEFTDOWN :
		{
			value = continuous_move_left_down( sensitive );
			break;
		}
		case PTZ_RIGHTDOWN :
		{
			value = continuous_move_right_down( sensitive );
			break;
		}
		case PTZ_STOP :
		{
			value = move_stop();
			break;
		}
		case PTZ_ZOOMIN :
		{
			value = continuous_zoom_in( sensitive );
			break;
		}
		case PTZ_ZO0MOUT :
		{
			value = continuous_zoom_out( sensitive );
			break;
		}
	}
	return value;
}

unsigned short lge_ptz_controller::get_current_absolute_position( unsigned short &pan, unsigned short &tilt, unsigned short &zoom )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short	lge_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	PTZSoapProxy proxy;
	_ns1__GetPresetList request;
	request.soap_default( &proxy );
	_ns1__GetPresetListResponse response;
	response.soap_default( &proxy );
	request.channel = 0;

	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
	int ret = fnGetPresetList( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( &proxy );
		return VMS_PTZ_FAIL;
	}
	else
	{
		if( response.GetPresetListResult )
		{
			std::string preset_list = "";
			if( response.GetPresetListResult->__sizePreset_USCORET>0 )
			{
				(*length) = response.GetPresetListResult->__sizePreset_USCORET;
				(*aliases) = static_cast<char**>( malloc(sizeof(char**)*(*length)) );
				for( int index=0; index<response.GetPresetListResult->__sizePreset_USCORET; index++ )
					(*aliases)[index] = strdup( response.GetPresetListResult->Preset_USCORET[index]->alias );
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

unsigned short	lge_ptz_controller::add_preset( char *alias )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	int sequence = 0;
	{
		PTZSoapProxy proxy;
		_ns1__GetPresetList request;
		request.soap_default( &proxy );
		_ns1__GetPresetListResponse response;
		response.soap_default( &proxy );
		request.channel = 0;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		int ret = fnGetPresetList( this, &proxy, szPTZUrl, _username, _password, &request, &response );
		if( ret!=SOAP_OK )
		{
			full_fault_code( &proxy );
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
		PTZSoapProxy proxy;
	
		_ns1__AddPreset request;
		_ns1__AddPresetResponse response;
		request.channel = 0;
		request.preset				= soap_new_ns1__Preset_USCORET( &proxy, -1 );
		request.preset->alias		= soap_strdup( &proxy, alias );
		request.preset->sequence	= sequence;
		request.preset->presetSpeed	= 80;
		request.preset->parkTime	= 3;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		int ret = fnAddPreset( this, &proxy, szPTZUrl, _username, _password, &request, &response );

		if( ret!=SOAP_OK )
		{
			full_fault_code( &proxy );
			return VMS_PTZ_FAIL;
		}
		else
		{
			return VMS_PTZ_SUCCESS;
		}
	}
}

unsigned short	lge_ptz_controller::remove_preset( char *alias )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	PTZSoapProxy proxy;
	
	_ns1__RemovePreset request;
	_ns1__RemovePresetResponse response;
	request.channel = 0;
	request.alias	= soap_strdup( &proxy, alias );


	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
	int ret = fnRemovePreset( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( &proxy );
		return VMS_PTZ_FAIL;
	}
	else
	{
		return VMS_PTZ_SUCCESS;
	}
}

unsigned short	lge_ptz_controller::goto_preset( char *alias )
{
	if( (_user_id!=NULL && _user_password!=NULL) && (strlen(_user_id)>0 && strlen(_user_password)>0) )
		set_wsse_token( _user_id, _user_password );

	PTZSoapProxy proxy;
	_ns1__GotoPreset request;
	_ns1__GotoPresetResponse response;
	request.channel = 0;
	request.alias	= soap_strdup( &proxy, alias );

	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
	int ret = fnGotoPreset( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	if( ret!=SOAP_OK )
	{
		full_fault_code( &proxy );
		return VMS_PTZ_FAIL;
	}
	else
	{
		return VMS_PTZ_SUCCESS;
	}
}

//private function
unsigned short  lge_ptz_controller::move_home( unsigned short sensitive )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short  lge_ptz_controller::move_up( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "up" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::move_left( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "left" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::move_right( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "right" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::move_down( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "down" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::move_left_up( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "upleft" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::move_right_up( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "upright" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::move_left_down( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "downleft" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::move_right_down( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "downright" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::zoom_in( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "zoomspeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "zoom" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "tele" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::zoom_out( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "zoomspeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "zoom" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "wide" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_move_up( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "1" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "up" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_move_left( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "left" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_move_right( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "right" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_move_down( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "down" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_move_left_up( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "upleft" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_move_right_up( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "upright" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_move_left_down( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "downleft" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_move_right_down( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "movespeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "move" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "downright" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_zoom_in( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "zoomspeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "zoom" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "tele" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::continuous_zoom_out( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "zoomspeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "zoom" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "wide" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}
	return VMS_PTZ_SUCCESS;
}

unsigned short	lge_ptz_controller::move_stop( void )
{
	PTZSoapProxy proxy;
	_ns1__Stop request;
	_ns1__StopResponse response;
	request.channel = 0;
	request.param	= soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
	request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*6) );
	request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
	request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "stop" );
	request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "pan" );

	request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
	request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "stop" );
	request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "tilt" );

	request.param->Parameter_USCORET[2] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
	request.param->Parameter_USCORET[2]->key = soap_strdup( &proxy, "stop" );
	request.param->Parameter_USCORET[2]->value = soap_strdup( &proxy, "zoom" );

	request.param->Parameter_USCORET[3] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
	request.param->Parameter_USCORET[3]->key = soap_strdup( &proxy, "stop" );
	request.param->Parameter_USCORET[3]->value = soap_strdup( &proxy, "focus" );

	request.param->Parameter_USCORET[4] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
	request.param->Parameter_USCORET[4]->key = soap_strdup( &proxy, "stop" );
	request.param->Parameter_USCORET[4]->value = soap_strdup( &proxy, "iris" );

	request.param->Parameter_USCORET[5] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
	request.param->Parameter_USCORET[5]->key = soap_strdup( &proxy, "stop" );
	request.param->Parameter_USCORET[5]->value = soap_strdup( &proxy, "move" );

	request.param->__sizeParameter_USCORET = 6;

	CHAR szPTZUrl[100] = {0,};
	snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
	fnStop( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::focus_far( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "focusspeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "focus" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "far" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned short  lge_ptz_controller::focus_near( unsigned short sensitive )
{
	{
		PTZSoapProxy proxy;
		_ns1__ControlPTZ request;
		_ns1__ControlPTZResponse response;

		request.channel = 0;
		request.param = soap_new_ns1__ArrayOfParameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(&proxy, sizeof(ns1__Parameter_USCORET*)*2) );
		request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[0]->key = soap_strdup( &proxy, "focusspeed" );
		request.param->Parameter_USCORET[0]->value = soap_strdup( &proxy, "100" );

		request.param->Parameter_USCORET[1] = soap_new_ns1__Parameter_USCORET( &proxy, -1 );
		request.param->Parameter_USCORET[1]->key = soap_strdup( &proxy, "focus" );
		request.param->Parameter_USCORET[1]->value = soap_strdup( &proxy, "near" );

		request.param->__sizeParameter_USCORET = 2;

		CHAR szPTZUrl[100] = {0,};
		snprintf( szPTZUrl, sizeof(szPTZUrl), "http://%s/soap", this->_xaddr );
		fnControlPTZ( this, &proxy, szPTZUrl, _username, _password, &request, &response );
	}

	unsigned int realSensitive = get_sensitive_value( sensitive );
	sleep_millisecond( realSensitive );
	move_stop();
	return VMS_PTZ_SUCCESS;
}

unsigned int lge_ptz_controller::get_sensitive_value( unsigned short sensitive )
{
	// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
	if( sensitive<_min ) return SENSITIVE_MIN;
	else if( sensitive>_max ) return SENSITIVE_MAX;
	else
	{
		float ratio = float(SENSITIVE_MAX-SENSITIVE_MIN)/float(_max-_min);
		unsigned short realSensitive = unsigned short( ceil(ratio*sensitive) );
		return realSensitive;
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

