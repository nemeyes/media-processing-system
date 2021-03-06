#ifndef _PTZ_CONTROL_CLIENT_H_
#define _PTZ_CONTROL_CLIENT_H_
#include <ptz_controller.h>
#include <string>
#include <vector>
#include <map>

class http_client;
#if defined(WIN32)
class __declspec(dllexport) hanil_ptz_controller : public base_ptz_controller
#else
class hanil_ptz_controller : public base_ptz_controller
#endif
{
public:
	hanil_ptz_controller( void );
	~hanil_ptz_controller( void );

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
	unsigned short	get_preset_list2( int **aliases, int *length );
	unsigned short	add_preset( char *alias );
	unsigned short	add_preset2( int &alias );
	unsigned short	remove_preset( char *alias );
	unsigned short	remove_preset2( int alias );
	unsigned short	goto_preset( char *alias );
	unsigned short	goto_preset2( int alias );

	unsigned short	add_preset_tour( char *tour_name, int size=0 );
	unsigned short	add_preset_tour2( int &tour_name, int size=0 );
	unsigned short	remove_preset_tour( char *tour_name );
	unsigned short	remove_preset_tour2( int tour_name );
	unsigned short	operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
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
	unsigned short	query_limits( void );
	unsigned short	query_position( float &pan, float &tilt, float &zoom );

	float			get_continuous_sensitive_value( float sensitive );

	float			get_rpan_sensitive_value( float sensitive );
	float			get_rtilt_sensitive_value( float sensitive );
	float			get_rzoom_sensitive_value( float sensitive );
	float			get_rfocus_sensitive_value( float sensitive );

	float			get_apan_sensitive_value( float sensitive );
	float			get_atilt_sensitive_value( float sensitive );
	float			get_azoom_sensitive_value( float sensitive );

	float			get_speed_sensitive_value( float sensitive );
	float           get_speed_quasi_sensitive_value( float real_sensitive );

	float			get_apan_quasi_sensitive_value( float real_sensitive );
	float			get_atilt_quasi_sensitive_value( float real_sensitive );
	float			get_azoom_quasi_sensitive_value( float real_sensitive );

	void			split2vector( std::string origin, std::string token, std::vector<std::string> *devided );
	void			split2map( std::string origin, std::string token, std::map<std::string,std::string> *devided );
	void			split2map2( std::string origin, std::string token, std::map<std::string,std::string> *devided );	
	float           get_max_abs_value( float a, float b );
	unsigned short  set_pan_speed( float speed );
	unsigned short  set_tilt_speed( float speed );

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
	
};

#if defined(WIN32)
extern "C" __declspec(dllexport) base_ptz_controller* create( void );
extern "C" __declspec(dllexport) void destroy( base_ptz_controller **ptz_controller );
#else
extern "C" base_ptz_controller* create ( void );
extern "C" void destroy( base_ptz_controller **ptz_controller );
#endif

#endif