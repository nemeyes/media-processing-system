#ifndef _PTZ_CONTROL_CLIENT_H_
#define _PTZ_CONTROL_CLIENT_H_
#include <ptz_controller.h>
#include "gsoap/soapH.h"
#include "gsoap/PTZSoap.nsmap"
#include "gsoap/soapPTZSoapProxy.h"
#include <map>
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
	unsigned short  operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
	unsigned short	add_preset_tour2( int &tour_name, int size=0 );
	unsigned short	remove_preset_tour2( int tour_name );
	unsigned short  operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
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
	unsigned short	continuous_move_up( float sensitive );
	unsigned short	continuous_move_left( float sensitive );
	unsigned short	continuous_move_right( float sensitive );
	unsigned short	continuous_move_down( float sensitive );
	unsigned short	continuous_move_left_up( float sensitive );
	unsigned short	continuous_move_right_up( float sensitive );
	unsigned short	continuous_move_left_down( float sensitive );
	unsigned short	continuous_move_right_down( float sensitive );
	unsigned short	continuous_zoom_in( float sensitive );
	unsigned short	continuous_zoom_out( float sensitive );

	unsigned int	get_sensitive_value( float sensitive );

private:
	char				_hostname[MAX_PATH];
	unsigned int		_port_number;
	char				_user_id[MAX_PATH];
	char				_user_password[MAX_PATH];
	float				_min;
	float				_max;
	std::map<int, int>	_tour_list;

};

#if defined(WIN32)
extern "C" __declspec(dllexport) base_ptz_controller* create( void );
extern "C" __declspec(dllexport) void destroy( base_ptz_controller **ptz_controller );
#else
extern "C" base_ptz_controller* create( void );
extern "C" void destroy( base_ptz_controller **ptz_controller );
#endif

#endif