#if defined(WIN32)
#include <tchar.h>
#include <windows.h>
#include <direct.h>
#endif

#include "ptz_controller.h"
#include "backend_ptz_controller.h"


ptz_controller::ptz_controller( unsigned int vendor, unsigned int vendor_device, unsigned int protocol, unsigned int firmware_version, bool block_mode )
	: _controller(NULL)
{
	_controller = new backend_ptz_controller( vendor, vendor_device, protocol, firmware_version, block_mode );
}

ptz_controller::~ptz_controller( void )
{
	if( _controller )
		delete _controller;
}

char* ptz_controller::get_vendor_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_name();
}

char* ptz_controller::get_vendor_device_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_name();
}

char* ptz_controller::get_vendor_device_protocol_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_protocol_name();
}

char* ptz_controller::get_vendor_device_version_name( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_version_name();
}

unsigned short ptz_controller::get_vendor_id( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_id();
}

unsigned short ptz_controller::get_vendor_device_id( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_id();
}

unsigned short ptz_controller::get_vendor_device_protocol_id( void )
{
	if( !_controller ) 
		return 0;
	return _controller->get_vendor_device_protocol_id();
}

unsigned short ptz_controller::get_vendor_device_version_id( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_vendor_device_version_id();
}

unsigned short	ptz_controller::set_host_name( char *host_name )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_host_name( host_name );
}

unsigned short	ptz_controller::set_port_number( unsigned short port_number )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_port_number( port_number );
}

unsigned short	ptz_controller::set_user_id( char *user_id )
{
	if( !_controller ) return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_user_id( user_id );
}

unsigned short	ptz_controller::set_user_password( char *password )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_user_password( password );
}

unsigned short	ptz_controller::set_angle_inverse( bool inverse )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_angle_inverse( inverse );
}

unsigned short	ptz_controller::set_profile_token( char *token )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_profile_token( token );
}

unsigned short	ptz_controller::set_pan_sensitive_boundary( float min, float max, unsigned int number_place )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_pan_sensitive_boundary( min, max, number_place );
}

unsigned short	ptz_controller::set_tilt_sensitive_boundary( float min, float max, unsigned int number_place )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_tilt_sensitive_boundary( min, max, number_place );
}

unsigned short	ptz_controller::set_zoom_sensitive_boundary( float min, float max, unsigned int number_place )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_zoom_sensitive_boundary( min, max, number_place );
}

unsigned short	ptz_controller::set_speed_sensitive_boundary( float min, float max, unsigned int number_place )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_speed_sensitive_boundary( min, max, number_place );
}

unsigned short	ptz_controller::is_enable_osd_menu( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_osd_menu();
}

unsigned short	ptz_controller::is_enable_home_position( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_home_position();
}

unsigned short	ptz_controller::is_enable_preset( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_preset();
}

unsigned short	ptz_controller::is_enable_preset_tour( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_preset_tour();
}

unsigned short	ptz_controller::is_enable_continuous_move( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_continuous_move();
}

unsigned short	ptz_controller::is_enable_relative_move( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_relative_move();
}

unsigned short	ptz_controller::is_enable_absolute_move( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_enable_absolute_move();
}

unsigned short	ptz_controller::is_preset_name_numberic( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_preset_name_numberic();
}

unsigned short	ptz_controller::is_preset_tour_name_numberic( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_preset_tour_name_numberic();
}

unsigned short	ptz_controller::is_preset_name_changable( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_preset_name_changable();
}

unsigned short	ptz_controller::is_preset_tour_name_changable( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->is_preset_tour_name_changable();
}

unsigned short	ptz_controller::osd_menu( PTZ_OSD_MENU_TYPE_T osd )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->osd_menu( osd );
}

unsigned short	ptz_controller::goto_home_position( float speed )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->goto_home_position( speed );
}

unsigned short	ptz_controller::set_home_position( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_home_position();
}

unsigned short	ptz_controller::get_preset_list( char ***aliases, int *length )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_preset_list( aliases, length );
}

unsigned short	ptz_controller::add_preset( char *alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->add_preset( alias );
}

unsigned short	ptz_controller::remove_preset( char *alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->remove_preset( alias );
}

unsigned short	ptz_controller::goto_preset( char *alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->goto_preset( alias );
}

unsigned short	ptz_controller::get_preset_list2( int **aliases, int *length )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_preset_list2( aliases, length );
}

unsigned short	ptz_controller::add_preset2( int &alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->add_preset2( alias );
}

unsigned short	ptz_controller::remove_preset2( int alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->remove_preset2( alias );
}

unsigned short	ptz_controller::goto_preset2( int alias )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->goto_preset2( alias );
}

unsigned short	ptz_controller::add_preset_tour( char *tour_name, int size )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->add_preset_tour( tour_name, size );
}

unsigned short	ptz_controller::remove_preset_tour( char *tour_name )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->remove_preset_tour( tour_name );
}

unsigned short	ptz_controller::operate_preset_tour( char *tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->operate_preset_tour( tour_name, cmd );
}

unsigned short	ptz_controller::add_preset_tour2( int &tour_name, int size )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->add_preset_tour2( tour_name, size );
}

unsigned short	ptz_controller::remove_preset_tour2( int tour_name )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->remove_preset_tour2( tour_name );
}

unsigned short	ptz_controller::operate_preset_tour2( int tour_name, PTZ_TOUR_CMD_TYPE_T cmd )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->operate_preset_tour2( tour_name, cmd );
}

unsigned short	ptz_controller::set_preset_tour( PTZ_TOUR_T *tour )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->set_preset_tour( tour );
}

unsigned short	ptz_controller::get_preset_tour( PTZ_TOUR_T *tour )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_preset_tour( tour );
}

unsigned short	ptz_controller::get_preset_tours( PTZ_TOUR_T **tour, unsigned int *size_of_tours )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_preset_tours( tour, size_of_tours );
}

unsigned short	ptz_controller::get_status( float &pan, float &tilt, float &zoom )	
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->get_status( pan, tilt, zoom );
}

unsigned short ptz_controller::continuous_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, long long timeout )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->continuous_move( pan_sensitive, tilt_sensitive, zoom_sensitive, timeout );
}

unsigned short ptz_controller::continuous_move( PTZ_CONTINUOUS_MOVE_TYPE_T move, float speed, long long timeout )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->continuous_move( move, speed, timeout );
}

unsigned short ptz_controller::relative_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )	
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->relative_move( pan_sensitive, tilt_sensitive, zoom_sensitive, speed );
}

unsigned short ptz_controller::relative_move( PTZ_RELATIVE_MOVE_TYPE_T move, float sensitive, float speed )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->relative_move( move, sensitive, speed );
}

unsigned short ptz_controller::absolute_move( float pan_sensitive, float tilt_sensitive, float zoom_sensitive, float speed )	
{
	if( !_controller ) return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->absolute_move( pan_sensitive, tilt_sensitive, zoom_sensitive, speed );
}

unsigned short ptz_controller::stop_move( void )
{
	if( !_controller ) 
		return VMS_PTZ_UNDEFINED_DEVICE;
	return _controller->stop_move();
}


unsigned short	ptz_controller::get_vendor_informations( int ***vendor_ids, char ***vendor_names, int *length )
{
	return backend_ptz_controller::get_vendor_informations( vendor_ids, vendor_names, length );
}

unsigned short	ptz_controller::get_vendor_device_informations( int vendor_id, int ***vendor_device_ids, char ***vendor_device_names, int *length )
{
	return backend_ptz_controller::get_vendor_device_informations( vendor_id, vendor_device_ids, vendor_device_names, length );
}

unsigned short	ptz_controller::get_vendor_device_protocol_informations( int vendor_id, int vendor_device_id, int ***vendor_device_protocol_ids, char ***vendor_device_protocol_names, int *length )
{
	return backend_ptz_controller::get_vendor_device_protocol_informations( vendor_id, vendor_device_id, vendor_device_protocol_ids, vendor_device_protocol_names, length );
}

unsigned short	ptz_controller::get_vendor_device_version_informations( int vendor_id, int vendor_device_id, int vendor_device_protocol_id, int ***vendor_device_version_ids, char ***vendor_device_version_names, int *length )
{
	return backend_ptz_controller::get_vendor_device_version_informations( vendor_id, vendor_device_id, vendor_device_protocol_id, vendor_device_version_ids, vendor_device_version_names, length );
}
