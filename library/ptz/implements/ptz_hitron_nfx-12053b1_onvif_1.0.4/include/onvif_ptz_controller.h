#ifndef _PTZ_CONTROL_CLIENT_H_
#define _PTZ_CONTROL_CLIENT_H_
#include <ptz_controller.h>
#include "gsoap/ptzH.h"
#include "onvif_client.h"
#include "gsoap/ptzPTZBindingProxy.h"
#include "gsoap/wsseapi.h"
#include <vector>
#include <map>

#define SOAP_CLIENT_FUNC(fname,onvif_client,onvif_proxy,format,reqtype,restype) \
	int fn##fname( onvif_client *client, onvif_proxy *proxy, format service_url, reqtype req, restype res ) \
	{ \
		int ret; \
		if( strlen(_host)<1 ) \
			return SOAP_ERR; \
		memset( _url, 0x00, sizeof(_url) ); \
		snprintf( _url, sizeof(_url), service_url, _host, _port_number ); \
		proxy->soap_endpoint = _url; \
		if( _user_id!=0 && _user_password!=0 && strlen(_user_id) && strlen(_user_password) ) \
		{ \
			soap_register_plugin( proxy, soap_wsse ); \
			soap_wsse_add_UsernameTokenDigest( proxy, "Username", _user_id, _user_password ); \
		} \
		ret = proxy->fname(req,res); \
		if( _user_id!=0 && _user_password!=0 && strlen(_user_id) && strlen(_user_password) ) \
		{ \
			soap_wsse_delete_Security( proxy ); \
		} \
		if( ret!=SOAP_OK ) \
		{ \
			client->fill_fault_code( proxy ); \
		} \
		return ret; \
	} \

#if defined(WIN32)
class __declspec(dllexport) onvif_ptz_controller : public base_ptz_controller, public onvif_client
#else
class onvif_ptz_controller : public base_ptz_controller, public onvif_client
#endif
{
public:
	onvif_ptz_controller( void );
	~onvif_ptz_controller( void );

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
	unsigned short	set_user_password( char *password );
	unsigned short	set_angle_inverse( bool inverse );
	unsigned short	set_pan_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short	set_tilt_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short	set_zoom_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short	set_speed_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short	set_profile_token( char *token );

	unsigned short	is_enable_osd_menu( void );
	unsigned short	is_enable_home_position( void );
	unsigned short	is_enable_preset( void );
	unsigned short	is_enable_preset_tour( void );
	unsigned short	is_enable_continuous_move( void );
	unsigned short	is_enable_relative_move( void );
	unsigned short	is_enable_absolute_move( void );
	unsigned short	is_preset_name_numberic( void );
	unsigned short	is_preset_tour_name_numberic( void );
	unsigned short	is_preset_name_changable( void );
	unsigned short	is_preset_tour_name_changable( void );

	unsigned short	osd_menu( PTZ_OSD_MENU_TYPE_T osd );
	unsigned short	goto_home_position( float speed=0.0 );
	unsigned short	set_home_position( void );

	unsigned short	get_preset_list( char ***aliases, int *length );
	unsigned short	add_preset( char *alias );
	unsigned short	remove_preset( char *alias );
	unsigned short	goto_preset( char *alias );
	unsigned short	get_preset_list2( int **aliases, int *length );
	unsigned short	add_preset2( int &alias );
	unsigned short	remove_preset2( int alias );
	unsigned short	goto_preset2( int alias );

	unsigned short	add_preset_tour( char *tour_name, int size=0 );
	unsigned short	remove_preset_tour( char *tour_name );
	unsigned short	operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
	unsigned short	add_preset_tour2( int &tour_name, int size=0 );
	unsigned short	remove_preset_tour2( int tour_name );
	unsigned short	operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
	unsigned short	set_preset_tour( PTZ_TOUR_T *tour );
	unsigned short	get_preset_tour( PTZ_TOUR_T *tour );
	unsigned short	get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours );

	unsigned short	continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout );
	unsigned short	continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout );
	unsigned short	relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 );
	unsigned short	relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed=0.0 );
	unsigned short	absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 );
	unsigned short	stop_move( void );
	unsigned short	get_status( float &pan, float &tilt, float &zoom );


private:
	SOAP_CLIENT_FUNC( GetServiceCapabilities, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GetServiceCapabilities*, _tptz__GetServiceCapabilitiesResponse* )
	SOAP_CLIENT_FUNC( GetConfigurations, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GetConfigurations*, _tptz__GetConfigurationsResponse* )
	SOAP_CLIENT_FUNC( GetPresets, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GetPresets*, _tptz__GetPresetsResponse* )
	SOAP_CLIENT_FUNC( SetPreset, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__SetPreset*, _tptz__SetPresetResponse* )
	SOAP_CLIENT_FUNC( RemovePreset, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__RemovePreset*, _tptz__RemovePresetResponse* )
	SOAP_CLIENT_FUNC( GotoPreset, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GotoPreset*, _tptz__GotoPresetResponse* )
	SOAP_CLIENT_FUNC( GetStatus, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GetStatus*, _tptz__GetStatusResponse* )
	SOAP_CLIENT_FUNC( GetConfiguration, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GetConfiguration*, _tptz__GetConfigurationResponse* )
	SOAP_CLIENT_FUNC( GetNodes, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GetNodes*, _tptz__GetNodesResponse* )
	SOAP_CLIENT_FUNC( GetNode, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GetNode*, _tptz__GetNodeResponse* )
	SOAP_CLIENT_FUNC( SetConfiguration, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__SetConfiguration*, _tptz__SetConfigurationResponse* )
	SOAP_CLIENT_FUNC( GetConfigurationOptions, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GetConfigurationOptions*, _tptz__GetConfigurationOptionsResponse* )
	SOAP_CLIENT_FUNC( GotoHomePosition, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__GotoHomePosition*, _tptz__GotoHomePositionResponse* )
	SOAP_CLIENT_FUNC( SetHomePosition, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__SetHomePosition*, _tptz__SetHomePositionResponse* )
	SOAP_CLIENT_FUNC( ContinuousMove, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__ContinuousMove*, _tptz__ContinuousMoveResponse* )
	SOAP_CLIENT_FUNC( RelativeMove, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__RelativeMove*, _tptz__RelativeMoveResponse* )
	SOAP_CLIENT_FUNC( SendAuxiliaryCommand, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__SendAuxiliaryCommand*, _tptz__SendAuxiliaryCommandResponse* )
	SOAP_CLIENT_FUNC( AbsoluteMove, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__AbsoluteMove*, _tptz__AbsoluteMoveResponse* )
	SOAP_CLIENT_FUNC( Stop, onvif_ptz_controller, PTZBindingProxy, char*, _tptz__Stop*, _tptz__StopResponse* )

	unsigned short	query_limits( void );
	unsigned short	query_status( float &pan, float &tilt, float &zoom );	

	float			get_speed_sensitive_value( float sensitive );
	float           get_speed_quasi_sensitive_value( float real_sensitive );
	float			get_rpan_sensitive_value( float sensitive );
	float			get_rtilt_sensitive_value( float sensitive );
	float			get_rzoom_sensitive_value( float sensitive );

	float			get_apan_sensitive_value( float sensitive );
	float			get_atilt_sensitive_value( float sensitive );
	float			get_azoom_sensitive_value( float sensitive );

	float			get_apan_quasi_sensitive_value( float sensitive );
	float			get_atilt_quasi_sensitive_value( float sensitive );
	float			get_azoom_quasi_sensitive_value( float sensitive );

	unsigned short  get_preset_tour_list(void);
	void			split2vector( std::string origin, std::string token, std::vector<std::string> *devided );
	void			split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided );	
	std::string     get_preset_tour_name(std::string tour_no);
	std::string     get_preset_tour_id( char *tour_name);
	std::string     get_preset_name_by_id(char *preset_id );
	unsigned short  get_preset_list_map();
	std::string     get_preset_tour_list_str(void);
	float           get_continuous_sensitive_value( float sensitive );

private:
	char				_host[MAX_PATH];
	unsigned short		_port_number;
	char				_url[MAX_PATH];
	char				_user_id[MAX_PATH];
	char				_user_password[MAX_PATH];
	char				_profile_token[MAX_PATH];

	unsigned short		_pan_number_place;
	float				_pan_min;
	float				_pan_max;
	unsigned short		_tilt_number_place;
	float				_tilt_min;
	float				_tilt_max;
	unsigned short		_zoom_number_place;
	float				_zoom_min;
	float				_zoom_max;
	unsigned short		_speed_number_place;
	float				_speed_min;
	float				_speed_max;

	float				_min_pan;
	float				_max_pan;
	float				_min_tilt;
	float				_max_tilt;
	float				_min_zoom;
	float				_max_zoom;
	float				_min_speed;
	float				_max_speed;
	float				_min_cspeed;
	float				_max_cspeed;

	bool				_is_limits_queried;
	bool				_is_inverse;
	std::map<std::string, std::string>	_preset_map;
	std::map< std::string, std::string > _tour_list;	                                     
};

#if defined(WIN32)
extern "C" __declspec(dllexport) base_ptz_controller* create( void );
extern "C" __declspec(dllexport) void destroy( base_ptz_controller **ptz_controller );
#else
extern "C" base_ptz_controller* create( void );
extern "C" void destroy( base_ptz_controller **ptz_controller );
#endif

#endif
