#ifndef _PTZ_CONTROL_CLIENT_H_
#define _PTZ_CONTROL_CLIENT_H_
#include <ptz_controller.h>


#if defined(WIN32)
class __declspec(dllexport) ini_ptz_controller : public base_ptz_controller
#else
class ini_ptz_controller : public base_ptz_controller
#endif
{
public:
	ini_ptz_controller( void );
	~ini_ptz_controller( void );

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

	unsigned short	continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout );
	unsigned short	continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout );
	unsigned short	relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 );
	unsigned short	relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed=1.0 );
	unsigned short	absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 );
	unsigned short	stop_move( void );
	unsigned short	get_status( float &pan, float &tilt, float &zoom );

	// Grouping
    unsigned short	add_preset_tour( char *tour_name, int size=0 );
	unsigned short	add_preset_tour2( int &tour_name, int size=0 );
	unsigned short	remove_preset_tour( char *tour_name );
	unsigned short	remove_preset_tour2( int tour_name );
	unsigned short  operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
	unsigned short  operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
	unsigned short	add_preset_tour( int tour_name, int size=0 );
	unsigned short	set_preset_tour( PTZ_TOUR_T *tour );
	unsigned short	get_preset_tour( PTZ_TOUR_T *tour );
	unsigned short	get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours ); 

private:
	float			get_rpan_sensitive_value( float sensitive );
	float			get_rtilt_sensitive_value( float sensitive );
	float			get_rzoom_sensitive_value( float sensitive );
	float			get_apan_sensitive_value( float sensitive );
	float			get_atilt_sensitive_value( float sensitive );
	float			get_azoom_sensitive_value( float sensitive );
	float			get_speed_sensitive_value( float sensitive );
	float			get_apan_quasi_sensitive_value( float real_sensitive );
	float			get_atilt_quasi_sensitive_value( float real_sensitive );
	float			get_azoom_quasi_sensitive_value( float real_sensitive );

	unsigned short	hexa_string_to_char_string( const char *hexa_string, unsigned char *char_string, int n );
	unsigned short	hexa_to_char( const char *hex, unsigned char &chr );

	unsigned short	query_limits( void );
	unsigned short	query_position(float &pan, float &tilt, float &zoom );
	

private:
	char			_hostname[200];
	unsigned int	_port_number;
	char			_user_id[100];
	char			_user_password[100];
	char			_profile_token[MAX_PATH];	
	unsigned int	_min;
	unsigned int	_max;	
		
	unsigned short	_pan_number_place;
	float			_pan_min;
	float			_pan_max;
	unsigned short	_tilt_number_place;
	float			_tilt_min;
	float			_tilt_max;
	unsigned short	_zoom_number_place;
	float			_zoom_min;
	float			_zoom_max;
	unsigned short	_speed_number_place;
	float			_speed_min;
	float			_speed_max;
	float			_min_pan;
	float			_max_pan;
	float			_min_tilt;
	float			_max_tilt;
	float			_min_zoom;
	float			_max_zoom;
	float			_min_speed;
	float			_max_speed;
	float			_min_cspeed;
	float			_max_cspeed;
	bool			_is_limits_queried;

	typedef union _CTRL
	{
		char msg[31];
		struct 
		{
			unsigned char common[24];
			unsigned char ez[7];				
		} ptz_cont;
	} CTRL;

	void			init_ctrl(CTRL* con);
	void            set_ez_data(CTRL* con, std::string cmd, int data=0);
	float           get_continuous_sensitive_value( float sensitive );
	float           get_max_abs_value( float a, float b );



};
         

#if defined(WIN32)
extern "C" __declspec(dllexport) base_ptz_controller* create( void );
extern "C" __declspec(dllexport) void destroy( base_ptz_controller **ptz_controller );
#else
extern "C" base_ptz_controller* create( void );
extern "C" void destroy( base_ptz_controller **ptz_controller );
#endif

#endif