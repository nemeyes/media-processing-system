#ifndef _PTZ_CONTROL_CLIENT_H_
#define _PTZ_CONTROL_CLIENT_H_
#include <ptz_controller.h>


class http_client;
#if defined(WIN32)
class __declspec(dllexport) ctec_ptz_controller : public base_ptz_controller
#else
class ctec_ptz_controller : public base_ptz_controller
#endif
{
public:
	ctec_ptz_controller( void );
	~ctec_ptz_controller( void );

	char*			get_vendor_name( void );
	char*			get_vendor_device_name( void );
	char*			get_vendor_device_protocol_name( void );
	char*			get_vendor_device_version_name( void );

	unsigned short	get_vendor_id( void );
	unsigned short	get_vendor_device_id( void );
	unsigned short	get_vendor_device_protocol_id( void );
	unsigned short	get_vendor_device_version_id( void );

	unsigned short	set_host_name( char *host );
	unsigned short	set_port_number( unsigned short port_number );
	unsigned short	set_user_id( char *user_id );
	unsigned short	set_user_password( char *user_password );
	unsigned short	set_angle_inverse( bool inverse );

	unsigned short	set_pan_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short	set_tilt_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short	set_zoom_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short	set_speed_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short	set_profile_token( char *token );

	unsigned short	osd_menu( PTZ_OSD_MENU_TYPE_T osd );
	unsigned short	goto_home_position( float speed=0.0 );
	unsigned short	set_home_position( void );
	unsigned short	get_preset_list( char ***aliases, int *length );
	unsigned short	add_preset( char *alias );
	unsigned short	remove_preset( char *alias );
	unsigned short	goto_preset( char *alias );

	unsigned short	continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, unsigned int timeout );
	unsigned short	relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 );
	unsigned short	relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed=1.0 );
	unsigned short	absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 );
	unsigned short	stop_move( void );
	unsigned short	get_status( float &pan, float &tilt, float &zoom );

	// Grouping
	unsigned short	add_preset_tour( char *tour_name );
	unsigned short	remove_preset_tour( char *tour_name );
	unsigned short  operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
	unsigned short	set_preset_tour( PTZ_TOUR_T *tour );
	unsigned short	get_preset_tour( PTZ_TOUR_T *tour );
	unsigned short	get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours ); 

private:
	unsigned short	hexa_string_to_char_string( const char *hexa_string, unsigned char *char_string, int n );
	unsigned short	hexa_to_char( const char *hex, unsigned char &chr );

	unsigned short	query_limits( void );
	unsigned short	query_position(float &pan, float &tilt, float &zoom );

	float	get_re_sensitive_value( unsigned short sensitive );

	float	get_re_speed_sensitive_value( unsigned short sensitive );
	float	get_ab_pan_sensitive_value( unsigned short sensitive );
	float	get_ab_tilt_sensitive_value( unsigned short sensitive);
	float	get_ab_zoom_sensitive_value( unsigned short sensitive );
	

	float	get_ab_pan_quasi_sensitive_value( float real_sensitive );
	float	get_ab_tilt_quasi_sensitive_value( float real_sensitive );
	float	get_ab_zoom_quasi_sensitive_value( float real_sensitive );

	unsigned short	make_current_ptz_information( void *param );
	unsigned short	make_current_presetposition_information( void *param );
	std::map< std::string, std::string >		get_preset_list_mapped( void );
	std::string		find_key_by_value( char* value );

private:
	char								_hostname[200];
	unsigned int						_port_number;
	char								_user_id[100];
	char								_user_password[100];
	char								_profile_token[MAX_PATH];
	
	unsigned int						_min;
	unsigned int						_max;

	unsigned int						_r_min;
	unsigned int						_r_max;
		
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

	float								_min_pan;
	float								_max_pan;
	float								_min_tilt;
	float								_max_tilt;
	float								_min_zoom;
	float								_max_zoom;
	float								_min_speed;
	float								_max_speed;

	int									_max_pantilt_velocity;
	int									_max_zoom_velocity;

	bool								_is_limits_queried;

	// eflip value true(on), false(off)
	int									_flipped;
	unsigned short						make_current_cam_information( void *param );
	unsigned short						is_flipped( void );

};

#if defined(WIN32)
extern "C" __declspec(dllexport) base_ptz_controller* create( void );
extern "C" __declspec(dllexport) void destroy( base_ptz_controller **ptz_controller );
#else
extern "C" base_ptz_controller* create( void );
extern "C" void destroy( base_ptz_controller **ptz_controller );
#endif

#endif