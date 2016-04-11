#include "ptz_controller.h"
#include <pthread.h>

#if defined(WIN32)
#elif defined(UBUNTU) || defined(ARM)
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#endif

enum PTZ_COMMAND_TYPE_T
{
	INVALID_PTZ_COMMAND = -1,
	PTZ_COMMAND_CONTINUOUS_MOVE = 0,
	PTZ_COMMAND_CONTINUOUS_MOVE2,
	PTZ_COMMAND_RELATIVE_MOVE,
	PTZ_COMMAND_RELATIVE_MOVE2,
	PTZ_COMMAND_ABSOLUTE_MOVE,
	PTZ_COMMAND_STOP_MOVE,
	PTZ_COMMAND_GOTO_HOME_POSITION,
	PTZ_COMMAND_GOTO_PRESET,
	PTZ_COMMAND_OPERATE_PRESET_TOUR,
};

typedef struct _PTZ_COMMAND_INFO_T
{
	PTZ_COMMAND_TYPE_T					command;
	PTZ_RELATIVE_MOVE_TYPE_T			rmove;
	PTZ_CONTINUOUS_MOVE_TYPE_T			cmove;
	PTZ_TOUR_CMD_TYPE_T					tour_command;
	float								sensitive;
	float								pan_sensitive;
	float								tilt_sensitive;
	float								zoom_sensitive;
	float								speed;
	long long							timeout;
	char								preset_token[50];
	int									preset_token_numberic;
	char								preset_tour_token[50];
	int									preset_tour_token_numberic;
	_PTZ_COMMAND_INFO_T( void )
	{
		command			= INVALID_PTZ_COMMAND;
		rmove			= PTZ_RELATIVE_MOVE_HOME;
		cmove			= PTZ_CONTINUOUS_MOVE_NOTING;
		sensitive		= 0.0f;
		pan_sensitive	= 0.0f;
		tilt_sensitive	= 0.0f;
		zoom_sensitive	= 0.0f;
		speed			= 0.0f;
		timeout			= 0;
		memset( preset_token, 0x00, sizeof(preset_token) );
		preset_token_numberic = 0;
		memset( preset_tour_token, 0x00, sizeof(preset_tour_token) );
		preset_tour_token_numberic = 0;
	}
} PTZ_COMMAND_INFO_T;

class backend_ptz_controller/* : public base_ptz_controller*/
{
public:
	backend_ptz_controller( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version, bool block_mode=false );
	virtual ~backend_ptz_controller( void );

	static unsigned short	get_vendor_informations( int ***vendor_ids, char ***vendor_names, int *length );
	static unsigned short	get_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length );
	static unsigned short	get_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length );
	static unsigned short	get_vendor_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length );

	char*			get_vendor_name( void );
	char*			get_vendor_device_name( void );
	char*			get_vendor_device_protocol_name( void );
	char*			get_vendor_device_version_name( void );

	unsigned short	get_vendor_id( void );
	unsigned short	get_vendor_device_id( void );
	unsigned short	get_vendor_device_protocol_id( void );
	unsigned short	get_vendor_device_version_id( void );

	unsigned short	set_host_name( char *host_name );
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
	static void*	run( void *param );
	void			process( void );

private:
	base_ptz_controller		*_controller;
#if defined(WIN32)
	HINSTANCE				_instance;
#elif defined(UBUNTU) || defined(ARM)
	void*					_instance;
	static int				getdir(std::string dir, std::vector<std::string> &files);
#endif
	bool					_block_mode;
	PTZ_COMMAND_INFO_T		*_command_info;
	pthread_mutex_t			_mutex;
	pthread_t				_tid;
	bool					_is_run;
	bool					_is_created;
	bool					_is_entered;
};
