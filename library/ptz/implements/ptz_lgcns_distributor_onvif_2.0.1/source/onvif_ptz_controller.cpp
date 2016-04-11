#include "platform.h"
#include <ptz_device_info.h>
#include "onvif_ptz_controller.h"

#if defined(USE_SPHERICAL_TRANSLATION)
#define SENSITIVE_MIN 0.0f
#define SENSITIVE_MAX 360.0f
#else
#define SENSITIVE_MIN 0.0f
#define SENSITIVE_MAX 1.0f
#endif

#define banollim(x,dig) (floor(float(x)*pow(10.0f,float(dig))+0.5f)/pow(10.0f,float(dig)))


onvif_ptz_controller::onvif_ptz_controller( void )
	: _is_inverse(false)
	, _port_number(80)
	, _is_limits_queried(false)
	, _is_properties_queried(false)
	, _is_osd_menu(false)
	, _is_home_position(false)
	, _is_preset(false)
	, _is_preset_tour(false)
	, _is_continuous_move(false)
	, _is_relative_move(false)
	, _is_absolute_move(false)
	, _is_numberic_preset_name(false)
	, _is_numberic_preset_tour_name(false)
	, _is_changable_preset_name(false)
	, _is_changable_preset_tour_name(false)
{
	memset( _profile_token, 0x00, MAX_PATH );
}


onvif_ptz_controller::~onvif_ptz_controller( void )
{

}

char* onvif_ptz_controller::get_vendor_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGCNS_DISTRIBUTOR_ONVIF_V_2_0_1][VENDOR];
}

char* onvif_ptz_controller::get_vendor_device_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGCNS_DISTRIBUTOR_ONVIF_V_2_0_1][DEVICE];
}

char* onvif_ptz_controller::get_vendor_device_protocol_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGCNS_DISTRIBUTOR_ONVIF_V_2_0_1][PROTOCOL];
}

char* onvif_ptz_controller::get_vendor_device_version_name( void )
{
	return VMS_PTZ_DEVICE_INFO[LGCNS_DISTRIBUTOR_ONVIF_V_2_0_1][VERSION];
}

unsigned short onvif_ptz_controller::get_vendor_id( void )
{

	return VMS_PTZ_DEVICE_ID[LGCNS_DISTRIBUTOR_ONVIF_V_2_0_1][VENDOR];
}

unsigned short onvif_ptz_controller::get_vendor_device_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGCNS_DISTRIBUTOR_ONVIF_V_2_0_1][DEVICE];
}

unsigned short onvif_ptz_controller::get_vendor_device_protocol_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGCNS_DISTRIBUTOR_ONVIF_V_2_0_1][PROTOCOL];
}

unsigned short onvif_ptz_controller::get_vendor_device_version_id( void )
{
	return VMS_PTZ_DEVICE_ID[LGCNS_DISTRIBUTOR_ONVIF_V_2_0_1][VERSION];
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
	return VMS_PTZ_FALSE; //onvif does not support OSD Menu Control
/*
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_osd_menu )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
*/
}

unsigned short onvif_ptz_controller::is_enable_home_position( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_home_position )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_enable_preset( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_preset )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_enable_preset_tour( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_preset_tour )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_enable_continuous_move( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_continuous_move )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_enable_relative_move( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_relative_move )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_enable_absolute_move( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_absolute_move )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_preset_name_numberic( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_numberic_preset_name )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_preset_tour_name_numberic( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_numberic_preset_tour_name )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_preset_name_changable( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_changable_preset_name )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
}

unsigned short onvif_ptz_controller::is_preset_tour_name_changable( void )
{
	if( query_properties()==VMS_PTZ_SUCCESS )
	{
		if( _is_changable_preset_tour_name )
			return VMS_PTZ_TRUE;
		else
			return VMS_PTZ_FALSE;
	}
	else
		return VMS_PTZ_FALSE;
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

	int result = fnGotoHomePosition( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::set_home_position( void )
{
	PTZBindingProxy proxy;
	_tptz__SetHomePosition request;
	_tptz__SetHomePositionResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnSetHomePosition( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::get_preset_list( char ***aliases, int *length )
{
	(*length)	= 0;
	(*aliases)	= 0;

	PTZBindingProxy proxy;
	_tptz__GetPresets request;
	_tptz__GetPresetsResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetPresets( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
	{
		(*length) = response.__sizePreset;
		if( (*length)>0 )
		{
			(*aliases) = static_cast<char**>( malloc(sizeof(char**)*(*length)) );
			for( int index=0; index<(*length); index++ )
				(*aliases)[index] = strdup( response.Preset[index]->Name );
		}
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::add_preset( char *alias )
{
	PTZBindingProxy proxy;
	_tptz__SetPreset request;
	_tptz__SetPresetResponse response;

	request.ProfileToken	= soap_strdup( &proxy, _profile_token );
	request.PresetName		= soap_strdup( &proxy, alias );
	int result = fnSetPreset( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::remove_preset( char *alias )
{
	PTZBindingProxy proxy;
	_tptz__GetPresets request;
	_tptz__GetPresetsResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetPresets( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
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

				result = fnRemovePreset( this, &proxy, "http://%s:%d/onvif/ptz_service", &request1, &response1 );
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
	PTZBindingProxy proxy;
	_tptz__GetPresets request;
	_tptz__GetPresetsResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetPresets( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
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

				result = fnGotoPreset( this, &proxy, "http://%s:%d/onvif/ptz_service", &request1, &response1 );
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

unsigned short onvif_ptz_controller::add_preset_tour( char *tour_name, int size )
{
	PTZBindingProxy proxy;
	_tptz__CreatePresetTour request;
	_tptz__CreatePresetTourResponse response;

	if( !tour_name || size<1 )
		return VMS_PTZ_FAIL;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnCreatePresetTour( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
	{
		if( strlen(response.PresetTourToken)>0 )
			strncpy( tour_name, response.PresetTourToken, size );
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::remove_preset_tour( char *tour_name )
{
	PTZBindingProxy proxy;
	_tptz__RemovePresetTour request;
	_tptz__RemovePresetTourResponse response;

	if( !tour_name || strlen(tour_name)<1 )
		return VMS_PTZ_FAIL;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnRemovePresetTour( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	PTZBindingProxy proxy;
	_tptz__OperatePresetTour request;
	_tptz__OperatePresetTourResponse response;

	if( !tour_name || strlen(tour_name)<1 )
		return VMS_PTZ_FAIL;

	request.ProfileToken	= soap_strdup( &proxy, _profile_token );
	request.PresetTourToken = soap_strdup( &proxy, tour_name );
	request.Operation		= tt__PTZPresetTourOperation( cmd );
	int result = fnOperatePresetTour( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
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
	PTZBindingProxy proxy;
	_tptz__ModifyPresetTour request;
	_tptz__ModifyPresetTourResponse response;

	if( !tour || !tour->tour_name || strlen(tour->tour_name)<1 )
		return VMS_PTZ_FAIL;

	request.ProfileToken										= soap_strdup( &proxy, _profile_token );
	request.PresetTour											= soap_new_tt__PresetTour( &proxy, -1 );
	request.PresetTour->Name									= soap_strdup( &proxy, tour->tour_name );
	request.PresetTour->AutoStart								= tour->tour_always_start;
	request.PresetTour->StartingCondition						= soap_new_tt__PTZPresetTourStartingCondition( &proxy, -1 );
	request.PresetTour->StartingCondition->Direction			= static_cast<tt__PTZPresetTourDirection*>( soap_malloc(&proxy, sizeof(tt__PTZPresetTourDirection)) );
	(*request.PresetTour->StartingCondition->Direction)			= tt__PTZPresetTourDirection(tour->tour_direction);
	request.PresetTour->StartingCondition->RecurringDuration	= soap_new_int64( &proxy );
	(*request.PresetTour->StartingCondition->RecurringDuration)	= tour->tour_recurring_duration;
	request.PresetTour->StartingCondition->RecurringTime		= soap_new_int( &proxy );
	(*request.PresetTour->StartingCondition->RecurringTime)		= tour->tour_recurring_time;
	
	if( tour->size_of_tour_spots>0 )
	{
		request.PresetTour->TourSpot								= static_cast<tt__PTZPresetTourSpot**>( soap_malloc(&proxy, sizeof(tt__PTZPresetTourSpot*)*tour->size_of_tour_spots) );
		request.PresetTour->__sizeTourSpot							= tour->size_of_tour_spots;
		for( int index=0; index<tour->size_of_tour_spots; index++ )
		{
			request.PresetTour->TourSpot[index]						= soap_new_tt__PTZPresetTourSpot( &proxy, -1 );

			request.PresetTour->TourSpot[index]->PresetDetail		= soap_new_tt__PTZPresetTourPresetDetail( &proxy, -1 );
			if( tour->tour_spots[index].preset_alias && strlen(tour->tour_spots[index].preset_alias)>0 )
			{
				request.PresetTour->TourSpot[index]->PresetDetail	= soap_new_tt__PTZPresetTourPresetDetail( &proxy, -1 );
				request.PresetTour->TourSpot[index]->PresetDetail->__union_PTZPresetTourPresetDetail			= SOAP_UNION__tt__union_PTZPresetTourPresetDetail_PresetToken;
				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PresetToken	= soap_strdup( &proxy, tour->tour_spots[index].preset_alias );
			}
			else
			{
				request.PresetTour->TourSpot[index]->PresetDetail	= soap_new_tt__PTZPresetTourPresetDetail( &proxy, -1 );
				request.PresetTour->TourSpot[index]->PresetDetail->__union_PTZPresetTourPresetDetail			= SOAP_UNION__tt__union_PTZPresetTourPresetDetail_PTZPosition;
				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition	= soap_new_tt__PTZVector( &proxy, -1 );
				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->PanTilt			= soap_new_tt__Vector2D( &proxy, -1 );
				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->PanTilt->space	= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace" );
				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->PanTilt->x		= tour->tour_spots[index].pan;
				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->PanTilt->y		= tour->tour_spots[index].tilt;

				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->Zoom			= soap_new_tt__Vector1D( &proxy, -1 );
				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->Zoom->space		= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace" );
				request.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->Zoom->x			= tour->tour_spots[index].zoom;
			}																								   
			
			request.PresetTour->TourSpot[index]->Speed					= soap_new_tt__PTZSpeed( &proxy, -1 );
			request.PresetTour->TourSpot[index]->Speed->PanTilt			= soap_new_tt__Vector2D( &proxy, -1 );
			request.PresetTour->TourSpot[index]->Speed->PanTilt->space	= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace" );
			request.PresetTour->TourSpot[index]->Speed->Zoom			= soap_new_tt__Vector1D( &proxy, -1 );
			request.PresetTour->TourSpot[index]->Speed->Zoom->space		= soap_strdup( &proxy, "http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace" );
			request.PresetTour->TourSpot[index]->Speed->PanTilt->x		= tour->tour_spots[index].speed;
			request.PresetTour->TourSpot[index]->Speed->PanTilt->y		= tour->tour_spots[index].speed;
			request.PresetTour->TourSpot[index]->Speed->Zoom->x			= tour->tour_spots[index].speed;

			request.PresetTour->TourSpot[index]->StayTime			= soap_new_int64( &proxy );
			(*request.PresetTour->TourSpot[index]->StayTime)		= tour->tour_spots[index].stay_time;
		}
	}
	int result = fnModifyPresetTour( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
		return VMS_PTZ_SUCCESS;
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	PTZBindingProxy proxy;
	_tptz__GetPresetTour request;
	_tptz__GetPresetTourResponse response;

	if( !tour || !tour->tour_name || strlen(tour->tour_name)<1 )
		return VMS_PTZ_FAIL;

	request.ProfileToken										= soap_strdup( &proxy, _profile_token );
	request.PresetTourToken										= soap_strdup( &proxy, tour->tour_name );
	int result = fnGetPresetTour( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK && response.PresetTour )
	{
		strncpy( tour->tour_name, response.PresetTour->Name, sizeof(tour->tour_name) );
		tour->tour_always_start	= response.PresetTour->AutoStart;
		if( response.PresetTour->StartingCondition )
		{
			if( response.PresetTour->StartingCondition->Direction )
				tour->tour_direction			= PTZ_TOUR_DIRECTION_TYPE_T( *response.PresetTour->StartingCondition->Direction );
			if( response.PresetTour->StartingCondition->RecurringDuration )
				tour->tour_recurring_duration	= *response.PresetTour->StartingCondition->RecurringDuration;
			if( response.PresetTour->StartingCondition->RecurringTime )
				tour->tour_recurring_time		= *response.PresetTour->StartingCondition->RecurringTime;
		}
			
		if( response.PresetTour->__sizeTourSpot>0 )
		{
			tour->size_of_tour_spots	= response.PresetTour->__sizeTourSpot;
			tour->tour_spots			= new PTZ_TOUR_SPOT_T[tour->size_of_tour_spots];	
			for( int index=0; index<response.PresetTour->__sizeTourSpot; index++ )
			{
				if( response.PresetTour->TourSpot[index]->PresetDetail )
				{
					if( response.PresetTour->TourSpot[index]->PresetDetail->__union_PTZPresetTourPresetDetail==SOAP_UNION__tt__union_PTZPresetTourPresetDetail_PresetToken )
					{
						if( response.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PresetToken )
							strncpy( tour->tour_spots[index].preset_alias, response.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PresetToken, sizeof(tour->tour_spots[index].preset_alias) );
					}
					if( response.PresetTour->TourSpot[index]->PresetDetail->__union_PTZPresetTourPresetDetail==SOAP_UNION__tt__union_PTZPresetTourPresetDetail_PTZPosition )
					{
						if( response.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition )
						{
							if( response.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->PanTilt )
							{
								tour->tour_spots[index].pan		= response.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->PanTilt->x;
								tour->tour_spots[index].tilt	= response.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->PanTilt->y;
							}
							if( response.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->Zoom )
							{
								tour->tour_spots[index].zoom	= response.PresetTour->TourSpot[index]->PresetDetail->union_PTZPresetTourPresetDetail.PTZPosition->Zoom->x;
							}
						}
					}
				}
				if( response.PresetTour->TourSpot[index]->StayTime )
					tour->tour_spots[index].stay_time = ( *response.PresetTour->TourSpot[index]->StayTime );

				if( response.PresetTour->TourSpot[index]->Speed )
				{
					if( response.PresetTour->TourSpot[index]->Speed->PanTilt )
					{
						tour->tour_spots[index].speed = response.PresetTour->TourSpot[index]->Speed->PanTilt->x;
						//tour->tour_spots[index].speed = response.PresetTour->TourSpot[index]->Speed->PanTilt->y;
					}
/*
					if( response.PresetTour->TourSpot[index]->Speed->Zoom )
					{
						tour->tour_spots[index].speed = response.PresetTour->TourSpot[index]->Speed->Zoom->x;
					}	
*/
				}
			}
		}
		return VMS_PTZ_SUCCESS;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	PTZBindingProxy proxy;
	_tptz__GetPresetTours request;
	_tptz__GetPresetToursResponse response;

	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetPresetTours( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
	if( result==SOAP_OK )
	{
		if( response.__sizePresetTour>0 )
		{
			(*size_of_tours)	= response.__sizePresetTour;
			*tour				= new PTZ_TOUR_T[response.__sizePresetTour];

			for( int index=0; index<response.__sizePresetTour; index++ )
				strncpy( (*tour+index)->tour_name, response.PresetTour[index]->Name, sizeof((*tour+index)->tour_name) );

			return VMS_PTZ_SUCCESS;
		}
		else
			return VMS_PTZ_FAIL;
	}
	else
		return VMS_PTZ_FAIL;
}

unsigned short onvif_ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	float real_pan_sensitive	= get_speed_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_speed_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_speed_sensitive_value( zoom_sensitive );

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

	int result = fnContinuousMove( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
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

unsigned short onvif_ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )
{
	float real_pan_sensitive	= get_rpan_sensitive_value( pan_sensitive );
	float real_tilt_sensitive	= get_rtilt_sensitive_value( tilt_sensitive );
	float real_zoom_sensitive	= get_rzoom_sensitive_value( zoom_sensitive );
	float real_speed			= get_speed_sensitive_value( speed );

	if( real_tilt_sensitive<0 )	//카메라가 tilt 방향으로 90도 이상 뒤로 넘어가는 것을 방지함.
	{
		float pan = 0.0;
		float tilt = 0.0;
		float zoom = 0.0;
		get_status( pan, tilt, zoom );
		if( tilt+real_tilt_sensitive<(_max_tilt-((_max_tilt-_min_tilt)/2)) )
			real_tilt_sensitive = 0;
	}

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


	int result = fnRelativeMove( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
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

	result = fnAbsoluteMove( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
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

	int result = fnStop( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
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

unsigned short  onvif_ptz_controller::query_properties( void )
{
	if( !_is_properties_queried )
	{
		IntelliVMSBindingProxy proxy;
		ns1__GetPTZPropertiesRequest request;
		ns1__GetPTZPropertiesResponse response;
		int result = fnGetPTZProperties( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
		if( result==SOAP_OK )
		{
			_is_osd_menu = response.osdMenu;
			_is_home_position = response.homePosition;
			_is_preset = response.preset;
			_is_preset_tour = response.presetTour;
			_is_continuous_move = response.continuousMove;
			_is_relative_move = response.relativeMove;
			_is_absolute_move = response.absoluteMove;
			_is_numberic_preset_name = response.numbericPresetName;
			_is_numberic_preset_tour_name = response.numbericPresetTourName;	
			_is_changable_preset_name = response.changablePresetName;
			_is_changable_preset_tour_name = response.changablePresetTourName;
			_is_properties_queried = true;
			return VMS_PTZ_SUCCESS;
		}
		else
			return VMS_PTZ_FAIL;
	}
	else
		return VMS_PTZ_SUCCESS;
}

unsigned short	onvif_ptz_controller::query_limits( void )
{
	if( _is_limits_queried ) 
		return VMS_PTZ_SUCCESS;
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	PTZBindingProxy proxy;
	_tptz__GetConfiguration request;
	_tptz__GetConfigurationResponse response;

	request.PTZConfigurationToken	= soap_strdup( &proxy, _profile_token );
	int result = fnGetConfiguration( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );

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
					_min_zoom = 0.0f;
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

unsigned short	onvif_ptz_controller::query_status( float &pan, float &tilt, float &zoom )
{
	char position[200] = {0,};
	unsigned short value = VMS_PTZ_UNSUPPORTED_COMMAND;

	PTZBindingProxy proxy;
	_tptz__GetStatus request;
	_tptz__GetStatusResponse response;
	request.ProfileToken = soap_strdup( &proxy, _profile_token );
	int result = fnGetStatus( this, &proxy, "http://%s:%d/onvif/ptz_service", &request, &response );
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
		return _min_tilt;
	else if( sensitive>_speed_max ) 
		return _max_tilt;
	else
	{
		float real_sensitive = float(sensitive*(_max_speed-_min_speed))/float(_speed_max-_speed_min);
		return real_sensitive;
	}
}

float	onvif_ptz_controller::get_rpan_sensitive_value( float sensitive )
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
		if( _pan_min==_min_pan && _pan_max==_max_pan )
			return sensitive;
		else
		{
			float real_sensitive = float(sensitive*(_max_pan-_min_pan))/float(_pan_max-_pan_min);
			return real_sensitive;
		}
	}
}

float	onvif_ptz_controller::get_rtilt_sensitive_value( float sensitive )
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
		if( _tilt_min==_min_tilt && _tilt_max==_max_tilt )
			return sensitive;
		else
		{
			float real_sensitive = (sensitive*(_max_tilt-_min_tilt))/(_tilt_max-_tilt_min);
			return real_sensitive;
		}
	}
}

float	onvif_ptz_controller::get_rzoom_sensitive_value( float sensitive )
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
			float real_sensitive = (sensitive)*(_max_zoom-_min_zoom)/(_zoom_max-_zoom_min);
			return real_sensitive;
		}
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