#ifndef _PTZ_CONTROL_CLIENT_H_
#define _PTZ_CONTROL_CLIENT_H_
#include <ptz_controller.h>
#include "gsoap/soapH.h"
#include "gsoap/PTZSoap.nsmap"
#include "gsoap/soapPTZSoapProxy.h"
#include "soap_api_client.h"

class PTZSoap;
#if defined(WIN32)
class __declspec(dllexport) lge_ptz_controller : public base_ptz_controller, public soap_api_client
#else
class lge_ptz_controller : public base_ptz_controller, public soap_api_client
#endif
{
public:
	lge_ptz_controller( void );
	virtual ~lge_ptz_controller( void );

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