#pragma once
#include <string>
#include <vector>
#include <iostream>

#include <stdlib.h>

#if defined(WIN32)
#if defined(EXPORT_VMS_PTZ_CONTROLLER)
#define EXPORT_CLASS __declspec(dllexport)
#else
#define EXPORT_CLASS __declspec(dllimport)
#endif

#elif defined(UBUNTU) || defined(ARM)
#define EXPORT_CLASS
#include <dlfcn.h>
#include <cstring>
#endif

#define PTZ_DEFAULT_SERVICE_ADDRESS		L"127.0.0.1"
#define PTZ_DEFAULT_USER				L"admin"
#define PTZ_DEFAULT_PASSWORD			L"admin"

#define PTZ_DEFAULT_SERVICE_PORT		9080

enum PTZ_ERROR_T
{
	VMS_PTZ_SUCCESS = 0,
	VMS_PTZ_FAIL = 1,
	VMS_PTZ_TRUE = 0,
	VMS_PTZ_FALSE = 1,
	VMS_PTZ_UNSUPPORTED_COMMAND,
	VMS_PTZ_SENSITIVE_VALUE_IS_NOT_VALID,
	VMS_PTZ_UNDEFINED_DEVICE,
	VMS_PTZ_HOST_NAME_IS_INVALID,
	VMS_PTZ_HOST_IS_NOT_CONNECTABLE,
	VMS_PTZ_CURRENTLY_WORKING,
	VMS_PTZ_PRESET_ALIAS_NAME_IS_NOT_ALLOWED,
};

enum PTZ_RELATIVE_MOVE_TYPE_T
{
	PTZ_RELATIVE_MOVE_HOME,
	PTZ_RELATIVE_MOVE_UP,			//for relative move
	PTZ_RELATIVE_MOVE_LEFT,		//for relative move
	PTZ_RELATIVE_MOVE_RIGHT,		//for relative move
	PTZ_RELATIVE_MOVE_DOWN,		//for relative move
	PTZ_RELATIVE_MOVE_LEFTUP,		//for relative move
	PTZ_RELATIVE_MOVE_RIGHTUP,	//for relative move
	PTZ_RELATIVE_MOVE_LEFTDOWN,	//for relative move
	PTZ_RELATIVE_MOVE_RIGHTDOWN,	//for relative move
	PTZ_RELATIVE_MOVE_ZOOMIN,		//for relative move
	PTZ_RELATIVE_MOVE_ZO0MOUT,	//for relative move
};

enum PTZ_CONTINUOUS_MOVE_TYPE_T
{
	PTZ_CONTINUOUS_MOVE_NOTING,
	PTZ_CONTINUOUS_MOVE_UP,			//for relative move
	PTZ_CONTINUOUS_MOVE_LEFT,		//for relative move
	PTZ_CONTINUOUS_MOVE_RIGHT,		//for relative move
	PTZ_CONTINUOUS_MOVE_DOWN,		//for relative move
	PTZ_CONTINUOUS_MOVE_LEFTUP,		//for relative move
	PTZ_CONTINUOUS_MOVE_RIGHTUP,	//for relative move
	PTZ_CONTINUOUS_MOVE_LEFTDOWN,	//for relative move
	PTZ_CONTINUOUS_MOVE_RIGHTDOWN,	//for relative move
	PTZ_CONTINUOUS_MOVE_ZOOMIN,		//for relative move
	PTZ_CONTINUOUS_MOVE_ZO0MOUT,	//for relative move
};

enum PTZ_OSD_MENU_TYPE_T
{
	PTZ_OSE_MENU_OPEN,
	PTZ_OSE_MENU_CLOSE,	
	PTZ_OSE_MENU_UP,	
	PTZ_OSE_MENU_DOWN,	
	PTZ_OSE_MENU_LEFT,	
	PTZ_OSE_MENU_RIGHT,	
	PTZ_OSE_MENU_SELECT,
	PTZ_OSE_MENU_BACK,	
};

enum PTZ_TOUR_CMD_TYPE_T
{
	PTZ_TOUR_CMD_START,
	PTZ_TOUR_CMD_STOP,
	PTZ_TOUR_CMD_PAUSE,
};

enum PTZ_TOUR_DIRECTION_TYPE_T
{
	PTZ_TOUR_DIRECTION_FORWARD,
	PTZ_TOUR_DIRECTION_BACKWARD,
};

typedef struct _PTZ_TOUR_SPOT_T
{
	char		preset_alias[150];	//parameter to specify if use preset position or use specific pan/tilt/zoom. if preset_alias is null, specified pan/tilt/zoom value is used.
	int			preset_numberic_alias;
	float		pan;			//Pan position.
	float		tilt;			//tilt position.
	float		zoom;			//zoom position.
	float		speed;			//parameter to specify Pan/Tilt and Zoom speed on moving toward this tour spot.
	long long	stay_time;		//parameter to specify time duration of staying on this tour sport.
	_PTZ_TOUR_SPOT_T( void )
		: pan(0.0f)
		, tilt(0.0f)
		, zoom(0.0f)
		, speed(0.0f)
		, stay_time(0)
		, preset_numberic_alias(0)
	{
		memset( preset_alias, 0x00, sizeof(preset_alias) );
	}
} PTZ_TOUR_SPOT_T;

typedef struct _PTZ_TOUR_T
{
	char						tour_name[150];
	bool						tour_always_start;
	unsigned int				tour_recurring_time;		//Optional parameter to specify how many times the preset tour is recurred.
	long long					tour_recurring_duration;	//milli second, Optional parameter to specify how long time duration the preset tour is recurred.
	PTZ_TOUR_DIRECTION_TYPE_T	tour_direction;				//Optional parameter to choose which direction the preset tour goes. Forward shall be chosen in case it is omitted.
	PTZ_TOUR_SPOT_T				*tour_spots;				//tour spots' list
	unsigned int				size_of_tour_spots;			//tour spots' size
	_PTZ_TOUR_T( void )
		: tour_always_start(false)
		, tour_recurring_time(0)
		, tour_recurring_duration(0)
		, tour_direction(PTZ_TOUR_DIRECTION_FORWARD)
		, tour_spots(0)
		, size_of_tour_spots(0)
	{
		memset( tour_name, 0x00, sizeof(tour_name) );
	}
	~_PTZ_TOUR_T( void )
	{
		if( tour_spots &&  size_of_tour_spots>0 )
		{
			delete [] tour_spots;
		}
	}
} PTZ_TOUR_T;

class base_ptz_controller
{
public:
	virtual char*			get_vendor_name( void )=0;
	virtual char*			get_vendor_device_name( void )=0;
	virtual char*			get_vendor_device_protocol_name( void )=0;
	virtual char*			get_vendor_device_version_name( void )=0;

	virtual unsigned short	get_vendor_id( void )=0;
	virtual unsigned short	get_vendor_device_id( void )=0;
	virtual unsigned short	get_vendor_device_protocol_id( void )=0;
	virtual unsigned short	get_vendor_device_version_id( void )=0;

	virtual unsigned short	set_host_name( char *host_name )=0;
	virtual unsigned short	set_port_number( unsigned short port_number )=0;
	virtual unsigned short	set_user_id( char *user_id )=0;
	virtual unsigned short	set_user_password( char *password )=0;
	virtual unsigned short	set_angle_inverse( bool inverse )=0;
	virtual unsigned short	set_pan_sensitive_boundary( float min, float max, unsigned int number_place )=0;
	virtual unsigned short	set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )=0;
	virtual unsigned short	set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )=0;
	virtual unsigned short	set_speed_sensitive_boundary( float min, float max, unsigned int number_place )=0;
	virtual unsigned short	set_profile_token( char *token )=0;

	virtual unsigned short	is_enable_osd_menu( void )=0;
	virtual unsigned short	is_enable_home_position( void )=0;
	virtual unsigned short	is_enable_preset( void )=0;
	virtual unsigned short	is_enable_preset_tour( void )=0;
	virtual unsigned short	is_enable_continuous_move( void )=0;
	virtual unsigned short	is_enable_relative_move( void )=0;
	virtual unsigned short	is_enable_absolute_move( void )=0;
	virtual unsigned short	is_preset_name_numberic( void )=0;
	virtual unsigned short	is_preset_tour_name_numberic( void )=0;
	virtual unsigned short	is_preset_name_changable( void )=0;
	virtual unsigned short	is_preset_tour_name_changable( void )=0;

	virtual unsigned short	osd_menu( PTZ_OSD_MENU_TYPE_T osd )=0;
	virtual unsigned short	goto_home_position( float speed=0.0 )=0;
	virtual unsigned short	set_home_position( void )=0;


	virtual unsigned short	get_preset_list( char ***aliases, int *length )=0;
	virtual unsigned short	add_preset( char *alias )=0;
	virtual unsigned short	remove_preset( char *alias )=0;
	virtual unsigned short	goto_preset( char *alias )=0;
	virtual unsigned short	get_preset_list2( int **aliases, int *length )=0;
	virtual unsigned short	add_preset2( int &alias )=0;
	virtual unsigned short	remove_preset2( int alias )=0;
	virtual unsigned short	goto_preset2( int alias )=0;

	virtual unsigned short	add_preset_tour( char *tour_name, int size=0 )=0;
	virtual unsigned short	remove_preset_tour( char *tour_name )=0;
	virtual	unsigned short  operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )=0;
	virtual unsigned short	add_preset_tour2( int &tour_name, int size=0 )=0;
	virtual unsigned short	remove_preset_tour2( int tour_name )=0;
	virtual	unsigned short  operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )=0;
	virtual unsigned short	set_preset_tour( PTZ_TOUR_T *tour )=0;
	virtual unsigned short	get_preset_tour( PTZ_TOUR_T *tour )=0;
	virtual unsigned short	get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )=0;


	virtual unsigned short	continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )=0;
	virtual unsigned short	continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )=0;
	virtual unsigned short	relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 )=0;
	virtual unsigned short	relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed=0.0 )=0;
	virtual unsigned short	absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 )=0;
	virtual unsigned short	stop_move( void )=0;
	virtual unsigned short	get_status( float &pan, float &tilt, float &zoom )=0;
};

class backend_ptz_controller;
class EXPORT_CLASS ptz_controller : public base_ptz_controller
{
public:
	ptz_controller( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version, bool block_mode=false );
	~ptz_controller( void );

	static unsigned short	get_vendor_informations( int ***vendor_ids, char ***vendor_names, int *length );
	static unsigned short	get_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length );
	static unsigned short	get_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length );
	static unsigned short	get_vendor_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length );

	char*					get_vendor_name( void );
	char*					get_vendor_device_name( void );
	char*					get_vendor_device_protocol_name( void );
	char*					get_vendor_device_version_name( void );

	unsigned short			get_vendor_id( void );
	unsigned short			get_vendor_device_id( void );
	unsigned short			get_vendor_device_protocol_id( void );
	unsigned short			get_vendor_device_version_id( void );

	unsigned short			set_host_name( char *host_name );
	unsigned short			set_port_number( unsigned short port_number );
	unsigned short			set_user_id( char *user_id );
	unsigned short			set_user_password( char *password );
	unsigned short			set_angle_inverse( bool inverse );
	unsigned short			set_pan_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short			set_tilt_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short			set_zoom_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short			set_speed_sensitive_boundary( float min, float max, unsigned int number_place );
	unsigned short			set_profile_token( char *token );

	unsigned short			is_enable_osd_menu( void );
	unsigned short			is_enable_home_position( void );
	unsigned short			is_enable_preset( void );
	unsigned short			is_enable_preset_tour( void );
	unsigned short			is_enable_continuous_move( void );
	unsigned short			is_enable_relative_move( void );
	unsigned short			is_enable_absolute_move( void );
	unsigned short			is_preset_name_numberic( void );
	unsigned short			is_preset_tour_name_numberic( void );
	unsigned short			is_preset_name_changable( void );
	unsigned short			is_preset_tour_name_changable( void );

	unsigned short			osd_menu( PTZ_OSD_MENU_TYPE_T osd );
	unsigned short			goto_home_position( float speed=0.0 );
	unsigned short			set_home_position( void );

	unsigned short			get_preset_list( char ***aliases, int *length );
	unsigned short			add_preset( char *alias );
	unsigned short			remove_preset( char *alias );
	unsigned short			goto_preset( char *alias );
	unsigned short			get_preset_list2( int **aliases, int *length );
	unsigned short			add_preset2( int &alias );
	unsigned short			remove_preset2( int alias );
	unsigned short			goto_preset2( int alias );

	unsigned short			add_preset_tour( char *tour_name, int size=0 );
	unsigned short			remove_preset_tour( char *tour_name );
	unsigned short			operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
	unsigned short			add_preset_tour2( int &tour_name, int size=0 );
	unsigned short			remove_preset_tour2( int tour_name );
	unsigned short			operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd );
	unsigned short			set_preset_tour( PTZ_TOUR_T *tour );
	unsigned short			get_preset_tour( PTZ_TOUR_T *tour );
	unsigned short			get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours );

	unsigned short			continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout );
	unsigned short			continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout );
	unsigned short			relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 );
	unsigned short			relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed=0.0 );
	unsigned short			absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed=0.0 );
	unsigned short			stop_move( void );
	unsigned short			get_status( float &pan, float &tilt, float &zoom );


private:
	backend_ptz_controller	*_controller;
};
