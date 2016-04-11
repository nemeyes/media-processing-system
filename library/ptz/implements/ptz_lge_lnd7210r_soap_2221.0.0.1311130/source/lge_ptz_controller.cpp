#include "platform.h"
#include <ptz_device_info.h>
#include "lge_ptz_controller.h"
#include "http_client.h"

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
	return VMS_PTZ_DEVICE_INFO[LGE_LND7210R_SOAP_2221_0_0_1311130][VENDOR];
}

char* lge_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LND7210R_SOAP_2221_0_0_1311130][DEVICE];
}

char* lge_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LND7210R_SOAP_2221_0_0_1311130][PROTOCOL];
}

char* lge_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGE_LND7210R_SOAP_2221_0_0_1311130][VERSION];
}

unsigned short lge_ptz_controller::get_vendor_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LND7210R_SOAP_2221_0_0_1311130][VENDOR];
}

unsigned short lge_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LND7210R_SOAP_2221_0_0_1311130][DEVICE];
}

unsigned short lge_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LND7210R_SOAP_2221_0_0_1311130][PROTOCOL];
}

unsigned short lge_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGE_LND7210R_SOAP_2221_0_0_1311130][VERSION];
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
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::is_enable_preset( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::is_enable_preset_tour( void )
{
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
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::is_preset_tour_name_numberic( void )
{
	return VMS_PTZ_FALSE;
}

unsigned short lge_ptz_controller::is_preset_name_changable( void )
{
	return VMS_PTZ_FALSE;
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
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::set_home_position( void )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
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
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::add_preset2( int &alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::remove_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::goto_preset2( int alias )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
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
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	return VMS_PTZ_UNSUPPORTED_COMMAND;
}

unsigned short lge_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	unsigned short value = VMS_PTZ_SUCCESS;

	if(pan_sensitive==0 && tilt_sensitive==0&&zoom_sensitive==0) //Á¤Áö
	{
		value = stop_move();
	}
	else if(pan_sensitive!=0 && tilt_sensitive==0)//ÁÂ¿ì
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
	else if(tilt_sensitive!=0&&pan_sensitive==0)//»óÇÏ
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
	else if(pan_sensitive<0&&tilt_sensitive>0)//ÁÂ»ó
	{		
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTUP,  (abs(pan_sensitive)+abs(tilt_sensitive))/2, 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive<0)//¿ìÇÏ
	{		
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTDOWN,  (abs(pan_sensitive)+abs(tilt_sensitive))/2, 0);
	}
	else if(pan_sensitive<0&&tilt_sensitive<0)//ÁÂÇÏ
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_LEFTDOWN,  (abs(pan_sensitive)+abs(tilt_sensitive))/2, 0);
	}
	else if(pan_sensitive>0&&tilt_sensitive>0)//¿ì»ó
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_RIGHTUP,  (abs(pan_sensitive)+abs(tilt_sensitive))/2, 0);
	}
	else if(zoom_sensitive>0)//ÁÜÀÎ
	{
		value = continuous_move( PTZ_CONTINUOUS_MOVE_ZOOMIN, zoom_sensitive, 0);	
	}
	else if(zoom_sensitive<0)//ÁÜ¾Æ¿ô
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
			d_value   = "Up";		
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFT :
		{
			_moveType = "Move";
			d_value   = "Left";	
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHT :
		{
			_moveType = "Move";
			d_value   = "Right";		
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_DOWN :
		{
			_moveType = "Move";
			d_value   = "Down";		
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTUP :
		{
			_moveType = "Move";
			d_value   = "LeftUp";		
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTUP :
		{
			_moveType = "Move";
			d_value   = "RightUp";	
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_LEFTDOWN :
		{
			_moveType = "Move";
			d_value   = "LeftDown";	
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_RIGHTDOWN :
		{
			_moveType = "Move";
			d_value   = "RightDown";
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZOOMIN :
		{
			_moveType = "Zoom";
			d_value   = "tele";			
			value = VMS_PTZ_SUCCESS;
			break;
		}
		case PTZ_CONTINUOUS_MOVE_ZO0MOUT :
		{
			_moveType = "Zoom";
			d_value   = "wide";			
			value = VMS_PTZ_SUCCESS;
			break;
		}
	}
    //2¹øÈ£ÃâÇØ¾ß continuous°¡ µÊ
	continuous_move_direction(d_value);			
    continuous_move_direction(d_value);

	return value;
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

unsigned short  lge_ptz_controller::continuous_move_direction( std::string value )
{
	PTZSoap proxy;
	_ns1__ControlPTZ request;
	_ns1__ControlPTZResponse response;

	request.channel = 0;
	request.param = soap_new_ns1__ArrayOfParameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(proxy.soap, sizeof(ns1__Parameter_USCORET*)*1) );
	request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[0]->key = soap_strdup( proxy.soap, _moveType.c_str() );
	request.param->Parameter_USCORET[0]->value = soap_strdup( proxy.soap, value.c_str() );
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

unsigned short lge_ptz_controller::stop_move( void )
{
	PTZSoap proxy;
	
	_ns1__Stop request;
	_ns1__StopResponse response;
	request.channel = 0;
	request.param	= soap_new_ns1__ArrayOfParameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET = static_cast<ns1__Parameter_USCORET**>( soap_malloc(proxy.soap, sizeof(ns1__Parameter_USCORET*)*1) );
	request.param->Parameter_USCORET[0] = soap_new_ns1__Parameter_USCORET( proxy.soap, -1 );
	request.param->Parameter_USCORET[0]->key = soap_strdup( proxy.soap, "Stop" );
	request.param->Parameter_USCORET[0]->value = soap_strdup( proxy.soap, _moveType.c_str() );
	request.param->__sizeParameter_USCORET = 1;

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
		if(_moveType=="Zoom")	
		{
			zoom_save();
		}

		return VMS_PTZ_SUCCESS;
	}
}

unsigned short lge_ptz_controller::focus_push(void)
{ 	
	http_client client( _hostname, _port_number, "/httpapi" );
	client.put_variable( "SetWebOSD", "" );
	client.put_variable( "key", "ONEPUSHAF" );
	client.put_variable( "value", "NULL" );
	client.put_variable( "_", "1402455533836" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();
	return VMS_PTZ_SUCCESS;
}

unsigned short lge_ptz_controller::zoom_save(void)
{ 
	//httpapi?SetWebOSD&key=FOCUSSAVE&value=NULL&_=1402462497003
	http_client client( _hostname, _port_number, "/httpapi" );
	client.put_variable( "SetWebOSD", "" );
	client.put_variable( "key", "FOCUSSAVE" );
	client.put_variable( "value", "NULL" );
	client.put_variable( "_", "1402462497003" );

	if( !client.send_request(_user_id, _user_password) ) 
	{
		client.clear_variable();
		return VMS_PTZ_FAIL;
	}
	client.clear_variable();

	return VMS_PTZ_SUCCESS;
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

