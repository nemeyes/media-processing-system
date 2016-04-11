#ifndef _PTZ_CONTROL_CLIENT_H_
#define _PTZ_CONTROL_CLIENT_H_
#include <ptz_controller.h>
#include "gsoap/soapH.h"
#include "gsoap/soapPTZSoapProxy.h"
#include "soap_api_client.h"

#define SOAP_CLIENT_FUNC(fname,onvif_client,onvif_proxy,reqtype,restype) \
	int fn##fname( onvif_client *client, onvif_proxy *proxy, char *xaddr, char* username, char* password, reqtype req, restype res ) \
	{ \
		int ret; \
		if( xaddr==0 ) return SOAP_ERR; \
		proxy->soap_endpoint = xaddr; \
		/*if( username!=0 && password!=0 ) \
		{ \
			soap_register_plugin( proxy, soap_wsse ); \
			soap_wsse_add_UsernameTokenDigest( proxy, "Username", username, password ); \
		}*/ \
		ret = proxy->fname(req,res); \
		/*if( username!=0 && password!=0 ) \
		{ \
			soap_wsse_delete_Security( proxy ); \
		}*/ \
		if( ret!=SOAP_OK ) \
		{ \
			client->full_fault_code( proxy ); \
		} \
		return ret; \
	} \


class PTZSoapProxy;
#if defined(WIN32)
class __declspec(dllexport) lge_ptz_controller : public base_ptz_controller, public soap_api_client
#else
class lge_ptz_controller : public base_ptz_controller, public soap_api_client
#endif
{
public:
	lge_ptz_controller( void );
	~lge_ptz_controller( void );

	char*			get_vendor_name( void );
	char*			get_vendor_device_name( void );
	char*			get_vendor_device_protocol_name( void );
	char*			get_vendor_device_version_name( void );

	unsigned short	get_vendor_id( void );
	unsigned short	get_vendor_device_id( void );
	unsigned short	get_vendor_device_protocol_id( void );
	unsigned short	get_vendor_device_version_id( void );

	unsigned short	set_host_name( char *hostname );
	unsigned short	set_port_number( unsigned short port_number );
	unsigned short	set_user_id( char *user_id );
	unsigned short	set_user_password( char *szPassword );
	unsigned short	set_sensitive_boundary( unsigned int min, unsigned int max );
	unsigned short	set_angle_inverse( bool inverse );

	unsigned short	relative_move( PTZ_TYPE_T move, unsigned short sensitive );
	unsigned short	relative_focus( PTZ_TYPE_T focus, unsigned short sensitive );
	unsigned short	absolute_move( PTZ_TYPE_T move, unsigned short sensitive );
	unsigned short	absolute_move2( int pan_sensitive, int tilt_sensitive, int zoom_sensitive );
	unsigned short	absolute_focus( unsigned short sensitive );
	unsigned short	continuous_move( PTZ_TYPE_T move, unsigned short sensitive );
	unsigned short	get_current_absolute_position( unsigned short &pan, unsigned short &tilt, unsigned short &zoom );

	unsigned short	get_preset_list( char ***aliases, int *length );
	unsigned short	add_preset( char *alias );
	unsigned short	remove_preset( char *alias );
	unsigned short	goto_preset( char *alias );

private:
	SOAP_CLIENT_FUNC( GetPTZProtocolList, lge_ptz_controller, PTZSoapProxy, _ns1__GetPTZProtocolList*, _ns1__GetPTZProtocolListResponse* )
	SOAP_CLIENT_FUNC( GetPTZProtocol, lge_ptz_controller, PTZSoapProxy, _ns1__GetPTZProtocol*, _ns1__GetPTZProtocolResponse* )
	SOAP_CLIENT_FUNC( SetPTZProtocol, lge_ptz_controller, PTZSoapProxy, _ns1__SetPTZProtocol*, _ns1__SetPTZProtocolResponse* )
	SOAP_CLIENT_FUNC( AddPTZProtocol, lge_ptz_controller, PTZSoapProxy, _ns1__AddPTZProtocol*, _ns1__AddPTZProtocolResponse* )
	SOAP_CLIENT_FUNC( RemovePTZProtocol, lge_ptz_controller, PTZSoapProxy, _ns1__RemovePTZProtocol*, _ns1__RemovePTZProtocolResponse* )
	SOAP_CLIENT_FUNC( GetPTZProtocolInfo, lge_ptz_controller, PTZSoapProxy, _ns1__GetPTZProtocolInfo*, _ns1__GetPTZProtocolInfoResponse* )
	SOAP_CLIENT_FUNC( GetPTZCommandList, lge_ptz_controller, PTZSoapProxy, _ns1__GetPTZCommandList*, _ns1__GetPTZCommandListResponse* )
	SOAP_CLIENT_FUNC( ControlPTZ, lge_ptz_controller, PTZSoapProxy, _ns1__ControlPTZ*, _ns1__ControlPTZResponse* )
	SOAP_CLIENT_FUNC( Stop, lge_ptz_controller, PTZSoapProxy, _ns1__Stop*, _ns1__StopResponse* )
	SOAP_CLIENT_FUNC( GetPresetList, lge_ptz_controller, PTZSoapProxy, _ns1__GetPresetList*, _ns1__GetPresetListResponse* )
	SOAP_CLIENT_FUNC( AddPreset, lge_ptz_controller, PTZSoapProxy, _ns1__AddPreset*, _ns1__AddPresetResponse* )
	SOAP_CLIENT_FUNC( GotoPreset, lge_ptz_controller, PTZSoapProxy, _ns1__GotoPreset*, _ns1__GotoPresetResponse* )
	SOAP_CLIENT_FUNC( RemovePreset, lge_ptz_controller, PTZSoapProxy, _ns1__RemovePreset*, _ns1__RemovePresetResponse* )
	SOAP_CLIENT_FUNC( GetPresetTour, lge_ptz_controller, PTZSoapProxy, _ns1__GetPresetTour*, _ns1__GetPresetTourResponse* )
	SOAP_CLIENT_FUNC( SetPresetTour, lge_ptz_controller, PTZSoapProxy, _ns1__SetPresetTour*, _ns1__SetPresetTourResponse* )
	SOAP_CLIENT_FUNC( PlayPresetTour, lge_ptz_controller, PTZSoapProxy, _ns1__PlayPresetTour*, _ns1__PlayPresetTourResponse* )
	SOAP_CLIENT_FUNC( StopPresetTour, lge_ptz_controller, PTZSoapProxy, _ns1__StopPresetTour*, _ns1__StopPresetTourResponse* )
	SOAP_CLIENT_FUNC( StartPatternRecording, lge_ptz_controller, PTZSoapProxy, _ns1__StartPatternRecording*, _ns1__StartPatternRecordingResponse* )
	SOAP_CLIENT_FUNC( StopPatternRecording, lge_ptz_controller, PTZSoapProxy, _ns1__StopPatternRecording*, _ns1__StopPatternRecordingResponse* )
	SOAP_CLIENT_FUNC( PlayRecordedPattern, lge_ptz_controller, PTZSoapProxy, _ns1__PlayRecordedPattern*, _ns1__PlayRecordedPatternResponse* )
	SOAP_CLIENT_FUNC( StopRecordedPattern, lge_ptz_controller, PTZSoapProxy, _ns1__StopRecordedPattern*, _ns1__StopRecordedPatternResponse* )
	SOAP_CLIENT_FUNC( GetPTZConfiguration, lge_ptz_controller, PTZSoapProxy, _ns1__GetPTZConfiguration*, _ns1__GetPTZConfigurationResponse* )
	SOAP_CLIENT_FUNC( SetPTZConfiguration, lge_ptz_controller, PTZSoapProxy, _ns1__SetPTZConfiguration*, _ns1__SetPTZConfigurationResponse* )
	SOAP_CLIENT_FUNC( GetPTZPort, lge_ptz_controller, PTZSoapProxy, _ns1__GetPTZPort*, _ns1__GetPTZPortResponse* )
	SOAP_CLIENT_FUNC( SetPTZPort, lge_ptz_controller, PTZSoapProxy, _ns1__SetPTZPort*, _ns1__SetPTZPortResponse* )
	SOAP_CLIENT_FUNC( PTZByPass, lge_ptz_controller, PTZSoapProxy, _ns1__PTZByPass*, _ns1__PTZByPassResponse* )
	SOAP_CLIENT_FUNC( OSDMenu, lge_ptz_controller, PTZSoapProxy, _ns1__OSDMenu*, _ns1__OSDMenuResponse* )


	unsigned short	move_home( unsigned short sensitive );

	unsigned short	move_up( unsigned short sensitive );
	unsigned short	move_left( unsigned short sensitive );
	unsigned short	move_right( unsigned short sensitive );
	unsigned short	move_down( unsigned short sensitive );
	unsigned short	move_left_up( unsigned short sensitive );
	unsigned short	move_right_up( unsigned short sensitive );
	unsigned short	move_left_down( unsigned short sensitive );
	unsigned short	move_right_down( unsigned short sensitive );
	unsigned short	zoom_in( unsigned short sensitive );
	unsigned short	zoom_out( unsigned short sensitive );

	unsigned short	continuous_move_up( unsigned short sensitive );
	unsigned short	continuous_move_left( unsigned short sensitive );
	unsigned short	continuous_move_right( unsigned short sensitive );
	unsigned short	continuous_move_down( unsigned short sensitive );
	unsigned short	continuous_move_left_up( unsigned short sensitive );
	unsigned short	continuous_move_right_up( unsigned short sensitive );
	unsigned short	continuous_move_left_down( unsigned short sensitive );
	unsigned short	continuous_move_right_down( unsigned short sensitive );
	unsigned short	continuous_zoom_in( unsigned short sensitive );
	unsigned short	continuous_zoom_out( unsigned short sensitive );

	unsigned short	move_stop( void );

	unsigned short	focus_far( unsigned short sensitive );
	unsigned short	focus_near( unsigned short sensitive );
	unsigned int	get_sensitive_value( unsigned short sensitive );

private:
	char				_hostname[MAX_PATH];
	unsigned int		_port_number;
	char				_user_id[MAX_PATH];
	char				_user_password[MAX_PATH];
	unsigned int		_min;
	unsigned int		_max;

};

#if defined(WIN32)
extern "C" __declspec(dllexport) base_ptz_controller* create( void );
extern "C" __declspec(dllexport) void destroy( base_ptz_controller **ptz_controller );
#else
extern "C" base_ptz_controller* create( void );
extern "C" void destroy( base_ptz_controller **ptz_controller );
#endif

#endif